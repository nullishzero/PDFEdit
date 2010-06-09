# includes basic rules for building 
# REL_ADDR has to be defined, because Makefile.rules refers 
# to the Makefile.flags
REL_ADDR = ../../
include $(REL_ADDR)/Makefile.rules

# list of utils modules used by all targets
UTILS_SRCS = common.cc
UTILS_OBJS = $(UTILS_SRCS:.cc=.o)

TOOLS_LIBS = $(MANDATORY_LIBS) $(BOOSTPROGRAMOPTIONS_LIBS)

CXXFLAGS += $(PNGFLAGS)
# sources for benchmark modules
TARGET_SRCS = displaycs.cc pagemetrics.cc parse_object.cc pdf_object_printer.cc \
	      pdf_page_from_ref.cc pdf_page_to_ref.cc flattener.cc delinearizator.cc \
	      pdf_object_comparer.cc pdf_to_text.cc add_text.cc pdf_to_bmp.cc add_image.cc \
	      pdf_images.cc replace_text.cc
SOURCES = $(UTILS_SRCS) $(TARGET_SRCS)

TARGET = displaycs pagemetrics parse_object pdf_object_printer \
	 pdf_page_from_ref pdf_page_to_ref flattener pdf_object_comparer \
	 pdf_to_text add_text add_image pdf_to_bmp pdf_images replace_text \
	 delinearizator

.PHONY: all clean
all: $(TARGET)

displaycs: displaycs.o
	$(LINK) $(LDFLAGS) -o displaycs displaycs.o $(TOOLS_LIBS)

pagemetrics: pagemetrics.o 
	$(LINK) $(LDFLAGS) -o pagemetrics pagemetrics.o $(TOOLS_LIBS)

pdf_object_printer: pdf_object_printer.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o pdf_object_printer pdf_object_printer.o $(UTILS_OBJS) $(TOOLS_LIBS)

pdf_page_from_ref: pdf_page_from_ref.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o pdf_page_from_ref pdf_page_from_ref.o $(UTILS_OBJS) $(TOOLS_LIBS)

pdf_page_to_ref: pdf_page_to_ref.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o pdf_page_to_ref pdf_page_to_ref.o $(UTILS_OBJS) $(TOOLS_LIBS)

delinearizator: delinearizator.o
	$(LINK) $(LDFLAGS) -o delinearizator delinearizator.o $(TOOLS_LIBS)

flattener: flattener.o
	$(LINK) $(LDFLAGS) -o flattener flattener.o $(TOOLS_LIBS)

pdf_object_comparer: pdf_object_comparer.o $(UTILS_OBJS)
	$(LINK) $(LDFLAGS) -o pdf_object_comparer pdf_object_comparer.o $(UTILS_OBJS) $(TOOLS_LIBS)

pdf_to_text: pdf_to_text.o
	$(LINK) $(LDFLAGS) -o pdf_to_text pdf_to_text.o $(TOOLS_LIBS)

parse_object: parse_object.o
	$(LINK) $(LDFLAGS) -o parse_object parse_object.o $(TOOLS_LIBS)

add_text: add_text.o
	$(LINK) $(LDFLAGS) -o add_text add_text.o $(TOOLS_LIBS)

add_image: add_image.o
	$(LINK) $(LDFLAGS) -o add_image add_image.o $(TOOLS_LIBS) $(PNG_LIBS)

pdf_to_bmp: pdf_to_bmp.o
	$(LINK) $(LDFLAGS) -o pdf_to_bmp pdf_to_bmp.o $(TOOLS_LIBS)

pdf_images: pdf_images.o
	$(LINK) $(LDFLAGS) -o pdf_images pdf_images.o $(TOOLS_LIBS)

replace_text: replace_text.o
	$(LINK) $(LDFLAGS) -o replace_text replace_text.o $(TOOLS_LIBS)

clean: 
	-rm $(UTILS_OBJS) || true
	rm *.o $(TARGET)

deps:
	$(CXX) $(MANDATORY_INCPATH) -M -MF deps $(SOURCES)

include deps
