## Process this file with automake to produce Makefile.in

bin_PROGRAMS = plot

plot_SOURCES = plot.c
plot_LDADD = ../lib/libcommon.a ../lib/libfontlist.a ../libplot/libplot.la $(X_LIBS) $(X_PRE_LIBS) $(XLIBS) $(X_EXTRA_LIBS)

INCLUDES = -I$(srcdir)/../include

# remove executables installed by previous versions of the package

OBSOLETEBINS = plot2plot plot2tek plot2hpgl plot2fig plot2ps plot2X

install-exec-hook:
	-for f in $(OBSOLETEBINS) ; do rm -f $(bindir)/$$f; done
