#Makefile for documentation

.PHONY: clean

docs=scripting.html pdfedit.html install.html pdfedit.1

all: $(docs)

clean:
	rm -f $(docs)

#rule for procesing docbook documentation
#prefer using xsltproc and docbook.xsl (nicer output and is also faster)
# over docbook2html (which uses jade)
%.html: %.xml
	../tools/docbook2html $< 2>/dev/null || docbook2html -u $< 2>&1 >/dev/null ||  true

#rule for generation manpage
%.1: %.xml ../tools/docbook2man.pl
	../tools/docbook2man.pl $< >$@

scripting.xml: scripting-src.xml scripting.gen.pl
	./scripting.gen.pl $<  >$@
