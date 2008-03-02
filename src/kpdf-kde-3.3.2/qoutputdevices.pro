TEMPLATE = lib
TARGET = qoutputdevices

CONFIG += staticlib

# We want to have debug functionality and warnings under controll
QMAKE_CFLAGS -= -g -Wall
QMAKE_CXXFLAGS -= -g -Wall
QMAKE_CFLAGS_RELEASE -= -g -Wall
QMAKE_CXXFLAGS_RELEASE -= -g -Wall

# include basic definitions from configuration process
include(../../Makefile.flags)

# check debug/release
contains( E_RELEASE, no ) {
 # debug mode
 # turns off optimalizations
 CONFIG -= release
 CONFIG -= warn_off
 CONFIG *= debug
 CONFIG *= warn_on
}
contains( E_RELEASE, yes ) {
 # release mode
 # turns on optimalizations
 CONFIG -= debug
 CONFIG -= warn_on
 CONFIG *= warn_off
 CONFIG *= release
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
