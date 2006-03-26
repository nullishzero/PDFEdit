#Project file for PDF Editor
TEMPLATE = app

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

#include headers from kernel and used by kernel
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../kernel

#must be specified, otherwise namespace debug will clash with debug() in QT
QMAKE_CXXFLAGS += -DQT_CLEAN_NAMESPACE

#debug information
QMAKE_CXXFLAGS += -g

#Widget classes
HEADERS += pdfeditwindow.h toolbutton.h toolbar.h propertyeditor.h aboutwindow.h commandwindow.h treewindow.h 
SOURCES += pdfeditwindow.cc toolbutton.cc toolbar.cc propertyeditor.cc aboutwindow.cc commandwindow.cc treewindow.cc 

#Non-widget classes
HEADERS += property.h  stringproperty.h  intproperty.h  boolproperty.h  realproperty.h  settings.h  treeitem.h  propertyfactory.h
SOURCES += property.cc stringproperty.cc intproperty.cc boolproperty.cc realproperty.cc settings.cc treeitem.cc propertyfactory.cc

#CObject Wrapper classes
HEADERS += qscobject.h  qspdf.h  qspage.h  qsdict.h  qsimporter.h  
SOURCES += qscobject.cc qspdf.cc qspage.cc qsdict.cc qsimporter.cc 

#Other source files
HEADERS += util.h 
SOURCES += util.cc main.cc 

#Testing files (will be removed later)
HEADERS += test.h 
SOURCES += test.cc

#Kernel objects
_KPATH = ../kernel
_ONELIBK = cobject.o

exists( $${_KPATH}/$${_ONELIBK} ) {
 LIBS += $$system( find $$_KPATH -name "[^m]*.o" )
}else{
 error( "You do not have kernel compiled. (../kernel/*.o)" )
}

include(../kernel/kernel-obj.pro)

