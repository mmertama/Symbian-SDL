#include	<sdl.h>
#include	"rain.h"

Uint32 getPixel1(const SDL_Surface *surface, int x, int y)
    {
    const Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * 1;
    return *p;
    }

Uint32 getPixel2(const SDL_Surface *surface, int x, int y)
    {
    const Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * 2;
    return *(Uint16 *)p;
    }

Uint32 getPixel3BE(const SDL_Surface *surface, int x, int y)
    {
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * 3;
    return p[0] << 16 | p[1] << 8 | p[2];  
    }

Uint32 getPixel3LE(const SDL_Surface *surface, int x, int y)
    {
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * 3;
    return p[0] | p[1] << 8 | p[2] << 16;
    }

Uint32 getPixel4(const SDL_Surface *surface, int x, int y)
    {
    const Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    return *(const Uint32 *)p;
    }


void panic(char* aWhen)
	{
	fprintf(stderr, "SDL error: %s: %s\n", aWhen, SDL_GetError());
	SDL_Delay(1000);
	SDL_Quit();
	exit(-1);
	}

class SurfaceCopier : public MCopier 
    {
public:
    SurfaceCopier(const SDL_Surface& aSurf);
private:
    TPix GetPixel(TInt, TInt) const;
    TSize Size()const;
private:
    const SDL_Surface& iSurf;
    Uint32 (*getPixel) (const SDL_Surface*, int, int);  
    };

SurfaceCopier::SurfaceCopier(const SDL_Surface& aSurf) : iSurf(aSurf)
    {
    int bpp = iSurf.format->BytesPerPixel;
    switch(bpp)
        {
        case 1: getPixel = getPixel1; break;
        case 2: getPixel = getPixel2; break;
        case 3: getPixel = SDL_BYTEORDER == SDL_BIG_ENDIAN ? getPixel3BE : getPixel3LE; break;
        case 4: getPixel = getPixel4; break;
        }
    }

TPix SurfaceCopier::GetPixel(TInt aX, TInt aY) const
    {
    return  getPixel(&iSurf, aX, aY); 
    }

TSize SurfaceCopier::Size()const
    {
    return TSize(iSurf.w, iSurf.h);
    }

class Button
    {
public:
    Button(SDL_Surface& numbers);
    inline int width() const {return iSurf.w; }
    inline int height() const {return iSurf.h >> 1;}
    void draw(SDL_Surface& surface) const;
    bool isIn(int x, int y) const;
    void press(bool);
    bool isPressed() const;
    void setPos(int x, int y);
private:
    const SDL_Surface& iSurf;
    bool iPressed;
    int iX;
    int iY;
    };
    
Button::Button(SDL_Surface& surf) : iSurf(surf), iPressed(false), iX(0), iY(0)
    {}

void Button::setPos(int x, int y)
    {
    iX = x;
    iY = y;
    }

void Button::draw(SDL_Surface& surface) const
    {
    SDL_Rect src = {0, iPressed ? height() : 0, width(), height()};
    SDL_Rect target = {iX, iY, width(), height()};
    SDL_BlitSurface(const_cast<SDL_Surface*>(&iSurf), &src, &surface, &target);
    }

bool Button::isIn(int x, int y) const
    {
    return x > iX && x < iX + width() && y > iY && y < iY + height();
    }

void Button::press(bool status)    
    {
    iPressed = status;
    }

bool Button::isPressed() const
    {
    return iPressed;
    }
    

class Numbers
    {
public:
    Numbers(SDL_Surface& numbers);
    void set(int number);
    void draw(SDL_Surface& surface, int x, int y) const;
    inline int width() const {return iNumbers.w * iDigits;}
    inline int height() const {return iSz;}
private:
    const SDL_Surface& iNumbers;
    int iNumber;
    int iDigits;
    const int iSz;
    };

Numbers::Numbers(SDL_Surface& numbers) : iNumbers(numbers), iNumber(0), iDigits(1), iSz(numbers.h / 10)
    {}

void Numbers::set(int number)
    {
    iNumber = number;
    iDigits = 0;
    do
        {
        number /= 10;
        ++iDigits;
        }while(number > 0);
    }

void Numbers::draw(SDL_Surface& surface, int x, int y) const
    {
    int nro = iNumber;
    int posx = x + iNumbers.w * iDigits;
    do  
        {
        const int digit = nro % 10;
        nro /= 10;
        SDL_Rect src = {0, digit * iSz, iNumbers.w, iSz};
        SDL_Rect target = {posx, y, iNumbers.w, iSz};
        SDL_BlitSurface(const_cast<SDL_Surface*>(&iNumbers), &src, &surface, &target);
        posx -= iNumbers.w;
        }while(nro > 0);
    }

