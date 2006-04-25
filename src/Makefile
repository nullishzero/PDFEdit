#makefile for PDF editor
.PHONY: clean pdfedit kernel xpdf kpdf-kde-3.3.2

#make PDF editor - source and documentation
all: pdfedit

# PDF editor
pdfedit: kernel xpdf qoutputdevices
	cd gui && qmake && make

# QOutputDevices
qoutputdevices: xpdf config_xpdf_variables
	cd kpdf-kde-3.3.2 && qmake && make staticlib

# Kernel
kernel: xpdf cppunit config_xpdf_variables
	cd kernel && qmake && make

# XPdf
xpdf: xpdf_configure
	cd xpdf && make

# XPdf configure
xpdf_configure: xpdf/config.status

xpdf/config.status:
	cd xpdf && ./configure

config_xpdf_variables: xpdf/config.status
	cat xpdf/config.status | grep s,@[^@,]*@, | sed 's/^s,@\([^@,]*\)@,\([^,]*\),.*/config_xpdf_\1=\2/g' > config_xpdf_variables

# CPPunit
cppunit: cppunit_configure
	cd cppunit && make

# CPPunit configure
cppunit_configure: cppunit/config.status

cppunit/config.status:
	cd cppunit && ./configure && libtoolize --copy --force


# Cleanup
clean:
	cd gui && make clean
	cd kernel && make clean
	cd xpdf && make clean
	cd cppunit && make clean
	cd kpdf-kde-3.3.2 && make clean

# Installation
install:
	cd gui && make install
