isEmpty(EDITORDIR):EDITORDIR=.

HEADERS		+= $$EDITORDIR/qsaeditor.h \
		   $$EDITORDIR/qsacompletion.h \
		   $$EDITORDIR/qsabrowser.h \
		   $$EDITORDIR/qsasyntaxhighlighter.h \
		   $$EDITORDIR/qsaeditorinterface.h 

SOURCES		+= \
		   $$EDITORDIR/qsaeditor.cpp \
		   $$EDITORDIR/qsacompletion.cpp \
		   $$EDITORDIR/qsabrowser.cpp \
		   $$EDITORDIR/qsasyntaxhighlighter.cpp \
		   $$EDITORDIR/qsaeditorinterface.cpp 


# Need this to include conf.h...
noide:!noeditor {
    INCLUDEPATH += ../ide
    SOURCES += ../ide/conf.cpp
    HEADERS += ../ide/conf.h
}


#The following line was inserted by qt3to4
QT +=  qt3support 
