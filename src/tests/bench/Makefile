# includes basic rules for building 
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../../
include $(REL_ADDR)/Makefile.rules

# list of utils modules used by all targets
UTILS_SRCS = utils.cc
UTILS_OBJS = $(UTILS_SRCS:.cc=.o)

# sources for benchmark modules
TARGET_SRCS = xrefwriter_bench.cc cpdf_bench.cc delinearize_bench.cc
SOURCES = $(UTILS_SRCS) $(TARGET_SRCS)

TARGET = xrefwriter_bench cpdf_bench file_info content_stream_bench delinearize_bench
.PHONY: all clean
all: $(TARGET)

xrefwriter_bench: xrefwriter_bench.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o xrefwriter_bench xrefwriter_bench.o $(UTILS_OBJS) $(MANDATORY_LIBS)

cpdf_bench: cpdf_bench.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o cpdf_bench cpdf_bench.o $(UTILS_OBJS) $(MANDATORY_LIBS)

content_stream_bench: content_stream_bench.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o content_stream_bench content_stream_bench.o $(UTILS_OBJS) $(MANDATORY_LIBS)

delinearize_bench: delinearize_bench.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o delinearize_bench delinearize_bench.o $(UTILS_OBJS) $(MANDATORY_LIBS)

file_info: file_info.o utils.o
	$(LINK) $(LDFLAGS) -o file_info file_info.o $(UTILS_OBJS) $(MANDATORY_LIBS)

clean: 
	-rm $(UTILS_OBJS) || true
	rm *.o $(TARGET)

deps:
	$(CXX) $(MANDATORY_INCPATH) -M -MF deps $(SOURCES)

include deps
