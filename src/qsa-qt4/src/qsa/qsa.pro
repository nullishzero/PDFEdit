TEMPLATE	= lib
CONFIG		+= warn_on link_prl
shared:CONFIG += qt dll
VERSION  	= 1.2.2
TARGET = qsa

isEmpty(QSA_INSTALL_PREFIX) {
  QSA_BUILD_PREFIX=$$(QTDIR)
} else {
  QSA_BUILD_PREFIX=../..
}

DESTDIR         = $$QSA_BUILD_PREFIX/lib
DLLDESTDIR      = $$QSA_BUILD_PREFIX/bin

HEADERS		= \
		  qsproject.h \
		  qsinterpreter.h \
		  qsobjectfactory.h \
		  qswrapperfactory.h \
                  qsscript.h \
                  qsutilfactory.h

SOURCES		= \
                  qsproject.cpp \
                  qsinterpreter.cpp \
                  qsobjectfactory.cpp \
                  qswrapperfactory.cpp \
                  qsscript.cpp \
                  qsutilfactory.cpp

!nogui {
	SOURCES += qsinputdialogfactory.cpp
	HEADERS += qsinputdialogfactory.h
}

!noeditor|neweditor{
	SOURCES += qseditor.cpp
	HEADERS += qseditor.h
	!noeditor:!neweditor:QT += qt3support
}


noeditor:!neweditor:!noide:CONFIG+=noide

shared {
	win32:DEFINES	+= QSA_DLL
} else {
	win32:DEFINES   += QSA_NO_DLL
}

# I think this belongs here
isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)
isEmpty(QT_INSTALL_PREFIX):QT_INSTALL_PREFIX=$(QTDIR)

INCLUDEPATH	+= ../engine \
		   ../kernel \
		    .

!noeditor:!neweditor: INCLUDEPATH += ../../tools/designer/editor

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
		qsscript.h \
		..\ide\qsworkbench.h

!isEmpty(QSA_INSTALL_PREFIX) {
  headers.path = $$QSA_INSTALL_PREFIX/include
  win32 {
    target.path = $$QSA_INSTALL_PREFIX/bin
  } else {
    target.path = $$QSA_INSTALL_PREFIX/lib
  }
  prf.files = qsa.prf
  prf.path = $$QSA_INSTALL_PREFIX/mkspecs/features
  documentation.files = ../../doc/html/*
  documentation.path = $$QSA_INSTALL_PREFIX/doc/html
  INSTALLS += headers target prf documentation
}

KERNELDIR = ../kernel
!noeditor {
  neweditor {
    EDITORDIR = ../neweditor
    INCLUDEPATH += ../neweditor
  } else {
    EDITORDIR = ../editor
    EDITOR_BRANCH = ../../tools/designer/editor
  }
}

!noide:IDEDIR = ../ide
include( ../kernel/kernel.pri )
!noeditor {
  neweditor {
    include( ../neweditor/editor.pri )
  } else {
    include( ../editor/editor.pri )
  }
}
!noide:include( ../ide/ide.pri )
!noeditor:!neweditor:include(../../tools/designer/editor/editor.pri)

load(uic)
contains(QT_CONFIG, release):contains(QT_CONFIG, debug) {
    # Qt was configued with both debug and release libs
    CONFIG += debug_and_release build_all
}
CONFIG(debug, debug|release) {
    unix:TARGET = $$member(TARGET, 0)_debug
    else:TARGET = $$member(TARGET, 0)d
}

PHEADERS_DIR = ../private_headers
include(../private_headers/headers.pri)

win32 {
    CONFIG += precompile_header
    PRECOMPILED_HEADER = qsa_pch.h
}

nogui {
    QT = core
    DEFINES += QSA_NO_GUI
}

