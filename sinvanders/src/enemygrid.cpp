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

#include "enemygrid.h"
#include "invander.h"


using namespace std;

#ifndef random
#define random() rand()
#endif


EnemyGrid::EnemyGrid(const Size& sz, InvanderInterface& invanderInterface, AudioInterface& audio) : iSize(sz), 
	iInvander(invanderInterface), iAudio(audio), iMovedir(1), iActionCount(0), iShootSpeed(KInitialShootSpeed), iShootCount(0)
	{
	const int count = iSize.w * iSize.h;
	for(int i = 0; i < count; i++)
		iGrid.push_back(NULL);
	}
	
vector<Enemy*>::iterator EnemyGrid::begin()
	{
	return iGrid.begin();
	}
	
vector<Enemy*>::iterator EnemyGrid::end()
	{
	return iGrid.end();
	}	
	
EnemyGrid::~EnemyGrid()
	{
	iGrid.clear();
	}
	
	
void EnemyGrid::applyChanges()
	{
	iLeftMost = limits(ELeft);
	iRightMost = limits(ERight);
	iTopMost = limits(ETop);
	iBottomMost = limits(EBottom);	
	}
	
Enemy* EnemyGrid::limits(EnemyGrid::Limit limitType) const
	{
	int limit = 0;
	int pos = 0;
	int jump = 0;
	int step = 0;
	int jumpStep = 0;
	int jumpLimit = 0;
	for(;;)
		{
		switch(limitType)
			{
			case ELeft:
				pos = jump;
				step = iSize.w;
				limit = iSize.w * iSize.h;
				jumpStep = 1;
				jumpLimit = iSize.w;
				break;
			case ERight: 
				pos = (iSize.w - 1) - jump;
				step = iSize. w;
				limit = iSize.w * iSize.h;
				jumpStep = 1;
				jumpLimit = iSize.w;
				break;
			case ETop:
				pos = jump;
				step = 1;
				limit = jump + iSize.w;
				jumpStep = iSize.w;
				jumpLimit = iSize.w * iSize.h;
				break;
			case EBottom:
				pos =  iSize.w * iSize.h - iSize.w - jump;
				step = 1;
				limit = iSize.w * iSize.h - jump;
				jumpStep = iSize.w;
				jumpLimit = iSize.w * iSize.h;
			}
		for(;;)
			{
			if(iGrid[pos]->isVisible())
				return iGrid[pos];
			pos += step;
			if(pos >= limit)
				{
				jump += jumpStep;
				if(jump >= jumpLimit)
					return NULL;
				break;
				}
			}
		}
	}	
	
	
void EnemyGrid::shoot()
	{
	const int count = iSize.w * iSize.h;
	int shooter = -1;
	for(int i = 0; i <= count - iVisibles && shooter < 0; i++ )
		{
		const int r = random() % iGrid.size();
		if(iGrid[r]->isVisible())
			shooter = r;
		}
	if(shooter > 0)
		{
		int next = shooter + iSize.w;
		while(next < count && !iGrid[next]->isVisible())
			{
			next += iSize.w;
			}
		if(next > count)
			iGrid[shooter]->shoot();
		}	
	}		
	
bool EnemyGrid::action()
	{	
	if(++iShootCount > iShootSpeed)
	    {
	    shoot();
	    iShootCount = 0;
	    }
	if(++iActionCount > iEnemySpeed)
		{
		iActionCount = 0;
		
		Enemy* enemy = iMovedir > 0 ? iRightMost : iLeftMost;
		
		enemy->move(iMovedir, 0);
		const bool turn = !enemy->isInView();
		enemy->move(-iMovedir, 0);
		int yPos = 0;
		if(turn)
			{
			iMovedir = -iMovedir;		
			yPos = 3;
			}
			
		iAudio.playAudio(AudioInterface::EEnemyMoveAudio);

		for(vector<Enemy*>::iterator iter = iGrid.begin(); iter != iGrid.end(); iter++)
			{
			(*iter)->move(iMovedir, yPos);
			}
			
		const Rect bottom = iBottomMost->rect();
		if(bottom.y + bottom.h > KShieldYPos)
				{
				iInvander.setHit(InvanderInterface::KInvaded);
				iAudio.stopAudio(AudioInterface::EEnemyMoveAudio);
				}
		}
		
	for(vector<Enemy*>::iterator iter = iGrid.begin(); iter != iGrid.end(); iter++)
		{
		(*iter)->action();
		}
	return true;
	}
	
	
bool EnemyGrid::hitTest(const Position& pos)	
	{
	if(iTopMost->rect().y > pos.y)
		return false;
	if(iBottomMost->rect().br().y < pos.y)
		return false;
	if(iLeftMost->rect().x > pos.x)
		return false;
	if(iRightMost->rect().br().x < pos.x)
		return false;
	

	for(vector<Enemy*>::iterator iter = iGrid.begin(); iter != iGrid.end(); iter++)
		{
		if((*iter)->isVisible() && (*iter)->rect().isIn(pos))
			{
			(*iter)->setVisible(false);
			--iVisibles;
			
			const int sz = iSize.w * iSize.h; 
			iEnemySpeed = (((sz / KEnemySpeed) + iVisibles) * KEnemySpeed) / sz; //a bit mystic, 6->1 when 55->1 
			if(iVisibles == 1)
				iEnemySpeed = 0; //secret weapon, earthlings will get invated!
			
			if(*iter == iTopMost || *iter == iBottomMost || *iter == iLeftMost || *iter == iRightMost)
				applyChanges();
			iInvander.setHit((*iter)->value());
			if(iTopMost == NULL)
				{
				iInvander.setHit(InvanderInterface::KCleared);
				iAudio.stopAudio(AudioInterface::EEnemyMoveAudio);
				}
			iAudio.playAudio(AudioInterface::EEnemyExplosionAudio);
			return true;
			}
		}
	return false;
	}
	
void EnemyGrid::resurrect()
	{
	
	--iShootSpeed;
	
	vector<Enemy*>::iterator items = begin();
	
	int height  = 0;
	
	iEnemySpeed = KEnemySpeed;	
	iVisibles = iSize.w * iSize.h;

	for(int j = 0; j < iSize.h; j++)
		{
		const Rect imRect =  (*items)->rect();
	
		const Size size = {imRect.w, imRect.h};
	
		height += size.h;
	
		int gapw = size.w;
		int gridWidth = 0xFFFFF;
		
		int margins = KView.w / 8;
	
		for(;;)
			{
			gridWidth = size.w * iSize.w + gapw * (iSize.w + 1);
			if(gridWidth < (KView.w - 2 * margins))
				break;
			--gapw;
			}
		
		int xpos = (KView.w - gridWidth + margins) / 2;
	
		gapw += size.w;
	
		for(int i = 0 ; i < iSize.w ; i++)
			{
			const Position p = {xpos, 0};
		
			(*items)->set(p);
			items++;
			xpos += gapw;
			}
		}
		
	items = begin();
		
	int gaph = (KView.h / 3 - height) / iSize.h;	
	
	int yPos = KView.h / 6;
		
	for(int j = 0; j < iSize.h; j++)
		{
		int height = (*items)->rect().h;
		for(int i = 0 ; i < iSize.w ; i++)
			{
			(*items)->move(0, yPos);
			(*items)->setVisible(true);
			items++;
			}
		yPos += height + gaph;
		}
	
	applyChanges();
	}	

