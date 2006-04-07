#Makefile for documentation

.PHONY: clean

docs=scripting.html application.html install.html

all: $(docs)

clean:
	rm -f $(docs)

#rule for procesing docbook documentation
%.html: %.xml
	docbook2html -u $< 2>/dev/null || true

