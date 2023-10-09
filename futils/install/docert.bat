pushd \
mkdir certificates
pushd certificates
call makekeys -cert -password foobar -len 2048 -dname "CN=SDL OR=Simple Media Library CO=FI" selfsigned.key selfsigned.cer
popd
popd
