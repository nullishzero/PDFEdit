#Project file
TEMPLATE = app

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

#Widget classes
HEADERS += pdfeditwindow.h toolbutton.h toolbar.h propertyeditor.h aboutwindow.h commandwindow.h treewindow.h 
SOURCES += pdfeditwindow.cc toolbutton.cc toolbar.cc propertyeditor.cc aboutwindow.cc commandwindow.cc treewindow.cc 

#Non-widget classes
HEADERS += property.h stringproperty.h intproperty.h settings.h 
SOURCES += property.cc stringproperty.cc intproperty.cc settings.cc 

#Other source files
HEADERS += util.h 
SOURCES += main.cc util.cc 
