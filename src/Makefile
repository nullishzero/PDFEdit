#makefile for PDF editor
.PHONY: distclean clean pdfedit kernel xpdf kpdf-kde-3.3.2 utils qoutputdevices xpdf_configure install qsa qsa_configure qsa4 qsa4_configure

include Makefile.flags

#make PDF editor - source and documentation
all: pdfedit

#make PDF editor + tests
tests: all kernel-tests

# kernel tests
kernel-tests: kernel
	cd kernel && $(QMAKE) -o Makefile-tests kernel-tests.pro && make -f Makefile-tests

# PDF editor
pdfedit: ../config.pro utils kernel xpdf qoutputdevices qsa qsa4
	cd gui && $(QMAKE) && make

# QOutputDevices
qoutputdevices: xpdf xpdf/config_xpdf_variables
	cd kpdf-kde-3.3.2 && $(QMAKE) && make staticlib

# Utils
utils: xpdf
	cd utils && make

#Both QSA's are used, although the 'wrong' one will just return

#QSA for Qt3
qsa: qsa/Makefile.qsa
	cd qsa && make staticlib

qsa/Makefile.qsa: qsa/qsa.pro
	cd qsa && ./configure

#QSA for Qt4
qsa4: qsa-qt4/Makefile.qsa
	cd qsa-qt4 && make staticlib

qsa-qt4/Makefile.qsa: qsa-qt4/qsa.pro
	cd qsa-qt4 && ./configure

# Kernel
kernel: xpdf xpdf/config_xpdf_variables utils
	cd kernel && $(QMAKE) && make staticlib

# XPdf
xpdf: xpdf_configure
	cd xpdf && make libxpdf

# XPdf configure
xpdf_configure: xpdf/config_xpdf_variables

#if any of the Makefile.in changes, we need to rebuild corresponding Makefile in xpdf -> we rebuild them all to be sure
xpdf/config_xpdf_variables: xpdf/Makefile.in xpdf/xpdf/Makefile.in xpdf/splash/Makefile.in xpdf/goo/Makefile.in xpdf/fofi/Makefile.in
	cd xpdf && ./configure $(xpdf_configure_flags)

# Cleanup
clean:
	cd gui && make clean || true
	cd qsa && make clean || true
	cd qsa-qt4 && make clean || true
	cd kernel && make clean || true
	cd utils && make clean
	cd xpdf && make clean || true
	cd kpdf-kde-3.3.2 && make clean || true
	rm -f kpdf-kde-3.3.2/Makefile 
	rm -f kernel/Makefile 
	rm -f gui/Makefile gui/menugenerator gui/pdfedit


# Dist Cleanup
distclean:
	cd gui && make distclean || true
	cd qsa && make distclean || true
	cd qsa-qt4 && make distclean || true
	cd kernel && make distclean || true
	cd utils && make distclean
	cd xpdf && make distclean || true
	cd kpdf-kde-3.3.2 && make distclean || true
	rm -f xpdf/config_xpdf_variables
	rm -f kpdf-kde-3.3.2/Makefile 
	rm -f kernel/Makefile 
	rm -f utils/aconf.h
	rm -f gui/config.h gui/Makefile gui/menugenerator gui/pdfedit

# Installation
install:
	cd gui && make install

# Uninstallation
uninstall:
	cd gui && make uninstall
