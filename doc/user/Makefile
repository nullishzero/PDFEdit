# Makefile for documentation
# TODO clean up - remove all unused stuff from here.
# 	we would like to keep onlu online_help and all (all_no_pdf)
# 	targets and everything else should go away

# root of doc/ directory
TOP_DIR=..

TOOL_DIR=$(TOP_DIR)/tools

# html docs get built here as PDF, XHTML and also in "online help" as HTML
# html_docs=scripts/scripting.html pdfedit.html
html_docs= keyboard.html cs/keyboard.html qsa-util.html qsa-inputdialog.html scripting.html

# html docs that won't get into online help
html_docs_all=user_doc.html appendix.html

# directory in which to place generated html help files
html_help_dir=../../src/gui/help

# PDF documents to generate
pdf_docs_all=$(html_docs_all:.html=.pdf)

# generated documentation
html_help_docs=$(addprefix $(html_help_dir)/,$(html_docs)) $(html_help_dir)/pdfedit.html
docs_no_pdf=$(html_docs_all)
docs=$(docs_no_pdf) $(pdf_docs_all)
help_docs=$(html_help_docs)

all: $(all_no_pdf) 

all_no_pdf: $(docs_no_pdf)

# Generates online help
online_help: $(help_docs)

clean:
	rm -f $(docs) scripts/scripting.xml keyboard.xml cs/keyboard.xml pdfedit.xml qsa-inputdialog.xml qsa-util.xml scripts/qsa-util.xml scripts/qsa-inputdialog.xml appendix.xml

# generate by parsing header files and adding scripting API documentation
scripts/scripting.xml: scripts/scripting-src.xml $(TOOL_DIR)/scripting.gen.pl
	$(TOOL_DIR)/scripting.gen.pl $< >$@

scripting.xml: scripts/scripting.xml version.xml

appendix.xml: appendix-src.xml version.xml $(TOOL_DIR)/scripting.gen.pl
	$(TOOL_DIR)/scripting.gen.pl $< >$@

# dependencies

install_deps=install/compiling.xml \
install/install.xml \
install/platforms.xml \
install/requiredlib.xml

scripting_deps=scripts/scripting_nodoc.xml \
scripts/qsa-util.xml scripts/qsa-inputdialog.xml version.xml

user_doc.html: $(scripting_deps) $(install_deps) qsa-inputdialog.xml qsa-util.xml version.xml

# generate from template by adding table of keyboard shortcuts parsed from pdfeditrc
keyboard.xml: $(TOOL_DIR)/kbd_gen.pl ../../src/gui/pdfeditrc keyboard-src.xml
	$(TOOL_DIR)/kbd_gen.pl <../../src/gui/pdfeditrc keyboard-src.xml >keyboard.xml

cs/keyboard.xml: $(TOOL_DIR)/kbd_gen.pl ../../src/gui/pdfeditrc cs/keyboard-src.xml
	$(TOOL_DIR)/kbd_gen.pl <../../src/gui/pdfeditrc cs/keyboard-src.xml ../../src/gui/lang/pdfedit_cs.ts >cs/keyboard.xml

pdfedit.xml: cmdline/pdfedit.head cmdline/description.xml cmdline/localization.xml cmdline/pdfedit.tail
	cat $+ >$@

# special rules rule for generating docbook XML from .leaf files (some tex-like Trolltech format)
scripts/qsa-inputdialog.xml: qsa-inputdialog.leaf $(TOP_DIR)/tools/leaf2xml.pl
	$(TOP_DIR)/tools/leaf2xml.pl <$< >$@ no

scripts/qsa-util.xml: qsa-util.leaf $(TOP_DIR)/tools/leaf2xml.pl
	$(TOP_DIR)/tools/leaf2xml.pl <$< >$@ no

# refresh version number
version.xml: 
	cd ../.. && ./getversion -v >doc/user/$@

# include rules to generate various documentation formats
include $(TOP_DIR)/doc_format.rules
