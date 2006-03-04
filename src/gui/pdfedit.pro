#Project file
TEMPLATE = app

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

HEADERS += pdfeditwindow.h settings.h util.h toolbutton.h toolbar.h property.h stringproperty.h propertyeditor.h intproperty.h aboutwindow.h
SOURCES += pdfeditwindow.cc settings.cc main.cc util.cc toolbutton.cc toolbar.cc property.cc stringproperty.cc propertyeditor.cc intproperty.cc aboutwindow.cc
