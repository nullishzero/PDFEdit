#
#Makefile for documentation
#

#HTML documents to generate
html_docs=design_doc.html

#PDF documents to generate
pdf_docs=$(html_docs:.html=.pdf)

#All documents
docs=$(html_docs) $(pdf_docs)

all: $(docs)

all_no_pdf: $(html_docs)

clean:
	rm -f $(docs)

#extra dependencies on included files

kernel_includes=kernel/cannotation.xml \
kernel/ccontentstream.xml \
kernel/cobjects.xml \
kernel/cpage.xml \
kernel/cpdf.xml \
kernel/cxref.xml \
kernel/filters.xml \
kernel/helper_objects.xml \
kernel/interface_objects.xml \
kernel/layer_model.xml \
kernel/main.xml \
kernel/modecontroller.xml \
kernel/observers.xml \
kernel/pdfoperators.xml \
kernel/pdfwriter.xml \
kernel/progressobserver.xml \
kernel/stateupdater.xml \
kernel/xrefwriter.xml \

gui_includes=gui/main.xml \
gui/menu.xml \
gui/settings.xml \
gui/tree.xml

includes=general.xml \
general_layers.xml \
general_technologies.xml \
general_utils.xml \
general_xpdf.xml \
glossary.xml \
purpose.xml \
$(gui_includes) $(kernel_includes)

design_doc.html: $(includes)

design_doc.pdf: $(includes)

#root of doc/ directory
TOP_DIR=..

#include rules to generate various documentation formats
include $(TOP_DIR)/doc_format.rules
