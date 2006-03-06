#
# Project file
#

TEMPLATE = app
CONFIG += complete
QMAKE_CXXFLAGS += -D DEBUG
 
#
# Path to xpdf object files
#

_PATH = /usr/xpdf


#
# Kernel special settings
#

HEADERS += cobject.h cobjectI.h cpdf.h iproperty.h ../utils/debug.h observer.h exceptions.h lib.h
SOURCES += cobject.cc cpdf.cc lib.cc cxref.cc xrefwriter.cc main.cc 
#QMAKE_CXXFLAGS += -Wuninitialized -frepo
#QMAKE_CXXFLAGS += -Wuninitialized -ansi -pedantic -Winline
QMAKE_CXXFLAGS += -Wuninitialized -ansi -pedantic -Wno-unused-variable -Winline -finline-limit=10000 --param inline-unit-growth=600 --param large-function-growth=600


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
