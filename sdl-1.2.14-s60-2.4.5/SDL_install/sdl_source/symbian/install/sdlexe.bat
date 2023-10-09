@echo off

set pkgfile=sdlexe

if "%2"=="gcce" set pkgfile=%pkgfile%_gcce

if "%1"=="debug" set pkgfile=%pkgfile%_debug

call makecert.bat

del %pkgfile%.sis
del %pkgfile%.sisx
call makesis %pkgfile%.pkg 

call signsis -v %pkgfile%.sis %pkgfile%.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar

