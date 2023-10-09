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
#include "invander.h"
#include "graphics.h"


using namespace std;

#ifndef random
#define random() rand()
#endif

Graphics::Graphics(const Image& image) : iImage(image), iVisibilityStatus(true)
    {
	iScreenSize.w = 0;
	iScreenSize.h = 0;
	}
	
	
void Graphics::resurrect()
	{
	}	
	
void Graphics::set(const Position& pos)
	{
	iViewPos.x = pos.x;
	iViewPos.y  = pos.y;
	mapPos();
	}
	
void Graphics::setVisible(bool visible)
	{
	iVisibilityStatus = visible;
	}	
	
bool Graphics::isInView() const
	{
	const Rect r = rect();
	const Position p = r.br();
	return (r.x >= 0 && r.y >= 0 && p.x <= KView.w && p.y <= KView.h);
	}
	
bool Graphics::isPartiallyInView() const
	{
	const Rect r = rect();
	const Position p = r.br();
	return (r.x + r.w >= 0 && r.y + r.h >= 0 && p.x - r.w <= KView.w && p.y - r.h <= KView.h);
	}	
		
	
void Graphics::setLayout(const Size& sz)
	{
	iScreenSize.w = sz.w;
	iScreenSize.h = sz.h;
	mapSizes();
	}

void Graphics::mapPos()
	{
	iScreenPos.x = Mapper::toScreen(iViewPos.x, iScreenSize.w);
	iScreenPos.y = Mapper::toScreen(iViewPos.y, iScreenSize.h);
	}

void Graphics::mapSizes()
	{
	mapPos();	
	const Size screenImage = iImage.imageSize();
	iViewSize.w = Mapper::toView(screenImage.w,  iScreenSize.w);
	iViewSize.h = Mapper::toView(screenImage.h,  iScreenSize.h);
	}
	
void Graphics::draw(SDL_Surface& aScreen) const
    {    
	if(iVisibilityStatus && iScreenSize.h > 0 && iScreenSize.w > 0)
		iImage.draw(iScreenPos, aScreen);
	}	


void Graphics::move(int amountX, int amountY)
	{
	const Position p = {iViewPos.x + amountX, iViewPos.y + amountY};
	set(p);
	}

Graphics::~Graphics()
    {
	}
	
void Graphics::drawOn(SurfaceManagerInterface& smi, const Position& pos, int info) const
	{
	if(iScreenSize.w > 0 && iScreenSize.h > 0)
		{
		const Position screenPos = 
			{
			Mapper::toScreen(pos.x, iScreenSize.w),
			Mapper::toScreen(pos.y, iScreenSize.h)
			};
		iImage.drawOn(smi, screenPos, info);
		}
	}
	
Missile::Missile(const Image& aImage) :  Graphics(aImage), iExploded(false)
	{
	setVisible(false);
	}

Missile::~Missile()
	{
	iTargets.clear();
	}
	
void Missile::addTarget(TargetInterface& target)
	{
	iTargets.push_back(&target);
	}


void Missile::activate(const Position& pos)	
	{
	setVisible(true);
	set(pos);
	iExploded = false;
	}
	
void Missile::resurrect()
	{
	setVisible(false);
	}

bool Missile::isExploded() const
	{
	return iExploded;
	}	
	
void Missile::check()
	{
	const Position  pos = warHeadPos();
	for(vector<TargetInterface*>::iterator iter = iTargets.begin(); iter != iTargets.end(); iter++)
		if((*iter)->hitTest(pos))
			{
			setVisible(false);
			iExploded = true;
			}
			
	if(pos.y < 0 || pos.y >= KView.h)
		{
		setVisible(false);
		}
	}

void Missile::action()
    {
    move();
    check();
    }   


TankMissile::TankMissile(const Image& aImage) : Missile(aImage)
    {
    }	
	
Position TankMissile::warHeadPos() const	
	{
	const Rect r = rect();
	const Position pos = {r.x + (r.w >> 1), r.y};
	return pos;
	}

void TankMissile::move()
	{
	Graphics::move(0, -1);
	}
	

Explosion::Explosion(const Image& aImage, int frames) : Graphics(aImage), iFrameCount(frames)
    {
    }

 void Explosion::setActive()
     {
     setVisible(true);
	 const Rect r = rect();
	 move(-(r.w >> 1), 0);
     iFrame = iFrameCount;
     }
 
void Explosion::action()
    {
    if(iFrameCount > 0)
        {
        --iFrame;
        if(iFrame == 0)
            {
            setVisible(false);
            }
        }
    }
 
 
void Explosion::resurrect()
    {
    iFrame = 0;
    setVisible(false);
    }

MissileOwner::MissileOwner(const Image& image) : Graphics(image), iMissile(NULL)
	{
	}	
	
void MissileOwner::setMissile(Missile* missile, Explosion* explosion)
	{
	iMissile = missile;
	iExplosion = explosion;
	}	
	
