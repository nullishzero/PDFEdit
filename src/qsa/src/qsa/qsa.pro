TEMPLATE	= lib
CONFIG		+= qt warn_on link_prl staticlib
TARGET 		= qsa
VERSION  	= 1.1.4m

isEmpty(QSA_INSTALL_PREFIX) {
  QSA_BUILD_PREFIX=$$(QTDIR)
} else {
  QSA_BUILD_PREFIX=../..
}

DESTDIR         = ../../lib
DLLDESTDIR      = $$QSA_BUILD_PREFIX/bin

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

headers.files 	= \
		qsaglobal.h \
                qsconfig.h \
		qsobjectfactory.h \
		qswrapperfactory.h \
		qseditor.h \
		qsproject.h \
		qsinterpreter.h \
		qsargument.h \
		qsinputdialogfactory.h \
		qsscript.h

!isEmpty(QSA_INSTALL_PREFIX) {
  headers.path = $$QSA_INSTALL_PREFIX/include
  win32 {
    target.path = $$QSA_INSTALL_PREFIX/bin
  } else {
    target.path = $$QSA_INSTALL_PREFIX/lib
  }
  INSTALLS += headers target
}

KERNELDIR = ../kernel
EDITORDIR = ../editor
EDITOR_BRANCH = ../../tools/designer/editor

include( ../kernel/kernel.pri )
include( ../editor/editor.pri )
include(../../tools/designer/editor/editor.pri)

PRECOMPILED_HEADER = qsa_pch.h
