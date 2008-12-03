#Top-level Makefile for documentation

# General rules (with configuration options)
include ../Makefile.flags

TOP_DIR=.
# Documentation generation rules
include doc_format.rules

.PHONY: clean all doc_dist doc_man doc_doxygen doc_user doc_extended doc_user_nopdf doc_extended_nopdf

all: doc_dist

# Build man pages and doxygen doc if required
doc_dist: $(MAN_TARGET) $(DOXYGEN_TARGET) $(ADVANCED_DOC_TARGET) $(USER_MANUAL_TARGET)

doc_dist_install: $(MAN_INSTALL_TARGET) \
	$(USER_MANUAL_INSTALL_TARGET) \
	$(DOXYGEN_INSTALL_TARGET) \
	$(ADVANCED_DOC_INSTALL_TARGET)
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/;
	$(COPY_FILE) LICENSE.GPL ../README ../Changelog AUTHORS README-* \
		README.cygwin $(INSTALL_ROOT)$(DOC_PATH)

# TODO maybe we should be less brutal here and delete only our files However we
# have no list of installed doc files. This list is probably too much effort
# with not worthy benefit. Therefore we are using DEL_FORCE_DIR here which
# removes everything. DEL_DIR used immediatelly after cleans whole empty upward
# hierarchy
doc_dist_uninstall:
	$(DEL_FORCE_DIR) $(INSTALL_ROOT)$(DOC_PATH)/* || true;
	$(DEL_DIR) $(INSTALL_ROOT)$(DOC_PATH) || true
	$(DEL_FILE) $(INSTALL_ROOT)$(MAN_PATH)/pdfedit.1
	$(DEL_DIR) $(INSTALL_ROOT)$(MAN_PATH)

# Man pages are the core for documenation
doc_man: 
	cd user && $(MAKE) pdfedit.1

doc_man_install: doc_man
	$(MKDIR) $(INSTALL_ROOT)$(MAN_PATH);
	$(COPY_FILE) user/pdfedit.1 $(INSTALL_ROOT)$(MAN_PATH)

# Doxygen documenation
doc_doxygen: $(DOXYGEN_BIN)
	$(DOXYGEN_BIN) -w html _header _footer programmer/html/doxygen.css || true
	$(DEL_FILE) _header _footer _header.bak _footer.bak || true
	$(DOXYGEN_BIN) || true

doc_doxygen_install: doc_doxygen
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/doxygen;
	$(COPY_DIR) programmer/html/* $(INSTALL_ROOT)$(DOC_PATH)/doxygen

# User documentation
doc_user: doc_user_nopdf

doc_user_install: doc_user
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/manual;
	$(COPY_FILE) user/*.html $(INSTALL_ROOT)$(DOC_PATH)/manual;
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/manual/images ;
	$(COPY_FILE) user/images/*.png $(INSTALL_ROOT)$(DOC_PATH)/manual/images;
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/manual/gui/images;
	$(COPY_FILE) user/gui/images/*.png $(INSTALL_ROOT)$(DOC_PATH)/manual/gui/images;
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/manual/examples;
	$(COPY_FILE) user/examples/*.qs $(INSTALL_ROOT)$(DOC_PATH)/manual/examples
	

doc_user_nopdf:
	cd user && $(MAKE) all_no_pdf

# Extended (docbook design) documenation
doc_advanced: doc_advanced_nopdf

# TODO what about pdf generated doc?
doc_advanced_install: doc_advanced
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/design ;
	$(COPY_FILE) design/*.html $(INSTALL_ROOT)$(DOC_PATH)/design;
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/design/images ;
	$(COPY_FILE) design/images/*.png $(INSTALL_ROOT)$(DOC_PATH)/design/images;
	$(MKDIR) $(INSTALL_ROOT)$(DOC_PATH)/design/kernel/images;
	$(COPY_FILE) design/kernel/images/*.png $(INSTALL_ROOT)$(DOC_PATH)/design/kernel/images;


doc_advanced_nopdf:
	cd design && $(MAKE) all_no_pdf

# Clean everything
clean:
	cd user && $(MAKE) clean
	cd design && $(MAKE) clean
	$(DEL_FORCE_DIR) programmer
	$(DEL_FILE) doxygen.warnings
