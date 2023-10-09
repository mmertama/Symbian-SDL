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

#include<vector> 
#include "graphics.h" 

class AudioInterface;

class EnemyGrid : public TargetInterface
	{
public:
	EnemyGrid(const Size& sz, InvanderInterface& invanderInterface, AudioInterface& audio);
	~EnemyGrid();
	bool action();
	vector<Enemy*>::iterator begin();
	vector<Enemy*>::iterator end();
	void resurrect();
private:
	enum Limit {ELeft, ERight, ETop, EBottom};
private:
	Enemy* limits(Limit limit) const;
	bool hitTest(const Position& pos);
	void shoot();
	void applyChanges();
private:	
	const Size iSize;
	InvanderInterface& iInvander;
	AudioInterface& iAudio;
	vector<Enemy*> iGrid;
	Enemy* iLeftMost;
	Enemy* iRightMost;
	Enemy* iTopMost;
	Enemy* iBottomMost;
	int iMovedir;
	int iActionCount;
	int iVisibles;
	int iShootSpeed;
	int iShootCount;
	int iEnemySpeed;
	};
	
