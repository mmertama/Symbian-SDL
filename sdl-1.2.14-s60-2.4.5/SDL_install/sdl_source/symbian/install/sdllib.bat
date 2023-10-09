@echo off

set pkgfile=sdllib.pkg
set gcce=

if "%2"=="gcce" set gcce=_gcce

if "%1"=="debug" set pkgfile=sdllib%gcce%_debug.pkg

call makecert.bat

del sdllib%gcce%.sis
del sdllib%gcce%.sisx
call makesis %pkgfile%
if exist sdllib%gcce%_debug.sis ren sdllib%gcce%_debug.sis sdllib%gcce%.sis

call signsis -v sdllib%gcce%.sis sdllib%gcce%.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar

