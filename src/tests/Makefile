# Makefile for all tests for PDFedit
#
# includes basic building rules
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR=../../
include $(REL_ADDR)Makefile.rules

.PHONY: distclean clean kernel_tests

# TODO add install and uninstall targets depending on configuration

# Compiles only configured tests
all: $(TEST_TARGETS)

# Defined by KERNEL_TESTS_TARGET variable
kernel_tests:
	cd $(KERNELTESTSROOT) && $(MAKE) kernel_tests

clean: 
	[ -n $(KERNEL_TESTS_TARGET) ] && cd $(KERNELTESTSROOT) && $(MAKE) clean

distclean: 
	[ -n $(KERNEL_TESTS_TARGET) ] && cd $(KERNELTESTSROOT) && $(MAKE) distclean
