#Project file
TEMPLATE = app

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

# Input
#HEADERS += pdfeditwindow.h settings.h util.h toolbutton.h toolbar.h property.h stringproperty.h propertyeditor.h intproperty.h
#SOURCES += pdfeditwindow.cc settings.cc main.cc util.cc toolbutton.cc toolbar.cc property.cc stringproperty.cc propertyeditor.cc intproperty.cc


#
# My special settings
#
HEADERS += cobject.h
SOURCES += main_kernel.cc

INCLUDEPATH = ../xpdf/ ../xpdf/xpdf ../xpdf/goo
CONFIG += WALL

#DESTDIR = ../output
