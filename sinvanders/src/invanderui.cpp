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

#include "invanderui.h"
#include "invander.h"

	

void Background::create(SDL_Surface& screen)
    {
    SDL_FreeSurface(iBg);
    iBg = NULL;
    iBg = SDL_CreateRGBSurface(SDL_SWSURFACE, screen.w, screen.h, screen.format->BitsPerPixel, 0, 0, 0, 0);
    }

Background::Background() : iBg(NULL)
    {    
    }

Background::~Background()
    {
    SDL_FreeSurface(iBg);
    }


void Background::draw(SDL_Surface& screen)
    {
    SDL_Rect rect = {0, 0, screen.w, screen.h};
    SDL_BlitSurface(iBg, &rect, &screen, &rect);
    }



	
NumberOutput::NumberOutput(AnimationImage& image) : iNumbers(image)
	{
	}
	
	
void NumberOutput::setValue(int number)
	{
	iDigits.clear();
	if(number == 0)
		iDigits.push_back(0);
	while(number > 0)
		{
		const int digit = number % 10;
		iDigits.push_front(digit);
		number /= 10;
		}
	}	
	
NumberOutput::~NumberOutput()
	{
	iDigits.clear();
	}
		
void NumberOutput::draw(const Position& pos, SDL_Surface& screen) const
	{
	const Size size = iNumbers.imageSize();
	Position npos = {Mapper::toScreen(pos.x, screen.w), Mapper::toScreen(pos.y, screen.h)};
	for(list<int>::const_iterator digit = iDigits.begin(); digit != iDigits.end(); digit++)
		{
		iNumbers.setFrame(*digit);
		iNumbers.draw(npos, screen);
		npos.x += size.w;
		}
	}	

Size NumberOutput::imageSize() const
	{
	const Size size = iNumbers.imageSize();
	const Size viewSize = {size.w * iDigits.size(), size.h};
	return viewSize;
	}
	
Points::Points(AnimationImage& numberImage) : NumberOutput(numberImage)
{}

void Points::setValue(int value)
	{
	iPoints = value;
	NumberOutput::setValue(value);
	}
	
int Points::value() const
	{
	return iPoints;
	}


DefenceForceView::DefenceForceView(const Image& defenceImage) : iImage(defenceImage)
	{
	}
	
void DefenceForceView::draw(const Position& pos, SDL_Surface& screen) const
	{
	Position screenPos = {Mapper::toScreen(pos.x, screen.w), Mapper::toScreen(pos.y, screen.h)};
	const Size sz = iImage.imageSize();
	for(int i = 0; i < iCount; i++)
		{
		iImage.draw(screenPos, screen);
		screenPos.x += sz.w;
		if(screenPos.x + sz.w > screen.w)
			{
			screenPos.x = Mapper::toScreen(pos.x, screen.w);
			screenPos.y += sz.h;
			}
		}
	}
	
void DefenceForceView::inc()
	{
	++iCount;
	}
void DefenceForceView::dec()
	{
	--iCount;
	}
		
bool DefenceForceView::isEmpty() const
	{
	return iCount < 0;
	}
	
void DefenceForceView::setCount(int count)
	{
	iCount = count;
	}	
	

	
Capturer::Capturer() : iInput(NULL)	
	{
	}
	
void Capturer::shoot() 
	{
	if(iInput != NULL)
		{
		*iInputPtr = iInput;
		iInput = NULL;
		}
	}	
	
	
void Capturer::capture(InputInterface** input)
	{
	iInputPtr = input;
	iInput = *input;
	*input = this;
	}	
		
	
 void Capturer::mouseEvent(const Position& /*pos*/)
	{
	shoot();
	}
	

	
Welcome::Welcome(Graphics& g, AudioInterface& audio) : iWelcome(g), iAudio(audio)
	{
	iWelcome.setVisible(true);
	iAudio.playAudio(AudioInterface::EWelcomeAudio);
	}	
	
void Welcome::shoot() 
	{
	iWelcome.setVisible(false);
	iAudio.stopAudio(AudioInterface::EWelcomeAudio);
	Capturer::shoot();	
	}	
	

void Welcome::setLayout(const Size& sz)
	{
	iWelcome.setLayout(sz);
	const Rect r = iWelcome.rect();
	const Position p = {KView.w / 2 - r.w / 2, KView.h / 2 - r.h / 2};
	iWelcome.set(p);
	}	
	

	
GameOver::GameOver(Graphics& g, AudioInterface& audio) : iGameOver(g), iAudio(audio)
	{
	iGameOver.setVisible(false);
	}
	
void GameOver::action()
	{
	--iTime;
	}
	
void GameOver::shoot() 
	{
	if(iTime <= 0)
		{
		iGameOver.setVisible(false);
		iAudio.stopAudio(AudioInterface::EGameOverAudio);
		Capturer::shoot();	
		}
	}

void GameOver::view(int delay)
	{
	iTime = delay;
	iAudio.playAudio(AudioInterface::EGameOverAudio);
	iGameOver.setVisible(true);
	}
	
bool GameOver::isVisible() const
	{
	return iGameOver.isVisible();
	}
	
void GameOver::setLayout(const Size& sz)
	{
	iGameOver.setLayout(sz);
	const Rect r = iGameOver.rect();
	const Position p = {KView.w / 2 - r.w / 2, KView.h / 2 - r.h / 2};
	iGameOver.set(p);
	}
	
