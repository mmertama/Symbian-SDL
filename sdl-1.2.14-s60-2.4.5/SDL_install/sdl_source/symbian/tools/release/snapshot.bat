@echo off

md "%2"
if exist "%2" goto ok

echo Create %2 failed

goto end

:ok

md "%2\snapshot.tmp\"

xcopy "%1" "%2\snapshot.tmp\" /s

perl snapshot.pl "%2" "%1" "%2\snapshot.tmp" BWINS EABI symbian

rd "%2\snapshot.tmp" /s /q

:end
