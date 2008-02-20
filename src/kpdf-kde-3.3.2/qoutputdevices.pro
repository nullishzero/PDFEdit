TEMPLATE = lib

CONFIG += staticlib

TARGET = qoutputdevices

# include basic definitions from configuration process
include(../../Makefile.flags)

#check debug/release
contains( E_RELEASE, no ) {
 CONFIG += debug
 CONFIG -= release
}
contains( E_RELEASE, yes ) {
 # release mode
 # turns on optimalizations
 CONFIG += release
 CONFIG -= debug
}

#Needed for Qt4. Qt3's Qmake does not know this variable, so it is ignored
#Note Qt4 is not (yet) supported
QT += qt3support

# include headers from kernel and used by kernel
INCPATH += $(MANDATORY_PATHS)

# Flags from configuration
QMAKE_CXXFLAGS += $(CONFIG_CXXFLAGS)

# Force configure compiler
QMAKE_CC = $(CONFIG_CC)
QMAKE_CPP = $(CONFIG_CXX)
QMAKE_LINK = $(CONFIG_LINK)
QMAKE_LINK_SHLIB = $(CONFIG_LINK)

#QOutputDevices source files
HEADERS += QOutputDevPixmap.h
SOURCES += QOutputDevPixmap.cpp
HEADERS += QOutputDev.h
SOURCES += QOutputDev.cpp
