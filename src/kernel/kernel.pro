#
# Project file
#

TEMPLATE = app
LANGUAGE = C++
CONFIG += complete 
CONFIG += console precompile_header
CONFIG += debug
#CONFIG -= qt
DEFINES += DEBUG

#PRECOMPILED_HEADER = static.h
 


#
# Kernel special settings
#

HEADERS += ../utils/debug.h ../utils/iterator.h
HEADERS += observer.h exceptions.h modecontroller.h filters.h xpdf.h
HEADERS += iproperty.h cobject.h cobjectI.h cobjecthelpers.h ccontentstream.h pdfoperators.h cpage.h cpdf.h 
HEADERS += tests/testmain.h tests/testcobject.h tests/testcpdf.h

SOURCES += modecontroller.cc filters.cc 
SOURCES += cxref.cc xrefwriter.cc 
SOURCES += cobject.cc cobjecthelpers.cc ccontentstream.cc pdfoperators.cc cpage.cc cpdf.cc 
SOURCES += ./tests/testcobject.cc 
SOURCES += main.cc 

#QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable -finline-limit=10000 --param inline-unit-growth=1000 --param large-function-growth=1000

QMAKE_CFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_DEBUG += -Wall -W -Wconversion -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable -O0


INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo

include(kernel-obj.pro)
