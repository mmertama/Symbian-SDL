rem call makekeys -cert -password yourpassword -expdays 3650 -len 2048 -dname "CN=Vendor OR=Vendor CO=FI" sdltest.key sdltest.cer
call makesis sos_test2.pkg
call signsis -v sos_test2.sis sos_test2.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar