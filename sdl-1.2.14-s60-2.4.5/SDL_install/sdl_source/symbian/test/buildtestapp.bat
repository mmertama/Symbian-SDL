@echo off

set winscw=
set armv5=
set gcce=

if "%1"=="all" goto setall
if "%1"=="" goto setall
if "%1"=="winscw" goto winscw
if "%1"=="armv5" goto armv5
if "%1"=="gcce" goto gcce
:setall
set winscw=ok
set armv5=ok
set gcce=ok
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

if "%winscw%"=="ok" echo build winscw test
if "%armv5%"=="ok" echo build armv5 test
if "%gcce%"=="ok" echo build gcce test


if "%gcce%"=="ok" call subst_arm_gcce.bat %2\sis\%2 %2

pushd %2


if "%winscw%"=="ok" call sbs -b bld.inf -c winscw_udeb
if "%armv5%"=="ok" call sbs -b bld.inf -c armv5_urel
if "%gcce%"=="ok" call sbs -b bld.inf -c gcce_urel

pushd sis

if not "%armv5%"=="ok" goto skip_gla
rem call %2.bat
call makesis %2.pkg
call signsis -v %2.sis %2.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar
copy %2.sisx ..\..\..\install
:skip_gla
if not "%gcce%"=="ok" goto skip_glc
rem call %2_gcce.bat
call makesis %2_gcce.pkg
call signsis -v %2_gcce.sis %2_gcce.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar
copy %2_gcce.sisx ..\..\..\install
:skip_glc

popd
popd