bool MissileOwner::shoot()
	{
	if(iMissile != NULL && !iMissile->isVisible())
		{
		const Position pos = shootPos();
		iMissile->activate(pos);
		return true;
		}
	return false;
	}	
	
void MissileOwner::action()
	{
	if(iMissile != NULL && iMissile->isVisible())
	    {
		iMissile->action();
	    if(iMissile->isExploded())
	        {
	        iExplosion->set(iMissile->warHeadPos());
	        iExplosion->setActive();
	        }
	    }
	if(iExplosion->isVisible())
	    iExplosion->action();
	}



Tank::Tank(const Image& aImage, InvanderInterface& invanderInterface, AudioInterface& audio) : MissileOwner(aImage), 
	iInvanders(invanderInterface), iAudio(audio), iMove(0), iDeathCount(0), iMouseMoves(0)
    {
    }
	
void Tank::move(int x)
	{
	iMove = x;
	}
	
void Tank::resurrect()
	{
	const Rect tankRect = rect();
	const Position tankPos = {(KView.w - tankRect.w) / 2, KView.h - tankRect.h};
	set(tankPos);
	cancel();
	}
	
Position Tank::shootPos() const
	{
	const Rect r = rect();
	const Position pos = {r.x + (r.w >> 1), r.y};
 	return pos;
	}
	
bool Tank::hitTest(const Position& pos)
	{
	const Rect r = rect();
	if(r.isIn(pos) && iDeathCount <= 0)
		{
		setVisible(false);
		iDeathCount = 50;
		iAudio.playAudio(AudioInterface::ETankExplosionAudio);
		iInvanders.setHit(InvanderInterface::KDeath);
		return true;
		}
	return false;
	}

void Tank::setLayout(const Size& sz)
    {
    Graphics::setLayout(sz);
    const Rect tankRect = rect();
    const Position tankPos = {tankRect.x, KView.h - tankRect.h};
    set(tankPos);
    }
	
void Tank::cancel()
	{
	iMove = 0;
	iMouseMoves = 0;
	}
	
void Tank::mouseEvent(const Position& pos)	
	{
	const Rect r = rect();
	if(r.isIn(pos))
		{
		shoot();
		}
	else if(pos.x < r.x)
		{
		iMouseMoves = (r.x + r.w / 2) - pos.x;
		iMove = -1;
		}
	else
		{
		iMouseMoves = pos.x - (r.x + r.w / 2);
		iMove = 1;
		}
	}
	
void Tank::shoot()
	{
	if(MissileOwner::shoot())
		iAudio.playAudio(AudioInterface::ETankShootAudio);
	}	

void Tank::action()
	{
	if(iDeathCount > 0)
		{
		--iDeathCount;
		if(iDeathCount < 10)
			setVisible(!static_cast<bool>(iDeathCount & 0x1));
		}
	MissileOwner::action();
	
	if(iMove != 0)
		{
		Graphics::move(iMove, 0);
		if(!isInView())
			{
			Graphics::move(-iMove, 0);
			cancel();
			}
		}
		
	if(iMouseMoves > 0)
		{
		--iMouseMoves;
		if(iMouseMoves == 0)
			{
			iMove = 0;
			}
		}	
	}


	
Enemy::Enemy(const Image& image, int enemyValue) : MissileOwner(image), iValue(enemyValue)
    {
    }	
	
int Enemy::value() const
	{
	return iValue;
	}	
	
Position Enemy::shootPos() const
	{
	const Rect r = rect();
	const Position pos = {r.x + (r.w >> 1), r.y + r.h};
	return pos;
	}
	


Shield::Shield(const Image& aImage, int pos, const Image& backup, AudioInterface& audio) :  
	Graphics(aImage), iPos(pos), iBackup(backup),  iAudio(audio), isHit(false)
    {
     }
	
void Shield::setBoom(const Image& boomImage)
	{
 	iBoom = &boomImage;
	if(iBoom->surface()->format->BytesPerPixel != 1)
		throw InvanderException("Boom format");
	}	
	
bool Shield::hitTest(const Position& pos)
	{
	const Rect r = rect();
	if(r.isIn(pos))
		{
		const Position inPos = {pos.x - r.x, pos.y - r.y};
		isHit = false;
		Graphics::drawOn(*this, inPos, KBoom);
		if(isHit)
			{
			iAudio.playAudio(AudioInterface::EShieldExplosionAudio);
			}
		return isHit;
		}
	return false;
	}
	

void Shield::drawOn(SDL_Surface& image, const Position& ppos, int info)
	{		
	if(info == KBoom)
		drawBoom(image, ppos);
	else
		drawAll(image, ppos);
	}
	
