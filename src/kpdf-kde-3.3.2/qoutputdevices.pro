TEMPLATE = lib

CONFIG += staticlib

TARGET = qoutputdevices

include(../../config.pro)

#check debug/release
contains( E_RELEASE, no ) {
 CONFIG += debug
 CONFIG -= release
 QMAKE_CXXFLAGS += -g
}
contains( E_RELEASE, yes ) {
 # release mode
 # turns on optimalizations
 CONFIG += release
 CONFIG -= debug
 QMAKE_CXXFLAGS += -O2 -DNDEBUG
}

#include headers from kernel and used by kernel
INCLUDEPATH += ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../xpdf/splash ../xpdf/fofi

#QOutputDevices source files
HEADERS += QOutputDevPixmap.h
SOURCES += QOutputDevPixmap.cpp
HEADERS += QOutputDev.h
SOURCES += QOutputDev.cpp
