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

EXPORT_C TStreamUtils::TStreamUtils(CDictionaryStore& aStore) : iStore(aStore)
    {
    }
 
EXPORT_C  void TStreamUtils::ReadFromStoreL(TDes8& aFullName, const TUid& aUid)
    {
    if(iStore.IsPresentL(aUid))
        {
        RDictionaryReadStream stream;
        stream.OpenLC(iStore, aUid);
        stream >> aFullName;
        CleanupStack::PopAndDestroy(); //stream
        return;
        }
    User::Leave(KErrNotFound);
    }
 
EXPORT_C  void TStreamUtils::WriteToStoreL(const TDesC8& aFullName, const TUid& aUid)
    {
    RDictionaryWriteStream stream;
    stream.AssignLC(iStore, aUid);
    //stream.WriteL(aFullName);
    stream << aFullName;
    stream.CommitL();
    iStore.CommitL();
    CleanupStack::PopAndDestroy(); //stream
    }


EXPORT_C  void TStreamUtils::Remove(const TUid& aUid)
    {
    iStore.Remove(aUid);
    iStore.Commit();
    }

//Reads a stored filename from store
EXPORT_C  void TStreamUtils::ReadFromStoreL(TDes& aFullName, const TUid& aUid)
    {
    if(iStore.IsPresentL(aUid))
        {
        RDictionaryReadStream stream;
        stream.OpenLC(iStore, aUid);
        stream >> aFullName;
        CleanupStack::PopAndDestroy(); //stream
        return;
        }
    User::Leave(KErrNotFound);
    }

//Writes a filename into store
EXPORT_C void TStreamUtils::WriteToStoreL(const TDesC& aFullName, const TUid& aUid)
    {
    RDictionaryWriteStream stream;
    stream.AssignLC(iStore, aUid);
    //stream.WriteL(aFullName);
    stream << aFullName;
    stream.CommitL();
    iStore.CommitL();
    CleanupStack::PopAndDestroy(); //stream
    }
