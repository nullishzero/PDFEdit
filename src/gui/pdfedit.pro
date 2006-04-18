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
menugenerator.commands   = $(LINK) $(LFLAGS) -o menugenerator .obj/menugenerator.o .obj/util.o $(SUBLIBS) -L$(QTDIR)/lib -L/usr/X11R6/lib -lqsa -lqt-mt -lXext -lX11 -lm
menugenerator.depends    = .obj/menugenerator.o .obj/util.o
menugenerator_o.target   = .obj/menugenerator.o
menugenerator_o.commands = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/menugenerator.o menugenerator.cc
menugenerator_o.depends  = menugenerator.cc menugenerator.h
QMAKE_EXTRA_UNIX_TARGETS += menugenerator menugenerator_o
POST_TARGETDEPS = menugenerator

#tohle nefunguje. Resim jinak
#load( qsa )

unix:LIBS       += -lqsa

#include headers from kernel and used by kernel
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../kernel ../kpdf-kde-3.3.2 ../xpdf/splash

#must be specified, otherwise namespace debug will clash with debug() in QT
QMAKE_CXXFLAGS += -DQT_CLEAN_NAMESPACE

#debug information
QMAKE_CXXFLAGS += -g

#Testing version : will look in current directory for data files
QMAKE_CXXFLAGS += -D TESTING

#Dialogs
HEADERS += aboutwindow.h  option.h  optionwindow.h  stringoption.h  realoption.h  intoption.h  booloption.h  combooption.h  dialog.h
SOURCES += aboutwindow.cc option.cc optionwindow.cc stringoption.cc realoption.cc intoption.cc booloption.cc combooption.cc dialog.cc

#Help
HEADERS += helpwindow.h
SOURCES += helpwindow.cc

#Tree window
HEADERS += treeitempage.h  treeitempdf.h  treeitem.h  treewindow.h  treedata.h
SOURCES += treeitempage.cc treeitempdf.cc treeitem.cc treewindow.cc treedata.cc 

#Property editor
HEADERS += property.h  stringproperty.h  intproperty.h  boolproperty.h  nameproperty.h  realproperty.h  refproperty.h
SOURCES += property.cc stringproperty.cc intproperty.cc boolproperty.cc nameproperty.cc realproperty.cc refproperty.cc
HEADERS += refvalidator.h  propertyeditor.h  propertyfactory.h
SOURCES += refvalidator.cc propertyeditor.cc propertyfactory.cc

#CObject Wrapper classes
HEADERS += qscobject.h  qspdf.h  qspage.h  qsdict.h  qsimporter.h  qsgraphics.h  qsiproperty.h
SOURCES += qscobject.cc qspdf.cc qspage.cc qsdict.cc qsimporter.cc qsgraphics.cc qsiproperty.cc

#Other source files
HEADERS += pdfutil.h  util.h  menu.h  settings.h 
SOURCES += pdfutil.cc util.cc menu.cc settings.cc main.cc 

#Main Window
HEADERS += pdfeditwindow.h  toolbutton.h  toolbar.h  commandwindow.h  pagespace.h  pageview.h
SOURCES += pdfeditwindow.cc toolbutton.cc toolbar.cc commandwindow.cc pagespace.cc pageview.cc

#Testing files (will be removed later)
HEADERS += test.h 
SOURCES += test.cc

#Dummy header file for menu translation
HEADERS += .menu-trans.h 


#Kernel objects - now using library
LIBS += -lkernel -L../kernel

#OutputDevice
LIBS += -lt1
LIBS += -lqoutputdevices -L../kpdf-kde-3.3.2
LIBS += -lpokus -L../kpdf-kde-3.3.2
LIBS += -lsplash -L../xpdf/splash

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
