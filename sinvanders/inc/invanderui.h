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

#ifndef INVANDERUI_H
#define INVANDERUI_H 

#include <list>
#include "graphics.h"


class AudioInterface;

class Background
    {
public:
    Background();
    ~Background();
    void create(SDL_Surface& screen);
    void draw(SDL_Surface& screen);
private:
    SDL_Surface* iBg;
    };


class NumberOutput
	{
public:
	NumberOutput(AnimationImage& numberImage);
	virtual ~NumberOutput();
	virtual void setValue(int number);
	void draw(const Position& pos, SDL_Surface& screen) const;
	Size imageSize() const;
private:
	AnimationImage& iNumbers;
	list<int> iDigits;
	};
	
class DefenceForceView
	{
public:
    enum {EEmpty = -1, EInitial = 3};
public:
	DefenceForceView(const Image& defenceImage);
	void draw(const Position& pos, SDL_Surface& aScreen) const;
	void inc();
	void dec();
	bool isEmpty() const;
	void setCount(int count);
private:
	int iCount;
	const Image& iImage;
	};	
	
class Capturer : public InputInterface	
	{
public:
	Capturer();
	void capture(InputInterface** input);
protected:
	void shoot();
	void move(int /*x*/) {}
	void cancel() {}
	void action(){}
	void mouseEvent(const Position& pos);
private:
	InputInterface** iInputPtr;
	InputInterface* iInput;
	};
		
class Welcome : public Capturer	
	{
public:
	Welcome(Graphics& g, AudioInterface& audio);
	void setLayout(const Size& sz);
	inline bool isVisible() const;
private:
	void shoot();
private:
	Graphics& iWelcome;
	AudioInterface& iAudio;
	};
	
inline bool Welcome::isVisible() const
	{
	return iWelcome.isVisible();
	}
		
	
class GameOver : public Capturer
	{
public:
	GameOver(Graphics& g, AudioInterface& audio);
	void action();
	void view(int delay);
	bool isVisible() const;
	void setLayout(const Size& sz);
private:
	virtual void shoot();
private:
	Graphics& iGameOver;
	AudioInterface& iAudio;
	int iTime;
	};					
	
	
class Points : public NumberOutput
	{
public:
	Points(AnimationImage& numberImage);
	void setValue(int value);
	int value() const;
private:
	int iPoints;
	};	
	

	
#endif	