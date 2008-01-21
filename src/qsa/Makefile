# -*- makefile -*-
#
# Main Makefile for building the QSA library.

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
	cd configure2 && rm *.o *.moc

distclean: clean
	$(MAKE) -f Makefile.qsa distclean
	rm -rf .moc .obj configure2/.moc configure2/.obj src/qsa/.obj src/qsa/.moc 
	rm -f configure2/configure2 src/qsa/qsconfig.h configure2/Makefile
	rm -f Makefile.qsa .qmake.cache lib/libqsa_pdfedit.prl lib/libqsa_pdfedit.a src/Makefile src/qsa/Makefile tools/designer/editor/preferences.h tools/designer/editor/preferences.cpp

staticlib: run-make
