## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libplot.la

INCLUDES = $(X_CFLAGS) -I$(srcdir)/../include

GSRC = g_affine.c g_alabel.c g_alab_her.c g_bgcolor.c g_box.c g_capmod.c \
g_circ.c g_clipper.c g_colors.c g_colorname.c g_concat.c g_cont.c \
g_cntrlify.c g_endpath.c g_error.c g_erase.c g_fakearc.c g_filltype.c \
g_fillcolor.c g_flushpl.c g_fontdb.c g_fontd2.c g_fontname.c g_fontsize.c \
g_havecap.c g_her_glyph.c g_integer.c g_iso.c g_joinmod.c g_label.c \
g_line.c g_linemod.c g_linewidth.c g_mark.c g_matrix.c g_move.c g_openpl.c \
g_outbuf.c g_outfile.c g_pagetype.c g_pencolor.c g_relative.c g_range.c \
g_reststate.c g_retrieve.c g_savestate.c g_space.c g_textangle.c g_vector.c \
g_xmalloc.c g_xstring.c

MSRC = m_affine.c m_alabel.c m_arc.c m_arcrel.c m_bgcolor.c m_box.c \
m_boxrel.c m_capmod.c m_circ.c m_circrel.c m_closepl.c m_concat.c m_cont.c \
m_contrel.c m_defplot.c m_defstate.c m_earc.c m_earcrel.c m_ellipse.c \
m_emit.c m_endpath.c m_erase.c m_filltype.c m_fillcolor.c m_fontname.c \
m_fontsize.c m_joinmod.c m_label.c m_line.c m_linemod.c m_linerel.c \
m_linewidth.c m_mark.c m_markrel.c m_move.c m_moverel.c m_openpl.c \
m_pencolor.c m_point.c m_pointrel.c m_reststate.c m_savestate.c m_space.c \
m_textangle.c

TSRC = t_attribs.c t_closepl.c t_color.c t_color2.c t_cont.c t_defplot.c \
t_defstate.c t_erase.c t_openpl.c t_point.c t_tek_md.c t_tek_mv.c \
t_tek_vec.c

HSRC = h_alab_pcl.c h_arc.c h_attribs.c h_circ.c h_closepl.c h_color.c \
h_defplot.c h_defstate.c h_endpath.c h_box.c h_font.c h_position.c \
h_linewidth.c h_point.c

FSRC = f_alab_ps.c f_arc.c f_box.c f_closepl.c f_color.c f_color2.c \
f_defplot.c f_defstate.c f_ellipse.c f_endpath.c f_erase.c f_linewidth.c \
f_point.c f_retrieve.c

PSRC = p_alab_ps.c p_closepl.c p_color.c p_color2.c p_defplot.c \
p_defstate.c p_ellipse.c p_endpath.c p_erase.c p_openpl.c p_point.c

XSRC = x_alab_X.c x_arc.c x_attribs.c x_box.c x_closepl.c x_color.c \
x_defplot.c x_defstate.c x_earc.c x_ellipse.c x_endpath.c x_erase.c \
x_flushpl.c x_openpl.c x_point.c x_reststate.c x_retrieve.c x_savestate.c \
x_xevents.c

YSRC = y_closepl.c y_defplot.c y_openpl.c

if NO_X
ALLSRC = api.c $(GSRC) $(MSRC) $(TSRC) $(HSRC) $(FSRC) $(PSRC)
else
ALLSRC = api.c $(GSRC) $(MSRC) $(TSRC) $(HSRC) $(FSRC) $(PSRC) $(XSRC) $(YSRC)
endif
libplot_la_SOURCES = $(ALLSRC)

libplot_la_LDFLAGS = -version-info 1:4:1

noinst_HEADERS = extern.h g_colorname.h g_control.h g_cntrlify.h \
g_fakearc.h g_her_metr.h g_jis.h g_pagetype.h g_params.h h_stick.h \
p_header.h

EXTRA_libplot_la_SOURCES = x_alab_X.c x_arc.c x_attribs.c x_box.c \
x_closepl.c x_color.c x_defplot.c x_defstate.c x_earc.c x_ellipse.c \
x_endpath.c x_erase.c x_flushpl.c x_openpl.c x_point.c x_reststate.c \
x_retrieve.c x_savestate.c x_xevents.c y_closepl.c y_defplot.c y_openpl.c

EXTRA_DIST = DEDICATION HUMOR README.fig README.hpgl README.tek VERSION
