#include<sdl.h>

void drawPix(SDL_Surface* surface, int x, int y);

void panic(char* aWhen)
	{
	fprintf(stderr, "SDL error: %s: %s\n", aWhen, SDL_GetError());
	SDL_Delay(1000);
	SDL_Quit();
	exit(-1);
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
	
	const Uint32 videoflags = SDL_SWSURFACE | SDL_ANYFORMAT ;	
	const int bpp = 32; // full color
		
	SDL_Surface* screen = SDL_SetVideoMode(width, height, bpp, videoflags);
	
	if(!screen)
		{
		panic("Screen Init failed");
		}
			
	int done = 0;
	
	const Uint32 color = SDL_MapRGB (screen->format, 0, 0xFF, 0);
	
	SDL_Rect r = {0, 0, width, height};
	
	SDL_FillRect(screen, &r, color);
	
	SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
	
	while(!done)
		{
		SDL_Event event;
		SDL_WaitEvent(&event);
		switch(event.type)
				{
				case SDL_KEYDOWN:
				    if(event.key.keysym.sym != '0')
				        break;
				case SDL_QUIT:
					done = 1;
					break;
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONDOWN:
					drawPix(screen, event.button.x, event.button.y);
					break;
				}
		if(!SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_ALLEVENTS))
			{
			SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
			}
		}
	SDL_FreeSurface(screen);
	SDL_Quit();
	return 0;
	}
	
/*
* Return the pixel value at (x, y)
* NOTE: The surface must be locked before calling this!
*/
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      return *p;

    case 2:
      return *(Uint16 *)p;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
      return *(Uint32 *)p;

    default:
      return 0; /* shouldn't happen, but avoids warnings */
  }
}
	
/*
* Set the pixel at (x, y) to the given value
* NOTE: The surface must be locked before calling this!
*/
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(Uint16 *)p = pixel;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      } else {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
      break;

    case 4:
      *(Uint32 *)p = pixel;
      break;
  }
}
	
void drawPix(SDL_Surface* surface, int x, int y)
	{
	if ( SDL_LockSurface(surface) < 0 ) 
			{
			panic("Couldn't lock the display surface");
			}
		
	//const Uint32 pixel = getpixel(surface, x, y);
	//Uint8 r, g, b;
	//SDL_GetRGB (pixel, surface->format, &r, &g, &b); 
	//const Uint32 color = SDL_MapRGB (surface->format, ~r, ~g, ~b); 
	const Uint32 color = SDL_MapRGB (surface->format, 0xFF, 0, 0xFF); 
	putpixel(surface, x, y, color);
    SDL_UnlockSurface(surface);				
	}
		


