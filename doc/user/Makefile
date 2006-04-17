#Makefile for documentation

.PHONY: clean

docs=scripting.html pdfedit.html install.html pdfedit.1

all: $(docs)

clean:
	rm -f $(docs)

#rule for procesing docbook documentation
%.html: %.xml
	docbook2html -u $< 2>/dev/null || true

%.1: %.xml docbook2man.pl
	./docbook2man.pl $< >$@
