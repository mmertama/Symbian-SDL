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
#include<epoclib.h>
#include"regexp.h"

/**************************
epoc lib main for posix, not used in SOS apps 
*****************************/


void MainL(const TDesC& aPat, const TDesC& aMat,
           const TDesC& aSub, const TDesC& aOpt)
    {
    TInt options = 0;
    if(aOpt.Length() > 0)
        {
        if(aOpt.Locate('g') > 0)
            options |= CRegExp::EGlobal;
        if(aOpt.Locate('i') > 0)
            options |= CRegExp::ECase;
        if(aOpt.Locate('m') > 0)
            options |= CRegExp::ESingleLine;
        }

    CRegExp* re = CRegExp::NewLC(aPat, options);

    TPtrC match(aMat);
    
    HBufC* data = NULL;;

    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL(fs);
    TEntry entry;
    if(fs.Entry(aMat, entry) == KErrNone)
        {
        RFile file;
        User::LeaveIfError(
            file.Open(fs, aMat, EFileRead));
        CleanupClosePushL(file);
        HBufC8* buf = HBufC8::NewLC(entry.iSize);
        TPtr ptr = buf->Des();
        file.Read(ptr);
        data = HBufC::NewL(buf->Length());
        data->Des().Copy(*buf);
        match.Set(*data);
        CleanupStack::PopAndDestroy(2); //buf, file
        }
   
    CleanupStack::PushL(data);

    if(re->Match(match))
        {
        RDebug::Print(_L("Matched"));
        TInt c = 0;
        for(;;)
            {
            TBool found;
            const TPtrC group(re->SubStr(c, found));
            if(!found)
                break;
            if(group.Length() > 0)
                {
                RDebug::Print(group);
                }
            else
                RDebug::Print(_L("-empty-"));
            c++;
            }
        if(aSub.Length() > 0)
            {
            HBufC* str = NULL;
            const TInt count = re->SubstituteL(str, aSub);
           
            if(data != NULL)
                {
                CRegExp* exp = CRegExp::NewLC(_L("(.*)\\.(.*)"));
                if(!exp->Match(aMat))
                    User::Leave(KErrArgument);
                TBool found1;
                TBool found2;
                const TPtrC name(exp->SubStr(0, found1));
                const TPtrC ext(exp->SubStr(1, found2));
                TFileName fileName;
                fileName.Append(name);
                fileName.Append(_L(".out."));
                fileName.Append(ext);
                RFile file;
                User::LeaveIfError(file.Open(fs, fileName, EFileWrite));
                CleanupClosePushL(file);
                file.Write(*str);
                file.Flush();
                CleanupStack::PopAndDestroy(2); //exp, file
                }

            RDebug::Print(_L("%d %S"), count, str);
            }
        }
    else
        RDebug::Print(_L("No Match"));
    CleanupStack::PopAndDestroy(3); //re, data, fs
    }


int main(int argc, char* argv[])
    {
    char* sbt = NULL;
    char* opt = NULL;
    if(argc == 5)
        {
        opt = argv[4];
        if(opt[0] != '-')
            {
            sbt = opt;
            opt = argv[3];
            }
        else
            sbt = argv[3];
        }
    else if(argc == 4)
        {
        if(argv[3][0] == '-')
            opt = argv[3];
        else
            sbt = argv[3];
        }
    else if(argc != 3)
        return -1;

    const TPtrC pat(argv[1]);
    const TPtrC mat(argv[2]);
    const TPtrC sub(sbt);
    const TPtrC opp(opt);
    CTrapCleanup* th = CTrapCleanup::New();
    TRAPD(err, MainL(pat, mat, sub, opp));
    delete th;
    return err;
    }