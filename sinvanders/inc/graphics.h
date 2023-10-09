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
#include <string>
#include <vector>
#include "images.h"
#include "base.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

class AudioInterface;


class Graphics
    {
public:
	inline Rect rect() const;
	inline bool isVisible() const;
    virtual ~Graphics();
    virtual void draw(SDL_Surface& aScreen) const ;
	virtual void resurrect();
	virtual void setLayout(const Size& sz);
	void set(const Position& pos);
	void move(int amountX, int amountY);
	void setVisible(bool visible);
	bool isInView() const;
	bool isPartiallyInView() const;
	Graphics(const Image& image); 
protected:
	void drawOn(SurfaceManagerInterface&, const Position& pos, int info) const;
	void mapSizes();
	void mapPos();
private:
    const Image&	iImage;
	bool			iVisibilityStatus; 
	Position		iViewPos;
	Size			iViewSize;
	Position		iScreenPos;
	Size			iScreenSize;
	};

inline Rect Graphics::rect() const
	{
	const Rect r = {iViewPos.x, iViewPos.y, iViewSize.w, iViewSize.h};
	return r;
	}
	
inline	bool Graphics::isVisible() const
	{
	return iVisibilityStatus;
	}
	

class TargetInterface
	{
public:
	virtual bool hitTest(const Position& pos) = 0;
	};

class Missile : public Graphics
	{
public:
	Missile(const Image& image);
	void addTarget(TargetInterface& target);
	void action();
	virtual Position warHeadPos() const = 0;
	virtual void move() = 0;
	void activate(const Position& pos);
	~Missile();
	void resurrect();
	bool isExploded() const;
private:
    void check();
private:
	bool iExploded;
	vector<TargetInterface*> iTargets;
	};
	

class TankMissile : public Missile
    {
public:
    TankMissile(const Image& aImage);
	void move();
protected:
	Position warHeadPos() const;
	};

class Explosion : public Graphics
    {
public:
    Explosion(const Image& image, int frames);
    void setActive();
    void action();
private:
    void resurrect();
private:    
    const int iFrameCount;
    int iFrame;
    };


class InputInterface 
	{
public:
	virtual void shoot() = 0;
	virtual void move(int x) = 0;
	virtual void cancel() = 0;
	virtual void action() = 0;
	enum Type {EUp, EDown};
	virtual void mouseEvent(const Position& pos) = 0;
	};
	

class MissileOwner : public Graphics
	{
public:
	MissileOwner(const Image& image);
	void setMissile(Missile* missile, Explosion* explosion);
	bool shoot();
	void action();
protected:
	virtual Position shootPos() const = 0;
protected:
	Missile* iMissile;
	Explosion* iExplosion;
	};


class InvanderInterface
	{
public:
	static const int KDeath = -1;
	static const int KCleared = -2;
	static const int KInvaded = -3;
	virtual void setHit(int value) = 0;
	};

class Tank : public MissileOwner, public InputInterface,  public TargetInterface
    {
public:
    Tank(const Image& aImage, InvanderInterface& invanderInterface, AudioInterface& audio);
private:
	void action();
	void move(int x);
	void cancel();
	void shoot();
	void mouseEvent(const Position& pos);
	Position shootPos() const;
	void resurrect();
	void setLayout(const Size& sz);
private:
	bool hitTest(const Position& pos);
private:
	InvanderInterface& iInvanders;
	AudioInterface& iAudio;
	int iMove;
	int iDeathCount;
	int iMouseMoves;
    };
		


class Enemy : public MissileOwner
    {
public:
    Enemy(const Image& image, int enemyValue);
	Position shootPos() const;	
	int value() const;
private:
	const int iValue;
	};
	

	
class Shield : public Graphics, public TargetInterface, public SurfaceManagerInterface
    {
public:
    Shield(const Image& aImage, int pos, const Image& backup, AudioInterface& audio);
	void setBoom(const Image& boomImage);
private:
	enum {KBoom, KAll};
	bool hitTest(const Position& pos);
	void drawOn(SDL_Surface& image, const Position& ppos, int info);
	void drawBoom(SDL_Surface& image, const Position& ppos);
	void drawAll(SDL_Surface& image, const Position& ppos);
	void resurrect();
private:
	const int iPos;
	const Image& iBackup;
	const Image* iBoom;
	AudioInterface& iAudio;
	bool isHit;
    };



class Ufo : public Graphics, public TargetInterface
    {
public:
    Ufo(const Image& aImage, InvanderInterface& invanderInterface, AudioInterface& audio);
	void action();
private:
	bool hitTest(const Position& pos);
private:
	void resurrect();
	void gone();
private:
	InvanderInterface& iInvander;
	int iLastVisible;
	int iStep;
	AudioInterface& iAudio;
	int iActionCount;
	int iDropped;
    };

	
class EnemyMissile : public Missile
    {
public:
    EnemyMissile(const Image& aImage);
	void move();
private:
	Position warHeadPos() const;
    };

#endif
