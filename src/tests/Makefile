# Makefile for all tests for PDFedit
#
# includes basic building rules
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR=../../
include $(REL_ADDR)Makefile.rules

.PHONY: distclean clean kernel_tests

# TDOD add all, install and uninstall targets depending on configuration

kernel_tests:
	cd $(KERNELTESTSROOT) && make kernel_tests

clean:
	cd $(KERNELTESTSROOT) && make clean

distclean: 
	cd $(KERNELTESTSROOT) && make distclean
