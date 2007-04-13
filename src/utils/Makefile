###############################################################################
# Make file for utils static library
#
################################################################################

#Makefile flags
include ../Makefile.flags

# Source files for library
SOURCES=debug.cc \
     	delinearizator.cc \
	confparser.cc

# Binary files to be included to the library
BINS=debug.o \
     delinearizator.o \
     confparser.o

AR       = ar cqs
DEL_FILE = rm -f
CXX      = g++
LEX      = flex
YACC     = yacc
CXXFLAGS = -pipe -Wall -W -Wconversion -Wcast-qual -Wwrite-strings -ansi -pedantic -Wno-unused-variable -Wunused-function -D_REENTRANT $(UTIL_CXXFLAGS)
INCPATH	 = -I./ -I../ -I../xpdf/ -I../xpdf/xpdf -I../xpdf/goo -I../xpdf/splash -I../xpdf/fofi 

.SUFFIXES: .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

libutils.a: $(BINS)
	$(AR) libutils.a $(BINS)

.PHONY: clean distclean
clean:
	$(DEL_FILE) $(BINS) libutils.a

distclean: clean
