@echo off
echo SDL for Symbian relese script Markus Mertama 2010


for /f "tokens=*"  %%O in ('perl ..\setvariables.pl "..\..\..\.." "^SDL-\d+\.\d+\.\d+$"') do SET %%O

if /i "%SYMBIAN_SDL_EPOCROOT%"=="%EPOCROOT%" goto epocrootok
set EPOCROOT=%SYMBIAN_SDL_EPOCROOT%
:epocrootok 


if "%1"=="" goto param
goto paramok
:param

if "%SYMBIAN_SDL_BUILD%" == "" set %SYMBIAN_SDL_BUILD%=2.3.19

echo sdl_release VERSION [nobuild], e.g. "sdl_release %SYMBIAN_SDL_BUILD%"

goto end

:paramok

echo version=%1 > %SYMBIAN_SDL_HOME%\symbian\version.txt

for /f "tokens=*"  %%O in ('perl ..\setvariables.pl "..\..\..\.." "^SDL-\d+\.\d+\.\d+$"') do SET %%O

for /f "usebackq tokens=*" %%i IN (`test_perl_r.bat`) do set ERROR=%%i


if "%ERROR%"=="errors"	goto noerror

echo missing modules :%ERROR%:
type t_perror

goto end

:noerror

@echo modules ok

echo Releasing %SYMBIAN_SDL_NAME%

perl setsdlver.pl %SYMBIAN_SDL_HOME%\symbian\install\sdllib.pkg %SYMBIAN_SDL_BUILD% 
perl setsdlver.pl %SYMBIAN_SDL_HOME%\symbian\install\sdlexe.pkg %SYMBIAN_SDL_BUILD%

if "%2"=="nobuild" goto nobuild

pushd  %SYMBIAN_SDL_HOME%\symbian

call configure.bat

pushd install

rem call sdllib.bat

del *-s60-*_armv5.sisx

ren sdllib.sisx %SYMBIAN_SDL_NAME%-s60-%2_armv5.sisx

popd
popd

:nobuild

if exist SDL_install\sdl_source.zip del SDL_install\sdl_source.zip
if exist %SYMBIAN_SDL_NAME%-s60-%SYMBIAN_SDL_BUILD%.zip del %SYMBIAN_SDL_NAME%-s60-%SYMBIAN_SDL_BUILD%.zip

call snapshot.bat %SYMBIAN_SDL_HOME% %SYMBIAN_FORTEMP%

call cleansnap.bat %SYMBIAN_FORTEMP%

if not "%2"=="nobuild" goto nosisdel
pushd  %SYMBIAN_FORTEMP%
del *.sisx /s
del *.sis /s
popd

:nosisdel

pushd %SYMBIAN_FORTEMP%

7za a -x!*-s60-*.zip -tzip sdl_source.zip -r * 

popd

if exist SDL_install\sdl_source.zip del SDL_install\sdl_source.zip
if not exist SDL_install mkdir SDL_install

copy %SYMBIAN_FORTEMP%\sdl_source.zip SDL_install\
copy ..\setvariables.pl SDL_install\

rd %SYMBIAN_FORTEMP% /s /q

if exist "%SYMBIAN_SDL_NAME%-s60-%SYMBIAN_SDL_BUILD%.zip" del "%SYMBIAN_SDL_NAME%-s60-%SYMBIAN_SDL_BUILD%.zip"

7za a -tzip "%SYMBIAN_SDL_NAME%-s60-%SYMBIAN_SDL_BUILD%.zip" -r SDL_install

echo release done

:end

if exist t_perror del t_perror
