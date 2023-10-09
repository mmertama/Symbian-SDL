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
#include <iostream>
#include <algorithm>
#include "base.h"
#include "images.h"
#include "graphics.h"
#include "enemygrid.h"
#include "invander.h"
#include "invanderui.h"
#include "audio.h"



#include"files.h"

using namespace std;

const int KMaxPointsId = 9;


class Invander : public InvanderInterface, public ContainerInterface, public AudioInterface
    {
public:
    Invander(const string& info);
    bool loop();
    ~Invander();
private:
    static Uint32 tick(Uint32 aInterval, void* aParam);
    void doTick();
    void update();
    void initSDL();
    void initElements(const InfoReader& reader);
	void setNumbers(const InfoReader& reader);
	void loadAudio(const InfoReader& reader);
	void setVideoMode(int width, int height);
	void setLayout();
	int addEnemies(int offs, const Image& image, vector<Enemy*>::iterator& iter, int lines,  int elementsInLine);
	void gameOver();
	void draw() const;
private:	
	void setHit(int value);
private:	
	void playAudio(AudioId id, bool loop);
	void stopAudio(AudioId id); 
private:
	void addAnimation(AnimationInterface* anim);
	void addImage(Image* image);
private:
	InputInterface*				iInput;
	vector<AnimationInterface*>	iAnim; 
    SDL_Surface*				iScreen;
    vector<Graphics*>			iGraphics;
	vector<Image*>				iImages;
	EnemyGrid*					iEnemies;
    SDL_TimerID					iTimer;
    Background*					iBg;
	Ufo*						iUfo;
	Points*						iPoints;
	Points*						iMaxPoints;
	DefenceForceView*			iDefenceForces;
	GameOver*					iGameOver;
	Welcome*					iWelcome;
	int							iWavesCompleted;
	Audio*                      iAudio;
	SettingsFile				iSettings;
	bool                  	    iPaused;     
	};

 Invander::Invander(const string& info) : iScreen(NULL), iPoints(NULL), iMaxPoints(NULL), 
     iWavesCompleted(0),
     iSettings("sinva.dat"), iPaused(false)
    {
	initSDL();
	
	const bool trySmallImages = iScreen->w < 320 || iScreen->h < 320;	
	InfoReader reader(*this, info, trySmallImages);

	iAudio = new Audio();
	loadAudio(reader);

    initElements(reader);
	setNumbers(reader);
	setLayout();
	
		
	iEnemies->resurrect();
	for ( vector<Graphics*>::iterator iter = iGraphics.begin(); iter != iGraphics.end(); iter++ )
	    {
	    (*iter)->resurrect();
	    }
	    
	iTimer = SDL_AddTimer(KTickRate, tick, this);
    }

void Invander::playAudio(AudioId id, bool loop)
	{
	iAudio->play(id, loop);
	}
	
void Invander::stopAudio(AudioId id) 
	{
	iAudio->stop(id);
	}

void Invander::addAnimation(AnimationInterface* anim)
	{
	iAnim.push_back(anim);
	}
	
void Invander::addImage(Image* image)
	{
	iImages.push_back(image);
	}
	
	
void Invander::loadAudio(const InfoReader& reader)
	{
	iAudio->set(reader.file("welcomegame"), AudioInterface::EWelcomeAudio);
	iAudio->set(reader.file("tankshoot"), AudioInterface::ETankShootAudio);	
	iAudio->set(reader.file("shieldexplosion"), AudioInterface::EShieldExplosionAudio);
	iAudio->set(reader.file("tankexplosion"), AudioInterface::ETankExplosionAudio);
	iAudio->set(reader.file("enemyexplosion"), AudioInterface::EEnemyExplosionAudio);	
	iAudio->set(reader.file("ufoexplosion"), AudioInterface::EUfoExplosionAudio);
	iAudio->set(reader.file("enemymove"), AudioInterface::EEnemyMoveAudio);
	iAudio->set(reader.file("ufomove"), AudioInterface::EUfoMoveAudio);	
	iAudio->set(reader.file("gameisover"), AudioInterface::EGameOverAudio);
	}	


