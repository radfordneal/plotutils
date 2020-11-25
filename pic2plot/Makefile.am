## Process this file with automake to produce Makefile.in

bin_PROGRAMS = pic2plot

pic2plot_SOURCES = common.cc lex.cc main.cc object.cc plot.cc gram.yy
pic2plot_LDADD = ../lib/libcommon.a ../lib/libfontlist.a ./libgroff/libgroff.a ../libplot/libplot.la

noinst_HEADERS = common.h object.h output.h pic.h

YFLAGS = -d

INCLUDES = -I$(srcdir)/../include -I$(srcdir)/include

EXTRA_DIST = gram.h

SUBDIRS = doc include libgroff
