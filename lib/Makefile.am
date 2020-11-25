## Makefile.am for plotutils/lib.

noinst_LIBRARIES = libcommon.a libfontlist.a
INCLUDES = -I$(srcdir)/../include

libcommon_a_SOURCES = xmalloc.c xstrdup.c display.c getopt.c getopt1.c 
libcommon_a_LIBADD = @ALLOCA@
libcommon_a_DEPENDENCIES = $(libcommon_a_LIBADD)

libfontlist_a_SOURCES = fontlist.c
