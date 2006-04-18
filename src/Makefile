#makefile for PDF editor
.PHONY: clean pdfedit kernel xpdf kpdf-kde-3.3.2

#make PDF editor - source and documentation
all: pdfedit

# PDF editor
pdfedit: kernel xpdf qoutputdevices
	cd gui && qmake && make

# QOutputDevices
qoutputdevices:
	cd kpdf-kde-3.3.2 && qmake && make staticlib

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
	cd kpdf-kde-3.3.2 && make clean

# Installation
install:
	cd gui && make install
