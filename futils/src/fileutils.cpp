/*471188***********************************************************
		This file is part of futils.
																	
    futils is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    futils is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with futils.  If not, see <http://www.gnu.org/licenses/>.
    
    Markus Mertama @ http://koti.mbnet.fi/mertama
    ******************************************************************/
#include"fileutils.h"
#include"flogger.h"
#include"badesca.h"

#define LOG(xx)



class TFinder
    {
    public:
        TFinder(TDes& aName);
        virtual TBool FindInDir(const CDir& aDir, const TDesC& aName, TInt aCount) = 0;
    protected:
        void Set(const TDesC& aName);
        TDes* iName;
    };

class TMatchFinder : public TFinder
    {
    public:
    TMatchFinder(TDes& aName);
    private:
    TBool FindInDir(const CDir& aDir, const TDesC& aName, TInt aCount);
    };

class TCompareFinder : public TFinder
    {
    public:
    TCompareFinder(TDes& aName);
    private:
    TBool FindInDir(const CDir& aDir, const TDesC& aName, TInt aCount);
    };

void TFinder::Set(const TDesC& aName)
    {
    iName->Copy(aName);
    }

TCompareFinder::TCompareFinder(TDes& aName) : TFinder(aName)
    {
    }


TMatchFinder::TMatchFinder(TDes& aName) : TFinder(aName)
    {
    }

TFinder::TFinder(TDes& aName) : iName(&aName)
    {
    }

EXPORT_C TFileUtils::TFileUtils(RFs& aFs) : iFs(aFs), iObserver(NULL)
    {
    }

EXPORT_C TInt TFileUtils::IsFileInDir(const TDesC& aPath, const TDesC& aPattern, TDes* aBuf) const
    {
    TParse parse;
    parse.Set(aPattern, NULL, &aPath);
    CDir* dir;
    iFs.GetDir(parse.FullName(), KEntryAttNormal, ESortNone ,dir);
    TInt found = dir->Count();
    if(aBuf != NULL && found)
        {
        *aBuf = (*dir)[0].iName;
        aBuf->Insert(0, parse.DriveAndPath());
        }
    delete dir;
    return found;
    }

//Find a name from dir
TBool TCompareFinder::FindInDir(const CDir& aDir, const TDesC& aName, TInt aCount)
    {
    TInt count = aCount;
    for(TInt i = 0; i < aDir.Count(); i++)
        {
        const TEntry& entry = aDir[i];
        LOG(entry.iName);
        if(!entry.IsDir() && entry.iName.CompareF(aName) == 0)
            {
            if(--count < 0)
                {
                LOG(_L("found"));
                Set(entry.iName);
                return ETrue;
                }
            }
        }
    return EFalse;
    }

TBool TMatchFinder::FindInDir(const CDir& aDir, const TDesC& aName, TInt aCount)
    {
    TInt count = aCount;
    for(TInt i = 0; i < aDir.Count(); i++)
        {
        const TEntry& entry = aDir[i];
        LOG(entry.iName);
        if(!entry.IsDir() && entry.iName.MatchF(aName) != KErrNotFound)
            {
            if(--count < 0)
                {
                LOG(_L("found"));
                Set(entry.iName);
                return ETrue;
                }
            }
        }
    return EFalse;
    }

EXPORT_C TBool TFileUtils::Exists(TInt aDrive) const
    {
    TVolumeInfo fo; //result differs from one get from DriveInfo
    const TInt err = iFs.Volume(fo, aDrive);
    return err == KErrNone 
        && fo.iDrive.iType != EMediaUnknown 
        && fo.iDrive.iType != EMediaNotPresent;
    }

typedef TBool (TFileUtils::*FindFunc)(TInt, const TDesC&, TDes&, TFinder&) const;

