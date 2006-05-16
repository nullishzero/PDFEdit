###############################################################################
# Make file for utils static library
#
################################################################################

# Source files for library
SOURCES=debug.cc \
     	delinearizator.cc

# Binary files to be included to the library
BINS=debug.o \
     delinearizator.o

AR=ar cqs
DEL_FILE=rm
CC       = gcc
CXX      = g++
LEX      = flex
YACC     = yacc
CFLAGS   = -pipe -Wall -W -g -D_REENTRANT  -DDEBUG
CXXFLAGS = -pipe -Wall -W -g -W -Wall -Wconversion -Wcast-qual -Wwrite-strings -ansi -pedantic -Wno-unused-variable -O0 -Wunused-function -D_REENTRANT  -DDEBUG -D DEFAULT_DEBUG_LEVEL=DBG_DBG
INCPATH	 = -I./ -I../ -I../xpdf/ -I../xpdf/xpdf -I../xpdf/goo -I../xpdf/splash -I../xpdf/fofi 

.SUFFIXES: .c .o .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

libutils.a: $(BINS)
	$(AR) libutils.a $(BINS)

.PHONY: clean
clean:
	-$(DEL_FILE)  $(BINS) libutils.a
