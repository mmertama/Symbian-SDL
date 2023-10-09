call sbs -b bld.inf -c armv5
call sbs -b bld.inf -c winscw_udeb

pushd ..\install
call futilssis.bat
popd
