@echo off

rem ** Even if your development enviroment is not windows
rem ** doing this manually is an easy task
rem ** Markus Mertama

rem ** assumes that SDL and this Symbian SDL install are both in SDK 
rem ** root folder and find their absolute paths
for /f "tokens=*"  %%O in ('perl setvariables.pl ".." "^SDL-\d+\.\d+\.\d+$"') do SET %%O

rem ** validity of current installation
if  "%SYMBIAN_SDL_FILES_FAIL%"=="SDL_FILES_OK" goto files_ok
echo Error: "%SYMBIAN_SDL_FILES_FAIL%" is not ok:
echo "%SYMBIAN_SDL_FILES_FAIL_REASON%"
goto nobuild
:files_ok


rem ** clean up old version from installation
perl clean.pl "%SYMBIAN_SDL_HOME%" symbian 


rem ** and extracts new on it
unzip -o  sdl_source.zip -d "%SYMBIAN_SDL_HOME%"  


rem ** keep only one 'official' copy
copy setvariables.pl "%SYMBIAN_SDL_HOME%\symbian\tools"

:start
set /p conf=Configure SDL (y/n)?
if /i "%conf%"=="y" goto build
if /i "%conf%"=="n" goto nobuild
if "%conf%"=="" goto start

:build
pushd "%SYMBIAN_SDL_HOME%\symbian\"
call configure.bat
popd

:nobuild
rem ** ok, hopefully :-)

echo End of script

