rem call makekeys -cert -password yourpassword -expdays 3650 -len 2048 -dname "CN=Vendor OR=Vendor CO=FI" sdltest.key sdltest.cer
call makesis rain.pkg
call signsis -v rain.sis rain.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar
