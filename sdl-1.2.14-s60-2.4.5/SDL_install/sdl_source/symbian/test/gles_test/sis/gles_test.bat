rem call makekeys -cert -password yourpassword -len 2048 -expdays 3650 -dname "CN=Vendor OR=Vendor CO=FI" sdltest.key sdltest.cer
call makesis gles_test.pkg
call signsis -v gles_test.sis gles_test.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar