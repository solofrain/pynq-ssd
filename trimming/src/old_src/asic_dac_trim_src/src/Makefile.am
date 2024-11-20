## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ \
	-I/usr/local/epics/base/include -I/usr/local/epics/base/include/os/Linux -I/usr/local/epics/extensions/include -I/usr/local/grace/include

bin_PROGRAMS = asic_dac_trim

asic_dac_trim_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	testfit.c

asic_dac_trim_LDADD = -L/usr/local/epics/base/lib/linux-x86 -L/usr/local/epics/extensions/lib/linux-x86  -L/usr/local/grace/lib @PACKAGE_LIBS@ $(INTLLIBS)  -lminpack  -lCom -lca -lezca -lgrace_np

