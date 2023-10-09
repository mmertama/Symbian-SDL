@echo off
perl ..\tools\subst_arm_gcce.pl %1.pkg  _gcce \\armv5\\ \gcce\
perl ..\tools\subst_arm_gcce.pl %1.bat  _gcce %2 %2_gcce
