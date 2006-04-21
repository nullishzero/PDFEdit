#
# Project file for kernel
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

#
# Build library (libkernel.a)
#
OBJDIR = .obj
libkernel.target     = libkernel.a
libkernel.commands   = $(AR) libkernel.a $(OBJECTS:$$OBJDIR/main.o=)
libkernel.depends    = $(OBJECTS:$$OBJDIR/main.o=)
QMAKE_EXTRA_UNIX_TARGETS += libkernel
POST_TARGETDEPS = libkernel.a

#
# Common utils
#
HEADERS += ../utils/debug.h ../utils/iterator.h ../utils/observer.h

#
# Special utils
#
HEADERS += exceptions.h modecontroller.h filters.h xpdf.h cxref.h xrefwriter.h factories.h
SOURCES += modecontroller.cc filters.cc 

#
# CObjects
#
HEADERS += iproperty.h cobject.h cobjectI.h cobjecthelpers.h ccontentstream.h pdfoperators.h cpage.h cpdf.h 
HEADERS += streamwriter.h
SOURCES += cxref.cc xrefwriter.cc streamwriter.cc
SOURCES += iproperty.cc cobject.cc cobjecthelpers.cc ccontentstream.cc pdfoperators.cc 
SOURCES += cpage.cc cpdf.cc 

#
# Tests
#
HEADERS += tests/testmain.h tests/testcobject.h tests/testcpdf.h
SOURCES += tests/testcobjectsimple.cc tests/testcobjectcomplex.cc tests/testcobjecthelpers.cc
SOURCES += tests/testcpage.cc tests/testccontentstream.cc
SOURCES += tests/testcpdf.cc
SOURCES += main.cc 

#QMAKE_LFLAGS += -static
INCLUDEPATH += ./tests/cppunit/include
LIBS += -ldl -lcppunit -L../cppunit/src/cppunit/.libs

#
# Kernel special settings
#
#QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable -finline-limit=10000 --param inline-unit-growth=1000 --param large-function-growth=1000
QMAKE_CXXFLAGS_DEBUG += -W -Wall -Wconversion -Wcast-qual -Wwrite-strings  -ansi -pedantic -Wno-unused-variable -O0 -Wunused-function

#
# xpdf and utils
#
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo

#
# Directories to creating files
#
unix {
  OBJECTS_DIR = $$OBJDIR
}

include(kernel-obj.pro)
