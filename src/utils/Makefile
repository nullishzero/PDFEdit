###############################################################################
# Make file for utils static library
#
################################################################################
# includes basic building rules
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../
include $(REL_ADDR)/Makefile.rules

C_FLAGS += $(EXTRA_UTILS_CFLAGS)
CXX_FLAGS += $(EXTRA_UTILS_CXXFLAGS)

# Source files for library
SOURCES=debug.cc \
     	delinearizator.cc \
	confparser.cc

# Binary files to be included to the library
BINS=debug.o \
     delinearizator.o \
     confparser.o

# FIXME use LIBPREFIX
TARGET = libutils.a

all: $(TARGET)
$(TARGET): $(BINS)
	-$(DEL_FILE) $(TARGET)
	$(AR) $(TARGET) $(BINS)
	$(RANLIB) $(TARGET)

.PHONY: clean distclean
clean:
	$(DEL_FILE) $(BINS)

distclean: clean
	$(DEL_FILE) $(TARGET)
