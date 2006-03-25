#
# Project file
#

TEMPLATE = app
LANGUAGE = C++
CONFIG += complete 
CONFIG += console precompile_header
#CONFIG -= qt
QMAKE_CXXFLAGS += -D DEBUG

PRECOMPILED_HEADER = static.h
 


#
# Kernel special settings
#

HEADERS += ../utils/debug.h ../utils/iterator.h
HEADERS += observer.h exceptions.h modecontroller.h filters.h xpdf.h
HEADERS += iproperty.h cobject.h cobjectI.h cobjecthelpers.h ccontentstream.h cpage.h cpdf.h 

SOURCES += modecontroller.cc filters.cc 
SOURCES += cxref.cc xrefwriter.cc 
SOURCES += cobject.cc cobjecthelpers.cc ccontentstream.cc cpage.cc cpdf.cc 
SOURCES += main.cc 

#QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable -finline-limit=10000 --param inline-unit-growth=1000 --param large-function-growth=1000
QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable


INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo

include(kernel-obj.pro)
