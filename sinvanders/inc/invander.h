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
 
#include <SDL.h>
#include "base.h"

const Uint32 KTickRate = 18;
const Size KView = {256, 256};
const int KEnemySpeed = 6;
const int KInitialShootSpeed = 10;
const int KShieldYPos(KView.h - (KView.w / 4));

class AudioInterface
	{
public:
	enum AudioId
		{
		EWelcomeAudio = 1,
		ETankShootAudio,
		EShieldExplosionAudio,
		ETankExplosionAudio,
		EEnemyExplosionAudio,
		EUfoExplosionAudio,
		EEnemyMoveAudio,
		EUfoMoveAudio,
		EGameOverAudio
		};
	virtual void playAudio(AudioId id, bool loop = false) = 0;
	virtual void stopAudio(AudioId id) = 0; 
	};


