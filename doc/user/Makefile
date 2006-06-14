#Makefile for documentation

#html docs get built here as PDF, XHTML and also in "online help" as HTML
html_docs=scripting.html pdfedit.html keyboard.html cs/keyboard.html

#html docs that won't get into online help
html_docs_all=$(html_docs) install.html

#directory in which to place generated html help files
html_help_dir=../../src/gui/help

#PDF documents to generate
pdf_docs_all=$(html_docs_all:.html=.pdf)

#generated documentation
docs_no_pdf=$(html_docs_all) $(addprefix $(html_help_dir)/,$(html_docs)) pdfedit.1
docs=$(docs_no_pdf) $(pdf_docs_all)

all: $(docs) 

all_no_pdf: $(docs_no_pdf)

clean:
	rm -f $(docs) scripting.xml keyboard.xml cs/keyboard.xml

#generate by parsing header files and adding scripting API documentation
scripting.xml: scripting-src.xml scripting.gen.pl
	./scripting.gen.pl $< >$@

#generate from template by adding table of keyboard shortcuts parsed from pdfeditrc
keyboard.xml: kbd_gen.pl ../../src/gui/pdfeditrc keyboard-src.xml
	./kbd_gen.pl <../../src/gui/pdfeditrc keyboard-src.xml >keyboard.xml

cs/keyboard.xml: kbd_gen.pl ../../src/gui/pdfeditrc cs/keyboard-src.xml
	./kbd_gen.pl <../../src/gui/pdfeditrc cs/keyboard-src.xml ../../src/gui/lang/pdfedit_cs.ts >cs/keyboard.xml

#root of doc/ directory
TOP_DIR=..

#include rules to generate various documentation formats
include $(TOP_DIR)/Makefile.rules
