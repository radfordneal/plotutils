## Makefile.am for plotutils/libxmi.

lib_LTLIBRARIES = libxmi.la

## No longer used, now that we keep a copy of sys-defines.h in this directory
## INCLUDES = -I$(srcdir)/../include

include_HEADERS = xmi.h

noinst_HEADERS = mi_api.h mi_arc.h mi_fllarc.h mi_fply.h mi_gc.h mi_line.h \
mi_ply.h mi_scanfill.h mi_spans.h mi_widelin.h mi_zerarc.h extern.h \
sys-defines.h

libxmi_la_LDFLAGS = -version-info 1:1:1

libxmi_la_SOURCES = mi_alloc.c mi_api.c mi_arc.c mi_canvas.c mi_fllarc.c \
mi_fllrct.c mi_fplycon.c mi_gc.c mi_ply.c mi_plycon.c mi_plygen.c \
mi_plypnt.c mi_plyutil.c mi_spans.c mi_widelin.c mi_zerarc.c mi_zerolin.c

EXTRA_DIST = CUSTOMIZE README-X VERSION drawing.c

SUBDIRS = info

##EOF
