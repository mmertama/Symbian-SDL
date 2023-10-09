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
#ifndef __LINEREADER__
#define __LINEREADER__

#include<f32file.h>

class CFileLineReaderBase;


class CFileLineReader : public CBase
    {
    public:
        IMPORT_C static CFileLineReader* NewLC(RFs& aFs, const TDesC& aFileName);
        IMPORT_C static CFileLineReader* NewL(RFs& aFs, const TDesC& aFileName);
        IMPORT_C static CFileLineReader* DuplicateL(RFile& aFile);
        IMPORT_C static CFileLineReader* DuplicateLC(RFile& aFile);
        IMPORT_C TBool NextL();
        IMPORT_C TPtrC Current() const;
        IMPORT_C ~CFileLineReader();
    private:
    		void ConstructL();
    private:
  			RFile  iFile;
   			CFileLineReaderBase* iReader;
   			TInt iSize;
   			TInt iCursor;
    };
    
#endif