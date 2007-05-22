isEmpty(IDEDIR):IDEDIR=.

HEADERS	+= $$IDEDIR/qsworkbench.h \
  	   $$IDEDIR/idewindow.h \
  	   $$IDEDIR/preferences.h \
  	   $$IDEDIR/conf.h

SOURCES	+= $$IDEDIR/qsworkbench.cpp \
           $$IDEDIR/idewindow.cpp \
           $$IDEDIR/preferences.cpp \
           $$IDEDIR/conf.cpp

FORMS = $$IDEDIR/findtext.ui \
	$$IDEDIR/gotoline.ui \
	$$IDEDIR/idewindow.ui \
	$$IDEDIR/outputcontainer.ui \
	$$IDEDIR/projectcontainer.ui \
	$$IDEDIR/replacetext.ui \
	$$IDEDIR/preferencescontainer.ui \
	$$IDEDIR/preferences.ui

RESOURCES += $$IDEDIR/ide.qrc

headers.files = $$headers.files $$IDEDIR/qsscripter.h

INCLUDEPATH+=$$IDEDIR