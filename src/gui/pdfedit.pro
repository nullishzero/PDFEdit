#Project file for PDF Editor
TEMPLATE = app
TARGET = pdfedit

# installation paths
DATA_PATH = /usr/share/pdfedit
DOC_PATH  = /usr/share/doc/pdfedit
BIN_PATH  = /usr/bin

# installation details
data.path       = $$DATA_PATH
data.files      = pdfeditrc
data_icon.path  = $$DATA_PATH/icon
data_icon.files = icon/*.png
data_lang.path  = $$DATA_PATH/lang
data_lang.files = lang/*.qm
doc.path        = $$DOC_PATH
doc.files       = ../../doc/user/*.html ../../doc/user/*.xml
pdfedit.path    = $$BIN_PATH
pdfedit.files   = pdfedit
INSTALLS = data_icon data_lang data doc pdfedit

#too complicated for small utility.
menugenerator.target     = menugenerator
menugenerator.commands   = $(LINK) $(LFLAGS) -o menugenerator .obj/menugenerator.o $(SUBLIBS) -L$(QTDIR)/lib -L/usr/X11R6/lib -lqsa -lqt-mt -lXext -lX11 -lm
menugenerator.depends    = .obj/menugenerator.o
menugenerator_o.target   = .obj/menugenerator.o
menugenerator_o.commands = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/menugenerator.o menugenerator.cc
menugenerator_o.depends  = menugenerator.cc menugenerator.h
QMAKE_EXTRA_UNIX_TARGETS += menugenerator menugenerator_o
POST_TARGETDEPS = menugenerator

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

#include headers from kernel and used by kernel
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../kernel

#must be specified, otherwise namespace debug will clash with debug() in QT
QMAKE_CXXFLAGS += -DQT_CLEAN_NAMESPACE

#debug information
#QMAKE_CXXFLAGS += -g

#testing version
#will look in current directory for data files
QMAKE_CXXFLAGS += -D TESTING

#Widget classes
HEADERS += pdfeditwindow.h toolbutton.h toolbar.h propertyeditor.h aboutwindow.h commandwindow.h treewindow.h pagespace.h pageview.h
SOURCES += pdfeditwindow.cc toolbutton.cc toolbar.cc propertyeditor.cc aboutwindow.cc commandwindow.cc treewindow.cc pagespace.cc pageview.cc

#Non-widget classes
HEADERS += property.h  stringproperty.h  intproperty.h  boolproperty.h  realproperty.h  settings.h  treeitem.h  propertyfactory.h
SOURCES += property.cc stringproperty.cc intproperty.cc boolproperty.cc realproperty.cc settings.cc treeitem.cc propertyfactory.cc

#CObject Wrapper classes
HEADERS += qscobject.h  qspdf.h  qspage.h  qsdict.h  qsimporter.h  qsgraphics.h  qsiproperty.h
SOURCES += qscobject.cc qspdf.cc qspage.cc qsdict.cc qsimporter.cc qsgraphics.cc qsiproperty.cc

#Other source files
HEADERS += util.h 
SOURCES += util.cc main.cc 

#Testing files (will be removed later)
HEADERS += test.h 
SOURCES += test.cc

#Kernel objects
_KPATH = ../kernel
_ONELIBK = cobject.o

exists( $${_KPATH}/$${_ONELIBK} ) {
 LIBS += $$system( find $$_KPATH -name "*.o" |grep -v main.o )
}else{
 error( "You do not have kernel compiled. (../kernel/*.o)" )
}

#Translations
DEPENDPATH += lang
TRANSLATIONS = lang/pdfedit_cs.ts lang/pdfedit_sk.ts

include(../kernel/kernel-obj.pro)

#directories to creating files
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
