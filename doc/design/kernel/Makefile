#
#Makefile for documentation
#

.PHONY: clean

docs=observers.html pdfoperators.html cobjects.html

all: $(docs)

clean:
	rm -f $(docs)

#rule for procesing docbook documentation
%.html: %.xml
	docbook2html -u $< 2>/dev/null || true

