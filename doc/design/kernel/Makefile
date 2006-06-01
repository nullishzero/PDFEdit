#
#Makefile for documentation
#

.PHONY: clean all

docs=observers.html pdfoperators.html cobjects.html cpdf.html document_changes.html xpdf_modifications.html filters.html

all: $(docs)

clean:
	rm -f $(docs)

#rule for procesing docbook documentation
#prefer using xsltproc and docbook.xsl (nicer output and is also faster)
# over docbook2html (which uses jade)
%.html: %.xml
	../../tools/docbook2html $< 2>/dev/null || docbook2html -u $< 2>&1 >/dev/null ||  true
