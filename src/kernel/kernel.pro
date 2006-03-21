#
# Project file
#

TEMPLATE = app
LANGUAGE = C++
CONFIG += complete 
CONFIG += console precompile_header
CONFIG -= qt
QMAKE_CXXFLAGS += -D DEBUG

#PRECOMPILED_HEADER = static.h
 


#
# Kernel special settings
#

HEADERS += ../utils/debug.h
HEADERS += observer.h exceptions.h modecontroller.h filters.h
HEADERS += iproperty.h cobject.h cobjectI.h cpage.h cpdf.h 

SOURCES += modecontroller.cc filters.cc 
SOURCES += cxref.cc xrefwriter.cc 
SOURCES += cobject.cc cpage.cc cpdf.cc
SOURCES += main.cc 

#QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable -finline-limit=10000 --param inline-unit-growth=1000 --param large-function-growth=1000
QMAKE_CXXFLAGS += -Wall -W -Wconversion -Wcast-qual -Wwrite-strings -Wuninitialized -ansi -pedantic -Wno-unused-variable


INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo


#
# Path to xpdf object files
#

_PATH = ../xpdf

#
# One specific lib, that must exist
#

_ONELIB = Stream.o


exists( $${_PATH}/xpdf/$${_ONELIB} ) {

	OBJECTS += $$system( find $$_PATH/goo -name "*.o" )
	OBJECTS += $$system( find $$_PATH/fofi -name "*.o" )
	OBJECTS += $$system( find $$_PATH/xpdf -name "[^pdf]*.o" )

}else{
	
	# Specific path
	_PATH = /usr/xpdf
	
	exists( $${_PATH}/$${_ONELIB} ) {

		OBJECTS += $$system( find $$_PATH/ -name "*.o" )
		
	}else{
		error( "You do not have xpdf libraries compiled. [../xpdf/xpdf/Object.o, ../xpdf/xpdf/Stream.o, ....]" )
	}
}
