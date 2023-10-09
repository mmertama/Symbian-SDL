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
#include"regexp.h"


enum
    {
    ENullFlag           =    0,
    ETokenAltClass      =    0x1,
    ETokenAltOr         =    0x2,
    ETokenAltBase       =    0x3, //composite
    ETokenGroupAnd      =    0x4,
    ETokenGroupBase     =    0x7, //composite
    ETokenRepeat        =    0x8,
    ETokenContainer     =    0x17, //composite
    ETokenRange         =    0x100,
    ETokenStatic         =   0x200,
    ETokenAlreadyMatched =   0x1000, 
    ETokenNoCase         =   0x2000,
    ETokenValidDes       =   0x4000,
    ETokenClose          =   0x10000,
    ETokenNegative       =   0x20000,
    ETokenNotGreed       =   0x40000,
    ETokenSingleLine     =   0x80000,
    ETokenAny            =   0xFFFFF
    };  


const TInt KRepeatMax(0xFFFFFFF);
const TInt KInvalidChar(-1);

const TInt KGroupGran(8);

const TInt KCharBeginLineOrNot('^');
const TInt KCharEndLine('$');
const TInt KCharWordBreak('b');
const TInt KCharNonWordBreak('B');
const TInt KCharAnyCharacter('.');
const TInt KCharDecimalDigit('d');
const TInt KCharNonDecimalDigit('D');
const TInt KCharNewline('n');
const TInt KCharReturn('r');
const TInt KCharTab('t');
const TInt KCharWhitespace('s');
const TInt KCharNonWhitespace('S');
const TInt KCharWord('w');
const TInt KCharNonWord('W');
const TInt KCharEscape('\\');

const TInt KCharClassStart('[');
const TInt KCharClassEnd(']');
const TInt KCharClassTo('-');

const TInt KCharGroupStart('(');
const TInt KCharGroupEnd(')');

const TInt KCharAlternative('|');

const TInt KCharRepeatZeroOrOnceOrNotGreed('?');
const TInt KCharRepeatZeroOrMany('*');
const TInt KCharRepeatOnceOrMany('+');
const TInt KCharRepeatStart('{');
const TInt KCharRepeatEnd('}');
const TInt KCharRepeatDelim(',');

const TInt KCharBinOct('0');
const TInt KCharBinHex('x');

void regExpPanic(TInt aErr)
    {
    User::Panic(_L("Regular Expression"), aErr);
    }


/******************************************************/

class CPatTokenBase : public CBase
    {
    public:
        inline TInt Flags() const;
	    inline CPatTokenBase* Next() const;
	    inline const TDesC& Ptr() const;
        inline CPatTokenBase* RemoveNext();
        inline void ClearMatched();
        inline void SetMatched();

	    virtual TBool Match(const TDesC& aMatch) = 0;
        virtual void SetNext(CPatTokenBase* aToken);

        virtual void Zero();
        ~CPatTokenBase();
    protected:
	    inline CPatTokenBase();
        inline void SetFlag(TBool aSet, TInt aFlag);
        inline void SetDes(const TDesC& aDes);
        inline void SetDes(const TText* aDes, TInt aLength);
    private:
        TInt iFlags;
	    	TPtrC iPtr;	
	    	CPatTokenBase* iNext;
    };

CPatTokenBase::~CPatTokenBase()
    {
    }

class CPatTokenContainer : public CPatTokenBase
    {
    public:
    inline CPatTokenContainer();
    virtual CPatTokenBase* Remove(CPatTokenBase* aPrev) = 0;
    };


class CPatTokenGroup : public CPatTokenContainer
    {
    public:
        void SetNext(CPatTokenBase* aToken);
        void Close();
        TBool Open() const;
        CPatTokenBase* Remove(CPatTokenBase* aPrev);
        CPatTokenBase* Last() const;
        ~CPatTokenGroup();
        void Zero();
        CPatTokenGroup* Container(CPatTokenBase* ) const;
    protected:
        inline CPatTokenGroup();
	protected:
	    CPatTokenBase* iChild;
    };

class CPatTokenStatic : public CPatTokenBase
    {
    public:
	    inline CPatTokenStatic(const TDesC& aPtr);
        TBool Match(const TDesC& aText);
        void Inc();
        void SetCase(TBool aCase);
    protected:
        TPtrC iPatPtr;
    };

class CPatTokenAlt : public CPatTokenGroup
    {
    public:
        TBool Match(const TDesC& aText);
    protected:
        inline CPatTokenAlt();
    };
   

class CPatTokenAltClass : public CPatTokenAlt
    {
    public:
        inline CPatTokenAltClass();
        inline void SetNegative(TBool aNegative);
    };

class CPatTokenAltOr : public CPatTokenAlt
    {
    public:
        inline CPatTokenAltOr(CPatTokenBase* aChild);
    };

class CPatTokenRepeat :  public CPatTokenContainer
    {
    public:
        inline CPatTokenRepeat(CPatTokenBase* aBase, TInt aMin, TInt aMax);
        inline void SetNotGreed(TBool aGreed);
        inline void SetMin(TInt aMin);
        inline void SetMax(TInt aMax);
        inline void SetParent(CPatTokenContainer* aParent);
        TBool Match(const TDesC& aText);
        ~CPatTokenRepeat();
        CPatTokenBase* Remove(CPatTokenBase* aPrev);
    private:
        inline CPatTokenBase* AnyNext() const;
        void CheckReverse(CPatTokenBase& aNext,
            const TDesC& aMatch,
            TInt& aLength, TInt& aFound);
    private:
        CPatTokenBase* iBase;
        CPatTokenContainer* iParent;
        TInt iMin;
        TInt iMax;
    };

class CPatTokenGroupAnd : public CPatTokenGroup
    {
    public:
        inline CPatTokenGroupAnd();
        TBool Match(const TDesC& aMatch);
    };

class CPatTokenRange : public CPatTokenBase
    {
    public:
        inline CPatTokenRange();
        inline void SetStart(TInt aStart);
        inline void SetEnd(TInt aEnd);
        TBool Match(const TDesC& aText);
    private:
        private:
        TInt iStart;
        TInt iEnd;
    };

class CPatTokenWhiteSpace : public CPatTokenBase
    {
    public:
        inline CPatTokenWhiteSpace();
        TBool Match(const TDesC& aText);
    private:
    };

class CPatTokenNonWhiteSpace : public CPatTokenBase
    {
    public:
        inline CPatTokenNonWhiteSpace();
        TBool Match(const TDesC& aText);
    private:
    };

