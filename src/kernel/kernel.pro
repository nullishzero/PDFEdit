#
# Project file
#

TEMPLATE = app
LANGUAGE = C++
CONFIG += complete console precompile_header
QMAKE_CXXFLAGS += -D DEBUG
PRECOMPILED_HEADER = static.h
 
#
# Path to xpdf object files
#

_PATH = /usr/xpdf



#
# Kernel special settings
#

HEADERS += cobject.h cobjectI.h cpdf.h iproperty.h ../utils/debug.h observer.h exceptions.h
SOURCES += cobject.cc cpdf.cc cxref.cc xrefwriter.cc main.cc 
#QMAKE_CXXFLAGS += -Wuninitialized -frepo
#QMAKE_CXXFLAGS += -Wuninitialized -ansi -pedantic -Wno-unused-variable -Winline -finline-limit=10000 --param inline-unit-growth=1000 --param large-function-growth=1000
#QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable -finline-limit=10000 --param inline-unit-growth=1000 --param large-function-growth=1000
QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable


#
# Complete build
#

complete {

INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo
LIBS += -L$${_PATH}
__LIBS = $$system(ls $$_PATH)
OBJECTS += $$join(__LIBS, " $$_PATH/", $$_PATH/ )

}else {

#
# Files that are just bare declarations
#

INCLUDEPATH += ../_xpdf/ ../_xpdf/xpdf ../utils ../

}
