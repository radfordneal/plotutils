## Process this file with automake to produce Makefile.in

bin_PROGRAMS = graph

graph_SOURCES = graph.c misc.c linemode.c reader.c plotter.c 
graph_LDADD = ../lib/libcommon.a ../lib/libfontlist.a ../libplot/libplot.la
noinst_HEADERS = extern.h

INCLUDES = -I$(srcdir)/../include

# remove executables installed by previous versions of the package, and
# remove obsolete library versions too

OBSOLETEBINS = graph-tek graph-hpgl graph-fig graph-ps graph-X
OBSOLETELIBS = libplottek.a libplothpgl.a libplotfig.a libplotps.a libplotX.a

install-exec-hook:
	-for f in $(OBSOLETEBINS) ; do rm -f $(bindir)/$$f; done
	-for f in $(OBSOLETELIBS) ; do rm -f $(libdir)/$$f; done
