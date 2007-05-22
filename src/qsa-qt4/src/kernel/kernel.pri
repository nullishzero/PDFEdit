isEmpty(KERNELDIR):KERNELDIR=.

HEADERS		+= $$KERNELDIR/quickinterpreter.h \
		$$KERNELDIR/quickdebugger.h \
		$$KERNELDIR/quickobjects.h \
		$$KERNELDIR/quickenv.h \
		$$KERNELDIR/quickclassparser.h \
		$$KERNELDIR/quickbytearrayobject.h \
		$$KERNELDIR/quickcoordobjects.h \
		$$KERNELDIR/quickdispatchobject.h \
		$$KERNELDIR/qsmetaobject.h \
		$$KERNELDIR/qsslot.h

!nogui:HEADERS  += $$KERNELDIR/quickpixmapobject.h \
		$$KERNELDIR/quickcolorobject.h \
		$$KERNELDIR/quickfontobject.h

SOURCES		+= $$KERNELDIR/quickinterpreter.cpp \
		$$KERNELDIR/quickdebugger.cpp \
		$$KERNELDIR/quickobjects.cpp \
		$$KERNELDIR/quickenv.cpp \
		$$KERNELDIR/quickclassparser.cpp \
		$$KERNELDIR/quickbytearrayobject.cpp \
		$$KERNELDIR/quickcoordobjects.cpp \
		$$KERNELDIR/quickdispatchobject.cpp \
		$$KERNELDIR/qsmetaobject.cpp

!nogui:SOURCES  += $$KERNELDIR/quickpixmapobject.cpp \
		$$KERNELDIR/quickcolorobject.cpp \
		$$KERNELDIR/quickfontobject.cpp

ENGDIR 		= ../engine
include($$ENGDIR/engine.pri)

isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)

DEPENDPATH	+= ../engine

DEFINES += HAVE_CONFIG_H QSDEBUGGER

