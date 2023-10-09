@echo off
echo errors> t_perror
perl -MFile::Copy::Recursive -e 1 2>> t_perror
type t_perror
