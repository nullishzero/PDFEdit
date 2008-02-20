# General definitions
# includes basic building rules
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../
include $(REL_ADDR)/Makefile.rules

####### Files
CFLAGS   += $(EXTRA_KERNEL_CFLAGS)
CXXFLAGS += $(EXTRA_KERNEL_CXXFLAGS)

HEADERS = static.h $(UTILSROOT)/debug.h $(UTILSROOT)/iterator.h $(UTILSROOT)/observer.h\
	  exceptions.h modecontroller.h filters.h xpdf.h utils.h cxref.h xrefwriter.h \
	  factories.h pdfwriter.h indiref.h iproperty.h cobject.h cobjectsimple.h \
	  cobjectsimpleI.h carray.h cdict.h cstream.h cstreamsxpdfreader.h \
	  cobjecthelpers.h ccontentstream.h pdfoperators.h pdfoperatorsiter.h \
	  displayparams.h textsearchparams.h  \
	  cpage.h cpdf.h streamwriter.h cinlineimage.h coutline.h \
	  stateupdater.h cannotation.h textoutput.h textoutputbuilder.h \
	  textoutputentities.h textoutputengines.h	\
	  delinearizator.h pdfspecification.h

SOURCES = static.cc xpdf.cc modecontroller.cc filters.cc factories.cc cannotation.cc \
	  cxref.cc xrefwriter.cc streamwriter.cc iproperty.cc carray.cc \
	  cdict.cc cstream.cc cobject.cc cobject2xpdf.cc cobject2string.cc cobjecthelpers.cc \
	  ccontentstream.cc pdfoperators.cc pdfoperatorsiter.cc \
	  stateupdater.cc pdfwriter.cc cinlineimage.cc coutline.cc \
	  cpage.cc cpdf.cc textoutputengines.cc textoutputentities.cc \
	  textoutputbuilder.cc pdfspecification.cc \
	  delinearizator.cc

OBJECTS = $(SOURCES:.cc=.o)
# FIXME use LIBPREFIX
TARGET   = libkernel.a

####### Build rules

all: $(TARGET) 

staticlib: $(TARGET)

deps: $(HEADERS)
	$(CXX) $(MANDATORY_INCPATH) -M $(SOURCES) > deps

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

clean:
	-$(DEL_FILE) $(OBJECTS) deps
	-$(DEL_FILE) *~ core *.core

distclean: clean
	-$(DEL_FILE) $(TARGET)


# FIXME change this because we are force to include this even if we want
# to do make clean. FreeBSD make is not able to include non existent file
#-include deps
