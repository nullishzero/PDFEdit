
isEmpty(EDITORDIR):EDITORDIR=.

HEADERS += $$EDITORDIR/qsaeditorinterface.h
HEADERS += $$EDITORDIR/qsaeditor.h
HEADERS += $$EDITORDIR/qsyntaxhighlighter.h
HEADERS += $$EDITORDIR/qsasyntaxhighlighter.h
HEADERS += $$EDITORDIR/parenmatcher.h
HEADERS += $$EDITORDIR/linenumberwidget.h
HEADERS += $$EDITORDIR/completionbox.h

SOURCES += $$EDITORDIR/qsaeditorinterface.cpp
SOURCES += $$EDITORDIR/qsaeditor.cpp
SOURCES += $$EDITORDIR/qsyntaxhighlighter.cpp
SOURCES += $$EDITORDIR/qsasyntaxhighlighter.cpp
SOURCES += $$EDITORDIR/parenmatcher.cpp
SOURCES += $$EDITORDIR/linenumberwidget.cpp
SOURCES += $$EDITORDIR/completionbox.cpp
SOURCES += $$EDITORDIR/yyindent.cpp

