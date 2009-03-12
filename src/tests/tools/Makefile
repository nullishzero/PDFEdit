# includes basic rules for building 
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../../
include $(REL_ADDR)/Makefile.rules

# list of utils modules used by all targets
UTILS_SRCS = common.cc
UTILS_OBJS = $(UTILS_SRCS:.cc=.o)

# sources for benchmark modules
TARGET_SRCS = displaycs.cc pagemetrics.cc parse_object.cc pdf_object_printer.cc \
	      pdf_page_from_ref.cc pdf_page_to_ref.cc delinearizator.cc \
	      pdf_object_comparer.cc
SOURCES = $(UTILS_SRCS) $(TARGET_SRCS)

TARGET = displaycs pagemetrics parse_object pdf_object_printer \
	 pdf_page_from_ref pdf_page_to_ref pdf_object_comparer
.PHONY: all clean
all: $(TARGET)

displaycs: displaycs.o
	$(LINK) $(LDFLAGS) -o displaycs displaycs.o $(UTILS_OBJS) $(MANDATORY_LIBS)

pagemetrics: pagemetrics.o
	$(LINK) $(LDFALGS) -o pagemetrics pagemetrics.o $(UTILS_OBJS) $(MANDATORY_LIBS)

pdf_object_printer: pdf_object_printer.o common.o
	$(LINK) $(LDFALGS) -o pdf_object_printer pdf_object_printer.o common.o $(MANDATORY_LIBS)

pdf_page_from_ref: pdf_page_from_ref.o common.o
	$(LINK) $(LDFALGS) -o pdf_page_from_ref pdf_page_from_ref.o common.o $(MANDATORY_LIBS)

pdf_page_to_ref: pdf_page_to_ref.o common.o
	$(LINK) $(LDFALGS) -o pdf_page_to_ref pdf_page_to_ref.o common.o $(MANDATORY_LIBS)

delinearizator: delinearizator.o
	$(LINK) $(LDFLAGS) -o delinearizator delinearizator.o $(MANDATORY_LIBS)

pdf_object_comparer: pdf_object_comparer.o common.o
	$(LINK) $(LDFLAGS) -o pdf_object_comparer pdf_object_comparer.o common.o $(MANDATORY_LIBS)

clean: 
	-rm $(UTILS_OBJS) || true
	rm *.o $(TARGET)

deps:
	$(CXX) $(MANDATORY_INCPATH) -M -MF deps $(SOURCES)

include deps