class CPatTokenWord : public CPatTokenBase
    {
    public:
        inline CPatTokenWord();
        TBool Match(const TDesC& aText);
    private:
    };


class CPatTokenNonWord : public CPatTokenBase
    {
    public:
        inline CPatTokenNonWord();
        TBool Match(const TDesC& aText);
    private:
    };

class CPatTokenWordBreak : public CPatTokenWord
    {
    public:
        inline CPatTokenWordBreak();
        TBool Match(const TDesC& aText);
    };


class CPatTokenNonWordBreak : public CPatTokenWordBreak
    {
    public:
        inline CPatTokenNonWordBreak();
        TBool Match(const TDesC& aText);
    };


class CPatTokenDecimal : public CPatTokenRange
    {
    public:
        inline CPatTokenDecimal();
    };

class CPatTokenNonDecimal : public CPatTokenDecimal
    {
    public:
        inline CPatTokenNonDecimal();
        TBool Match(const TDesC& aText);
    };


class CPatTokenChar : public CPatTokenBase
    {
    public:
        inline CPatTokenChar(TInt aChar);
        TBool Match(const TDesC& aText);
    protected:
        TInt iChar;    
    };

class CPatTokenBin : public CPatTokenChar
    {
    public:
        inline CPatTokenBin();
        TInt SetBinChar(const TDesC& aData);
    };

class CPatTokenNonChar : public CPatTokenBase
    {
    public:
        inline CPatTokenNonChar(TInt aChar);
        TBool Match(const TDesC& aText);
    private:
        TInt iChar;    
    };





class CPatTokenLine : public CPatTokenBase
    {
    public:
        inline void SetSingleLine(TBool aSingleLine);
    protected:
        inline CPatTokenLine(const TDesC& aBuf);
    protected:
        const TDesC& iBuffer;
    };

class CPatTokenLineBegin : public CPatTokenLine
    {
    public:
        inline CPatTokenLineBegin(const TDesC& aBuf);
        TBool Match(const TDesC& aText);
    };


class CPatTokenLineEnd : public CPatTokenLine
    {
    public:
        inline CPatTokenLineEnd(const TDesC& aBuf);
        TBool Match(const TDesC& aText);
    };

class CPatTokenReference : public CPatTokenStatic
    {
    public:
        CPatTokenReference(const CGroupList& aList, TInt aIndex);
        TBool Match(const TDesC& aText);
    private:
        const CGroupList& iList;
        TInt iIndex;
    };

/********************************************************/


inline CPatTokenBase::CPatTokenBase() : iFlags(0),  iPtr(KNullDesC), iNext(NULL)
    {
    }

inline TInt CPatTokenBase::Flags() const
    {
    return iFlags;
    }

inline const TDesC& CPatTokenBase::Ptr() const
    {
    return iPtr;
    }

inline CPatTokenBase* CPatTokenBase::Next() const
    {
    return iNext;
    }


inline CPatTokenBase* CPatTokenBase::RemoveNext()
    {
    CPatTokenBase* p = Next();
    iNext = NULL;
    return p;
    }

inline void CPatTokenBase::SetFlag(TBool aTrue, TInt aFlag)
    {
    if(aTrue)
        iFlags |= aFlag;
    else
        iFlags &= ~aFlag;
    }

inline void CPatTokenBase::ClearMatched()
    {
    SetFlag(EFalse, ETokenAlreadyMatched);
    }

inline void CPatTokenBase::SetMatched()
    {
    SetFlag(ETrue, ETokenAlreadyMatched);
    }

void CPatTokenBase::SetNext(CPatTokenBase* aToken)
    {
    __ASSERT_DEBUG(iNext == NULL || aToken == NULL, regExpPanic(KErrAlreadyExists));
    __ASSERT_DEBUG(aToken != this, regExpPanic(KErrArgument));
    iNext = aToken;
    }
    
    
void CPatTokenBase::Zero()
    {
    iPtr.Set(KNullDesC); 
    SetFlag(EFalse, ETokenValidDes);
    ClearMatched();
    }

void CPatTokenBase::SetDes(const TDesC& aDes)
    {
    SetFlag(ETrue, ETokenValidDes);
    iPtr.Set(aDes);
    }
inline void CPatTokenBase::SetDes(const TText* aDes, TInt aLength)
    {
    SetFlag(ETrue, ETokenValidDes);
    iPtr.Set(aDes, aLength);
    }
    
inline CPatTokenContainer::CPatTokenContainer() : CPatTokenBase()
    {
    }

inline CPatTokenGroup::CPatTokenGroup() : CPatTokenContainer()
    {
    }


CPatTokenGroup* CPatTokenGroup::Container(CPatTokenBase* aToken) const
    {
    CPatTokenBase* cc = iChild;
    while(cc != NULL)
        {
        if(aToken == cc)
            return CONST_CAST(CPatTokenGroup*, this);
        if(cc->Flags() & ETokenGroupBase)
            {
            CPatTokenGroup* container =
                STATIC_CAST(CPatTokenGroup*, cc)->Container(aToken);
            if(container != NULL)
                return container;
            }
        cc = cc->Next();
        }
    return NULL;
    }

void CPatTokenGroup::SetNext(CPatTokenBase* aToken)
    {
    if(Open())
        {
        if(aToken->Flags() & ETokenRepeat)
            STATIC_CAST(CPatTokenRepeat*, aToken)->SetParent(this);

        if(iChild == NULL)
            {
            iChild = aToken;
            }
        else
            {
            CPatTokenBase* last = Last();
            last->SetNext(aToken);
            }
        }
    else
        CPatTokenBase::SetNext(aToken);
    }

// Close
//Function closes a group, thus current cannot be added there
//Function also closes all childs, since also in those any 
//tokens cannot be added.

void CPatTokenGroup::Close()
    {
    CPatTokenBase* cc = iChild;
    while(cc != NULL)
        {
        if(cc->Flags() & ETokenGroupBase)
            {
            CPatTokenGroup* rep = STATIC_CAST(CPatTokenGroup*, cc);
            if(rep->Open()) //there can be only one open in level
                {
                rep->Close();
                }
            }
        cc = cc->Next();
        }
    
    SetFlag(ETrue, ETokenClose);
    //return aParent;
    }


TBool CPatTokenGroup::Open() const
    {
    CPatTokenBase* cc = iChild;
    while(cc != NULL)
        {
        if(cc->Flags() & ETokenGroupBase)
            {
            CPatTokenGroup* rep = STATIC_CAST(CPatTokenGroup*, cc);
            if(rep->Open()) //if any is open is open
                return ETrue;
            }
        cc = cc->Next();
        }
    return (Flags() & ETokenClose) == 0;
    }

void CPatTokenGroup::Zero()
    {
    CPatTokenBase* prv = iChild;
    while(prv != NULL)
        {
        prv->Zero();
        prv = prv->Next();
        }
    CPatTokenBase::Zero();
    }

CPatTokenGroup::~CPatTokenGroup()
    {
    if(iChild != NULL)
        {
        CPatTokenBase* prv ;
        CPatTokenBase* last;
        for(;;)
            {
            prv = iChild;
            last = iChild->Next();
            if(last != NULL)
                {
                CPatTokenBase* tok;
                for(;;)
                    {
                    tok = last->Next();
                    if(tok == NULL)
                        break;
                    prv = last;
                    last = tok;
                    }
                delete last;
                prv->SetNext(NULL); 
                }
            else
                break;
            }
        delete iChild;
        }
    }
//destructor: childs are deleted from end
/*
CPatTokenGroup::~CPatTokenGroup()
    {
    if(iChild != NULL)
        {
        CPatTokenBase* prv;
        while((prv = iChild->Next()) != NULL)
            {
            CPatTokenBase* tok;
            while((tok = prv->Next()) != NULL)
                prv = tok;
            delete tok;
            prv->SetNext(NULL); 
            }
        delete iChild;
        }
    }
*/
/*
CPatTokenBase* CPatTokenGroup::Find(TInt aType, TInt& aIndex)
    {
    if(aIndex <= 0)
        return this;
    if(Flags() & aType)
        aIndex--;
    CPatTokenBase* cc = iChild;
    while(cc != NULL)
        {
        if(cc->Flags() & aType)
            {
            if(aIndex == 0)
                return cc;
            }

        if(cc->Flags() & ETokenGroupBase)
            {
            CPatTokenGroup* rep = STATIC_CAST(CPatTokenGroup*, cc);
            rep->Find(aType, aIndex);
            }
        
        aIndex--;
        cc = cc->Next();
        }
    return NULL;
    }
*/

CPatTokenBase* CPatTokenRepeat::Remove(CPatTokenBase* aPrev)
    {
    if(iBase == aPrev)
        {
        iBase = NULL;
        return this;
        }
    return NULL;
    }

CPatTokenBase* CPatTokenGroup::Last() const
    {
    CPatTokenBase* cc = iChild;
    for(;;)
        {
        CPatTokenBase* next = cc->Next();
        if(next == NULL)
            {
            return cc;
            }
        cc = next;
        }
    }

CPatTokenBase* CPatTokenGroup::Remove(CPatTokenBase* aPrev)
    {
    if(iChild != NULL )
        {
        if(aPrev == iChild)
            {
            iChild = iChild->Next();
            return this;
            }
        CPatTokenBase* c = iChild;
        while(c != NULL)
            {
            if(c->Next() == aPrev)
                {
                c->RemoveNext();
                c->SetNext(aPrev->Next());
                return c;
                }
            if(c->Flags() & ETokenContainer)
                {
                CPatTokenBase* tb = STATIC_CAST(CPatTokenContainer*, c)->Remove(aPrev);
                if(tb != NULL)
                    return tb;
                }
            c = c->Next();
            }
        }
    return NULL;
    }





inline CPatTokenStatic::CPatTokenStatic(const TDesC& aPtr) : CPatTokenBase(), iPatPtr(aPtr.Ptr(), 1)
    {
    SetFlag(ETrue, ETokenStatic);
    }

void CPatTokenStatic::SetCase(TBool aCase)
    {
    SetFlag(aCase, ETokenNoCase);
    }

const TInt KCapitalCh('A' - 'a');

inline TBool NotF(TInt aA, TInt aB)
    {
    if(aA != aB)
        {
        if(aA >= 'a' && aA <= 'z')
            return (aA + KCapitalCh) != aB;
        if(aA >= 'A' && aA <= 'Z')
            return (aA - KCapitalCh) != aB;
        return ETrue;
        }
    return EFalse;
    }

TBool CPatTokenStatic::Match(const TDesC& aText)
    {
    const TInt len = iPatPtr.Length();
    if(aText.Length() < len)
        return EFalse;
    if(Flags() & ETokenNoCase)
        {
        for(TInt i = 0; i < len; i++)
            if(NotF(aText[i], iPatPtr[i]))
                return EFalse;
        }
    else
        {
        for(TInt i = 0; i < len; i++)
            if(aText[i] != iPatPtr[i])
                return EFalse;
        }
    SetDes(aText.Left(len));
    return ETrue;
    }

void CPatTokenStatic::Inc()
    {
    iPatPtr.Set(iPatPtr.Ptr(), iPatPtr.Length() + 1);
    }

inline CPatTokenGroupAnd::CPatTokenGroupAnd() : CPatTokenGroup()
    {
    SetFlag(ETrue, ETokenGroupAnd);
    }

inline CPatTokenAlt::CPatTokenAlt() : CPatTokenGroup()
    {
    }


inline CPatTokenAltClass::CPatTokenAltClass() : CPatTokenAlt()
    {
    SetFlag(ETrue, ETokenAltClass);
    }

inline void CPatTokenAltClass::SetNegative(TBool aNegative)
    {
    SetFlag(aNegative, ETokenNegative);
    }

inline CPatTokenAltOr::CPatTokenAltOr(CPatTokenBase* aChild) : CPatTokenAlt()
    {
    SetFlag(ETrue, ETokenAltOr);
    SetNext(aChild);
    }

inline CPatTokenRepeat::CPatTokenRepeat(CPatTokenBase* aBase, TInt aMin, TInt aMax) 
: CPatTokenContainer(), iBase(aBase), iMin(aMin), iMax(aMax < 0 ? KRepeatMax : aMax)
    {
    SetFlag(ETrue, ETokenRepeat);
    __ASSERT_ALWAYS(aBase != NULL, regExpPanic(KErrNotFound));
    }

inline void CPatTokenRepeat::SetNotGreed(TBool aGreed)
    {
    SetFlag(aGreed, ETokenNotGreed);
    }

inline void CPatTokenRepeat::SetMax(TInt aMax)
    {
    iMax = aMax;
    }

CPatTokenRepeat::~CPatTokenRepeat()
    {
    delete iBase;
    }

