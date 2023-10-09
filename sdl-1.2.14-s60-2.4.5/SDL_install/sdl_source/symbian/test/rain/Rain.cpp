/*471188***********************************************************
		This file is part of rain.
																	
    rain is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    rain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with rain.  If not, see <http://www.gnu.org/licenses/>.
    
    Markus Mertama @ http://koti.mbnet.fi/mertama
    ******************************************************************/
#include "rain.h"


/*
#define FIX_SHIFT 8
#define ONE_PER_SIX 43  //1/6 << 16
#define DAMPFACTOR 3    //0.1 << 16
#define HALF 127
*/

const TInt FIX_SHIFT (16);
const TInt ONE_PER_SIX (10923);  //1/6 << 16
//#define DAMPFACTOR 6556    //0.1 << 16
const TInt DAMPFACTOR (1); //1?
//const TInt HALF (32768);

const TInt RAD = 12; //12
const TInt DEPTH = 32; //16

//#define FIX_SHIFT 24
//#define ONE_PER_SIX 2796203  //1/6 << 16
//#define DAMPFACTOR 167772    //0.1 << 16

//const TInt GRAY (0x777); //Grey

const TInt KSqrt[] = {0,    //1
                    1,1,1, //3
                    2,2,2,2,2, //5
                    3,3,3,3,3,3,3, //7
                    4,4,4,4,4,4,4,4,4, //9
                    5,5,5,5,5,5,5,5,5,5,5, //11
                    6,6,6,6,6,6,6,6,6,6,6,6,6, //13
                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  //15
                    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, //17
                    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, //19
                    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10, //21
                    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11, //23
                    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12 //25
                };

inline TInt Max(TInt aX, TInt aY) {return aX > aY ? aX : aY;}
inline TInt Min(TInt aX, TInt aY) {return aX < aY ? aX : aY;}


class CMap : public CBase
    {
    public:
        CMap() : iMap(0), iSize(0) {}
        virtual ~CMap();
        void SetSize(const TSize& aSize);
        inline TPix* Map();
        void Swap(CMap& aMap);
    private:
        TPix* iMap;
        TInt iSize;
    };

CMap::~CMap()
    {
    delete[] iMap;
    }

void CMap::SetSize(const TSize& aSize)
    {
   	const TInt sz = aSize.iWidth * aSize.iHeight;
   	if(sz != iSize)
   	    {
   	    delete[] iMap;
   	    iMap = 0;
   	    iMap = new TPix[sz];
   	    }
    for(int i = 0; i < sz; i++)
    	iMap[i] = 0;
    }


void CMap::Swap(CMap& aMap)
    {
    TPix* tmp = aMap.iMap;
    aMap.iMap = iMap;
    iMap = tmp;
    }

inline TPix* CMap::Map()
    {
    return iMap;
    }


CRain::CRain() : iSize(0, 0)
    {
	iRBuf = new CMap();
	iWBuf = new CMap();
    iCanvas = new  CMap();
	}

void CRain::SetSize(const TSize& aSize)
    {
    iSize = aSize;
    iCanvas->SetSize(iSize);
    iRBuf->SetSize(iSize);
    iWBuf->SetSize(iSize);
    }

void CRain::Copy(MCopier& aCopier)
    {
    const TPoint pos (
               Max(0, (iSize.iWidth - aCopier.Size().iWidth) / 2),
               Max(0, (iSize.iHeight - aCopier.Size().iHeight) / 2));
       
       TPix* p = iCanvas->Map();
       
       p += pos.iX + pos.iY * iSize.iWidth;
       
       for(TInt j = 0; j < aCopier.Size().iHeight; j++)
           {
           for(TInt i = 0; i < aCopier.Size().iWidth; i++)
               {
               *p = aCopier.GetPixel(i, j);
               ++p;
               }
           p +=  iSize.iWidth - aCopier.Size().iWidth;
           }
    }

CRain::~CRain()
	{
	delete iRBuf;
	delete iWBuf;
	delete iCanvas;
	}

inline TInt SqrDist(TInt aX1, TInt aY1, TInt aX2, TInt aY2)
    {
		const TInt dx = aX2 - aX1;
		const TInt dy = aY2 - aY1;
		return (dx * dx) + (dy * dy);
    }


void CRain::Drop(const TPoint& aPoint)
	{

	const TInt mrady = Max(0, aPoint.iY - RAD);
	const TInt mradx = Max(0, aPoint.iX - RAD);
	const TInt maxrady = Min(iSize.iHeight, aPoint.iY + RAD);
	const TInt maxradx = Min(iSize.iWidth, aPoint.iX + RAD);

	for(TInt j = mrady ; j < maxrady ; j++)
			for(TInt i = mradx ; i < maxradx ; i++)
				{
				const TInt dist = SqrDist(aPoint.iX,aPoint.iY,i,j);
				if(dist < RAD * RAD)
					{
					const TInt sq = KSqrt[dist];
					const TInt fd = DEPTH * ((RAD - sq)) /RAD;
					//fd &= 0x7F;
					iWBuf->Map()[j * iSize.iWidth + i] = static_cast<TPix>(fd & 0x7FFF);
					}
				}
	}


void CRain::Rain(TPix* aPix)
	{
	iRBuf->Swap(*iWBuf);

 	const TInt start = iSize.iWidth * 2 + 2;
 	const TInt end = (iSize.iWidth * (iSize.iHeight - 2)) - 2;
 	const TInt w2 = iSize.iWidth * 2;
 	const TInt w = iSize.iWidth;

 	TPix* ptr = iCanvas->Map();

 	TPix i = 0;
 	TPix j = 0;

 	TPix* wbuf = iWBuf->Map();
 	TPix* rbuf = iRBuf->Map();

	for(TInt p = start ; p < end; p++)
		{
 		i++;
 		
        if(i >= iSize.iWidth)
            {
            i = 0;
            j++;
            }

 		TInt value = 
            rbuf[p - 2] +
            rbuf[p + 2] +
            rbuf[p - w2] +
            rbuf[p + w2] +
            rbuf[p - 1] +
            rbuf[p + 1] +
            rbuf[p - w] +
            rbuf[p + w] +
            rbuf[p - 1 - w] +
            rbuf[p + 1 - w] +
            rbuf[p - 1 + w] +
            rbuf[p + 1 + w];

 		
 		if(value == 0 && wbuf[p] == 0)
 		    {
 		    aPix[p] = ptr[p];
 		    continue;
 		    }
			
		value *= ONE_PER_SIX;
        value -= wbuf[p] << FIX_SHIFT;

		value += ((1 << FIX_SHIFT) - 1); 

		value >>= FIX_SHIFT;
			
		const TInt subval = value * DAMPFACTOR;

		value <<= FIX_SHIFT;
		value -= subval;
		value >>= FIX_SHIFT;
		
		if(value > (1 << FIX_SHIFT))
		    value = (1 << FIX_SHIFT);
		
		if(value < -(1 << FIX_SHIFT))
		        value = -(1 << FIX_SHIFT);

		wbuf[p] = value;

		TPix xoff = i;
		TPix yoff = j;
    
		xoff -= rbuf[p - 1];
		xoff -= rbuf[p + 1];

		yoff -= rbuf[p - w];
		yoff += rbuf[p + w];

		if (xoff < 0) xoff =  0;
		if (xoff > iSize.iWidth) xoff =  iSize.iWidth;

		if (yoff < 0) yoff =  0;
		if (yoff > iSize.iHeight) yoff =  iSize.iHeight;

        
		const TPix co = ptr[xoff + yoff * iSize.iWidth];
		aPix[p] = co;
		}
	}

