# -*- makefile -*-
#
# Main Makefile for building the QSA library with examples.

all: run-make

Makefile.qsa:
	@echo .
	@echo QSA must first be configured using the "configure" script.
	@echo .
	@exit 1	

install: Makefile.qsa
	$(MAKE) -f Makefile.qsa install

run-make: Makefile.qsa
	$(MAKE) -f Makefile.qsa

sub-src: Makefile.qsa
	$(MAKE) -f Makefile.qsa sub-src

clean:
	$(MAKE) -f Makefile.qsa clean

distclean:
	$(MAKE) -f Makefile.qsa distclean
	rm -f configure2/configure2 configure2/Makefile
	rm -f .qmake.cache lib/libqsa_pdfedit.prl lib/libqsa_pdfedit.a
	rm -f src/kernel/plugin/Makefile.qsa
	rm -f src/qsa/qsa.prf
	rm -f src/qsa/qsconfig.h

staticlib: Makefile.qsa
	$(MAKE) -f Makefile.qsa