inline void CPatTokenRepeat::SetMin(TInt aMin)
    {
    iMin = aMin;
    }

inline void CPatTokenRepeat::SetParent(CPatTokenContainer* aParent)
    {
    iParent = aParent;
    }

inline CPatTokenBase* CPatTokenRepeat::AnyNext() const
    {
    CPatTokenBase* next = Next();
    if(next == NULL && iParent != NULL)
        return iParent->Next();
    return next;
    }

TBool CPatTokenRepeat::Match(const TDesC& aMatch)
    {
	TInt found = 0;
	TInt len = 0;
 
	for(;;)
        {
        const TPtrC match(aMatch.Right(aMatch.Length() - len));
        if(!iBase->Match(match))
            break;
		found ++;
        if(found > iMin)
            {
            //if not greed then matched if there
            // are enough found and next matches 
		    if(ETokenNotGreed & Flags())
                {
                CPatTokenBase* next = AnyNext();
                if(next == NULL || next->Match(match))
                    {
                    SetDes(aMatch.Left(len));
			        return ETrue;
                    }
                }
            }
        const TInt baseLenght = iBase->Ptr().Length();
        if(baseLenght == 0)
            break; //hmm base has no length, but that might be acceptable
        len += baseLenght;
		if(found > iMax)
			return EFalse;
        }

    //propably too greed, have to come backwards
    //until next matches, unless parent is alternative
    if(iParent == NULL || !(iParent->Flags() & ETokenAltOr))
        {
        CPatTokenBase* next = AnyNext();
        if(next != NULL) //alterntive does not have to match
            CheckReverse(*next, aMatch, len, found);
        }

  
        
    SetDes(aMatch.Left(len));
	return found >= iMin;
    }

//if next wont match, we have to return until it fits
void CPatTokenRepeat::CheckReverse(CPatTokenBase& aNext,
                                   const TDesC& aMatch,
                                   TInt& aLength, TInt& aFound)
    {
    while(aFound > iMin)
            {
            if(aNext.Match(aMatch.Mid(aLength, aMatch.Length() - aLength)))
                {
                aNext.SetMatched();
                break;
                }
            aFound--;
            aLength -= iBase->Ptr().Length();
            }
    }

TBool CPatTokenAlt::Match(const TDesC& aMatch)
    {
    CPatTokenBase* cc = iChild;
    if(cc == NULL) //every string contains null string
        {
        SetDes(aMatch.Ptr(), 0);
        return (Flags() & ETokenNegative) == 0; //true if not set
        }
    const TBool nega = (Flags() & ETokenNegative) == 0;
	while(cc != NULL)
        {
		if(nega == cc->Match(aMatch)) //if set then not m 
            {
			SetDes(cc->Ptr());
			return ETrue;
            }
		cc = cc->Next(); 
        }
	return EFalse;
    }



TBool CPatTokenGroupAnd::Match(const TDesC& aMatch)
    {
    if(iChild == NULL)
        {
        SetDes(aMatch.Ptr(), 0);
        return ETrue;
        }

    CPatTokenBase* cc = iChild;

    TInt len = 0;
    
    for(;;)
        {
		if(!cc->Match(aMatch.Right(aMatch.Length() - len)))
			return EFalse;
        len += cc->Ptr().Length();
        CPatTokenBase* next = cc->Next();
        if(next == NULL)
            break;
		cc = next;
        }
    SetDes(aMatch.Left(len));
	return ETrue;
    }


inline CPatTokenRange::CPatTokenRange() : CPatTokenBase()
    {
    SetFlag(ETrue, ETokenRange);
    }

inline void CPatTokenRange::SetStart(TInt aStart)
    {
    iStart = aStart;
    }

inline void CPatTokenRange::SetEnd(TInt aEnd)
    {
    iEnd = aEnd;
    }


TBool CPatTokenRange::Match(const TDesC& aText)
    {
    if(aText.Length() <= 0)
        return EFalse;
    const TInt c = (aText[0]);
    SetDes(aText.Left(1));
    return (c >= iStart && c <= iEnd);
    }

inline CPatTokenWord::CPatTokenWord() : CPatTokenBase()
    {
    }

TBool CPatTokenWord::Match(const TDesC& aText)
    {
    if(aText.Length() <= 0)
        return EFalse;
    const TInt c = aText[0];
    if((c >= 'a' &&  c <= 'z') ||
       (c >= 'A' &&  c <= 'Z')  ||
       (c >= '0' &&  c <= '9') ||
       c == '_')
        {
        SetDes(aText.Left(1));
        return ETrue;
        }
    return EFalse;
    }

inline CPatTokenWhiteSpace::CPatTokenWhiteSpace() : CPatTokenBase()
    {
    }

TBool CPatTokenWhiteSpace::Match(const TDesC& aText)
    {
    if(aText.Length() <= 0)
        return EFalse;
    const TInt c = aText[0];
    if( c == ' ' ||
        c == '\t' ||
        c == '\n' ||
        c == '\r' ||
        c == '\f') 
        {
        SetDes(aText.Left(1));
        return ETrue;
        }
    return EFalse;
    }

inline CPatTokenNonWhiteSpace::CPatTokenNonWhiteSpace() : CPatTokenBase()
    {
    }

TBool CPatTokenNonWhiteSpace::Match(const TDesC& aText)
    {
    if(aText.Length() <= 0)
        return EFalse;
    const TInt c = aText[0];
   if( c == ' ' ||
        c == '\t' ||
        c == '\n' ||
        c == '\r' ||
        c == '\f') 
        {    
        return EFalse;
        }
    SetDes(aText.Left(1));
    return ETrue;
    }

inline CPatTokenNonWord::CPatTokenNonWord() : CPatTokenBase()
    {
    }

TBool CPatTokenNonWord::Match(const TDesC& aText)
    {
    if(aText.Length() <= 0)
        return ETrue;
    const TInt c = aText[0];
     if((c >= 'a' &&  c <= 'z') ||
       (c >= 'A' &&  c <= 'Z')  ||
       (c >= '0' &&  c <= '9') ||
       c == '_')
        {
        return EFalse;
        }
    SetDes(aText.Left(1));
    return ETrue;
    }

inline CPatTokenWordBreak::CPatTokenWordBreak() : CPatTokenWord()
    {
    }

