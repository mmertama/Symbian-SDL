rem call makekeys -cert -password yourpassword -expdays 3650 -len 2048 -dname "CN=Vendor OR=Vendor CO=FI" sdltest.key sdltest.cer
call makesis sdltest2_gcce.pkg
call signsis -v sdltest2_gcce.sis sdltest2_gcce.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar