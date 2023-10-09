rem @echo off

set target=_urel

set winscw=
set armv5=
set gcce=

set version=%1
shift

:loop
if "%1"=="" goto outloop
if "%1"=="armv5" set armv5=armv5
if "%1"=="gcce" set gcce=gcce
if "%1"=="winscw" set winscw=winscw
if "%1"=="debug" set target=_udeb
if "%1"=="openc" set clib=OPENC
if "%1"=="symbianc" set clib=SYMBIANC
if "%1"=="debug" set debug=debug
if "%1"=="symbian3" set symbian3=SYMBIAN3

shift
goto loop
:outloop


echo target is %target%

if "%winscw%"=="winscw" goto winscw
if "%armv5"=="armv5" goto armv5
if "%gcce%"=="gcce" goto gcce


goto start

:winscw
set winscw=ok
goto start

:armv5
set armv5=ok
goto start

:gcce
set gcce=ok
goto start

:start

if "%gcce%"=="ok" call subst_arm_gcce.bat ..\install\sdllib sdllib

if "%winscw%"=="ok" echo build winscw
if "%armv5%"=="ok" echo build armv5
if "%gcce%"=="ok" echo build gcce


pushd ..
rem call bldmake bldfiles
rem call abld build armv5 urel
rem call abld build gcce urel


if "%winscw%"=="ok" call sbs -b bld.inf -c winscw_udeb
if "%armv5%"=="ok" call sbs -b bld.inf -c armv5%target%
if "%gcce%"=="ok" call sbs -b bld.inf -c gcce%target%



pushd install
if "%symbian3%"=="" goto skip_patch
call sdlhwa.bat
if exist %version%_hwaplugin_armv5.sisx del %version%_hwaplugin_armv5.sisx
ren sdlhwa.sisx %version%_hwaplugin_armv5.sisx
:skip_patch

if not "%armv5%"=="ok" goto skip_arm
call sdllib.bat %debug%
if exist %version%_armv5.sisx del %version%_armv5.sisx
ren sdllib%debug%.sisx %version%_armv5.sisx

:skip_arm

if not "%gcce%"=="ok" goto skip_gcce 
call sdllib_gcce.bat %debug%
if exist %version%_gcce.sisx del %version%_gcce.sisx
ren sdllib_gcce%debug%.sisx %version%_gcce.sisx
:skip_gcce


popd
popd

:end
