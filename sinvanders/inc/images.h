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
 
#include<SDL.h> 
#include "base.h"

#ifndef IMAGES_H
#define IMAGES_H

class SurfaceManagerInterface
	{
public:
	virtual void drawOn(SDL_Surface& image, const Position& ppos, int info) = 0;
	};
	
class Image
	{
public:
	inline const SDL_Surface* surface() const;
	Image();
	virtual void draw(const Position& pos, SDL_Surface& aScreen) const;
	virtual void loadBmp(const std::string& aBmp, bool trySmallImages, bool rawmode = false);
	virtual Size imageSize() const;
	virtual ~Image();
	void drawOn(SurfaceManagerInterface& surface, const Position& pos, int info) const;
private:
	bool loadMask(const std::string& aMask);
	bool loadFaceBmp(const std::string& aBmp, bool rawmode);
private:
	SDL_Surface* iImage;
	};

class AnimationInterface	
	{
public:
	virtual void play() = 0;
	};
	
	
class AnimationImage : public Image, public AnimationInterface
	{
public:
	AnimationImage();
	~AnimationImage();
	void draw(const Position& pos, SDL_Surface& aScreen) const;
	void next();	
	void loadBmp(const std::string& aBmp, bool trySmallImages);
	Size imageSize() const; 
	void play();
public:
	void setFrame(int frame);
	void setFrameCount(int frames);
	void setFrameRate(int rate);
private:
	int iFrameRate;
	int iFrameRequest;
	int iFrames;
	int iFrameHeight;
	int iCurrentFrame;
	};
    
inline const SDL_Surface* Image::surface() const
    {
    return iImage;
    }   
 

#endif
