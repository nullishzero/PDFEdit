# Project file for PDF Editor
TEMPLATE = app
TARGET = pdfedit

# We want to have debug functionality and warnings under controll
QMAKE_CFLAGS -= -g -Wall
QMAKE_CXXFLAGS -= -g -Wall
QMAKE_CFLAGS_RELEASE -= -g -Wall
QMAKE_CXXFLAGS_RELEASE -= -g -Wall

# include basic definitions from configuration process
include(../../Makefile.flags)

# check debug/release
contains( E_RELEASE, no ) {
 # debug mode
 # turns off optimalizations
 CONFIG -= release
 CONFIG -= warn_off
 CONFIG *= debug
 CONFIG *= warn_on
}
contains( E_RELEASE, yes ) {
 # release mode
 # turns on optimalizations
 CONFIG -= debug
 CONFIG -= warn_on
 CONFIG *= warn_off
 CONFIG *= release
}

# cygwin hack 
# FIXME where do we define this? Is it qmake feature?
#contains( QMAKE_CYGWIN_EXE, 1 ) {
#TARGET   = pdfedit.exe
#}


# Needed for Qt4. Qt3's Qmake does not know this variable, so it is ignored
# Note Qt4 is not (yet) supported
QT += qt3support

# QT_CLEAN_NAMESPACE must be specified, otherwise namespace debug will clash with debug() in QT
QMAKE_CXXFLAGS += -DQT_CLEAN_NAMESPACE -fexceptions

# Check installation prefix
isEmpty( PREFIX ) {
 message("No prefix defined - check Makefile.flags in top-level directory")
 message("Run ./configure there if the file does not exist")
 error("PREFIX not defined");
}

# Translations
DEPENDPATH += lang
TRANSLATIONS += lang/pdfedit_cs.ts lang/pdfedit_sk.ts lang/pdfedit_es.ts lang/pdfedit_ru.ts lang/pdfedit_de.ts lang/pdfedit_fr.ts lang/pdfedit_zh_TW.ts

# All .qm files must exist before continuing, otherwise translations may be omitted from installing. (qmake bug)
# In release, qm file are already compiled, so lrelease is not necessary
DUMMY = $$system(lrelease $$TRANSLATIONS 2>/dev/null)

# Installation details

# Binary file installed in binary path (typically /usr/bin)
target.path    = $$BIN_PATH

TARGETDEPS	+= $(MANDATORY_LIB_FILES)