TBool CPatTokenWordBreak::Match(const TDesC& aText)
    {
    const TInt len = aText.Length();
    if(len <= 0)
        return EFalse;
    const TBool a = CPatTokenWord::Match(aText);
    TBool b = EFalse;
    //iPtr.Set(aText.Left(1));
    if(aText.Length() > 1)
        {
        b = CPatTokenWord::Match(aText.Right(aText.Length() - 1));
        }
    SetDes(aText.Ptr(), 0);
    return a != b;
    }

inline CPatTokenNonWordBreak::CPatTokenNonWordBreak() : CPatTokenWordBreak()
    {
    }

TBool CPatTokenNonWordBreak::Match(const TDesC& aText)
    {
    return !CPatTokenWordBreak::Match(aText);
    }

inline CPatTokenDecimal::CPatTokenDecimal() : CPatTokenRange()
    {
    SetStart('0');
    SetEnd('9');
    }

inline CPatTokenNonDecimal::CPatTokenNonDecimal() : CPatTokenDecimal()
    {
    }

TBool CPatTokenNonDecimal::Match(const TDesC& aText)
    {
    return !CPatTokenDecimal::Match(aText); 
    }

CPatTokenLine::CPatTokenLine(const TDesC& aBuf) : CPatTokenBase(), iBuffer(aBuf)
    {
    }

inline void CPatTokenLine::SetSingleLine(TBool aSingle)
    {
    SetFlag(aSingle, ETokenSingleLine);
    }

CPatTokenLineBegin::CPatTokenLineBegin(const TDesC& aBuf) : CPatTokenLine(aBuf)
    {
    }

TBool CPatTokenLineBegin::Match(const TDesC& aText)
    {
    SetDes(aText.Left(0));
    if(aText.Ptr() == iBuffer.Ptr())
        return ETrue;
    else if(Flags() & ETokenSingleLine)
        return EFalse;
    else
        {
        const TInt pos = aText.Ptr() - iBuffer.Ptr();
        const TInt cc = iBuffer[pos - 1];
        const TInt c = iBuffer[pos];
        if(cc == '\n' || cc == '\r')
            return c != '\n' && c != '\r';
        }
    return EFalse;
    }

CPatTokenLineEnd::CPatTokenLineEnd(const TDesC& aBuf) : CPatTokenLine(aBuf)
    {
    }

TBool CPatTokenLineEnd::Match(const TDesC& aText)
    {
    SetDes(aText.Left(0));
   
   
   
    const TInt tl = aText.Length();
    const TInt bl = iBuffer.Length();
    /*if(aText.Ptr() + tl == iBuffer.Ptr() + bl)
        return ETrue;*/

    if(aText.Ptr() == iBuffer.Ptr() + bl)  //to end of text
        return ETrue;

    else if(Flags() & ETokenSingleLine)
        return EFalse;

    else
        {
        const TInt c = 	tl > 0 ? aText[0] : 0;
        const TInt cc = tl > 1 ? aText[1] : 0;
        return 
           (cc == '\n' ||
            cc == '\r' || 
            c == '\n' || 
            c == '\r');
        }
    }

inline CPatTokenChar::CPatTokenChar(TInt aChar) : CPatTokenBase(), iChar(aChar)
    {
    }


TBool CPatTokenChar::Match(const TDesC& aText)
    {
    if(aText.Length() <= 0)
        return EFalse;
    SetDes(aText.Left(1));
    return STATIC_CAST(TInt, aText[0]) == iChar;
    }

inline CPatTokenBin::CPatTokenBin() : CPatTokenChar('\0')
    {
    }
       
TInt CPatTokenBin::SetBinChar(const TDesC& aData)
    {
    const TRadix radix = (aData[0] == '0') ? EOctal : EHex;
    __ASSERT_ALWAYS(aData.Length() >  2, regExpPanic(KErrCorrupt)); //0|x??
    TPtrC num(KNullDesC);
    TInt consumed;
    if(aData[1] == '{')
        {
        __ASSERT_ALWAYS(radix == EHex, regExpPanic(KErrCorrupt));
        __ASSERT_ALWAYS(aData.Length() >  6, regExpPanic(KErrCorrupt)); //x{????}
        num.Set(aData.Mid(2, 4));
        consumed = 7;
        }
    else
        {
        num.Set(aData.Mid(1, 2));
        consumed = 3;
        }
    TUint val;
    const TInt err = TLex(num).Val(val, radix);
    __ASSERT_ALWAYS(err == KErrNone, regExpPanic(err));
    iChar = STATIC_CAST(TInt, val);
    return consumed;
    }

inline CPatTokenNonChar::CPatTokenNonChar(TInt aChar) : CPatTokenBase(), iChar(aChar)
    {
    }


TBool CPatTokenNonChar::Match(const TDesC& aText)
    {
    if(aText.Length() <= 0)
        return EFalse;
    SetDes(aText.Left(1));
    return STATIC_CAST(TInt, aText[0]) != iChar;
    }

CPatTokenReference::CPatTokenReference(const CGroupList& aList, TInt aIndex) :
    CPatTokenStatic(KNullDesC), iList(aList), iIndex(aIndex - 1)
    {
    __ASSERT_ALWAYS(aList.Count() > iIndex, regExpPanic(KErrCorrupt));
    }

TBool CPatTokenReference::Match(const TDesC& aMatch)
    {
    iPatPtr.Set(iList[iIndex]->Ptr());
    return CPatTokenStatic::Match(aMatch);
    }


/***********************************************************/

inline CPatTokenBase* CRegExp::Previous() const
    {
    return iCurrent;
    }

inline CPatTokenGroup* CRegExp::Parent() const 
    {
    return iParent;
    }


inline TBool CRegExp::IsParentType(TInt aType) const
    {
    return Parent() != NULL ? Parent()->Flags() & aType: EFalse;
    }

inline TBool CRegExp::IsCurrentType(TInt aType) const
    {
    return iCurrent != NULL ? iCurrent->Flags() & aType: EFalse;
    }


CRegExp::CRegExp(const TDesC& aPattern, TInt aOptions) :
    iOptions(aOptions), iPatPtr(aPattern), iBuffer(KNullDesC)
    {
    }

EXPORT_C CRegExp* CRegExp::NewLC(const TDesC& aPattern, TInt aOptions)
    {
    CRegExp* re = new (ELeave) CRegExp(aPattern, aOptions);
    CleanupStack::PushL(re);
    re->SetL();
    return re;
    }

EXPORT_C CRegExp* CRegExp::NewL(const TDesC& aPattern, TInt aOptions)
    {
    CRegExp* re = CRegExp::NewLC(aPattern,aOptions);
    CleanupStack::Pop();
    return re;
    }

