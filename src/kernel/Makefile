CC = g++
CFLAGS = -g -Wall
INCLUDES = -I. -I../ -I../xpdf/ 

.SUFFIXES: .cc

.deps:
	$(CC) -M $(INCLUDES) $(MODULES) *.cc *.h > .deps
	
.cc.o: 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< 

.PHONY: clean
clean:
	-rm *.o .deps