void Invander::setNumbers(const InfoReader& reader)
	{
	AnimationImage* numbers = new AnimationImage();
	numbers->setFrameRate(0);
	numbers->setFrameCount(10);
	numbers->loadBmp(reader.file("numbers"), false);
	addImage(numbers);
	
	delete iPoints;
	iPoints = NULL;
	
	iPoints = new Points(*numbers);
	iPoints->setValue(0);
	
	delete iMaxPoints;
	iMaxPoints = NULL;
	
	iMaxPoints = new Points(*numbers);
	int val = 0;
	iSettings.readValue(KMaxPointsId, val);
	iMaxPoints->setValue(val);
	}

void Invander::initElements(const InfoReader& reader)
    {
		
	Image* e1 = reader.loadImage("enemy1");
	Image* e2 = reader.loadImage("enemy2");
	Image* e3 = reader.loadImage("enemy3");
			
	const Size gridSz = {11, 5};
	iEnemies = new EnemyGrid(gridSz, *this, *this);
	
	vector<Enemy*>::iterator gridIterator = iEnemies->begin();
	
	for(int i = 0 ; i < gridSz.w; i++) 
		{
		Enemy* eg = new Enemy(*e1, 30);
		iGraphics.push_back(eg);
		*gridIterator = eg;
		gridIterator++;
		}
		
	for(int i = 0 ; i < gridSz.w * 2; i++) 
		{
		Enemy* eg = new Enemy(*e2, 20);
		iGraphics.push_back(eg);
		*gridIterator = eg;
		gridIterator++;
		}
		
	for(int i = 0 ; i < gridSz.w * 2; i++) 
		{
		Enemy* eg = new Enemy(*e3, 10);
		iGraphics.push_back(eg);
		*gridIterator = eg;
		gridIterator++;
		}

	
	Image* mt = reader.loadImage("tankmissile1");
	
	TankMissile* tankMissile = new TankMissile(*mt);
	tankMissile->addTarget(*iEnemies);
	iGraphics.push_back(tankMissile);
	
	Image* explosionImage = reader.loadImage("explosion1");
		    
	Explosion* explosion = new Explosion(*explosionImage, 10);
	iGraphics.push_back(explosion);
	
	Image* tankImage = reader.loadImage("tank1");
		
	Tank* tank = new Tank(*tankImage, *this, *this);
	tank->setMissile(tankMissile, explosion);
	iInput = tank;
	
	iDefenceForces = new DefenceForceView(*tankImage);
	iDefenceForces->setCount(DefenceForceView::EInitial);
	
	iGraphics.push_back(tank);
	
	Image* ufoImage = reader.loadImage("ufo1");
	
	iUfo = new Ufo(*ufoImage, *this, *this);
	iGraphics.push_back(iUfo);
	
	tankMissile->addTarget(*iUfo);
	
	Image* boom = reader.loadImage("boom1");
	
	vector<Shield*> shields;
	
	Image* backup = reader.loadImage("shield1");
	
	for(int i  = 0 ; i < 3; i++)
		{
		Image* s = reader.loadImage("shield1");
		
		Shield* g = new Shield(*s, i, *backup, *this);
		g->setBoom(*boom);
		iGraphics.push_back(g);
		shields.push_back(g);
		tankMissile->addTarget(*g);
		}
	
	iBg = new Background();
	
	Image* enemyI = reader.loadImage("enemymissile1");

	for(vector<Enemy*>::iterator iter = iEnemies->begin(); iter != iEnemies->end(); iter++)
		{
		EnemyMissile* em = new EnemyMissile(*enemyI);
		em->addTarget(*tank);
		
		for(vector<Shield*>::iterator shield = shields.begin(); shield != shields.end(); shield++) 
			em->addTarget(*(*shield));
		
		iGraphics.push_back(em);
		
		Explosion* explosion = new Explosion(*explosionImage, 10);
		iGraphics.push_back(explosion);
		
		(*iter)->setMissile(em, explosion);
		}
	
	shields.clear();	
	
	Image* gameOverImage = reader.loadImage("gameover");
	
	Graphics* gameOverGraphics = new Graphics(*gameOverImage);
	iGraphics.push_back(gameOverGraphics);
	iGameOver = new GameOver(*gameOverGraphics, *this);
	
	Image* welcomeImage = reader.loadImage("welcome");
	
	Graphics* welcomeGraphics = new Graphics(*welcomeImage);
	iGraphics.push_back(welcomeGraphics);
	iWelcome = new Welcome(*welcomeGraphics, *this);
	iWelcome->capture(&iInput);
	}
	

