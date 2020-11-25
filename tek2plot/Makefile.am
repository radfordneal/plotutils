## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tek2plot

tek2plot_SOURCES = tek2plot.c Tektable.c
tek2plot_LDADD = ../lib/libcommon.a ../lib/libfontlist.a ../libplot/libplot.la $(X_LIBS) $(X_PRE_LIBS) $(XLIBS) $(X_EXTRA_LIBS)
noinst_HEADERS = Tekparse.h

INCLUDES = -I$(srcdir)/../include

SUBDIRS = teksamples

# remove executables installed by previous versions of the package

OBSOLETEBINS = tek2tek tek2hpgl tek2fig tek2ps tek2X

install-exec-hook:
	-for f in $(OBSOLETEBINS) ; do rm -f $(bindir)/$$f; done
