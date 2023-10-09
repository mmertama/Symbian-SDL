@echo off
if not exist test mkdir test
del /q 	test\*.*
perl ..\tools\common_files.pl "test\bld.inf" "test\sdl_test.pkg" %1
perl ..\tools\make_parse.pl ..\..\test\Makefile.in all "perl ..\tools\mmpmake.pl" -sdkroot %1 -uinc ..\..\..\include;..\..\..\test -srcdir ..\..\..\test -uid 0xE0001000 -pkgfile test\sdl_test.pkg -targetdir test -bldinffile test\bld.inf -sinc %2 -sinc ..\..\..\include -lib %3  -excl %4 -mathlib libm.lib



