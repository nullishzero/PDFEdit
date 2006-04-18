#makefile for PDF editor
.PHONY: clean pdfedit kernel xpdf

#make PDF editor - source and documentation
all: pdfedit

# PDF editor
pdfedit: kernel xpdf
	cd gui && qmake && make

# Kernel
kernel: xpdf
	cd kernel && qmake && make

# XPdf
xpdf: xpdf_configure
	cd xpdf && make

# XPdf configure
xpdf_configure: xpdf/config.status

xpdf/config.status:
	cd xpdf && ./configure 

# Cleanup
clean:
	cd gui && make clean
	cd kernel && make clean
	cd xpdf && make clean

# Installation
install:
	cd gui && make install
