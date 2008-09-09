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
	basic_types.h \
	coordinates.h


install-dev:
	$(MKDIR) $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/types
	$(COPY_FILE) $(HEADERS) $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/types

uninstall-dev:
	cd $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/types/ && $(DEL_FILE) $(HEADERS)
	$(DEL_DIR)  $(INSTALL_ROOT)$(INCLUDE_PATH)/utils/types
