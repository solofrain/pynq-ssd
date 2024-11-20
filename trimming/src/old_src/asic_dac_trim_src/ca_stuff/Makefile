CC = gcc
G77 = g77
EPICSDIR = /usr/local/epics
BASEDIR = $(EPICSDIR)/base
EXTENDIR = $(EPICSDIR)/extensions
EZCAINCDIR = -I$(EXTENDIR)/include
EPICSINCDIR = -I$(BASEDIR)/include -I$(BASEDIR)/include/os/Linux
INCLUDEDIRS = $(EZCAINCDIR) $(EPICSINCDIR)
EZCALIBDIR = $(EXTENDIR)/lib/linux-x86
EZCALIB = -lezca
CALIBDIR = $(BASEDIR)/lib/linux-x86
CALIBS = -lca -lCom
GRACELIB = -lgrace_np
LIBDIRS = -L$(CALIBDIR) -L$(EZCALIBDIR)
LIBS = $(EZCALIB) $(CALIBS) $(GRACELIB) -lminpack
CFLAGS = -c

libfit.so: testfit.o
	$(CC) -shared -o libfit.so testfit.o $(LIBDIRS) $(LIBS)
clean:
	rm -f *.o libfit.so
testfit.o:
	$(G77) $(CFLAGS) $(INCLUDEDIRS) testfit.c