void Shield::drawAll(SDL_Surface& image, const Position& /*ppos*/)
	{
	SDL_Surface* surface = const_cast<SDL_Surface*>(iBackup.surface());
	SDL_Rect rect = {0, 0, surface->w, surface->h};
    SDL_FillRect(&image , &rect, 0xFFFF00FF); //alpha!
	SDL_BlitSurface(surface, NULL, &image, &rect);
	}	
	
void Shield::drawBoom(SDL_Surface& image, const Position& ppos) 	
	{
	isHit = false;		
	if(SDL_MUSTLOCK((&image)) && SDL_LockSurface(&image) < 0)
			throw InvanderException("Image format");

	Uint32* t = static_cast<Uint32*>(image.pixels);
	t += ppos.x + ppos.y * (image.pitch >> 2);
	Uint8 r, g, b, a;
	SDL_GetRGBA(*t, image.format, &r, &g, &b, &a);
	if(a > 0xF)
		{
		*t = SDL_MapRGBA(image.format, r, g, b, 0);
				
		SDL_Surface& mask = *const_cast<SDL_Surface*>(iBoom->surface());
		if(SDL_MUSTLOCK((&mask)) && SDL_LockSurface(&mask) < 0)
			throw InvanderException("Mask format");	
		
		const Position pos = {ppos.x - mask.w / 2, ppos.y - mask.h / 4};
		
		const int tx = max<int>(pos.x, 0);
		const int ty = max<int>(pos.y, 0);  
	
		const int mx = -min<int>(pos.x, 0);
		const int my = -min<int>(pos.y, 0); 
	
		const int w = min<int>(mask.w - mx, image.w - tx);
		const int h = min<int>(mask.h - my, image.h - ty);
	
		for(int j = 0; j < h; j++)
			{
			Uint32* t = static_cast<Uint32*>(image.pixels);
			t += (j + ty) * (image.pitch >> 2);
			t += tx;
			Uint8* m = static_cast<Uint8*>(mask.pixels);
			m += (j + my) * mask.pitch;
			m += mx;
			 
			for(int i = 0; i < w; i++)
				{
				Uint8 r, g, b, a;
				SDL_GetRGBA(*t, image.format, &r, &g, &b, &a);
				*t = SDL_MapRGBA(image.format, r, g, b, *m & a);
				++m;
				++t;
				}
			}
			
		if(SDL_MUSTLOCK((&mask)))
			SDL_UnlockSurface(&mask);
		isHit = true;
		}
	if(SDL_MUSTLOCK((&image)))
		SDL_UnlockSurface(&image);
	}

void Shield::resurrect()
	{
  const int sfrac = KView.h / 3;
	const Rect r = rect();
	int posx = (sfrac - r.w) / 2;
	posx += iPos * sfrac;
	const Position pos = {posx, KShieldYPos};
	set(pos);
	const Position p = {0, 0};
	Graphics::drawOn(*this, p, KAll);
	}

Ufo::Ufo(const Image& aImage, InvanderInterface& invanderInterface, AudioInterface& audio) : Graphics(aImage), 
    iInvander(invanderInterface), iAudio(audio), iActionCount(0), iDropped(0)
    {
	setVisible(false);
	iLastVisible = random() % 1000;
    }
	
bool Ufo::hitTest(const Position& pos)
	{
	const Rect r = rect();
	if(isVisible() && r.isIn(pos))
		{
		gone();
		++iDropped;
		iInvander.setHit(iDropped *  100);
		iAudio.playAudio(AudioInterface::EUfoExplosionAudio);
		return true;
		}
	return false;
	}
	
void Ufo::gone()
	{
	iLastVisible = random() % 5000;
	iAudio.stopAudio(AudioInterface::EUfoMoveAudio);
	setVisible(false);
	}	
	
void Ufo::action()
	{
	if(isVisible() && isPartiallyInView())
		{
		if(++iActionCount > KEnemySpeed / 2)
			{
			iActionCount = 0;
			move(iStep, 0);
			 }
		}
	else
		{
		--iLastVisible;
		if(isVisible())
			{
			gone();
			}
		if(iLastVisible <= 0)
			{
			Position pos = {0, 0};
			if(iLastVisible & 0x1)
				{
				iStep = 1;
				const Rect r = Rect();
				pos.x = -r.w + 1;
				}
			else
				{
				iStep = -1;
				pos.x = KView.w - 1;
				}
			set(pos);
			setVisible(true);
			iAudio.playAudio(AudioInterface::EUfoMoveAudio, true);
			}
		}
	}
	
	
void Ufo::resurrect()
	{
	iDropped = 0;
	gone();
	}
	
		
EnemyMissile::EnemyMissile(const Image& aImage) : Missile(aImage)
	{
	}
	
Position EnemyMissile::warHeadPos() const	
	{
	const Rect r = rect();
	const Position pos = {r.x + (r.w >> 1), r.y + r.h};
	return pos;
	}	
	
void EnemyMissile::move()
	{
	Graphics::move(0, 1);
	}	

