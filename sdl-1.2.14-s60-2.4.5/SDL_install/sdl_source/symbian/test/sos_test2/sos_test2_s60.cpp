#include<eikstart.h>
#include<sdlmain.h>
#include<sdlepocapi.h>

/*
This file demonstrates how to change CSDL when using sdlexe.lib - i.e. if default flags are not
ok  - you dont have to use CSDL API directly, you can write this file add add it in to your S60 SDL 
compilation. Then you dont statically link  sdlmain.lib or  sdlmaint.lib libraries 
*/

GLREF_C TInt E32Main()
    {
    return SDLEnv::SetMain(SDL_main, CSDL::EEnableFocusStop | CSDL::EImageResizeZoomOut 
            | CSDL::EAutoOrientation | CSDL::EAllowImageResizeKeepRatio /*| CSDL::EDrawModeGdi*/,
     NULL, SDLEnv::EParamQuery | SDLEnv::EEnableVirtualMouseMoveEvents);
    }

