# top-level makefile for PDF editor

.PHONY: distclean clean doc-dist source install install-no all uninstall \
	uninstall-no install-gui uninstall-gui doc-dist-install \
	doc-dist-uninstall check-configure

# includes basic building rules
include Makefile.rules

# make PDF editor - source and documentation
all: check-configure source 

check-configure: configure

configure: configure.in
	@echo configure script is outdated. You should do autoconf and
	@echo rerun ./configure script 
	exit 1

# Common install target. It depends on configuration which specific 
# installation target will be used.
install:  $(INSTALL_TARGET) doc-dist-install 

install-no:
	@echo There is no installation target defined!!! You probably have
	@echo not configured PDFedit with installation target. 
	exit 1

# Installs gui part
install-gui: source 
	cd $(GUIROOT) && $(MAKE) -f Makefile.gui install

# TODO - maybe we can create install-dev target too later when we can 
# extract also some functionality from gui (e.g. scripting layer)

# Installs core development package consisting of header files and
# configuration script for 3rd party applications depending on kernel
# (non-GUI) functionality
install-core-dev: pdfedit-core-dev
	$(MKDIR) $(INSTALL_ROOT)$(INCLUDE_PATH)
	$(MKDIR) $(INSTALL_ROOT)$(LIB_PATH)
	cd $(XPDFROOT) && $(MAKE) install-dev
	cd $(UTILSROOT) && $(MAKE) install-dev
	cd $(KERNELROOT) && $(MAKE) install-dev
	cd $(OSROOT) && $(MAKE) install-dev


# Common uninstall target. It depends on configuration which specific
# uninstallation target will be used
uninstall:  $(UNINSTALL_TARGET) uninstall-gui doc-dist-uninstall

uninstall-no:
	@echo There is no uninstallation target defined!!! You probably have
	@echo not configured PDFedit with installation target so that 
	@echo uninstallation is neither evailable. 
	exit 1

# Uninstalls gui part
uninstall-gui:
	cd $(GUIROOT) && $(MAKE) -f Makefile.gui uninstall

# TODO - maybe we can create uninstall-dev target too later when we can 
# extract also some functionality from gui (e.g. scripting layer)

# Uninstalls core dev packages
# DEL_DIR here is just to be super-sure that we don't forget anything.
# In fact we should put some assert here that those directories don't
# exist because if all dedicated uninstall-devs are correct there 
# shouldn't be nothing left
uninstall-core-dev:
	cd $(XPDFROOT) && $(MAKE) uninstall-dev
	cd $(UTILSROOT) && $(MAKE) uninstall-dev
	cd $(KERNELROOT) && $(MAKE) uninstall-dev
	cd $(OSROOT) && $(MAKE) uninstall-dev
	$(DEL_DIR) $(INSTALL_ROOT)$(INCLUDE_PATH) || true
	$(DEL_DIR) $(INSTALL_ROOT)$(LIB_PATH) || true

# Target for distributed documentation. 
doc-dist:
	cd $(DOCROOT) && $(MAKE) doc_dist

# Target for distributed documentation (un)installation 
doc-dist-install: doc-dist
	cd $(DOCROOT) && $(MAKE) doc_dist_install

doc-dist-uninstall:
	cd $(DOCROOT) && $(MAKE) doc_dist_uninstall

# make application
source: 
	cd $(SRCROOT) && $(MAKE)

# make all static libraries for pdfedit-core-dev package
pdfedit-core-dev:
	cd $(SRCROOT) && $(MAKE) pdfedit-core-dev

cscope:
	( git ls-files '*.[h,c]' '*.cc' '*.cpp' 2>/dev/null || find . \( -name .git -type d -prune \) -o \( -name '*.[h,c]' -type f -print \) -o \( -name '*.cpp' -type f -print \) -o \( -name "*.cc" -type f -print \) ) | xargs cscope -b

# cleanup
clean:
	cd $(DOCROOT) && $(MAKE) clean || true
	cd $(SRCROOT) && $(MAKE) clean || true
	$(DEL_FILE) config.log

# dist cleanup
distclean:
	cd $(DOCROOT) && $(MAKE) clean || true
	cd $(SRCROOT) && $(MAKE) distclean || true
	$(DEL_FILE) config.status config.log Makefile.flags Makefile.rules|| true
	$(DEL_FILE) autom4te.cache/* || true
	$(DEL_DIR) autom4te.cache || true
