


if not exist %EPOCROOT%/GL/glu.h goto skipgl
call buildtestapp gles_test
goto skipglskip
:skipgl
perl ..\tools\note.pl "WARN GL not installed! See %EPOCROOT%tools\sbs" "Note" 10
:skipglskip

call buildtestapp %1 sos_test
call buildtestapp %1 sos_test2
call buildtestapp %1 rain


call buildtest.bat %1 %2

goto exit

pushd gles_test

if "%winscw%"=="ok" call sbs -b bld.inf -c winscw_udeb
if "%armv5%"=="ok" call sbs -b bld.inf -c armv5_urel
if "%gcce%"=="ok" call sbs -b bld.inf -c gcce_urel

pushd sis

if not "%armv5%"=="ok" goto skip_gla
call gles_test.bat
copy gles_test.sisx ..\..\..\install
:skip_gla
if not "%gcce%"=="ok" goto skip_glc
call gles_test_gcce.bat
copy gles_test_gcce.sisx ..\..\..\install
:skip_glc

popd
popd





pushd sos_test
rem call bldmake bldfiles
rem call abld build armv5 urel
rem call abld build gcce urel
if "%winscw%"=="ok" call sbs -b bld.inf -c winscw_udeb
if "%armv5%"=="ok" call sbs -b bld.inf -c armv5_urel
if "%gcce%"=="ok" call sbs -b bld.inf -c gcce_urel
pushd sis

if not "%armv5%"=="ok" goto skip_t1a
call sdltest.bat
copy sdltest.sisx ..\..\..\install
:skip_t1a
if not "%gcce%"=="ok" goto skip_t1g
call sdltest_gcce.bat
copy sdltest_gcce.sisx ..\..\..\install
:skip_t1g
popd
popd
pushd sos_test2
rem call bldmake bldfiles
rem call abld build armv5 urel
rem call abld build gcce urel
if "%winscw%"=="ok" call sbs -b bld.inf -c winscw_udeb
if "%armv5%"=="ok" call sbs -b bld.inf -c armv5_urel
if "%gcce%"=="ok" call sbs -b bld.inf -c gcce_urel
pushd sis

if not "%armv5%"=="ok" goto skip_t2a
call sdltest2.bat
copy sdltest2.sisx ..\..\..\install
:skip_t2a
if not "%gcce%"=="ok" goto skip_t2g
call sdltest2_gcce.bat
copy sdltest2_gcce.sisx ..\..\..\install
:skip_t2g
popd
popd

:exit
