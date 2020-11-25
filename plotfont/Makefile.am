## Process this file with automake to produce Makefile.in

bin_PROGRAMS = plotfont

plotfont_SOURCES = plotfont.c
plotfont_LDADD = ../lib/libcommon.a ../lib/libfontlist.a ../libplot/libplot.la $(X_LIBS) $(X_TOOLKIT_LIBS) $(X_PRE_LIBS) $(X_BASIC_LIBS) $(X_EXTRA_LIBS)

INCLUDES = -I$(srcdir)/../include