EXPORT_C TBool TFileUtils::FindFileL(const TDesC& aName, TDes& aFullName, TInt aFlags) const
    {
    TDriveList drives;
    User::LeaveIfError(iFs.DriveList(drives));

    TCompareFinder cmp(aFullName);
    TMatchFinder match(aFullName);

    TFinder* finder = NULL;
    
    if(aFlags & EMatch)
        finder = &match;
    else
        finder = &cmp;

    /*
    const FindFunc func = (aFlags & EFindAll) ? FindAllFromRootL : FindFromRootL;

    for(TInt  i = 0; i < KMaxDrives; i++)
        {
        if(drives[i] != 0 && (this->*func)(i, aName, aFullName, *finder))
            return ETrue;
        }
        */
  for(TInt  i = 0; i < KMaxDrives; i++)
        {
        if(drives[i] != 0)
            {
            if(aFlags & EFindAll)
                {
                if(FindAllFromRootL(i, aName, aFullName, *finder))
                    return ETrue;
                }
            else
                {
                if(FindFromRootL(i, aName, aFullName, *finder))
                    return ETrue;
                }
            }

        }
    
    return EFalse;
    }

EXPORT_C void TFileUtils::SetObserver(MFileObserver& aObserver)
    {
    iObserver = &aObserver;
    }


TBool TFileUtils::FindAllFromRootL(TInt aDrive,const TDesC& aName, TDes& aFullName, TFinder& aFinder) const
    {
    TBuf<4> root;
    TChar drive;
    User::LeaveIfError(iFs.DriveToChar(aDrive, drive));
    root.Format(_L("%c:\\"), TUint(drive));

    if(!Exists(aDrive))
        return EFalse;

    CDirScan* ds = CDirScan::NewL(iFs);
    CleanupStack::PushL(ds);
    ds->SetScanDataL(root, KEntryAttMatchMask/*KEntryAttDir*/, ESortNone, CDirScan::EScanDownTree);

    CDir* dirs  = NULL;
    ds->NextL(dirs);
   
    TBool find = EFalse;

    if(iObserver == NULL)
        User::Leave(KErrArgument);

    while(dirs != NULL && !find)
        {
        LOG(ds->FullPath());
        
        if(iObserver != NULL)
            iObserver->Progress();
    
        TInt matches = 0;

        TBool found = EFalse;

        do
            {
            find = aFinder.FindInDir(*dirs, aName, matches);
            if(find)
                {
                matches++;
                TFileName name(ds->FullPath());
                name.Append(aFullName);
                found = iObserver->FileFoundL(name);
                if(found)
                    {
                    aFullName = name;
                    break;
                    }
                }

            }while(find);

        find = found;

        delete dirs;
        dirs = NULL;
        ds->NextL(dirs);
        }

    delete dirs;
    CleanupStack::PopAndDestroy(); // ds
    return find;
    }


//Goes trought every directory from roor from drive 'aDrive'
TBool TFileUtils::FindFromRootL(TInt aDrive,const TDesC& aName, TDes& aFullName, TFinder& aFinder) const
    {
    TBuf<4> root;
    TChar drive;
    User::LeaveIfError(iFs.DriveToChar(aDrive, drive));
    root.Format(_L("%c:\\"), TUint(drive));

    if(!Exists(aDrive))
        return EFalse;

    CDirScan* ds = CDirScan::NewL(iFs);
    CleanupStack::PushL(ds);
    ds->SetScanDataL(root, KEntryAttMatchMask/*KEntryAttDir*/, ESortNone, CDirScan::EScanDownTree);

    CDir* dirs  = NULL;
    ds->NextL(dirs);
   
    TBool find = EFalse;


    while(dirs != NULL)
        {
        LOG(ds->FullPath());
        
        if(iObserver != NULL)
            iObserver->Progress();

        find = aFinder.FindInDir(*dirs, aName, 0);

        if(find && (iObserver == NULL || iObserver->FileFoundL(aFullName)))
                {
                aFullName.Insert(0, ds->FullPath());
                break;
                }
        delete dirs;
        dirs = NULL;
        ds->NextL(dirs);
        }
/*
    if(find)
        {
        TParse file;
        const TPtrC path = ds->FullPath();
        file.Set(aName, NULL, &path);
        aFullName = file.FullName();
        }
    */
    delete dirs;

    CleanupStack::PopAndDestroy(); // ds
    return find;
    }

const TInt KDrvAny = 0xFF; 

EXPORT_C CFileUtils* CFileUtils::NewL(RFs& aFs, MFileObserver& aObs)
    {
    CFileUtils* fu = new (ELeave) CFileUtils(aFs, aObs);
    CleanupStack::PushL(fu);
    fu->ConstructL();
    CleanupStack::Pop();
    return fu;
    }
    
