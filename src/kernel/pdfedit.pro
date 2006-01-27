#Project file
TEMPLATE = app

#
# Kernel special settings
#
HEADERS += cobject.h iproperty.h ../utils/debug.h cxref.h
SOURCES += main.cc cobject.cc iproperty.cc
INCLUDEPATH += ../xpdf/ ../xpdf/xpdf ../xpdf/goo ../utils ../
CONFIG += warn_on fexceptions
