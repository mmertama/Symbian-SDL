I decided to reorganize Symbian SDL directory structure.  For clean
up amd install, a batch file (install.bat) should be run to upgrade on 
vanilla SDL 1.2.13

Install S60 SDL:

Download (from www.libsdl.org) and extract SDL 1.2.13 to root of 
development drive. (Its assumed that S60 SDK is installed into its own 
drive, likely substed)

Then extract this zip file which creates S60_SDL_install folder, goto
there and run install.bat.

After installation its recommended to run SDL-1.2.13/symbian/configure.bat
that completes installation and build libraries and test cases. 

Release Notes for 2.3.2:
 Minor code polish
 Bug fix for 16 bit colors
 New S60 semaphore and mutex implementation
 Thread fixes

mertama@mbnet.fi


 
 


