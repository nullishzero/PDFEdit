# includes basic rules for building 
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../../
include $(REL_ADDR)/Makefile.rules


MANDATORY_LIBS    += $(CPPUNIT_LIBS) 
MANDATORY_INCPATH += $(CPPUNITPATH)

SOURCES = testcobjectsimple.cc \
		testcobjectcomplex.cc \
		testcstream.cc \
		teststream.cc \
		teststreamwriter.cc \
		testcobjecthelpers.cc \
		testpdfoperators.cc \
		testcpage.cc \
		testccontentstream.cc \
		testcpdf.cc \
		testutils.cc \
		testoutlines.cc \
		testtextoutput.cc \
		testparams.cc \
		main.cc
HEADERS = testcobject.h \
	  testcpage.h \
	  testcpdf.h \
	  testmain.h \
	  testparams.h
DEP_LIBS = $(KERNELROOT)/libkernel.a \
	   $(UTILSROOT)/libutils.a \
	   $(XPDFROOT)/xpdf/libxpdf.a $(XPDFROOT)/fofi/libfofi.a \
	   $(XPDFROOT)/goo/libGoo.a $(XPDFROOT)/splash/libsplash.a

OBJECTS = $(SOURCES:.cc=.o)
TARGET   = kernel_tests

all: $(TARGET)

$(TARGET): deps $(OBJECTS) $(DEP_LIBS)
	$(LINK) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(MANDATORY_LIBS)

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core

distclean: clean
	-$(DEL_FILE) $(TARGET)

deps: $(HEADERS)
	$(CXX) $(MANDATORY_INCPATH) -M $(SOURCES) > deps

# This requires GNU make (or compatible) because deps file doesn't
# exist in time when invoked for the first time and thus has to
# be generated
include deps
