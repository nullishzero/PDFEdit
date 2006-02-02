#
#
# Project file
#
#
TEMPLATE = app

_PATH = /usr/xpdf

#
# Kernel special settings
#
HEADERS += cobject.h cobjectI.h cpdf.h cpdfI.h iproperty.h ../utils/debug.h observer.h
SOURCES += main.cc 
QMAKE_CXXFLAGS += -Wuninitialized -frepo

#
# Files that are just bare declarations
#
#INCLUDEPATH += ../_xpdf/ ../_xpdf/xpdf ../utils ../



#
# Complete build
#
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo
#LIBS += -L$${_PATH}
#__LIBS = $$system(ls $$_PATH)
#OBJECTS += $$join(__LIBS, " $$_PATH/", $$_PATH/ )


