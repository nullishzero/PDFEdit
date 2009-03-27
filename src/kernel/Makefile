# General definitions
# includes basic building rules
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../
include $(REL_ADDR)/Makefile.rules

####### Files
CFLAGS   += $(EXTRA_KERNEL_CFLAGS)
CXXFLAGS += $(EXTRA_KERNEL_CXXFLAGS)

HEADERS = static.h\
	  exceptions.h modecontroller.h xpdf.h utils.h cxref.h xrefwriter.h \
	  factories.h pdfwriter.h indiref.h iproperty.h cobject.h cobjectsimple.h \
	  cobjectsimpleI.h carray.h cdict.h cstream.h cstreamsxpdfreader.h \
	  cobjecthelpers.h ccontentstream.h pdfoperatorsbase.h pdfoperators.h pdfoperatorsiter.h \
	  displayparams.h textsearchparams.h  \
	  cpage.h cpageattributes.h cpagechanges.h cpagefonts.h cpagedisplay.h cpagecontents.h contentschangetag.h cpageannots.h cpagemodule.h \
	  cpdf.h streamwriter.h cinlineimage.h coutline.h \
	  stateupdater.h cannotation.h textoutput.h textoutputbuilder.h \
	  textoutputentities.h textoutputengines.h	\
	  delinearizator.h flattener.h pdfspecification.h operatorhinter.h \
	  pdfedit-core-dev.h

SOURCES = static.cc xpdf.cc modecontroller.cc factories.cc cannotation.cc \
	  cxref.cc xrefwriter.cc streamwriter.cc iproperty.cc carray.cc \
	  cdict.cc cstream.cc cobject.cc cobject2xpdf.cc cobject2string.cc cobjecthelpers.cc \
	  ccontentstream.cc pdfoperatorsbase.cc  pdfoperators.cc pdfoperatorsiter.cc \
	  stateupdater.cc pdfwriter.cc cinlineimage.cc coutline.cc \
	  cpage.cc cpageattributes.cc cpagechanges.cc cpagefonts.cc cpagedisplay.cc cpagecontents.cc contentschangetag.cc cpageannots.cc \
	  cpdf.cc textoutputengines.cc textoutputentities.cc \
	  textoutputbuilder.cc pdfspecification.cc \
	  delinearizator.cc flattener.cc \
	  pdfedit-core-dev.cc 

OBJECTS = $(SOURCES:.cc=.o)
# FIXME use LIBPREFIX

TARGET   = libkernel.a

# Configuration script name
DEV_CONFIG = pdfedit-core-dev-config

# Template for configuration script generation
DEV_CONFIG_TMPL = pdfedit-core-dev-config.tmpl

####### Build rules

all: $(TARGET) 

staticlib: $(TARGET)


deps: $(HEADERS)
	$(CXX) $(MANDATORY_INCPATH) -M -MF deps $(SOURCES)

$(TARGET): deps $(OBJECTS)
	-$(DEL_FILE) $(TARGET)
	$(AR) $(TARGET) $(OBJECTS)
	$(RANLIB) $(TARGET)

.PHONY: dist clean disclean
dist: 
	@mkdir -p .obj/kernel && \
		$(COPY_FILE) --parents $(SOURCES) $(HEADERS) .obj/kernel/ \
		&& ( cd `dirname .obj/kernel` \
		&& $(TAR) kernel.tar kernel \
		&& $(GZIP) kernel.tar ) \
		&& $(MOVE) `dirname .obj/kernel`/kernel.tar.gz . \
		&& $(DEL_FILE) -r .obj/kernel

# Generates pdfedit-core-dev-config script from template
.PHONY: $(DEV_CONFIG)
$(DEV_CONFIG): 
	sed     -e 's@\(^ *prefix=\).*@\1"$(PREFIX)"@'\
		-e 's@\(^ *exec_prefix=\).*@\1"$(EPREFIX)"@'\
		-e 's@\(^ *cflags=\).*@\1"$(CXX_EXTRA) $(DIST_INCPATH)"@'\
		-e 's@\(^ *ldflags=\).*@\1"$(DIST_LIBS)"@'\
		-e 's@\(^ *version=\).*@\1"$(version)"@' $(DEV_CONFIG_TMPL) > $(DEV_CONFIG)
	chmod 755 $(DEV_CONFIG)

.PHONY: install-dev uninstall-dev
install-dev: staticlib $(DEV_CONFIG)
	$(MKDIR) $(INSTALL_ROOT)$(INCLUDE_PATH)/kernel
	$(COPY_FILE) $(HEADERS) $(INSTALL_ROOT)$(INCLUDE_PATH)/kernel
	$(MKDIR) $(INSTALL_ROOT)$(LIB_PATH)/kernel
	$(COPY_FILE) $(TARGET) $(INSTALL_ROOT)$(LIB_PATH)/kernel
	$(MKDIR) $(INSTALL_ROOT)$(BIN_PATH)
	$(COPY_FILE) $(DEV_CONFIG) $(INSTALL_ROOT)$(BIN_PATH)

uninstall-dev:
	cd $(INSTALL_ROOT)$(INCLUDE_PATH)/kernel/ && $(DEL_FILE) $(HEADERS)
	$(DEL_DIR)  $(INSTALL_ROOT)$(INCLUDE_PATH)/kernel/
	cd $(INSTALL_ROOT)$(LIB_PATH)/kernel/ && $(DEL_FILE) $(TARGET)
	$(DEL_DIR)  $(INSTALL_ROOT)$(LIB_PATH)/kernel/
	$(DEL_FILE) $(INSTALL_ROOT)$(BIN_PATH)/$(DEV_CONFIG)

clean:
	-$(DEL_FILE) $(OBJECTS) deps
	-$(DEL_FILE) *~ core *.core

distclean: clean
	-$(DEL_FILE) $(TARGET)


# This requires GNU make (or compatible) because deps file doesn't
# exist in time when invoked for the first time and thus has to
# be generated
include deps
