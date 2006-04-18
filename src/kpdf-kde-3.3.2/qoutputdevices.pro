TEMPLATE = lib


#
# Build library (libpokus.a)
#
OBJDIR = ../xpdf/xpdf
libpokus.target     = libpokus.a
libpokus.commands   = $(AR) libpokus.a $$OBJDIR/SplashOutputDev.o
libpokus.depends    = $$OBJDIR/SplashOutputDev.o
QMAKE_EXTRA_UNIX_TARGETS += libpokus
POST_TARGETDEPS +=  libpokus.a

#debug information
QMAKE_CXXFLAGS += -g

#include headers from kernel and used by kernel
INCLUDEPATH += ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../xpdf/splash

#QOutputDevices source files
HEADERS += QOutputDevPixmap.h
SOURCES += QOutputDevPixmap.cpp
HEADERS += QOutputDev.h
SOURCES += QOutputDev.cpp

#
# Path to xpdf object files
#

_PATH = ../xpdf

#LIBS += -lxpdf -L$$_PATH/xpdf
LIBS += -lsplash -L$$_PATH/splash
LIBS += -lpokus -L.
