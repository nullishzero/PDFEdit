VERSION=2.0

all: executable commit-patch.1

# Darcs loses the x bit. :-(
executable:
	chmod +x commit-patch

commit-patch.1: commit-patch
	pod2man -c "User Commands" $< > $@

commit-patch.html: commit-patch
	pod2html $< > $@

release: commit-patch-$(VERSION)

commit-patch-$(VERSION): commit-patch commit-patch-buffer.el commit-patch.1 Makefile README COPYING
	mkdir commit-patch-$(VERSION)
	cp -p $^ commit-patch-$(VERSION)
	tar czf commit-patch-$(VERSION).tar.gz commit-patch-$(VERSION)
	rm -rf commit-patch-$(VERSION)
