# includes basic rules for building 
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../../
include $(REL_ADDR)/Makefile.rules

# list of utils modules used by all targets
UTILS_SRCS = 
UTILS_OBJS = $(UTILS_SRCS:.cc=.o)

# sources for benchmark modules
TARGET_SRCS = displaycs.cc
SOURCES = $(UTILS_SRCS) $(TARGET_SRCS)

TARGET = displaycs
.PHONY: all clean
all: $(TARGET)

displaycs: displaycs.o
	$(LINK) $(LDFLAGS) -o displaycs displaycs.o $(UTILS_OBJS) $(MANDATORY_LIBS)

clean: 
	-rm $(UTILS_OBJS) || true
	rm *.o $(TARGET)

deps:
	$(CXX) $(MANDATORY_INCPATH) -M -MF deps $(SOURCES)

include deps
