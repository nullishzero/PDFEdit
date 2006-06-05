#Makefile for documentation

.PHONY: clean all 

#html docs get built here as XHTML and also in "online help" as HTML
html_docs=scripting.html pdfedit.html keyboard.html cs/keyboard.html

#directory in which to place generated html help files
html_help_dir=../../src/gui/help

#generated documentation
docs=$(html_docs) $(addprefix $(html_help_dir)/,$(html_docs)) install.html pdfedit.1

all: $(docs)

clean:
	rm -f $(docs) scripting.xml keyboard.xml cs/keyboard.xml

#rule for procesing docbook documentation
#prefer using xsltproc and docbook.xsl (nicer XHTML output and is also faster)
# over docbook2html (which uses jade)
%.html: %.xml
	../tools/docbook2html $< || docbook2html -u $< 2>&1 >/dev/null || true

#generate similar documentation in HTML format in application help directory too
$(html_help_dir)/%.html: %.xml
	../tools/docbook2html $< $@ html || docbook2html -o $(html_help_dir) -u $< 2>&1 >/dev/null || true

#rule for generating manpage
%.1: %.xml ../tools/docbook2man.pl
	../tools/docbook2man.pl $< >$@

#generate by parsing header files and adding scripting API documentation
scripting.xml: scripting-src.xml scripting.gen.pl
	./scripting.gen.pl $< >$@

keyboard.xml: kbd_gen.pl ../../src/gui/pdfeditrc keyboard-src.xml
	./kbd_gen.pl <../../src/gui/pdfeditrc keyboard-src.xml >keyboard.xml

cs/keyboard.xml: kbd_gen.pl ../../src/gui/pdfeditrc cs/keyboard-src.xml
	./kbd_gen.pl <../../src/gui/pdfeditrc cs/keyboard-src.xml ../../src/gui/lang/pdfedit_cs.ts >cs/keyboard.xml