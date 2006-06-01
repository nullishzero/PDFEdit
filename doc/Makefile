#Top-level Makefile for documentation

.PHONY: clean all

all:
	cd user && ( gmake || make )
	cd design/kernel && ( gmake || make )
	cd design/gui && ( gmake || make )
	doxygen

clean:
	cd user && ( gmake clean || make clean )
	cd design/kernel && ( gmake clean || make clean )
	cd design/gui && ( gmake clean || make clean )
	rm -rf programmer
