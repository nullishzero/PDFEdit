###############################################################################
# Make file for os header files installation - there are no modules to compile
# so we need only {un}install-dev targets here
# We could place it also to another Makefile but it is much cleaner to have
# Makefiles with only one directory responsibility
################################################################################
# includes basic building rules
# REL_ADDR has to be defined, because Makefile.rules refers
# to the Makefile.flags
REL_ADDR = ../../../
include $(REL_ADDR)/Makefile.rules

HEADERS = \
	basic_algos.h


install-dev:
	$(MKDIR) $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/algorithms
	$(COPY_FILE) $(HEADERS) $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/algorithms

uninstall-dev:
	cd $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/algorithms && $(DEL_FILE) $(HEADERS)
	$(DEL_DIR)  $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/algorithms
