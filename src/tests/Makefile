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
	cd $(KERNELTESTSROOT) && $(MAKE) kernel_tests

clean:
	cd $(KERNELTESTSROOT) && $(MAKE) clean

distclean: 
	cd $(KERNELTESTSROOT) && $(MAKE) distclean