# Data files installed in application data path (typically /usr/share/pdfedit)
# Basic data files
data.path       = $$DATA_PATH
data.files      = pdfeditrc *.conf *.qs operator.hint
# Icons
data_icon.path  	= $$DATA_PATH/icon
data_icon.files 	= icon/*.png icon/*.svg
# Hi-color icon theme
data_icon_hicolor.path  = $$DATA_PATH/icon/hicolor
data_icon_hicolor.files = icon/hicolor/*.png 
# Tango icon theme
data_icon_tango.path  = $$DATA_PATH/icon/Tango
data_icon_tango.files = icon/Tango/*.png 
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
#doc.path        = $$DOC_PATH
#doc.files       = ../../doc/user/*.html ../../doc/user/*.pdf  

# FIXME what is difference between user/images and user/gui/images?
#doc_images.path	= $$DOC_PATH/images
#doc_images.files	= ../../doc/user/images/*.png
#doc_gui_images.path	= $$DOC_PATH/gui/images
#doc_gui_images.files	= ../../doc/user/gui/images/*.png
#doc_gui_m_images.path	= $$DOC_PATH/gui/menuAndToolbarsFun/images
#doc_gui_m_images.files	= ../../doc/user/gui/menuAndToolbarsFun/images/*.png

# TODO where are examples
#doc_examples.path 	= $$DOC_PATH/examples
#doc_examples.files	= ../../doc/examples/*.qs

# List of installed targets
INSTALLS  = data data_icon data_icon_hicolor data_icon_tango data_help data_help_cs data_help_images data_lang data_scripts
#INSTALLS += doc doc_images doc_gui_images doc_gui_m_images
#INSTALLS += doc_examples
INSTALLS += target 

# too complicated for small utility.
menugenerator.target     = menugenerator
menugenerator.commands   = $(LINK) $(LFLAGS) -o menugenerator .obj/menugenerator.o .obj/util.o .obj/staticsettings.o $(LIBS)
menugenerator.depends    = .obj/menugenerator.o .obj/util.o .obj/staticsettings.o
menugenerator_o.target   = .obj/menugenerator.o
menugenerator_o.commands = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/menugenerator.o menugenerator.cc
menugenerator_o.depends  = menugenerator.cc menugenerator.h
QMAKE_EXTRA_UNIX_TARGETS += menugenerator menugenerator_o
# It is not necessary to build this utility for installation. Commented out
# POST_TARGETDEPS = menugenerator
QMAKE_CLEAN += .obj/menugenerator.o

# Dialogs
HEADERS += additemdialog.h  aboutwindow.h  option.h  optionwindow.h  dialog.h  imagewidget.h
SOURCES += additemdialog.cc aboutwindow.cc option.cc optionwindow.cc dialog.cc imagewidget.cc
HEADERS += stringoption.h  realoption.h  intoption.h  booloption.h  combooption.h
SOURCES += stringoption.cc realoption.cc intoption.cc booloption.cc combooption.cc
HEADERS += dialogoption.h  fileoption.h  fontoption.h
SOURCES += dialogoption.cc fileoption.cc fontoption.cc
SOURCES += annotdialog.cc
HEADERS += annotdialog.h
HEADERS += mergeform.h selectpagesdialog.h
SOURCES += mergeform.cc selectpagesdialog.cc
SOURCES += version.cc

# Help window
HEADERS += helpwindow.h
SOURCES += helpwindow.cc

# Tree window
HEADERS += treeitemabstract.h  treewindow.h  treedata.h  multitreewindow.h
SOURCES += treeitemabstract.cc treewindow.cc treedata.cc multitreewindow.cc
# Drag and drop component (disabled)
#HEADERS += draglistview.h
#SOURCES += draglistview.cc
# Tree item observers
HEADERS += treeitemcontentstreamobserver.h  treeitemobserver.h  treeitempageobserver.h treeitemgenericobserver.h
# Tree item types
HEADERS += treeitemref.h  treeitemarray.h  treeitemsimple.h  treeitemdict.h  treeitempage.h
SOURCES += treeitemref.cc treeitemarray.cc treeitemsimple.cc treeitemdict.cc treeitempage.cc
HEADERS += treeitemcstream.h  treeitempdf.h  treeitem.h  treeitemcontentstream.h
SOURCES += treeitemcstream.cc treeitempdf.cc treeitem.cc treeitemcontentstream.cc
HEADERS += treeitempdfoperator.h  treeitemoperatorcontainer.h  treeitemoutline.h
SOURCES += treeitempdfoperator.cc treeitemoperatorcontainer.cc treeitemoutline.cc
HEADERS += treeitemannotation.h  treeitemannotationcontainer.h  
SOURCES += treeitemannotation.cc treeitemannotationcontainer.cc

# Property editor
HEADERS += refvalidator.h  propertyeditor.h  propertyfactory.h  property.h  propertymodecontroller.h
SOURCES += refvalidator.cc propertyeditor.cc propertyfactory.cc property.cc propertymodecontroller.cc
HEADERS += refpropertydialog.h
SOURCES += refpropertydialog.cc
# Property types
HEADERS += stringproperty.h  intproperty.h  boolproperty.h  nameproperty.h  realproperty.h
SOURCES += stringproperty.cc intproperty.cc boolproperty.cc nameproperty.cc realproperty.cc
HEADERS += refproperty.h
SOURCES += refproperty.cc

# QSA Wrapper classes
HEADERS += qscobject.h  qscontentstream.h  qspdf.h  qspage.h  qsdict.h
SOURCES += qscobject.cc qscontentstream.cc qspdf.cc qspage.cc qsdict.cc
HEADERS += qstreeitem.h  qsmenu.h  qsarray.h  qsstream.h  qsiproperty.h  qspdfoperator.h
SOURCES += qstreeitem.cc qsmenu.cc qsarray.cc qsstream.cc qsiproperty.cc qspdfoperator.cc
HEADERS += qsipropertyarray.h  qspdfoperatorstack.h  qstreeitemcontentstream.h
SOURCES += qsipropertyarray.cc qspdfoperatorstack.cc qstreeitemcontentstream.cc
HEADERS += qspdfoperatoriterator.h  qsannotation.h
SOURCES += qspdfoperatoriterator.cc qsannotation.cc

# QSA Helper classes
HEADERS += qsimporter.h  qswrapper.h
SOURCES += qsimporter.cc qswrapper.cc

# Exceptions
HEADERS += invalidmenuexception.h  nullpointerexception.h
SOURCES += invalidmenuexception.cc nullpointerexception.cc

# Toolbar
HEADERS += revisiontool.h  colortool.h  zoomtool.h  pagetool.h  toolbutton.h  toolbar.h  toolfactory.h  
HEADERS += edittool.h  numbertool.h  selecttool.h
SOURCES += revisiontool.cc colortool.cc zoomtool.cc pagetool.cc toolbutton.cc toolbar.cc toolfactory.cc
SOURCES += edittool.cc numbertool.cc selecttool.cc

# Main Window
HEADERS += pdfeditwindow.h  commandwindow.h  pagespace.h  pageviewS.h  statusbar.h  progressbar.h
SOURCES += pdfeditwindow.cc commandwindow.cc pagespace.cc pageviewS.cc statusbar.cc progressbar.cc

# Commandline mode
HEADERS += consolewindow.h
SOURCES += consolewindow.cc

# Other source files
HEADERS += pdfutil.h  util.h  menu.h  settings.h  iconcache.h  args.h  main.h 
SOURCES += pdfutil.cc util.cc menu.cc settings.cc iconcache.cc args.cc main.cc
HEADERS += selfdestructivewidget.h  staticsettings.h  pageviewmode.h
SOURCES += selfdestructivewidget.cc staticsettings.cc pageviewmode.cc
HEADERS += operatorhint.h  units.h  passworddialog.h
SOURCES += operatorhint.cc units.cc passworddialog.cc

# Scripting base files
HEADERS += base.h  basecore.h  basegui.h  baseconsole.h
SOURCES += base.cc basecore.cc basegui.cc baseconsole.cc
HEADERS += consolewriter.h  consolewritergui.h  consolewriterconsole.h
SOURCES += consolewriter.cc consolewritergui.cc consolewriterconsole.cc

# Misc. headers
HEADERS += types.h version.h config.h rect2Darray.h qtcompat.h

# Dummy header file for menu translation, needed by lupdate
exists( .menu-trans.h  ) {
 HEADERS += .menu-trans.h 
}

# Force configure compiler and all other programs used for installation
QMAKE_CC = $(CONFIG_CC)
QMAKE_CXX = $(CONFIG_CXX)
QMAKE_LINK = $(CONFIG_LINK)
QMAKE_LINK_SHLIB = $(CONFIG_LINK)
QMAKE_RANLIB = $(CONFIG_RANLIB)
QMAKE_AR = $(CONFIG_AR)
QMAKE_COPY = $(CONFIG_COPY)
QMAKE_COPY_FILE = $(CONFIG_COPY_FILE)
QMAKE_COPY_DIR = $(CONFIG_COPY_DIR)
QMAKE_DEL_FILE = $(CONFIG_DEL_FILE)
# qmake doesn't like QMAKE_SYMLINK and ignores it
QMAKE_SYMLINK = $(CONFIG_SYMLINK)
QMAKE_DEL_DIR = $(CONFIG_DEL_DIR)
QMAKE_MOVE = $(CONFIG_MOVE)
QMAKE_CHK_DIR_EXISTS = $(CONFIG_CHK_DIR_EXISTS)
QMAKE_MKDIR = $(CONFIG_MKDIR)

# QSA include
INCPATH += $(QSAPATH)
# QSA lib
LIBS += $(QSALIB)

# OutputDevice headers
INCLUDEPATH += $(QOUTPUTDEVPATH)
# OutputDevice library
LIBS += $(QOUTPUTDEV_LIBS)

# include headers from kernel and used by kernel
INCPATH += $(MANDATORY_PATHS)
# adds kernel libraries
LIBS += $(MANDATORY_LIBS)

# Flags from configuration
QMAKE_CFLAGS += $(CONFIG_CFLAGS)
QMAKE_CXXFLAGS += $(CONFIG_CXXFLAGS) $(EXTRA_GUI_CXXFLAGS)

# directories to creating files
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
