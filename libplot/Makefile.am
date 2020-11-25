## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libplot.la

EXTRA_DIST = DEDICATION HUMOR README-gif README-hpgl README-tek README-X \
VERSION

noinst_HEADERS = extern.h g_arc.h g_colorname.h g_control.h g_cntrlify.h \
g_her_metr.h g_jis.h \
g_mi.h g_mifllarc.h g_mifply.h g_miline.h g_miply.h g_miscanfill.h g_mispans.h \
g_miwidelin.h g_mizerarc.h \
g_pagetype.h g_params.h h_stick.h i_rle.h p_header.h

libplot_la_LDFLAGS = -version-info 2:0:0

INCLUDES = $(X_CFLAGS) -I$(srcdir)/../include

GSRC = g_affine.c g_alabel.c g_alab_her.c g_arc.c g_bez.c g_bgcolor.c g_box.c \
g_capmod.c g_circ.c g_clipper.c g_closepl.c g_colors.c g_colorname.c \
g_concat.c g_cont.c g_cntrlify.c g_dash.c g_dash2.c g_defplot.c g_defstate.c g_ellipse.c g_endpath.c \
g_error.c g_erase.c g_filltype.c g_fillcolor.c g_fillrule.c g_flushpl.c g_fontdb.c \
g_fontd2.c g_fontname.c g_fontsize.c g_havecap.c g_her_glyph.c g_integer.c \
g_iso.c g_joinmod.c g_label.c g_line.c g_linemod.c g_linewidth.c g_mark.c \
g_matrix.c g_miscmi.c g_miter.c g_move.c \
g_mialloc.c g_miarc.c g_mifllarc.c g_mifllrct.c g_mifllspn.c g_mifplycon.c g_miply.c g_miplycon.c \
g_miplygen.c g_miplypnt.c g_miplyutil.c g_mispans.c g_miwidelin.c \
g_mizerarc.c g_mizerolin.c \
g_openpl.c g_outbuf.c g_outfile.c g_pagetype.c g_params.c g_pencolor.c \
g_point.c g_relative.c g_range.c g_reststate.c g_retrieve.c g_savestate.c \
g_space.c g_textangle.c g_vector.c g_write.c g_xmalloc.c g_xstring.c

MSRC = m_alabel.c m_arc.c m_arcrel.c m_bgcolor.c m_box.c m_bez.c \
m_bezrel.c m_boxrel.c m_capmod.c m_circ.c m_circrel.c m_closepl.c \
m_concat.c m_cont.c m_contrel.c m_dash.c m_defplot.c m_earc.c m_earcrel.c \
m_ellipse.c m_emit.c m_endpath.c m_erase.c m_filltype.c m_fillcolor.c \
m_fillrule.c m_fontname.c m_fontsize.c m_joinmod.c m_label.c m_line.c \
m_linemod.c m_linerel.c m_linewidth.c m_mark.c m_markrel.c m_miter.c \
m_move.c m_moverel.c m_openpl.c m_pencolor.c m_point.c m_pointrel.c \
m_reststate.c m_savestate.c m_space.c m_textangle.c

TSRC = t_attribs.c t_closepl.c t_color.c t_color2.c t_cont.c t_defplot.c \
t_erase.c t_openpl.c t_point.c t_tek_md.c t_tek_mv.c t_tek_vec.c

HSRC = h_alab_pcl.c h_attribs.c h_circ.c h_closepl.c h_color.c h_defplot.c \
h_endpath.c h_box.c h_font.c h_openpl.c h_position.c h_linewidth.c \
h_point.c

FSRC = f_alab_ps.c f_box.c f_closepl.c f_color.c f_color2.c f_defplot.c \
f_ellipse.c f_endpath.c f_erase.c f_linewidth.c f_openpl.c f_point.c \
f_retrieve.c

PSRC = p_alab_ps.c p_closepl.c p_color.c p_color2.c p_defplot.c p_ellipse.c \
p_endpath.c p_erase.c p_openpl.c p_point.c

ASRC = a_alab_ps.c a_attribs.c a_color.c a_closepl.c a_defplot.c \
a_endpath.c a_openpl.c a_point.c

ISRC = i_closepl.c i_color.c i_defplot.c i_endpath.c i_ellipse.c i_erase.c \
i_openpl.c i_point.c i_rle.c

NSRC = n_closepl.c n_defplot.c n_ellipse.c n_endpath.c n_erase.c n_openpl.c \
n_point.c

XSRC = x_alab_x.c x_attribs.c x_box.c x_closepl.c x_color.c x_cont.c \
x_defplot.c x_ellipse.c x_endpath.c x_erase.c x_flushpl.c x_openpl.c \
x_point.c x_reststate.c x_retrieve.c x_savestate.c

YSRC = y_closepl.c y_defplot.c y_erase.c y_openpl.c

if NO_X
ALLSRC = $(GSRC) $(MSRC) $(TSRC) $(HSRC) $(FSRC) $(PSRC) \
$(ASRC) $(ISRC) $(NSRC)
else
ALLSRC = $(GSRC) $(MSRC) $(TSRC) $(HSRC) $(FSRC) $(PSRC) \
$(ASRC) $(ISRC) $(NSRC) $(XSRC) $(YSRC)
endif

EXTRA_libplot_la_SOURCES = i_closepl.c i_color.c i_defplot.c i_endpath.c \
i_erase.c i_openpl.c i_point.c i_rle.c \
x_alab_x.c x_attribs.c x_box.c x_closepl.c x_color.c x_cont.c x_defplot.c \
x_ellipse.c x_endpath.c x_erase.c x_flushpl.c x_openpl.c x_point.c \
x_reststate.c x_retrieve.c x_savestate.c \
y_closepl.c y_defplot.c y_erase.c y_openpl.c

libplot_la_SOURCES = api.c $(ALLSRC)

if NO_X
libplot_la_LIBADD = 
else
libplot_la_LIBADD = $(X_LIBS) $(X_TOOLKIT_LIBS) $(X_PRE_LIBS) $(X_BASIC_LIBS) $(X_EXTRA_LIBS)
endif

##EOF
