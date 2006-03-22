#Project file for PDF Editor
TEMPLATE = app

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

#include headers from kernel and used by kernel
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../kernel

#must be specified, otherwise namespace debug will clash with debug() in QT
QMAKE_CXXFLAGS += -DQT_CLEAN_NAMESPACE

#Widget classes
HEADERS += pdfeditwindow.h toolbutton.h toolbar.h propertyeditor.h aboutwindow.h commandwindow.h treewindow.h 
SOURCES += pdfeditwindow.cc toolbutton.cc toolbar.cc propertyeditor.cc aboutwindow.cc commandwindow.cc treewindow.cc 

#Non-widget classes
HEADERS += property.h stringproperty.h intproperty.h settings.h treeitem.h
SOURCES += property.cc stringproperty.cc intproperty.cc settings.cc treeitem.cc

#Other source files
HEADERS += util.h 
SOURCES += main.cc util.cc 
