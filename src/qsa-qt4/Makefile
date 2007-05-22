# -*- makefile -*-
#
# Main Makefile for building the QSA library with examples.

all: run-make
	@echo .
	@echo The QSA library is now built.
	@echo The QSA examples are built in examples directories.
	@echo .
	@echo Enjoy!   - the Trolltech team
	@echo .

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
