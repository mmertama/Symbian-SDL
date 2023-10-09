#include<sdl.h>

extern int Frames;
extern int Ticks;
extern int Done;
extern "C" 
{
#include<GLES/gl.h>	
}


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
		
	int width = 160;
	int height = 120;	
	if(modes != (SDL_Rect**) -1)
		{
		int mode_count = 0;
		while(modes[mode_count] != NULL)
			{
			printf("Mode %d: %d %d", mode_count, modes[mode_count]->w, modes[mode_count]->h);
			mode_count++;
			}
		width = modes[0]->w;
		height = modes[0]->h;
		}
				
	const Uint32 videoflags = SDL_SWSURFACE | SDL_ANYFORMAT ;	
	const int bpp = 32; // full color

	SDL_Surface* bmp = SDL_LoadBMP("ball.bmp");
	
	if(bmp == NULL)
		panic("image not found");
		
	SDL_Surface* screen = SDL_SetVideoMode(width, height, bpp, videoflags);	
	

	SDL_Rect sourceRect;
	sourceRect.x = 0;
	sourceRect.y = 0;
	sourceRect.w = bmp->w;
	sourceRect.h = bmp->h;
	
	int then = SDL_GetTicks();

	
	SDL_Rect targetRect;
	targetRect.x = (screen->w - bmp->w) / 2;
	targetRect.y = (screen->h - bmp->h) / 2;
		
	targetRect.w = bmp->w;
	targetRect.h = bmp->h;

	int upwards = 0;
	int leftwards = 0;
	
	int frames = 0;
	
	while(!Done)
		{
		SDL_Event event;
		while(SDL_PollEvent(&event))
			{
			switch(event.type)
				{
				case SDL_KEYDOWN:
				case SDL_QUIT:
				Done = 1;
				break;
				}
			}

		if(upwards)
			{
			targetRect.y--;
			if(targetRect.y < 0)
				{
				upwards = 0;
				targetRect.y = 0;
				}
			}		
		else
			{
			targetRect.y++;
			if(targetRect.y + targetRect.h > screen->h)
				{
				upwards = 1;
				targetRect.y = screen->h - targetRect.h;
				}
			}		
			
		if(leftwards)
			{
			targetRect.x--;
			if(targetRect.x < 0)
				{
				leftwards = 0;
				targetRect.x = 0;
				}
			}		
		else
			{
			targetRect.x++;
			if(targetRect.x + targetRect.w > screen->w)
				{
				leftwards = 1;
				targetRect.x = screen->w - targetRect.w;
				}
			}		
		
		SDL_BlitSurface(bmp, &sourceRect, screen, &targetRect);
		SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
		++frames;
		}
		
	int now = SDL_GetTicks();
	Ticks = now - then;
	Frames = frames;
	SDL_FreeSurface(bmp);
	SDL_FreeSurface(screen);
	SDL_Quit();
	return 0;
	}

