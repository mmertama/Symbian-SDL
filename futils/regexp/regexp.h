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
/*

  CRegExp 
  Regular Expression class
  Copyright Markus Mertama 2001-2002

*/
#ifndef __REGEXP_H__
#define __REGEXP_H__

#include<e32base.h>

//internal declarations
class CPatTokenBase;
class CPatTokenGroup;


typedef CArrayFixFlat<CPatTokenGroup*> CGroupList;

// CRegExp class
// Implements regular expressions functionality
class CRegExp : public CBase
    {
    public:

//Options
//ESingleLine - text is treated as a single line
//and newlines are omitted when text end and begin
//marks are seeked.
//EGlobal - When sustituting all occurences are
//replaced.
//ECase - Capital differences are ignored
        enum
            {
            EOptNone    = 0x0,
            ESingleLine = 0x1,
            EGlobal     = 0x2,
            ECase       = 0x4
            };
//See function Matched
        enum TMatchString
            {
            EEntire,
            EBefore,
            EAfter
            };

//Constructors
//aPattern is a normal regular expression. It is not copied
//and user should ensure it exists during CRegExp 
//lifetime. If that cannot be guarateed; CRegExpBuf class
//must be used instead.

        IMPORT_C static CRegExp* NewL(const TDesC& aPattern, TInt aOptions = 0);
        IMPORT_C static CRegExp* NewLC(const TDesC& aPattern, TInt aOptions = 0);

    public:
//Match
//a text is given text to match and function returns boolean depending
//if it is matched with pattern
        IMPORT_C TBool Match(const TDesC& aText);
//Substitute
//a match is replaced with aText and written in allocated aBuffer
//Number of suvstitutions is returned. Match have to be called first.
        IMPORT_C TInt SubstituteL(HBufC*& aBuffer, const TDesC& aText);
//SubsStr
//If pattern contains groups, blocks inside of brackets '(' and ')'
//those can matches can be referred with index. aFound is set depending
//if such group really exists. Match have to be called first.
//Note that returned string lifetime is as long as aText parameter
//in Match function.
        
        IMPORT_C TPtrC SubStr(TInt aIndex, TBool& aFound) const;
//Last substring Match have to be called first.
//Note that returned string lifetime is as long as aText parameter
//in Match function.
        IMPORT_C TPtrC Last(TBool& aFound) const;
//Return substring after match.
//EEntire return matched string.
//EBefore return everything before match
//EAfter return everything after match
//Note that returned string lifetime is as long as aText parameter
//in Match function.
        IMPORT_C TPtrC Matched(TMatchString aMatch) const;
        IMPORT_C ~CRegExp();

        IMPORT_C TInt SubCount() const;
        IMPORT_C static TInt Check(const TDesC& aPattern);

    protected:
        
        CRegExp(const TDesC& aMatch, TInt aOptions);
        virtual void SetL();

    private:
    
        TInt Number(TInt& aNumber) const;
        void ConsumeData(TInt aCount);
        
       
        void CreateAltClassL();
        void CreateAltOrL();
        void CreateGroupL();
        void CreateRepeatL(TInt aMin, TInt aMax);
        void CreateStaticL();
        void CreateSpecialL(TInt aChar);
        void CreateRangeL(TInt aStart);
        void CreateReferenceL(TInt aIndex);
 
        TInt ParseL(TInt aChar, TInt aPrevious);
        
        void AddToken(CPatTokenBase* aToken);
        void EndToken();

        CPatTokenGroup* Container(CPatTokenBase* aChild) const;
        
        CPatTokenBase* Remove(CPatTokenBase* aPrevious);

        inline CPatTokenBase* Previous() const;
        inline CPatTokenGroup* Parent() const;
 

        inline TBool IsCurrentType(TInt aType) const;
        inline TBool IsParentType(TInt aType) const;

        void Zero();

    private:
        TInt iOptions;
        TPtrC iPatPtr;
        TPtrC iBuffer;
        CPatTokenBase* iFirst;
        CPatTokenBase* iCurrent;
        CPatTokenGroup* iParent;

        CGroupList* iGroups;

    };

//CRegExpBuf
//Implemtns regular expression with allocated pattern buffer 
class CRegExpBuf : public CRegExp
    {
    public:
        IMPORT_C static CRegExp* NewL(const TDesC& aPattern, TInt aOptions = 0);
        IMPORT_C static CRegExp* NewLC(const TDesC& aPattern, TInt aOptions = 0);
        IMPORT_C ~CRegExpBuf();
    private:
        CRegExpBuf(HBufC* aPatBuf, TInt aOptions);
        void SetL();
    private:
        HBufC* const iPatBuffer;
    };




#endif
