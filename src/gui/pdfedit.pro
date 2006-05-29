#Project file for PDF Editor
TEMPLATE = app
TARGET = pdfedit

# installation paths
USR_PREFIX= /usr/local
DATA_PATH = $$USR_PREFIX/share/pdfedit
DOC_PATH  = $$USR_PREFIX/share/doc/pdfedit
MAN_PATH  = $$USR_PREFIX/share/man/man1
BIN_PATH  = $$USR_PREFIX/bin

# installation details
data.path       = $$DATA_PATH
data.files      = pdfeditrc
data_icon.path  = $$DATA_PATH/icon
data_icon.files = icon/*.png
data_help.path  = $$DATA_PATH/help
data_help.files = icon/*.html
data_lang.path  = $$DATA_PATH/lang
data_lang.files = lang/*.qm
doc.path        = $$DOC_PATH
doc.files       = ../../doc/user/*.html ../../doc/user/*.xml ../../doc/design/gui/menu.*
doc_kernel.path = $$DOC_PATH/kernel
doc_kernel.files= ../../doc/design/kernel/*.html ../../doc/design/kernel/*.xml
doc_kernel_images.path = $$DOC_PATH/kernel/images
doc_kernel_images.files=../../doc/design/kernel/images/*.png
man.path	= $$MAN_PATH
man.file	= ../../doc/user/*.1
pdfedit.path    = $$BIN_PATH
pdfedit.files   = pdfedit
INSTALLS = data_icon data_lang data_help data doc doc_kernel doc_kernel_images pdfedit man

#too complicated for small utility.
menugenerator.target     = menugenerator
menugenerator.commands   = $(LINK) $(LFLAGS) -o menugenerator .obj/menugenerator.o .obj/util.o $(SUBLIBS) -L$(QTDIR)/lib -L../utils -L/usr/X11R6/lib -lqsa -lutils -lqt-mt -lXext -lX11 -lm
menugenerator.depends    = .obj/menugenerator.o .obj/util.o
menugenerator_o.target   = .obj/menugenerator.o
menugenerator_o.commands = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/menugenerator.o menugenerator.cc
menugenerator_o.depends  = menugenerator.cc menugenerator.h
QMAKE_EXTRA_UNIX_TARGETS += menugenerator menugenerator_o
POST_TARGETDEPS = menugenerator

LIBS       += -lqsa

#include headers from kernel and used by kernel
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../kernel ../kpdf-kde-3.3.2 ../xpdf/splash

#must be specified, otherwise namespace debug will clash with debug() in QT
QMAKE_CXXFLAGS += -DQT_CLEAN_NAMESPACE

#debug information
QMAKE_CXXFLAGS += -g

#Dialogs
HEADERS += additemdialog.h  aboutwindow.h  option.h  optionwindow.h  dialog.h
SOURCES += additemdialog.cc aboutwindow.cc option.cc optionwindow.cc dialog.cc
HEADERS += stringoption.h  realoption.h  intoption.h  booloption.h  combooption.h
SOURCES += stringoption.cc realoption.cc intoption.cc booloption.cc combooption.cc
HEADERS += dialogoption.h  fileoption.h  fontoption.h
SOURCES += dialogoption.cc fileoption.cc fontoption.cc

#Help window
HEADERS += helpwindow.h
SOURCES += helpwindow.cc

#Tree window
HEADERS += treeitemabstract.h  treeitemref.h  treeitemarray.h  treeitemsimple.h  treeitemdict.h  treeitempage.h
SOURCES += treeitemabstract.cc treeitemref.cc treeitemarray.cc treeitemsimple.cc treeitemdict.cc treeitempage.cc
HEADERS += treeitemcstream.h  treeitempdf.h  treeitem.h  treewindow.h  treedata.h  treeitemcontentstream.h
SOURCES += treeitemcstream.cc treeitempdf.cc treeitem.cc treewindow.cc treedata.cc treeitemcontentstream.cc
HEADERS += treeitempdfoperator.h  draglistview.h  multitreewindow.h  treeitemobserver.h  treeitemcontentstreamobserver.h
SOURCES += treeitempdfoperator.cc draglistview.cc multitreewindow.cc treeitemobserver.cc treeitemcontentstreamobserver.cc

#Property editor
HEADERS += property.h  stringproperty.h  intproperty.h  boolproperty.h  nameproperty.h  realproperty.h
SOURCES += property.cc stringproperty.cc intproperty.cc boolproperty.cc nameproperty.cc realproperty.cc
HEADERS += refvalidator.h  refproperty.h  propertyeditor.h  propertyfactory.h
SOURCES += refvalidator.cc refproperty.cc propertyeditor.cc propertyfactory.cc

#QSA Wrapper classes
HEADERS += qscobject.h  qscontentstream.h  qspdf.h  qspage.h  qsdict.h  qsimporter.h  qsgraphics.h
SOURCES += qscobject.cc qscontentstream.cc qspdf.cc qspage.cc qsdict.cc qsimporter.cc qsgraphics.cc
HEADERS += qstreeitem.h  qsmenu.h  qsarray.h  qsstream.h  qsiproperty.h  qspdfoperator.h
SOURCES += qstreeitem.cc qsmenu.cc qsarray.cc qsstream.cc qsiproperty.cc qspdfoperator.cc

#Other source files
HEADERS += pdfutil.h  util.h  menu.h  settings.h  iconcache.h  args.h  main.h  selfdestructivewidget.h  base.h  rect2Darray.h
SOURCES += pdfutil.cc util.cc menu.cc settings.cc iconcache.cc args.cc main.cc selfdestructivewidget.cc base.cc rect2Darray.cc

#Exceptions
HEADERS += invalidmenuexception.h
SOURCES += invalidmenuexception.cc

#Toolbar
HEADERS += revisiontool.h  zoomtool.h  pagetool.h  toolbutton.h  toolbar.h  
SOURCES += revisiontool.cc zoomtool.cc pagetool.cc toolbutton.cc toolbar.cc 

#Main Window
HEADERS += pdfeditwindow.h  commandwindow.h  pagespace.h  pageview.h
SOURCES += pdfeditwindow.cc commandwindow.cc pagespace.cc pageview.cc


#Dummy header file for menu translation
HEADERS += .menu-trans.h 

#Kernel objects - now using library
LIBS += -lkernel -L../kernel

#OutputDevice
LIBS += -lqoutputdevices -L../kpdf-kde-3.3.2

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
