TEMPLATE	= lib
CONFIG		+= qt warn_on link_prl staticlib
CONFIG		-= dll
TARGET 		= qsa_pdfedit
VERSION  	= 1.1.5m

QSA_BUILD_PREFIX=../..

DESTDIR         = ../../lib

HEADERS		= \
		  qsproject.h \
		  qsinterpreter.h \
		  qsobjectfactory.h \
		  qswrapperfactory.h \
		  qseditor.h \
		  qsinputdialogfactory.h \
		  qsargument.h \
                  qsscript.h \
                  qsutilfactory.h

SOURCES		= \
		  qsproject.cpp \
		  qsinterpreter.cpp \
		  qsobjectfactory.cpp \
		  qswrapperfactory.cpp \
		  qsinputdialogfactory.cpp \
		  qseditor.cpp \
		  qsargument.cpp \
                  qsscript.cpp \
                  qsutilfactory.cpp

win32:DEFINES   += QSA_NO_DLL

# I think this belongs here
isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)
isEmpty(QT_INSTALL_PREFIX):QT_INSTALL_PREFIX=$(QTDIR)

INCLUDEPATH	+= ../engine \
		   ../../tools/designer/editor \
		   ../kernel \
		    . 

DEFINES += HAVE_CONFIG_H QSDEBUGGER
DEFINES += QT_INTERNAL_XML
DEFINES += QT_INTERNAL_WORKSPACE
DEFINES += QT_INTERNAL_ICONVIEW
DEFINES += QT_INTERNAL_TABLE

LIBS += -L$${QT_INSTALL_PREFIX}/lib

mac:QMAKE_LFLAGS_PREBIND    = -prebind -seg1addr 0xBC000000

KERNELDIR = ../kernel
EDITORDIR = ../editor
EDITOR_BRANCH = ../../tools/designer/editor

include( ../kernel/kernel.pri )
include( ../editor/editor.pri )
include(../../tools/designer/editor/editor.pri)

win32:PRECOMPILED_HEADER = qsa_pch.h
