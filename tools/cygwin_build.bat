@echo off
rem -- Update this to point to where cygwin is installed --
set CYGWIN_ROOT=e:\cygwin

set PATH=%CYGWIN_ROOT%\bin;%CYGWIN_ROOT%\usr\X11R6\bin;%CYGWIN_ROOT%\usr\lib\qt3\bin
set QTDIR=/lib/qt3
set QMAKESPEC=cygwin-g++
rem move one direcctory up
cd ..
bash tools/cygwinbuild