CRegExpBuf::CRegExpBuf(HBufC* aPatBuf, TInt aOptions) :
    CRegExp(*aPatBuf, aOptions), iPatBuffer(aPatBuf)
    {
    }

EXPORT_C CRegExp* CRegExpBuf::NewLC(const TDesC& aPattern, TInt aOptions)
    {
    HBufC* buf = aPattern.AllocLC();
    CRegExpBuf* re = new (ELeave) CRegExpBuf(buf, aOptions);
    CleanupStack::Pop(); //buf
    CleanupStack::PushL(re);
    re->SetL();
    return re;
    }

EXPORT_C CRegExp* CRegExpBuf::NewL(const TDesC& aPattern, TInt aOptions)
    {
    CRegExp* re = CRegExpBuf::NewLC(aPattern, aOptions);
    CleanupStack::Pop();
    return re;
    }

void CRegExpBuf::SetL()
    {
    CRegExp::SetL();
    }

TInt CRegExp::Number(TInt& aNum) const
    {
    TInt i = 0;
    for(; i < iPatPtr.Length();i++)
        {
         const TInt cc = iPatPtr[i];
         if(cc < '0' || cc > '9')
            break;
        }
    if(i == 0)
        return -1;
    const TPtrC num = iPatPtr.Left(i);
    TLex lex(num);
    if(lex.Val(aNum) == KErrNone)
        return i;
    return -1;
    }

void CRegExp::ConsumeData(TInt aCount)
    {
    if(aCount > 0)
        iPatPtr.Set(iPatPtr.Right(iPatPtr.Length() - aCount));
    }

void CRegExp::Zero()
    {
    CPatTokenBase* prv = iFirst;
    while(prv != NULL)
        {
        prv->Zero();
        prv = prv->Next();
        }
    }

EXPORT_C CRegExp::~CRegExp()
    {
    if(iFirst != NULL)
        {
        CPatTokenBase* prv ;
        CPatTokenBase* last;
        for(;;)
            {
            prv = iFirst;
            last = iFirst->Next();
            if(last != NULL)
                {
                CPatTokenBase* tok;
                for(;;)
                    {
                    tok = last->Next();
                    if(tok == NULL)
                        break;
                    prv = last;
                    last = tok;
                    }
                delete last;
                prv->SetNext(NULL); 
                }
            else
                break;
            }
        delete iFirst;
        }
    delete iGroups;
    }

EXPORT_C CRegExpBuf::~CRegExpBuf()
    { 
    delete iPatBuffer;
    }

void CRegExp::SetL()
    {
	TInt prev = 0;

    __ASSERT_DEBUG(iFirst == NULL, regExpPanic(KErrAlreadyExists));

    iGroups = new (ELeave) CGroupList(KGroupGran);
  
	while(iPatPtr.Length() > 0)
        {
		const TInt c = iPatPtr[0];
        if(c <= ' ')
            {
            ConsumeData(1);
            continue;
            }
        if(ParseL(c, prev))
            {
            CreateStaticL();
            ConsumeData(1);
            }
        prev = c;
        }
    EndToken();
    }

TBool CRegExp::ParseL(TInt aCurrent, TInt aPrevious)
    {
    switch(aPrevious)
        {
        case KCharEscape:
            switch(aCurrent)
                {
                case KCharWordBreak:
                case KCharNonWordBreak:
                case KCharDecimalDigit:
                case KCharNonDecimalDigit:
                case KCharNewline:
                case KCharReturn:
                case KCharTab:
                case KCharWhitespace:
                case KCharNonWhitespace:
                case KCharWord:
                case KCharNonWord:
                case KCharBinHex:
                case KCharBinOct:
                    CreateSpecialL(aCurrent);
                
                  break;
               default:
                    {
                    TInt value;
                    const TInt n = Number(value);
                    if(n > 0)
                        {
                        CreateReferenceL(value);
                        ConsumeData(n);
                        return EFalse;
                        }
                    return ETrue;
                    }
                }
            break;
        case KCharRepeatStart:
            if(IsCurrentType(ETokenRepeat))
                {
                TInt value;
                const TInt n = Number(value);
                __ASSERT_ALWAYS(n > 0, regExpPanic(KErrCorrupt));
                ConsumeData(n);
                CPatTokenRepeat* rep = STATIC_CAST(CPatTokenRepeat*, Previous());
                rep->SetMin(value);
                return EFalse;
                }
        case KCharRepeatDelim:
            if(IsCurrentType(ETokenRepeat))
                {
                CPatTokenRepeat* rep = STATIC_CAST(CPatTokenRepeat*, Previous());
                if(aCurrent == KCharRepeatEnd)
                    {
                    rep->SetMax(KRepeatMax);
                    break;
                    }
                
                TInt value;
                const TInt n = Number(value);
                __ASSERT_ALWAYS(n > 0, regExpPanic(KErrCorrupt));
                ConsumeData(n);
                rep->SetMax(value);
                return EFalse;
                }
        case KCharClassTo:
            {
            //was return if not, but current have 
            //to check for e.g. -? case
            if(IsCurrentType(ETokenRange))
                {
                CPatTokenRange* rep = STATIC_CAST(CPatTokenRange*, Previous());
                rep->SetEnd(aCurrent);
                break;
                }
            }
        default:
            if(IsParentType(ETokenAltClass))
                    {
                    switch(aCurrent)
                        {
                        case KCharClassEnd:
                            EndToken();
                            break;
                        case KCharBeginLineOrNot:
                            {
                             CPatTokenAltClass* rep = STATIC_CAST(CPatTokenAltClass*, Parent());
                             rep->SetNegative(ETrue);
                            }
                            break;
                        case KCharClassTo:
                            CreateRangeL(aPrevious);
                            break;
                        default:
                            {
                            CreateSpecialL(-aCurrent);
                            break;
                            }
                        }
                    break; 
                    }
            else
            switch(aCurrent)
                {
                case KCharEscape:
                    break;
                case KCharClassStart:
                    CreateAltClassL();
                    break;
                //case KCharClassEnd:
                //    EndToken();
                //    break;
                case KCharGroupStart:
                    CreateGroupL();
                    break;
                case KCharGroupEnd:
                    EndToken();
                    break;
                case KCharAlternative:
                    CreateAltOrL();
                    break;
                case KCharRepeatZeroOrOnceOrNotGreed:
                    if(IsCurrentType(ETokenRepeat))
                         STATIC_CAST(CPatTokenRepeat*, Previous())->SetNotGreed(ETrue);
                    else
                        CreateRepeatL(0, 1);
                    break;
                case KCharRepeatZeroOrMany:
                    CreateRepeatL(0, KRepeatMax);
                    break;
                case KCharRepeatOnceOrMany:
                    CreateRepeatL(1, KRepeatMax);
                   break;
                case KCharRepeatStart:
                    CreateRepeatL(0, KRepeatMax);
                    break;
                case KCharRepeatEnd:
                    break;
                case KCharRepeatDelim:
                     if(IsCurrentType(ETokenRepeat))
                       break;
                     return ETrue;
                case KCharAnyCharacter:
                case KCharBeginLineOrNot:
                case KCharEndLine:
                     CreateSpecialL(aCurrent);
                     break;
                case KCharClassTo:
                    if(IsCurrentType(ETokenRange))
                        break;
                default:
                    return ETrue;
                }
        }
    ConsumeData(1);
    return EFalse;
    }


void CRegExp::CreateReferenceL(TInt aIndex)
    {
    CPatTokenBase* pat = new (ELeave) CPatTokenReference(*iGroups, aIndex);
    AddToken(pat);
    }

void CRegExp::CreateRangeL(TInt aStart)
    {
    //there is a static before dash
    //thus it is removed
    //and considered as a range start
    CPatTokenBase* prev = Remove(iCurrent);
    CPatTokenBase* del = iCurrent;
    iCurrent = prev;
    CPatTokenRange* pat = new (ELeave) CPatTokenRange();
    pat->SetStart(aStart);
    delete del;
    AddToken(pat);
    }

void CRegExp::CreateGroupL()
    {
    CPatTokenGroupAnd* pat = new (ELeave) CPatTokenGroupAnd();
    iGroups->AppendL(pat);
    AddToken(pat);
    }

void CRegExp::CreateAltClassL()
    {
    CPatTokenBase* pat = new (ELeave) CPatTokenAltClass();
    AddToken(pat);
    }

CPatTokenBase* CRegExp::Remove(CPatTokenBase* aPrev)
    {
    CPatTokenBase* cc = iFirst;
    if(aPrev == iFirst)
        iFirst = NULL;
    while(cc != NULL)
        {
        if(cc->Next() == aPrev)
            {
            cc->RemoveNext();
            cc->CPatTokenBase::SetNext(aPrev->Next());
            return cc;
            }
        if(cc->Flags() & ETokenContainer)
            {
            CPatTokenContainer* g = STATIC_CAST(CPatTokenContainer*, cc);
//            TInt index = 0;
            
            CPatTokenBase* prev = g->Remove(aPrev);

            if(prev != NULL)
                return prev;

            /*
            TBool chld = EFalse;

            for(;;)
                {
                TInt i = index;
                CPatTokenBase* gg = g->Find(ETokenAny, i); 
             
                if(gg == aPrev)
                    chld = ETrue;
         
                if(gg == NULL)
                    break;
                if(gg->Next() == aPrev)
                    {
                    gg->RemoveNext();
                    gg->CPatTokenBase::SetNext(aPrev->Next());
                    return gg;
                    }
                index++;
                }

            if(chld)
                {
                g->RemoveLast();
                return g;
                }
            */
            }
        cc = cc->Next();
        }
    return NULL;
    }

void CRegExp::CreateAltOrL()
    {
    if(IsParentType(ETokenAltOr))
        {
        iCurrent = Parent();
        return;
        }
    //Let's find currents prevoius and set 
    //that to current, current current is
    //belonings of alt
    CPatTokenBase* pat = new (ELeave) CPatTokenAltOr(iCurrent);
    CPatTokenBase* prev = Remove(iCurrent);
    iCurrent = prev;
    AddToken(pat);
    }

void CRegExp::CreateRepeatL(TInt aMin, TInt aMax)
    {
    __ASSERT_ALWAYS(Previous() != NULL, regExpPanic(KErrCorrupt));
    CPatTokenBase* pat = new (ELeave) CPatTokenRepeat(Previous(), aMin, aMax);
    iCurrent = Remove(iCurrent);
    AddToken(pat);
    }

void CRegExp::CreateStaticL()
    {
    if(IsCurrentType(ETokenStatic))
        {
        STATIC_CAST(CPatTokenStatic*, Previous())->Inc();
        }
    else
        {
        CPatTokenStatic* pat = new (ELeave) CPatTokenStatic(iPatPtr);
        pat->SetCase(iOptions & ECase);
        AddToken(pat);
        }
    }


void CRegExp::CreateSpecialL(TInt aChar)
    {
    CPatTokenBase* tok = NULL;
    switch(aChar)
        {
        case KCharWordBreak:
            tok = new (ELeave) CPatTokenWordBreak();
            break;
        case KCharNonWordBreak:
            tok = new (ELeave) CPatTokenNonWordBreak();
            break;
        case KCharDecimalDigit:
             tok = new (ELeave) CPatTokenDecimal();
            break;
        case KCharNonDecimalDigit:
             tok = new (ELeave) CPatTokenNonDecimal();
            break;
        case KCharNewline:
            tok = new (ELeave) CPatTokenChar('\n');
            break;
        case KCharReturn:
            tok = new (ELeave) CPatTokenChar('\r');
            break;
        case KCharTab:
            tok = new (ELeave) CPatTokenChar('\t');
            break;
        case KCharWhitespace:
            tok = new (ELeave) CPatTokenWhiteSpace();
            break;
        case KCharNonWhitespace:
             tok = new (ELeave) CPatTokenNonWhiteSpace();
             break;
        case KCharWord:
             tok = new (ELeave) CPatTokenWord();
             break;
        case KCharNonWord:
            tok = new (ELeave) CPatTokenNonWord();
            break;
        case KCharBeginLineOrNot:
            tok = new (ELeave) CPatTokenLineBegin(iBuffer);
            STATIC_CAST(CPatTokenLine*, tok)->SetSingleLine(TBool(iOptions & ESingleLine));
            break;
        case KCharEndLine:
            tok = new (ELeave) CPatTokenLineEnd(iBuffer);
            STATIC_CAST(CPatTokenLine*, tok)->SetSingleLine(TBool(iOptions & ESingleLine));
            break;
        case KCharAnyCharacter:
             if(iOptions & ESingleLine)
                tok = new (ELeave) CPatTokenNonChar(KInvalidChar);
             else
                tok = new (ELeave) CPatTokenNonChar('\n');
             break;
        case KCharBinOct:
        case KCharBinHex:
            {
            tok = new (ELeave) CPatTokenBin();
            const TInt consumed = STATIC_CAST(CPatTokenBin*, tok)->SetBinChar(iPatPtr);
            ConsumeData(consumed - 1); // if e.g \x12 the \is before and 12 is now and x by default
            }
            break;
        default:
            __ASSERT_DEBUG(aChar < 0, regExpPanic(KErrNotFound));
            tok = new (ELeave) CPatTokenChar(-aChar);
            break;
        }
    AddToken(tok);
    }


