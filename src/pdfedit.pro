#Project file
TEMPLATE = app

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

# Input
HEADERS += pdfedit.h settings.h util.h tbutton.h toolbar.h
SOURCES += pdfedit.cc settings.cc main.cc util.cc tbutton.cc toolbar.cc
