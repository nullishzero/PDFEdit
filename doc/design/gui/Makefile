#
#Makefile for documentation
#

#HTML documents to generate
html_docs=menu.html tree.html

#PDF documents to generate
pdf_docs=$(html_docs:.html=.pdf)

#All documents
docs=$(html_docs) $(pdf_docs)

all: $(docs)

all_no_pdf: $(html_docs)

clean:
	rm -f $(docs)

#root of doc/ directory
TOP_DIR=../..

#include rules to generate various documentation formats
include $(TOP_DIR)/Makefile.rules