void dropLot(CRain& rain, int w, int h)
    {
    for(int i = 0; i < w ; i += 20)
        for(int j = 0; j < h ; j += 20)
            {
            rain.Drop(TPoint(i, j));
            }
    }

bool buttonUp(Button& bt, int x, int y)
    {
    if(bt.isPressed())
       {
       if(bt.isIn(x, y))
          return true;
       bt.press(false);
       }
    return false;
    } 


bool buttonDown(Button& bt, int x, int y)
    {
    if(bt.isIn(x, y))
        {
        bt.press(true);
        return true;
        }
    return false;
    }

void setVideoMode(SDL_Surface*& screen, int width, int height)
    {
    const Uint32 videoflags = /*SDL_SWSURFACE*/SDL_HWSURFACE | SDL_ANYFORMAT ;   
    
    const int bpp = 32; // full color
    
    SDL_Surface* old = screen;
    
    screen = NULL;
    
    SDL_FreeSurface(old);
    
    screen = SDL_SetVideoMode(width, height, bpp, videoflags);

    if(!screen)
        {
        panic("Screen Init failed");
        }

    const Uint32 color1 = SDL_MapRGB (screen->format, 0, 3, 76);
    const Uint32 color2 = SDL_MapRGB (screen->format, 135, 0, 135);

    const TInt div = 12;

    const int ww = ((width < height) ? width : height) / div;

    int oo = 0;

    int ii = 0;
    int jj = 0;

    for(jj = 0 ; jj <= height; jj+= ww)
        {
        ++oo;
        int o = oo;
        for(ii = 0; ii <= width; ii+= ww)
            {
            ++o;
            SDL_Rect r = {ii, jj, ww, ww};
            const Uint32 color = o & 1 ? color1 : color2; 
            SDL_FillRect(screen, &r, color);
            }
        }
/*
    if(jj < height)
        {
        const int gap = height - jj;
            for(int u = 0; u <= width - ww; u += ww)
                {
                ++o;
                SDL_Rect r = {u, jj, ww, gap};
                const Uint32 color = o & 1 ? color1 : color2; 
                SDL_FillRect(screen, &r, color);
                }
        }

    if(ii < width)
        {
        const int gap = width - ii;
        for(int u = 0; u <= height - ww; u += ww)
            {
            ++o;
            SDL_Rect r = {ii, u, gap, ww};
            const Uint32 color = o & 1 ? color1 : color2; 
            SDL_FillRect(screen, &r, color);
            }
        }*/
    SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
    }




void setRainSize(CRain& rain, const SDL_Surface& screen)
    {
    
    rain.SetSize(TSize(screen.w, screen.h));
    
    SurfaceCopier copy1(screen);
    rain.Copy(copy1);
    
    SDL_Surface* bg = SDL_LoadBMP("background.bmp");
        
        
    if(!bg)
        {
        panic("\"background.bmp\" load failed.");
        }
       
    if ( SDL_LockSurface(bg) < 0 ) 
        {
        panic("Couldn't lock the display surface");
        }
        
    
    SurfaceCopier copy2(*bg);
    rain.Copy(copy2);

    SDL_UnlockSurface(bg);
    SDL_FreeSurface(bg);
    }

SDL_Surface* loadBmp(SDL_Surface& screen, const char* bmpName)
    {
    SDL_Surface* bmpSurf = SDL_LoadBMP(bmpName);        
    if(!bmpSurf)
           {
           char name[256];
           sprintf(name, "\"%s\" load failed.", bmpName);
           panic(name);
           }
    SDL_Surface* convertedSurface = SDL_ConvertSurface(bmpSurf, screen.format, SDL_SWSURFACE);
    if(NULL == convertedSurface)
        {
        panic("invalid");
        return bmpSurf;
        }
    SDL_FreeSurface(bmpSurf);
    return convertedSurface;
    }

