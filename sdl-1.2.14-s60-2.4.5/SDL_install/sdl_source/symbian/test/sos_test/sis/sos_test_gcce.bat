@echo off
call makesis sdltest.pkg
call signsis -v sdltest.sis sdltest.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar