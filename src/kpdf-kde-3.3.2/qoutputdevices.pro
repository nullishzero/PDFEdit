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

# Force configure compiler and all other programs used for installation
QMAKE_CC = $(CONFIG_CC)
QMAKE_CXX = $(CONFIG_CXX)
QMAKE_LINK = $(CONFIG_LINK)
QMAKE_LINK_SHLIB = $(CONFIG_LINK)
QMAKE_RANLIB = $(CONFIG_RANLIB)
QMAKE_AR = $(CONFIG_AR)
QMAKE_COPY = $(CONFIG_COPY)
QMAKE_COPY_FILE = $(CONFIG_COPY_FILE)
QMAKE_COPY_DIR = $(CONFIG_COPY_DIR)
QMAKE_DEL_FILE = $(CONFIG_DEL_FILE)
# qmake doesn't like QMAKE_SYMLINK and ignores it
QMAKE_SYMLINK = $(CONFIG_SYMLINK)
QMAKE_DEL_DIR = $(CONFIG_DEL_DIR)
QMAKE_MOVE = $(CONFIG_MOVE)
QMAKE_CHK_DIR_EXISTS = $(CONFIG_CHK_DIR_EXISTS)
QMAKE_MKDIR = $(CONFIG_MKDIR)

#QOutputDevices source files
HEADERS += QOutputDevPixmap.h
SOURCES += QOutputDevPixmap.cpp
HEADERS += QOutputDev.h
SOURCES += QOutputDev.cpp
