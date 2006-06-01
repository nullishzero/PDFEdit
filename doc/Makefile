#Top-level Makefile for documentation

.PHONY: clean all doc_nodoxygen

all:	doc_nodoxygen
	doxygen

doc_nodoxygen:
	cd user && ( gmake || make )
	cd design/kernel && ( gmake || make )
	cd design/gui && ( gmake || make )

clean:
	cd user && ( gmake clean || make clean )
	cd design/kernel && ( gmake clean || make clean )
	cd design/gui && ( gmake clean || make clean )
	rm -rf programmer