int main(int argc, char** argv)
	{
	if(SDL_Init(SDL_INIT_VIDEO) < 0 )
		{
		panic("init video");
		}
	
	SDL_Rect** modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
	if(modes == NULL)
		{
		panic("No available video modes");
		}

	
	int width = 300; //fixed
	int height = 180; //fixed
		
	
	if(argc == 3)
		{
	  width = atoi(argv[1]);
	  height = atoi(argv[2]);
	  }
	else if(modes != (SDL_Rect**) -1)
		{
		width = modes[0]->w;
		height = modes[0]->h;
		}
	
	SDL_Surface* screen = NULL;
	
	setVideoMode(screen, width, height);
	
	CRain* rain = new CRain();
	
	setRainSize(*rain, *screen);
	    
    SDL_Surface* numbers = loadBmp(*screen, "numbers.bmp");
    	
    Numbers fps(*numbers);
    
    Numbers fps1(*numbers);
    Numbers fps2(*numbers);
    
    Numbers fps3(*numbers);
    
    
    SDL_Surface* ext = loadBmp(*screen, "exit.bmp");
        
    Button exit(*ext);
     
    SDL_Surface* drp = loadBmp(*screen, "drop.bmp");
                
    Button drop(*drp);
  
    exit.setPos(screen->w - ext->w - 10, 10);
    drop.setPos(screen->w - ext->w - 10 - 10 - drp->w, 10);
      
	int last_now = SDL_GetTicks();

	bool skip_next = false;
	
	int frames = 0;
	int frametime = 0;
	
	int d1 = 0;
	int d2 = 0;
	int d3 = 0;
	
	bool done = false;
	
	while(!done)
		{
		SDL_Event event;
		if(SDL_PollEvent(&event))
			{
			switch(event.type)
				{
			    case SDL_VIDEORESIZE:
			        setVideoMode(screen, event.resize.w, event.resize.h);
			        setRainSize(*rain, *screen);
			        exit.setPos(screen->w - ext->w - 10, 10);
			        drop.setPos(screen->w - ext->w - 10 - 10 - drp->w, 10);
			         
			        break;
				case SDL_KEYDOWN:
				    if(event.key.keysym.sym == '1')
				        {
				        dropLot(*rain, screen->w, screen->h);
				        break;
				        }
				    if(event.key.keysym.sym != '0')
				        break;
				case SDL_QUIT:
					done = 1;
					break;
				case SDL_MOUSEBUTTONUP:
				    if(buttonUp(exit, event.button.x, event.button.y))
				        done = 1;
				    if(buttonUp(drop, event.button.x, event.button.y))
				        dropLot(*rain, screen->w, screen->h);
				    break;
				case SDL_MOUSEBUTTONDOWN:
				    if(buttonDown(exit, event.button.x, event.button.y))
				        break;
				    if(buttonDown(drop, event.button.x, event.button.y))
				        break;
				 //no break
				case SDL_MOUSEMOTION:
				        rain->Drop(TPoint(event.button.x, event.button.y));
					continue;
				}
			}
		 
	
		 
        if ( SDL_LockSurface(screen) < 0 ) 
            {
            panic("Couldn't lock the display surface");
            }
        
        TPix *p = (TPix*)screen->pixels;
        
        const int r1 = SDL_GetTicks();
        rain->Rain(p);
        
        d1 += SDL_GetTicks() - r1;
        
        SDL_UnlockSurface(screen);		
        
        const int now = SDL_GetTicks();
        const int delta = now - last_now;
        if(delta < 5)
           SDL_Delay(delta);
        
        last_now = now;
        
        ++frames;
        frametime += delta;
        
        
        if(frametime > 1000)
            {
            int fps_val = (((frames << 16) / frametime) * 1000) >> 16;
            fps.set(fps_val);
            
            int fps1_val = (((d1 << 16) / frametime) * 100) >> 16;
            int fps2_val = (((d2 << 16) / frametime) * 100) >> 16;
            
            int fps3_val = (((d3 << 16) / frametime) * 100) >> 16;
            
            fps1.set(fps1_val);
            fps2.set(fps2_val);
            
            fps3.set(fps3_val);
            
            frametime = 0;
            frames = 0;
            d1 = 0;
            d2 = 0;
            d3 = 0;
            }

        const int r3 = SDL_GetTicks();
        
        int ypos = 10;
        fps.draw(*screen, 10, ypos);
        ypos += fps.height() + 5; 
        fps1.draw(*screen, 10, ypos);
        ypos += fps.height() + 5; 
        fps2.draw(*screen, 10, ypos);
        
        ypos += fps.height() + 5; 
        fps3.draw(*screen, 10, ypos);
        
        exit.draw(*screen);
        drop.draw(*screen);
        
        d3 += SDL_GetTicks() - r3;
        
        const int r2 = SDL_GetTicks();
        if(skip_next)
            skip_next = false;
        else
            {
            SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
            if(delta > 50)
                skip_next = true;
            }
        d2 += SDL_GetTicks() - r2;
		}
		
	delete rain;
	

	SDL_FreeSurface(drp);
	SDL_FreeSurface(ext);
	SDL_FreeSurface(numbers);
	SDL_FreeSurface(screen);
	SDL_Quit();
	return 0;
	}
	


	
