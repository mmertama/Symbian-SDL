@echo off

for /f "tokens=*"  %%O in ('perl tools\setvariables.pl "..\.." "^SDL-\d+\.\d+\.\d+$"') do SET %%O

if /i "%SYMBIAN_SDL_FILES_FAIL%"=="SDL_FILES_OK" goto nofail
echo Configure failed at %SYMBIAN_SDL_FILES_FAIL% :
echo %SYMBIAN_SDL_FILES_FAIL_REASON%
goto end

:nofail

set version=%SYMBIAN_SDL_BUILD%

if /i "%SYMBIAN_SDL_EPOCROOT%"=="%EPOCROOT%" goto epocrootok
echo EPOCROOT value "%EPOCROOT%" is not ok, it should be "%SYMBIAN_SDL_EPOCROOT%" Configuration set it temporarily
goto setvar
:start_ask
set /p conf=Temporary set and go (y/n)?
if /i "%conf%"=="y" goto setvar
if /i "%conf%"=="n" goto end
goto start_ask
:setvar
set EPOCROOT=%SYMBIAN_SDL_EPOCROOT%
:epocrootok 

set b_all=
set test=
set clib=SYMBIANC
set debug=
set symbian3=
set b_armv5=
set b_gcce=
set b_winscw=

if /i "%EPOCROOT%"=="%SYMBIAN_SDL_SYMBIAN3%" set symbian3=SYMBIAN3
if /i "%EPOCROOT%"=="%SYMBIAN_SDL_SYMBIAN3%" set clib=OPENC

if "%1"=="help" goto help

:loop
if "%1"=="" goto outloop
if "%1"=="all" set b_all=all
if "%1"=="armv5" set b_armv5=armv5
if "%1"=="gcce" set b_gcce=gcce
if "%1"=="winscw" set b_winscw=winscw
if "%1"=="notest" set test=notest
if "%1"=="openc" set clib=OPENC
if "%1"=="symbianc" set clib=SYMBIANC
if "%1"=="debug" set debug=debug
if "%1"=="symbian3" set symbian3=SYMBIAN3

shift
goto loop
:outloop

if "%b_all%"=="all" goto skipthisset
if "%b_armv5%"=="armv5" goto skipthisset
if "%b_winscw%"=="winscw" goto skipthisset
if "%b_gcce%"=="gcce" goto skipthisset
set b_all=all
:skipthisset

if not "%b_all%"=="all" goto skipallset

set b_armv5=armv5
set b_gcce=gcce
set b_winscw=winscw

if /i "%SYMBIAN_GCCE%"=="" set set b_gcce=
if /i "%SYMBIAN_RVCT%"=="" set set b_armv5=

:skipallset

perl tools\setsbs.pl %EPOCROOT%

make -f sdl_s60.mk EXPORT


del ..\..\epoc32\include\sdl_linkage.h
echo #define %clib% %clib% >> ..\..\epoc32\include\sdl_linkage.h

if "%symbian3%"=="SYMBIAN3" echo #define %symbian3% %symbian3% >> ..\..\epoc32\include\sdl_linkage.h


pushd test

if "%b_armv5%"=="" goto skip_arm
call make_libs.bat %SYMBIAN_SDL_NAME%-s60-%version% %b_armv5% %debug% %symbian3%

if /i "%test%"=="notest" goto skiptest_a
call make_tests.bat %b_armv5% %clib%
:skiptest_a
:skip_arm

if "%b_gcce%"=="" goto skip_gcce
call make_libs.bat %SYMBIAN_SDL_NAME%-s60-%version% %b_gcce% %debug% %symbian3%
if "%test%"=="notest" goto skiptest_g
call make_tests.bat %b_gcce% %clib%
:skiptest_g
:skip_gcce

if "%b_winscw%"=="" goto skip_winscw
call make_libs.bat %SYMBIAN_SDL_NAME%-%version% %b_winscw% %debug% %symbian3%
if "%test%"=="notest" goto skiptest_w
call make_tests.bat %b_winscw% %clib%
:skiptest_w
:skip_winscw

echo ok

popd

goto end

:help

echo
echo configure [all|wincw|armv5|gcce] [notest] [openc|symbianc] [debug] [symbian3]
echo

goto end

:end
