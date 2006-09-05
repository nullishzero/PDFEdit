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
	$(MAKE) -f Makefile.qsa src/Makefile
	cd src && $(MAKE) qsa/Makefile
	cd src/qsa && $(MAKE) staticlib

sub-src: Makefile.qsa
	$(MAKE) -f Makefile.qsa sub-src

clean:
	$(MAKE) -f Makefile.qsa clean

distclean:
	rm -f configure2/configure2 src/qsa/qsconfig.h configure2/Makefile
	$(MAKE) -f Makefile.qsa distclean

staticlib: Makefile.qsa
	$(MAKE) -f Makefile.qsa src/Makefile
	cd src && $(MAKE) qsa/Makefile
	cd src/qsa && $(MAKE) staticlib

