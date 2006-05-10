#
# Project file for kernel
#
TEMPLATE = lib
LANGUAGE = C++
TARGET = kernel

CONFIG += complete 
CONFIG += console precompile_header
CONFIG += debug
CONFIG -= qt
DEFINES += DEBUG

#PRECOMPILED_HEADER = static.h

#
# Building tests
#
tests.commands	= qmake -o Makefile-tests kernel-tests.pro && make -f Makefile-tests
tests.depends	= libkernel.a
QMAKE_EXTRA_UNIX_TARGETS += tests

#
# Common utils
#
HEADERS += ../utils/debug.h ../utils/iterator.h ../utils/observer.h

#
# Special utils
#
HEADERS += exceptions.h modecontroller.h filters.h xpdf.h cxref.h xrefwriter.h factories.h
HEADERS += pdfwriter.h
SOURCES += modecontroller.cc filters.cc 

#
# CObjects
#
HEADERS += iproperty.h cobject.h cobjectI.h cobjecthelpers.h ccontentstream.h pdfoperators.h cpage.h cpdf.h 
HEADERS += streamwriter.h cinlineimage.h
SOURCES += cxref.cc xrefwriter.cc streamwriter.cc
SOURCES += iproperty.cc cobject.cc cobjecthelpers.cc ccontentstream.cc pdfoperators.cc 
SOURCES += pdfwriter.cc
SOURCES += cinlineimage.cc
SOURCES += cpage.cc cpdf.cc 

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
