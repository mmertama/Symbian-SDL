@echo off

set pkgfile=sdllib_gcce.pkg
set gcce=

if "%2"=="gcce" set gcce=_gcce

if "%1"=="debug" set pkgfile=sdllib_gcce%gcce%_debug.pkg

call makecert.bat

del sdllib_gcce%gcce%.sis
del sdllib_gcce%gcce%.sisx
call makesis %pkgfile%
if exist sdllib_gcce%gcce%_debug.sis ren sdllib_gcce%gcce%_debug.sis sdllib_gcce%gcce%.sis

call signsis -v sdllib_gcce%gcce%.sis sdllib_gcce%gcce%.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar

