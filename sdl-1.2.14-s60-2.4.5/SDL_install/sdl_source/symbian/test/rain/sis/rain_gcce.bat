rem call makekeys -cert -password yourpassword -expdays 3650 -len 2048 -dname "CN=Vendor OR=Vendor CO=FI" sdltest.key sdltest.cer
call makesis rain_gcce.pkg
call signsis -v rain_gcce.sis rain_gcce.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar
