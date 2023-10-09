rem call makekeys -cert -password yourpassword -len 2048 -expdays 3650 -dname "CN=Vendor OR=Vendor CO=FI" sdltest.key sdltest.cer
call makesis gles_test_gcce.pkg
call signsis -v gles_test_gcce.sis gles_test_gcce.sisx \certificates\selfsigned.cer \certificates\selfsigned.key foobar