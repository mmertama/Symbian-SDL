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


#ifndef __RAIN_H__
#define __RAIN_H__



class CMap;

 // some classes used in Symbian...
 
typedef signed int TInt;
typedef signed short TInt16; 
 
class CBase
	{
	public:
		virtual ~CBase();
	};
	
class TPoint
	{
	public:
		TPoint(TInt aX, TInt aY) : iX(aX), iY(aY) {};
		TInt iX;
		TInt iY;
	};

class TSize
	{
	public:
			TSize(TInt aWidth, TInt aHeight) : iWidth(aWidth), iHeight(aHeight) {};
			TInt iWidth;
			TInt iHeight;
	};
	

typedef int TPix;	

class MCopier
    {
public:
    virtual TPix GetPixel(TInt, TInt) const = 0;
    virtual TSize Size()const = 0;
    };

class CRain : public CBase
{
public:
	void Rain(TPix* aPix);
	~CRain();
	void Drop(const TPoint& aPoint);
	void Copy(MCopier& aCopier);
	CRain();
	void SetSize(const TSize& aSize);
private:
    TSize iSize;
    CMap* iCanvas;
	CMap* iRBuf;
	CMap* iWBuf;
};


#endif
