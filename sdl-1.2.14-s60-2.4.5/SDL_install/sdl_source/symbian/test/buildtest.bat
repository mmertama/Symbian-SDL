@echo off

rem for /f "tokens=*"  %%O in ('perl ..\tools\setvariables.pl "..\..\.." "^SDL-\d+\.\d+\.\d+$"') do SET %%O

if "%2"=="" goto ERROR
if /i "%2"=="openc" goto OPENC
if /i "%2"=="symbianc" goto SYMBIANC
:ERROR
echo "buildtest OPENC|SYMBIANC"
goto END
:OPENC
call maketestsuites.bat ..\..\..\..\ \epoc32\include\stdapis libc.lib "testcdrom;testdyngl;testgl;testiconv;testjoystick;testlock"
goto NEXT
:SYMBIANC
call maketestsuites.bat ..\..\..\..\ \epoc32\include\libc estlib.lib "testcdrom;testdyngl;testgl;testiconv;testjoystick;testlock;torturethread;testhread;testsem"
goto NEXT
:NEXT


perl ..\tools\subst_arm_gcce.pl test\sdl_test.pkg  _gcce \\armv5\\ \gcce\ 

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

pushd test
rem call bldmake bldfiles
rem call abld build winscw udeb
rem call abld build armv5 urel
rem call abld build gcce urel
if "%winscw%"=="ok" call sbs -b bld.inf -c winscw_udeb
if "%armv5%"=="ok" call sbs -b bld.inf -c armv5_urel
if "%gcce%"=="ok" call sbs -b bld.inf -c gcce_urel
rem call makekeys -cert -password yourpassword -expdays 3650 -len 2048 -dname "CN=SDL OR=Simple Media Library CO=FI" sdl_test.key sdl_test.cer
if not "%armv5%"=="ok" goto skip_testas
call makesis sdl_test.pkg
call signsis -v sdl_test.sis sdl_test.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar
copy sdl_test.sisx ..\..\install
:skip_testas
if not "%gcce%"=="ok" goto skip_testgs
call makesis sdl_test_gcce.pkg
call signsis -v sdl_test_gcce.sis sdl_test_gcce.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar
copy sdl_test_gcce.sisx ..\..\install
:skip_testgs
popd
:END
