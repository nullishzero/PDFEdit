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
OBJECTS = $(SOURCES:.cc=.o)
TARGET   = kernel_tests

all: $(TARGET)

$(TARGET): deps $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(MANDATORY_LIBS)

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core

distclean: clean
	-$(DEL_FILE) $(TARGET)

deps: $(SOURCES)
	$(CXX) $(MANDATORY_INCPATH) -M $(SOURCES) > deps

# FIXME FreeBSD make is not able to include non existent file
#-include deps
