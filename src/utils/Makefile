###############################################################################
# Make file for utils static library
#
################################################################################
# includes basic building rules
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../
include $(REL_ADDR)/Makefile.rules

CFLAGS += $(EXTRA_UTILS_CFLAGS)
CXXFLAGS += $(EXTRA_UTILS_CXXFLAGS)

# Source files for library
SOURCES=debug.cc \
	confparser.cc

# Binary files to be included to the library
BINS=debug.o \
     confparser.o

HEADERS= \
	aconf.h \
	algorithms.h \
	confparser.h \
	debug.h \
	doxygen.h \
	iterator.h \
	objectstorage.h \
	observer.h \
	rulesmanager.h \
	types.h \

# FIXME use LIBPREFIX
TARGET = libutils.a

all: $(TARGET)
$(TARGET): deps $(BINS)
	-$(DEL_FILE) $(TARGET)
	$(AR) $(TARGET) $(BINS)
	$(RANLIB) $(TARGET)

.PHONY: clean distclean
clean:
	$(DEL_FILE) $(BINS) deps

distclean: clean
	$(DEL_FILE) $(TARGET)
	
deps: $(HEADERS)
	$(CXX) $(MANDATORY_INCPATH) -M -MF deps $(SOURCES)

# This requires GNU make (or compatible) because deps file doesn't
# exist in time when invoked for the first time and thus has to
# be generated
include deps
