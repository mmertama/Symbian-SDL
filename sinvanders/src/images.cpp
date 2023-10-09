/*
    SDL SPACE INVADERS - The Game
    Copyright (C) 2009  Markus Mertama

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "images.h"
#include <sstream>

using namespace std;


Image::Image() : iImage(NULL)
	{
	}	
	
class InvanderBmpException : public InvanderException
    {
public:
    InvanderBmpException(const string& bmp);
    };

InvanderBmpException::InvanderBmpException(const string& bmp) : InvanderException("Cannot load:" + bmp)
    {}
	
void Image::draw(const Position& pos, SDL_Surface& screen) const
	{
	SDL_Rect targetRect = {pos.x, pos.y, iImage->w, iImage->h};
	SDL_BlitSurface(iImage, NULL, &screen, &targetRect);
	}
	
	
	
	
void Image::loadBmp(const string& bmp, bool trySmallImages, bool rawmode)
	{
	
	const string prefix = trySmallImages ? "_small" : ""; 
	const string maskName = bmp + "_mask" + prefix + ".bmp";
	const string name = bmp + prefix + ".bmp";
	
	if(!loadFaceBmp(name, rawmode))
	    {
	    if(trySmallImages)
	        {
	        loadBmp(bmp, false, rawmode);
	        return;
	        }
	    else
	        throw InvanderBmpException(name);
	    }
	if(!rawmode)
		loadMask(maskName);
	}	
	
Image::~Image()
	{
	SDL_FreeSurface(iImage);
	}
	
bool Image::loadFaceBmp(const string& aBmp, bool rawmode)
	{
	SDL_Surface* source = SDL_LoadBMP(aBmp.c_str());
	if(source == NULL)
        return false;
	
	SDL_FreeSurface(iImage);
	iImage = NULL;
	
	if(rawmode)
		{
		iImage = source;
		}
	else
		{
		iImage = SDL_DisplayFormatAlpha(source);
		SDL_FreeSurface(source);
		}
	return true;
	}

bool Image::loadMask(const string& aMask)
	{
	SDL_Surface* mask = SDL_LoadBMP(aMask.c_str());
	if(mask == NULL)
		return false;
	
	if(!(mask->h == iImage->h && iImage->w == mask->w))
		throw InvanderException("Inproper mask");
	
	if(iImage->format->BytesPerPixel == 4 && SDL_MUSTLOCK(iImage) && SDL_LockSurface(iImage) < 0)
			throw InvanderException("Image format");

	if(mask->format->BytesPerPixel == 1 && SDL_MUSTLOCK(mask) && SDL_LockSurface(mask) < 0)
		throw InvanderException("Mask format");		
	
	for(int j = 0; j < mask->h; j++)
		{
		Uint32* t = static_cast<Uint32*>(iImage->pixels);
		t += j * (iImage->pitch >> 2);
		Uint8* m = static_cast<Uint8*>(mask->pixels);
		m += j * mask->pitch;
		for(int i = 0; i  < mask->w; i++)
			{
			Uint8 r, g, b;
			SDL_GetRGB(*t, iImage->format, &r, &g, &b);
			*t = SDL_MapRGBA(iImage->format, r, g, b, *m);
			++m;
			++t;
			}
		}
			
	if(SDL_MUSTLOCK(iImage))
		SDL_UnlockSurface(iImage);	 	
	if(SDL_MUSTLOCK(mask))
		SDL_UnlockSurface(mask);
	SDL_FreeSurface(mask);
	mask = NULL;
	return true;
	}
	
Size Image::imageSize() const
	{
	const Size s = {iImage->w, iImage->h};
	return s;
	}	
	
void Image::drawOn(SurfaceManagerInterface& smi, const Position& pos, int info) const
	{
	smi.drawOn(*iImage, pos, info);
	}
	
AnimationImage::AnimationImage() : iFrameRate(1), iFrameRequest(0),
	iFrames(1),  iFrameHeight(0), iCurrentFrame(0) 
	{
	}
	
AnimationImage::~AnimationImage()
	{
	}
	
void AnimationImage::setFrameRate(int rate)
	{
	iFrameRate = rate;
	}	

void AnimationImage::draw(const Position& pos, SDL_Surface& aScreen) const
	{
	const int offset = iFrameHeight * iCurrentFrame;
	SDL_Rect sourceRect = {0, offset, surface()->w, iFrameHeight};
	SDL_Rect targetRect = {pos.x, pos.y, surface()->w, iFrameHeight};
	SDL_BlitSurface(const_cast<SDL_Surface*>(surface()), &sourceRect, &aScreen, &targetRect);
	}
	
void AnimationImage::setFrameCount(int frameCount)
	{
	if(frameCount <= 0)
		{
		throw InvanderException("Invalid frameCount");
		}
	iFrames = frameCount;	
	if(surface() != NULL)
		iFrameHeight = (surface()->h / iFrames); 
	setFrame(0);
	}	
	
void AnimationImage::play()
	{
	if(++iFrameRequest > iFrameRate)
		{
		iFrameRequest = 0;
		next();
		}
	}
	
void AnimationImage::next()
	{
	++iCurrentFrame;
	if(iCurrentFrame >= iFrames)
		iCurrentFrame = 0;
	}	
	
void AnimationImage::setFrame(int frame)
	{
	if(frame < 0 && frame >= iFrames)
		{
		throw InvanderException("Invalid frame");
		}
	iCurrentFrame = frame;
	}
	
void AnimationImage::loadBmp(const string& aBmp, bool trySmallImages)
	{
	Image::loadBmp(aBmp, trySmallImages);
	iFrameHeight = (surface()->h / iFrames); 
	setFrame(0);
	}
	
Size AnimationImage::imageSize() const
	{
	const Size s = {surface()->w, iFrameHeight};
	return s;
	}	
	
