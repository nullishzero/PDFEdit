#
# Project file for kernel tests
#
TEMPLATE = app
LANGUAGE = C++
TARGET = kernel

CONFIG += complete 
CONFIG += console precompile_header
CONFIG += debug
CONFIG -= qt
DEFINES += DEBUG

#PRECOMPILED_HEADER = static.h

POST_TARGETDEPENDS = libkernel.a

LIBS += -lkernel -L./ -lutils -L../utils/

#
# Common utils
#
HEADERS += ../utils/debug.h ../utils/iterator.h ../utils/observer.h

#
# Tests
#
HEADERS += tests/testmain.h tests/testcobject.h tests/testcpdf.h
SOURCES += tests/testcobjectsimple.cc tests/testcobjectcomplex.cc tests/testcstream.cc
SOURCES += tests/teststream.cc tests/teststreamwriter.cc
SOURCES += tests/testcobjecthelpers.cc tests/testpdfoperators.cc
SOURCES += tests/testcpage.cc tests/testccontentstream.cc tests/testcpdf.cc
SOURCES += tests/testutils.cc tests/testoutlines.cc
SOURCES += main.cc 

LIBS += -ldl -lcppunit 

#
# Kernel special settings
#
#QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable -finline-limit=10000 --param inline-unit-growth=1000 --param large-function-growth=1000
QMAKE_CXXFLAGS_DEBUG += -W -Wall -Wconversion -Wcast-qual -Wwrite-strings  -ansi -pedantic -Wno-unused-variable -O0 -Wunused-function

#
# xpdf and utils
#
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../xpdf/splash ../xpdf/fofi

#
# Directories to creating files
#
unix {
  OBJECTS_DIR = .obj
}

include(kernel-obj.pro)