void CFileUtils::ConstructL()
    {
    User::LeaveIfError(iFs.DriveList(iDrives));
    iDs = CDirScan::NewL(iFs);
    }
    
EXPORT_C void CFileUtils::FindFilesL(const CDesCArray& aNames)
    {
    iNames = &aNames;
    iCurrentDrive = -1;
    iCurrentFile = 0;
    Start();
    }
    
 void CFileUtils::Start()
    {
    SetActive();
    iStatusPtr = &iStatus;
    User::RequestComplete(iStatusPtr, KErrNone);
    }
    
CFileUtils::CFileUtils(RFs& aFs, MFileObserver& aObs) :
 CActive(CActive::EPriorityIdle),
 iFs(aFs), iObserver(aObs),  iTargetDrv(KDrvAny)
    {
    CActiveScheduler::Add(this);
    }

CFileUtils::~CFileUtils()
    {
    Cancel();
    delete iDs;
    delete iRoot;
    }

void CFileUtils::CreateDirsL()
    {
    TFileName root;
    TChar drive;
    
    User::LeaveIfError(iFs.DriveToChar(iCurrentDrive, drive));
  
 	if(iRoot)
 		root.Format(_L("%c:%S"), TUint(drive), iRoot);
    else
    	root.Format(_L("%c:\\"), TUint(drive));
    
    iDs->SetScanDataL(root, KEntryAttMatchMask/*KEntryAttDir*/, ESortNone, CDirScan::EScanDownTree);
    }

TBool CFileUtils::FindFromDirL(const TEntry& aEntry)
    {
    if(aEntry.IsDir())
        return ETrue;
    for(TInt i = 0; i < iNames->Count(); i++)
        {
        if(aEntry.iName.MatchF((*iNames)[i]) != KErrNotFound)
            {
            TParse fullname;
            const TPtrC path = iDs->FullPath(); 
            fullname.Set(aEntry.iName, &path, NULL);
            if(!iObserver.FileFoundL(fullname.FullName()))
                {
                return EFalse;
                }
            }
        }
     return ETrue;
     }

EXPORT_C void CFileUtils::SetRootL(const TDesC& aPath)
	{
	iTargetDrv = KDrvAny;
	delete iRoot;
	iRoot = NULL;
	if(aPath.Length() > 0)
		{
		TParsePtrC parse(aPath);
		const TPtrC path = parse.Path(); 
		iRoot = path.AllocL();
		const TPtrC drv = parse.Drive();
		TInt drvindex;
		if(drv.Length() > 0 && RFs::CharToDrive(drv[0], drvindex) == KErrNone)
			iTargetDrv = drvindex;
		}
	}

void CFileUtils::RunL()
    {
    TFileUtils fu(iFs);
    TBool cont = ETrue;
    if(iDir == NULL)
        {
        ++iCurrentDrive;
        if(iCurrentDrive < KMaxDrives)
            {
            if((iTargetDrv == KDrvAny || iTargetDrv == iCurrentDrive) &&
            iDrives[iCurrentDrive] != 0 && fu.Exists(iCurrentDrive))
                {
                CreateDirsL();
                TRAPD(err, iDs->NextL(iDir));
                if(err != KErrNone)
                	{
                	delete iDir;
                	iDir = NULL;
                	}
                iCurrentFile = 0;
                }
            }
        else
            {
            cont = EFalse;
            }
        }
    else
        {
        if(iCurrentFile < iDir->Count())
            {
            cont = FindFromDirL((*iDir)[iCurrentFile]);
            ++iCurrentFile;
            }
        else
            {
            cont = iObserver.Progress();
            delete iDir;
            iDir = NULL;
            TRAPD(err, iDs->NextL(iDir));
            if(err != KErrNone)
                {
                delete iDir;
                iDir = NULL;
                }
            iCurrentFile = 0;    
            }
        }
    if(cont)
        {
        Start();
        }
    else
        {
        iObserver.Complete();
        }
    }
    
void CFileUtils::DoCancel()
   { 
   if(iStatusPtr != NULL)
       User::RequestComplete(iStatusPtr, KErrCancel);
   }


