# change application name here (executable output name)
TARGET=trimming
 
# compiler
CC=gcc
# debug
DEBUG=-g
# optimisation
OPT=-O0
# warnings
WARN=-Wall
 
PTHREAD=-pthread
EPICS=/home/peter/epics
BASE=$(EPICS)/base
EXT=$(EPICS)/extensions
BASEINC=$(BASE)/include
EXTINC=$(EXT)/include
BASELIB=$(BASE)/lib/linux-x86_64
EXTLIB=$(EXT)/lib/linux-x86_64
 
CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe
 
GTKLIB=`pkg-config --cflags --libs gtk+-3.0`
FITLIBS= -lezca -lm -lminpack -lgrace_np
 
# linker
LD=gcc
LDFLAGS=$(PTHREAD) $(GTKLIB) $(FITLIBS) -L$(BASELIB) -L$(EXTLIB) -export-dynamic
 
OBJS=    main.o testfit.o
 
all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)
    
main.o: src/main.c
	$(CC) -c $(CCFLAGS) src/main.c $(GTKLIB) -o main.o

testfit.o: src/testfit.c
	$(CC) -c  -I$(EXTINC) -I$(BASEINC)/os/Linux -I$(BASEINC)/compiler/gcc/ -I$(BASEINC) -L$(BASELIB) -L$(EXTLIB)  src/testfit.c -o testfit.o

clean:
	rm -f *.o $(TARGET)

