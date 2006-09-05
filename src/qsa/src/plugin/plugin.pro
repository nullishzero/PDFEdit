isEmpty(QSA_INSTALL_PREFIX) {
  QSA_BUILD_PREFIX=$$(QTDIR)
} else {
  QSA_BUILD_PREFIX=../..
}
SOURCES        += qseditorplugin.cpp
DESTDIR	        = $$QSA_BUILD_PREFIX/plugins/designer
TARGET          = qseditorplugin
INCLUDEPATH    += ../../src/qsa
win32 {
    QMAKE_LIBDIR   += $$QSA_BUILD_PREFIX\lib
} else {
    QMAKE_LIBDIR   += $$QSA_BUILD_PREFIX/lib
}

!isEmpty(QSA_INSTALL_PREFIX)
  target.path=$$plugins.path/designer

INSTALLS    += target
TEMPLATE     = lib
CONFIG      += qt warn_on plugin
load( ../qsa/qsa )
