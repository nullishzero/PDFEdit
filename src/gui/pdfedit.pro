#Project file for PDF Editor
TEMPLATE = app
TARGET = pdfedit

# turns off optimalizations
CONFIG += debug

include(../../config.pro)

# Check installation prefix
isEmpty( PREFIX ) {
 message("No prefix defined - check config.pro (generated from config.pro.in) in top-level directory")
 message("Run ./configure there if the file does not exist")
 error("PREFIX not defined");
}

# Installation details

#Data files installed in application data path (typically /usr/share/pdfedit)
# Basic data files
data.path       = $$DATA_PATH
data.files      = pdfeditrc *.conf *.qs
# Icons
data_icon.path  	= $$DATA_PATH/icon
data_icon.files 	= icon/*.png 
# Hi-color icon theme
data_icon_hicolor.path  = $$DATA_PATH/icon/hicolor
data_icon_hicolor.files = icon/hicolor/*.png 
# Help files (english)
data_help.path  	= $$DATA_PATH/help
data_help.files 	= help/*.html
# Help files (czech)
data_help_cs.path 	= $$DATA_PATH/help/cs
data_help_cs.files	= help/cs/*.html
# Help images
data_help_images.path 	= $$DATA_PATH/help/images
data_help_images.files	= ../../doc/user/images/*.png
# Language files
data_lang.path  	= $$DATA_PATH/lang
data_lang.files 	= lang/*.qm
# Scripts
data_scripts.path	= $$DATA_PATH/scripts
			# do not include script files starting with underscore,
			# as they are only for development/testing and not for production use
data_scripts.files 	= scripts/[a-z0-9A-Z]*.qs

# Documentation installed in documentation path (typically /usr/share/doc)
doc.path        = $$DOC_PATH
doc.files       = ../../doc/user/*.html ../../doc/user/*.xml ../../doc/design/gui/menu.* ../../doc/LICENSE.GPL
doc_images.path		= $$DOC_PATH/images
doc_images.files	= ../../doc/user/images/*.png
doc_kernel.path 	= $$DOC_PATH/kernel
doc_kernel.files	= ../../doc/design/kernel/*.html ../../doc/design/kernel/*.xml
doc_kernel_images.path	= $$DOC_PATH/kernel/images
doc_kernel_images.files	= ../../doc/design/kernel/images/*.png

# Man page installed in man path (typically /usr/share/man/man1)
man.path	= $$MAN_PATH
man.file	= ../../doc/user/*.1

#Binary file installed in binary path (typically /usr/bin)
pdfedit.path    = $$BIN_PATH
pdfedit.files   = pdfedit

#List of installed targets
INSTALLS  = data data_icon data_icon_hicolor data_help data_help_cs data_help_images data_lang data_scripts
INSTALLS += doc doc_images doc_kernel doc_kernel_images
INSTALLS += man
INSTALLS += pdfedit

#too complicated for small utility.
menugenerator.target     = menugenerator
menugenerator.commands   = $(LINK) $(LFLAGS) -o menugenerator .obj/menugenerator.o .obj/util.o .obj/staticsettings.o $(SUBLIBS) -L$(QTDIR)/lib -L../utils -L/usr/X11R6/lib -lqsa -lutils -lqt-mt -lXext -lX11 -lm
menugenerator.depends    = .obj/menugenerator.o .obj/util.o .obj/staticsettings.o
menugenerator_o.target   = .obj/menugenerator.o
menugenerator_o.commands = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/menugenerator.o menugenerator.cc
menugenerator_o.depends  = menugenerator.cc menugenerator.h
QMAKE_EXTRA_UNIX_TARGETS += menugenerator menugenerator_o
POST_TARGETDEPS = menugenerator
QMAKE_CLEAN += .obj/menugenerator.o

LIBS       += -lqsa

#include headers from kernel and used by kernel
INCLUDEPATH += ../ ../utils ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../kernel ../kpdf-kde-3.3.2 ../xpdf/splash

#must be specified, otherwise namespace debug will clash with debug() in QT
QMAKE_CXXFLAGS += -DQT_CLEAN_NAMESPACE


QMAKE_CXXFLAGS_DEBUG += -O0 

#debug information
QMAKE_CXXFLAGS += -g

#Dialogs
HEADERS += additemdialog.h  aboutwindow.h  option.h  optionwindow.h  dialog.h
SOURCES += additemdialog.cc aboutwindow.cc option.cc optionwindow.cc dialog.cc
HEADERS += stringoption.h  realoption.h  intoption.h  booloption.h  combooption.h
SOURCES += stringoption.cc realoption.cc intoption.cc booloption.cc combooption.cc
HEADERS += dialogoption.h  fileoption.h  fontoption.h
SOURCES += dialogoption.cc fileoption.cc fontoption.cc
SOURCES += annotdialog.cc
HEADERS += annotdialog.h
HEADERS += mergeform.h
SOURCES += mergeform.cc

#Help window
HEADERS += helpwindow.h
SOURCES += helpwindow.cc

#Tree window
HEADERS += treeitemabstract.h  treewindow.h  treedata.h  draglistview.h  multitreewindow.h
SOURCES += treeitemabstract.cc treewindow.cc treedata.cc draglistview.cc multitreewindow.cc
#Tree item observers
HEADERS += treeitemcontentstreamobserver.h  treeitemobserver.h  treeitempageobserver.h treeitemgenericobserver.h
#Tree item types
HEADERS += treeitemref.h  treeitemarray.h  treeitemsimple.h  treeitemdict.h  treeitempage.h
SOURCES += treeitemref.cc treeitemarray.cc treeitemsimple.cc treeitemdict.cc treeitempage.cc
HEADERS += treeitemcstream.h  treeitempdf.h  treeitem.h  treeitemcontentstream.h
SOURCES += treeitemcstream.cc treeitempdf.cc treeitem.cc treeitemcontentstream.cc
HEADERS += treeitempdfoperator.h  treeitemoperatorcontainer.h  treeitemoutline.h
SOURCES += treeitempdfoperator.cc treeitemoperatorcontainer.cc treeitemoutline.cc
HEADERS += treeitemannotation.h  treeitemannotationcontainer.h  
SOURCES += treeitemannotation.cc treeitemannotationcontainer.cc

#Property editor
HEADERS += refvalidator.h  propertyeditor.h  propertyfactory.h  property.h  propertymodecontroller.h
SOURCES += refvalidator.cc propertyeditor.cc propertyfactory.cc property.cc propertymodecontroller.cc
HEADERS += refpropertydialog.h
SOURCES += refpropertydialog.cc
#Property types
HEADERS += stringproperty.h  intproperty.h  boolproperty.h  nameproperty.h  realproperty.h
SOURCES += stringproperty.cc intproperty.cc boolproperty.cc nameproperty.cc realproperty.cc
HEADERS += refproperty.h
SOURCES += refproperty.cc

#QSA Wrapper classes
HEADERS += qscobject.h  qscontentstream.h  qspdf.h  qspage.h  qsdict.h
SOURCES += qscobject.cc qscontentstream.cc qspdf.cc qspage.cc qsdict.cc
HEADERS += qstreeitem.h  qsmenu.h  qsarray.h  qsstream.h  qsiproperty.h  qspdfoperator.h
SOURCES += qstreeitem.cc qsmenu.cc qsarray.cc qsstream.cc qsiproperty.cc qspdfoperator.cc
HEADERS += qsipropertyarray.h  qspdfoperatorstack.h  qstreeitemcontentstream.h
SOURCES += qsipropertyarray.cc qspdfoperatorstack.cc qstreeitemcontentstream.cc
HEADERS += qspdfoperatoriterator.h  qsannotation.h
SOURCES += qspdfoperatoriterator.cc qsannotation.cc

#QSA Helper classes
HEADERS += qsimporter.h  qswrapper.h
SOURCES += qsimporter.cc qswrapper.cc

#Exceptions
HEADERS += invalidmenuexception.h  nullpointerexception.h
SOURCES += invalidmenuexception.cc nullpointerexception.cc

#Toolbar
HEADERS += revisiontool.h  colortool.h  zoomtool.h  pagetool.h  toolbutton.h  toolbar.h  toolfactory.h  
HEADERS += edittool.h  numbertool.h  selecttool.h
SOURCES += revisiontool.cc colortool.cc zoomtool.cc pagetool.cc toolbutton.cc toolbar.cc toolfactory.cc
SOURCES += edittool.cc numbertool.cc selecttool.cc

#Main Window
HEADERS += pdfeditwindow.h  commandwindow.h  pagespace.h  pageviewS.h  statusbar.h  progressbar.h
SOURCES += pdfeditwindow.cc commandwindow.cc pagespace.cc pageviewS.cc statusbar.cc progressbar.cc

#Commandline mode
HEADERS += consolewindow.h
SOURCES += consolewindow.cc

#Other source files
HEADERS += pdfutil.h  util.h  menu.h  settings.h  iconcache.h  args.h  main.h 
SOURCES += pdfutil.cc util.cc menu.cc settings.cc iconcache.cc args.cc main.cc
HEADERS += selfdestructivewidget.h  rect2Darray.h  staticsettings.h  pageviewmode.h
SOURCES += selfdestructivewidget.cc rect2Darray.cc staticsettings.cc pageviewmode.cc
HEADERS += operatorhint.h
SOURCES += operatorhint.cc

#Scripting base files
HEADERS += base.h  basecore.h  basegui.h  baseconsole.h
SOURCES += base.cc basecore.cc basegui.cc baseconsole.cc
HEADERS += consolewriter.h  consolewritergui.h  consolewriterconsole.h
SOURCES += consolewriter.cc consolewritergui.cc consolewriterconsole.cc

#Misc. headers
HEADERS += types.h version.h config.h

#Dummy header file for menu translation, needed by lupdate
exists( .menu-trans.h  ) {
 HEADERS += .menu-trans.h 
}

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
