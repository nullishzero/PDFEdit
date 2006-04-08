#top-level makefile for PDF editor

.PHONY: clean doc src install all regen

#make PDF editor - source and documentation
all: doc src

#install PDF editor
install: all
	cd src && make install

#make documentation
doc:
	cd doc/user && make

#make application
src:
	cd src && make

#cleanup
clean:
	cd doc/user && make clean
	cd src && make clean

#re-generate programmer documentation
regen:
	cd doc && doxygen