void Invander::setLayout()
	{
	iBg->create(*iScreen);
	const Size sz = {iScreen->w, iScreen->h};
	for ( vector<Graphics*>::iterator iter = iGraphics.begin(); iter != iGraphics.end(); iter++ )
	    {
	    (*iter)->setLayout(sz);
	    }
	iGameOver->setLayout(sz);
	iWelcome->setLayout(sz);
	}

void Invander::initSDL()
    {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO ) < 0 )
        {
        string err = SDL_GetError();
        throw InvanderException(err);
        }
    SDL_Rect **modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
    if(modes == NULL)
        {
        throw InvanderException("No available video modes");
        }
    
    const SDL_VideoInfo* vinfo = SDL_GetVideoInfo();

    
	const Size sz = {
	        min<int>(640, vinfo->current_w),
	        min<int>(480, vinfo->current_h)};
        
    setVideoMode(sz.w, sz.h);
	}
	
void Invander::gameOver()
	{
	stopAudio(AudioInterface::EUfoMoveAudio);
	iGameOver->view(100);
	iGameOver->capture(&iInput);
	iSettings.writeValue(KMaxPointsId, iMaxPoints->value());
	}	
	
void Invander::setHit(int value)
	{
	int newpoints = 0;
	if(value == InvanderInterface::KDeath)
		{
		iDefenceForces->dec();
		if(iDefenceForces->isEmpty())
			{
			gameOver();
			}
		}
	else if(value == KInvaded)
		{
		iDefenceForces->setCount(DefenceForceView::EEmpty);
		gameOver();
		}
	else if(value == InvanderInterface::KCleared)
		{
		iDefenceForces->inc();
		iEnemies->resurrect();
		newpoints += 1000;
		if(iWavesCompleted <= KTickRate)
			iWavesCompleted++;
		}
	else
		{
		newpoints += value;
		}
	const int newval = iPoints->value() + newpoints;
	iPoints->setValue(newval);
	if(iMaxPoints->value() < iPoints->value())
		{
		iMaxPoints->setValue(iPoints->value());
		}
	}
	
void Invander::setVideoMode(int width, int height)
	{
	const bool paused = iPaused;
	iPaused = true;
	
  const int bpp = 0; // let device decide
  const Uint32 videoflags = SDL_SWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE;
  SDL_Surface* tobedeleted = iScreen;
  iScreen = NULL;
  SDL_FreeSurface(tobedeleted);
	iScreen = SDL_SetVideoMode(width, height, bpp, videoflags);
  if(iScreen == NULL)
  	{
   	throw InvanderException("No screen");
    }
 	iPaused = paused;
	}
    
 Uint32 Invander::tick(Uint32 aInterval, void* aParam)
	{
	Invander* ii =  static_cast<Invander*>(aParam);
	if(!ii->iPaused)
		ii->doTick();
	ii->draw();
	ii->update();
 	return aInterval - ii->iWavesCompleted;
}
 
void Invander::doTick()
    {
	for (vector<AnimationInterface*>::iterator iter = iAnim.begin(); iter != iAnim.end(); iter++ )
       {
       (*iter)->play();
       }
	   
	if(!iWelcome->isVisible())
		{
		if(!iDefenceForces->isEmpty())   
			{
			iInput->action();
			iEnemies->action();
			iUfo->action();
			}
		else
			{
			if(iGameOver->isVisible())
				{
				iGameOver->action();
				}
			else
				{
				iDefenceForces->setCount(DefenceForceView::EInitial);
				iWavesCompleted = 0;
				iPoints->setValue(0);
				iEnemies->resurrect();

				for ( vector<Graphics*>::iterator iter = iGraphics.begin(); iter != iGraphics.end(); iter++ )
					{
					(*iter)->resurrect();
					}
				}
			}
		}
		
	}
	
	
