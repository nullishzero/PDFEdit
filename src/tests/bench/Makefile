# includes basic rules for building 
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../../
include $(REL_ADDR)/Makefile.rules

# list of utils modules used by all targets
UTILS_SRCS = utils.cc
UTILS_OBJS = $(UTILS_SRCS:.cc=.o)

# sources for benchmark modules
TARGET_SRCS = xrefwriter_bench.cc cpdf_bench.cc
SOURCES = $(UTILS_SRCS) $(TARGET_SRCS)

TARGET = xrefwriter_bench cpdf_bench
.PHONY: all clean
all: $(TARGET)

xrefwriter_bench: xrefwriter_bench.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o xrefwriter_bench xrefwriter_bench.o $(UTILS_OBJS) $(MANDATORY_LIBS)

cpdf_bench: cpdf_bench.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o cpdf_bench cpdf_bench.o $(UTILS_OBJS) $(MANDATORY_LIBS)

clean: 
	-rm $(UTILS_OBJS) *.o $(TARGET)

deps:
	$(CXX) $(MANDATORY_INCPATH) -M -MF deps $(SOURCES)

include deps