void CRegExp::AddToken(CPatTokenBase* aToken)
    {
    if(iCurrent != NULL)
        {
        iCurrent->SetNext(aToken);
        }
    else
        {
        __ASSERT_DEBUG(iFirst == NULL, regExpPanic(KErrAlreadyExists));
        iFirst = aToken;
        }
    iCurrent = aToken;
    if(IsCurrentType(ETokenGroupBase))
        iParent = STATIC_CAST(CPatTokenGroup*, iCurrent);
    }


CPatTokenGroup* CRegExp::Container(CPatTokenBase* aChild) const
    {
    CPatTokenBase* cc = iFirst;
    while(cc != 0)
        {
        if(cc->Flags() & ETokenGroupBase)
            {
            CPatTokenGroup* parent = STATIC_CAST(CPatTokenGroup*, cc)->
                Container(aChild);
            if(parent != NULL)
                return parent;
            }
        cc = cc->Next();
        }
    return NULL;
    }


void CRegExp::EndToken()
    {
    if(IsParentType(ETokenGroupBase))
        {
        CPatTokenGroup* rep = STATIC_CAST(CPatTokenGroup*, Parent());
        rep->Close();
        iParent = Container(rep);
        iCurrent = rep;
        }
    }



EXPORT_C TPtrC CRegExp::SubStr(TInt aIndex, TBool& aFound) const
    {
    if(aIndex < iGroups->Count())
        {
        CPatTokenBase* pat = (*iGroups)[aIndex];
        if(pat->Flags() & ETokenValidDes)
            {
            aFound = ETrue;
            return pat->Ptr();
            }
        }
    aFound = EFalse;
    return KNullDesC();
    }
    /*
    CPatTokenBase* cc = iFirst;
	TInt index = aIndex;
    aFound = EFalse;
    while(cc != NULL)
        {
        if(cc->Flags() & ETokenGroupBase)
            {
            CPatTokenGroup* g = STATIC_CAST(CPatTokenGroup*, cc);
            CPatTokenBase* gg = g->Find(ETokenGroupAnd, index);   
            if(gg != NULL)
                {
                aFound = ETrue;
	            return gg->Ptr();
                }
            }			
        cc = cc->Next();
        }
    return KNullDesC;
    */
    


EXPORT_C TBool CRegExp::Match(const TDesC& aMatch)
    {
    __ASSERT_DEBUG(iFirst != NULL, regExpPanic(KErrNotFound));
    TInt len = aMatch.Length();
    iBuffer.Set(aMatch);
    TPtrC buffer(KNullDesC);
    do
        {
	    TInt pos = 0;
        Zero();
        buffer.Set(aMatch.Right(len));
        len--;
        CPatTokenBase* tok = iFirst;
        TBool already = EFalse;
	    while(already || tok->Match(buffer.Right(buffer.Length() - pos)))
            {
            pos += tok->Ptr().Length();
		    tok = tok->Next();
	        if(tok == NULL)
		        return ETrue;
            already = tok->Flags() & ETokenAlreadyMatched;
            if(already)
                tok->ClearMatched();
            }
        }while(len > 0);
    return EFalse;
    }	

EXPORT_C TInt CRegExp::SubstituteL(HBufC*& aSubstitute, const TDesC& aText)
    {
    TPtrC before = Matched(EBefore);
    
    aSubstitute = HBufC::NewL(aText.Length() + before.Length());
    TPtr ptr = aSubstitute->Des();
    ptr.Append(before);
    ptr.Append(aText);

    TInt count = 1;

    TPtrC after = Matched(EAfter);

    if(iOptions & EGlobal)
        {
        for(;;)
            {
            const TPtrC rest = Matched(EAfter);
            if(!Match(rest))
                {
                after.Set(rest);
                break;
                }
            const TPtrC before = Matched(EBefore);
            aSubstitute = aSubstitute->ReAllocL(
                aSubstitute->Length() +
                aText.Length() + 
                before.Length());
            TPtr ptr = aSubstitute->Des();
            ptr.Append(before);
            ptr.Append(aText);
            count++;
            }
        }
    aSubstitute = aSubstitute->ReAllocL(
                aSubstitute->Length() + after.Length());
    TPtr aft = aSubstitute->Des();
    aft.Append(after);
    return count;
    }
        
EXPORT_C TPtrC CRegExp::Last(TBool& aFound) const
    {
    aFound = iGroups->Count() > 0;
    return (*iGroups)[iGroups->Count()]->Ptr();
    }

EXPORT_C TPtrC CRegExp::Matched(TMatchString aMatch) const
    {
    __ASSERT_ALWAYS(iFirst != NULL, regExpPanic(KErrNotFound));
    CPatTokenBase* pat = iFirst;
    TInt len = 0;
    while(pat != NULL)
        {
        len += pat->Ptr().Length();
        CPatTokenBase* next = pat->Next();
        if(next == NULL)
            break;
        pat = next;
        }

   TText const * pos = NULL;

   const TInt lenbefore = iFirst->Ptr().Ptr() - iBuffer.Ptr();

    switch(aMatch)
        {
        case EEntire:
            pos = iFirst->Ptr().Ptr();
            break;
        case EBefore:
            pos = iBuffer.Ptr();
            len = lenbefore;
            break;
        case EAfter:
           pos = pat->Ptr().Ptr() + pat->Ptr().Length();
           len = iBuffer.Length() - lenbefore - len;
           break;
        }
    __ASSERT_ALWAYS(pos != NULL, regExpPanic(KErrArgument));
    return TPtrC(pos, len);
    }


EXPORT_C TInt CRegExp::SubCount() const
    {
    return iGroups->Count();
    }

EXPORT_C TInt CRegExp::Check(const TDesC& aPattern)
    {
    TRAPD(err, 
        CRegExp::NewLC(aPattern);
        CleanupStack::PopAndDestroy(); //re
        )
    return err;
    }



