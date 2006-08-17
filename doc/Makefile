#Top-level Makefile for documentation

.PHONY: clean all doc_core doc_core_nopdf doc_dist

#Build all documentation, including doxygen programmer documentation and doxygen's default stylesheet
all:	doc_core
	doxygen -w html /dev/null /dev/null programmer/html/doxygen.css
	doxygen

#Core documentation, without PDF
doc_core_nopdf:
	cd user && ( gmake all_no_pdf || make all_no_pdf )
	cd design/kernel && ( gmake all_no_pdf || make all_no_pdf )
	cd design && ( gmake all_no_pdf || make all_no_pdf )

#Documentation that will be build before packing in distributiin tarball
doc_dist:	doc_core_nopdf

#Core documentation, with PDF
doc_core:
	cd user && ( gmake || make )
	cd design/kernel && ( gmake || make )
	cd design && ( gmake || make )

#Clean everything
clean:
	cd user && ( gmake clean || make clean )
	cd design/kernel && ( gmake clean || make clean )
	cd design && ( gmake clean || make clean )
	rm -rf programmer
