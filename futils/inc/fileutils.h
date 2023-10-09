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
#ifndef FILEUTILS
#define FILEUTILS

#include<f32file.h>
#include<s32stor.h>
#include<badesca.h>

class MFileObserver
    {
    public:
        virtual TBool Progress() = 0;
        virtual TBool FileFoundL(const TDesC& aFilename) = 0;
        virtual TBool Complete() = 0;
    };


class TFinder;


class TFileUtils
    {
    public:
        enum {ENone = 0, EMatch = 1, EFindAll = 2};
        IMPORT_C TFileUtils(RFs& aFs);
        IMPORT_C TInt IsFileInDir(const TDesC& aPath, const TDesC& aPattern, TDes* aBuf = NULL) const;
        IMPORT_C TBool FindFileL(const TDesC& aName, TDes& aFullName, TInt aFlags = ENone) const;
        IMPORT_C TBool Exists(TInt aDrive) const;
        IMPORT_C void SetObserver(MFileObserver& aObserver);
    private:
        TBool FindFromRootL(TInt aDrive,const TDesC& aName, TDes& aFullName, TFinder& aFinder) const;
        TBool FindAllFromRootL(TInt aDrive,const TDesC& aName, TDes& aFullName, TFinder& aFinder) const;
    private:
        RFs& iFs;
        MFileObserver* iObserver;
    };

class CFileUtils : public CActive
        {
        public:
            IMPORT_C static CFileUtils* NewL(RFs& aFs, MFileObserver& aObs);
            IMPORT_C void FindFilesL(const CDesCArray& aNames);
            IMPORT_C ~CFileUtils();
            IMPORT_C void SetRootL(const TDesC& aPath);
        private:
            CFileUtils(RFs& aFs, MFileObserver& aObs);
            TBool FindFromDirL(const TEntry& aEntry);
            void Start();
            void CreateDirsL();
            void ConstructL();
            void RunL();
            void DoCancel();
        private:
            RFs& iFs;
            MFileObserver& iObserver;
            TInt    iCurrentDrive;
            TInt    iCurrentFile;
            CDir*   iDir;
            const CDesCArray* iNames;
            TDriveList iDrives;
            CDirScan* iDs;
            HBufC* iRoot;
            TInt iTargetDrv;
            TRequestStatus* iStatusPtr;
        };

class TStreamUtils
    {
    public:
        IMPORT_C TStreamUtils(CDictionaryStore& aStore);
        IMPORT_C void ReadFromStoreL(TDes& aDes, const TUid& aUid);
        IMPORT_C void WriteToStoreL(const TDesC& aDes, const TUid& aUid);
        IMPORT_C void ReadFromStoreL(TDes8& aDes, const TUid& aUid);
        IMPORT_C void WriteToStoreL(const TDesC8& aDes, const TUid& aUid);
        IMPORT_C void Remove(const TUid& aUid);
    private:
        CDictionaryStore& iStore;
    };

#endif
