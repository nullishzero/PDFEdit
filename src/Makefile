#makefile for PDF editor

.PHONY: clean pdfedit kernel xpdf

#make PDF editor - source and documentation
all: pdfedit

# PDF editor
pdfedit: kernel xpdf
	cd gui && qmake && make

# XPdf
kernel: xpdf
	cd kernel && qmake && make

# Kernel
xpdf:
	cd xpdf && make

# Cleanup
clean:
	cd xpdf && make clean
	cd kernel && make clean
	cd gui && make clean

# Installation
install:
	cd gui && make install