void Invander::draw()	const
	{
	if(iScreen != NULL)
		{
		iBg->draw(*iScreen);
	
 		const Position dpos = {KView.w - KView.w / 3, 5};
 		iDefenceForces->draw(dpos, *iScreen);	   
	
		for ( vector<Graphics*>::const_iterator iter = iGraphics.begin(); iter != iGraphics.end(); iter++ )
  			{
  			(*iter)->draw(*iScreen);
   			}
	   
   	const Position ppos = {KView.w / 6, 5};
   	iPoints->draw(ppos, *iScreen);	   
   
   	const Size psize = iPoints->imageSize();
    
   	const Position mpos = {KView.w / 6, 5 + Mapper::toView(psize.h, iScreen->h)};
   	iMaxPoints->draw(mpos, *iScreen);
   	}
  }
   
void Invander::update()
    {
    if(iScreen != NULL)
        SDL_UpdateRect(iScreen, 0, 0, iScreen->w, iScreen->h);
    }

Invander::~Invander()
	{
 	iPaused = true;
 	SDL_RemoveTimer(iTimer);
 	SDL_Delay(2 * KTickRate); //hmm at least osx SDL missing mutex there, so we just wait until timer thread is completed
  	SDL_FreeSurface(iScreen);
	delete iEnemies;
	delete iBg;
	
	iAnim.clear();
	
	for (vector<Graphics*>::iterator iter = iGraphics.begin(); iter != iGraphics.end(); iter++ )
       {
       delete (*iter);
       }


    iGraphics.clear();
	
	for (vector<Image*>::iterator iter = iImages.begin(); iter != iImages.end(); iter++ )
       {
       delete (*iter);
       }

    iImages.clear();

	delete iPoints;
	delete iMaxPoints;
	delete iDefenceForces;
	delete iGameOver;
	delete iWelcome;
	delete iAudio;
	
	SDL_Quit();
    }
	

bool Invander::loop()
    {
	SDL_Event event;
	bool running = true;
	while(running && SDL_WaitEvent(&event))
		{
		switch(event.type)
			{
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
					{
					case '9':
					case 'q':
					case 'Q':
						iPaused = false;
						running = false;
						break;
					case SDLK_LEFT:
						iInput->move(-1);
						break;
					case SDLK_RIGHT:
						iInput->move(1);
						break;
					case ' ':
					case '5':
						iInput->shoot();
						break;
					case '3':
					case 'p':
					case 'P':
                        iPaused = !iPaused;
                        break;
					case '0':
					case 'O':
					case 'o':
						iPaused = true;
						return true;
					}
			break;
			case SDL_KEYUP:
				switch(event.key.keysym.sym)
					{
					case SDLK_RIGHT:
					case SDLK_LEFT:
						iInput->cancel();
					break;
					}
				break;
			case SDL_QUIT:
				iPaused = false;
				running = false;
				break;
			case SDL_VIDEORESIZE:
				setVideoMode(event.resize.w, event.resize.h);
				setLayout();
				break;
			case SDL_MOUSEBUTTONDOWN:
				{
				const Position pos = {Mapper::toView(event.button.x, iScreen->w), Mapper::toView(event.button.y, iScreen->h)};
				iInput->mouseEvent(pos);
				}
				break;
			}
		}
	return false;
    }
	

int main(int argc, char** argv)
    {
    Invander* invander = NULL;
	InfoFile* file = NULL;
    try
        {
		file = new InfoFile();
		if(argc > 1)
			file->set(argv[1]);
		for(;;)
			{
			invander = new Invander(file->current());
			if(!invander->loop())
				break;
			delete invander;
			invander = NULL;
			file->next();
			}
        }
    catch(InvanderException e)
        {
        cerr << "SInvander:" << e.what() << endl;
		remove("infos");
        }
    catch (AudioException e)
        {
        cerr << "SInvander:" << e.what() << endl;
        }
    catch(exception e)
        {
        cerr << "Unhandled exception"<< e.what() << endl;
        } 
	delete file;
    delete invander;
    return 0;
    }





