
#include <SDL_opengl.h>
#include <stdio.h>
#include <SDL.h>


static const GLbyte vertices[3 * 3] =
	{
	-1,	1,	0,
	1,	-1, 0,
	1, 1, 	0
	};
	
static const GLubyte colors[3 * 4] = 
	{
	255,	0, 		0,		255,
	0,		255,	255,	255,
	0,		0,		255,	255
	};
	
int panic(char* str)
	{
	fprintf(stderr,"%s: %s\n",str, SDL_GetError());
	exit( 1 );
	return 0;
	}
	
void renderFrame()
	{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, -5.f);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	}

int main(int argc, char** argv)
	{
	int width;
	int height;
	SDL_Surface* screen;
	SDL_Rect** modes;
	int videoFlags = SDL_OPENGL | SDL_FULLSCREEN;
	int done = 0;
	SDL_Event event;
	
	argc;
	argv;
	
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	 	{
		panic("SDL Init");
		}

	modes = SDL_ListModes(NULL, videoFlags);
	
	if(modes == NULL)
		{
		panic("No available video modes");
		}

	width = 240;
	height = 320;
	
	if(modes != (SDL_Rect**) -1)
		{
		width = modes[0]->w;
		height = modes[0]->h;
		}
	
	SDL_GL_LoadLibrary(NULL);	
		
	screen = SDL_SetVideoMode(width, height, 16, videoFlags);	
	
	if(screen == NULL)
		{
		panic("No GLES surface available");
		}
			
	glDisable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.f, 0.f, 0.1f, 1.f);
	
	glVertexPointer(3, GL_BYTE, 0, vertices);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glFrustumf(-1.f, 1.f, -1.f, 1.f, 3.f, 1000.f);
	glMatrixMode(GL_MODELVIEW);

	while(!done)
		{
		if(SDL_PollEvent(&event))
			{
			switch(event.type)
				{
				case SDL_KEYDOWN:
				case SDL_QUIT:
				done = 1;
				break;
				}
			}
		else
			{
			GLenum gl_error;
			renderFrame();
			SDL_GL_SwapBuffers();
			/* Check for error conditions. */
			gl_error = glGetError();	
			if(gl_error != GL_NO_ERROR ) 
				{
				fprintf(stderr, "OpenGL error: %d\n", gl_error );
				SDL_FreeSurface(screen);
    			SDL_Quit();
				exit( 1 );
				}
			SDL_Delay(20);
			}
		}
		
	SDL_FreeSurface(screen);
	/* Shutdown all subsystems */
    SDL_Quit();
	return 0;
	}


