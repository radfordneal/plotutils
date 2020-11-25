## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libplotter.la

libplotter_la_LDFLAGS = -version-info 2:0:0

INCLUDES = $(X_CFLAGS) -I$(srcdir)/../include -DLIBPLOTTER

GSRC = g_affine.cc g_alabel.cc g_alab_her.cc g_arc.cc g_bez.cc g_bgcolor.cc g_box.cc \
g_capmod.cc g_circ.cc g_clipper.cc g_closepl.cc g_colors.cc g_colorname.cc \
g_concat.cc g_cont.cc g_cntrlify.cc g_dash.cc g_dash2.cc g_defplot.cc g_defstate.cc g_ellipse.cc g_endpath.cc \
g_error.cc g_erase.cc g_filltype.cc g_fillcolor.cc g_fillrule.cc g_flushpl.cc g_fontdb.cc \
g_fontd2.cc g_fontname.cc g_fontsize.cc g_havecap.cc g_her_glyph.cc g_integer.cc \
g_iso.cc g_joinmod.cc g_label.cc g_line.cc g_linemod.cc g_linewidth.cc g_mark.cc \
g_matrix.cc g_miscmi.cc g_miter.cc \
g_miarc.cc \
g_mialloc.cc g_mifllarc.cc g_mifllrct.cc g_mifllspn.cc g_mifplycon.cc g_miply.cc g_miplycon.cc \
g_miplygen.cc g_miplypnt.cc g_miplyutil.cc g_mispans.cc g_miwidelin.cc \
g_mizerarc.cc g_mizerolin.cc \
g_move.cc \
g_openpl.cc g_outbuf.cc g_outfile.cc g_pagetype.cc \
g_params.cc g_pencolor.cc g_point.cc g_relative.cc g_range.cc g_reststate.cc \
g_retrieve.cc g_savestate.cc g_space.cc g_textangle.cc \
g_vector.cc g_write.cc g_xmalloc.cc \
g_xstring.cc

MSRC = m_alabel.cc m_arc.cc m_arcrel.cc m_bez.cc m_bezrel.cc \
m_bgcolor.cc m_box.cc m_boxrel.cc m_capmod.cc m_circ.cc m_circrel.cc \
m_closepl.cc m_concat.cc m_cont.cc m_contrel.cc m_dash.cc m_defplot.cc m_earc.cc \
m_earcrel.cc m_ellipse.cc m_emit.cc m_endpath.cc m_erase.cc m_filltype.cc \
m_fillcolor.cc m_fillrule.cc m_fontname.cc m_fontsize.cc m_joinmod.cc \
m_label.cc m_line.cc m_linemod.cc m_linerel.cc m_linewidth.cc m_mark.cc \
m_markrel.cc m_miter.cc m_move.cc m_moverel.cc m_openpl.cc m_pencolor.cc \
m_point.cc m_pointrel.cc m_reststate.cc m_savestate.cc m_space.cc \
m_textangle.cc

TSRC = t_attribs.cc t_closepl.cc t_color.cc t_color2.cc t_cont.cc \
t_defplot.cc t_erase.cc t_openpl.cc t_point.cc t_tek_md.cc t_tek_mv.cc \
t_tek_vec.cc

HSRC = h_alab_pcl.cc h_attribs.cc h_circ.cc h_closepl.cc h_color.cc \
h_defplot.cc h_endpath.cc h_box.cc h_font.cc h_openpl.cc h_position.cc \
h_linewidth.cc h_point.cc

FSRC = f_alab_ps.cc f_box.cc f_closepl.cc f_color.cc f_color2.cc \
f_defplot.cc f_ellipse.cc f_endpath.cc f_erase.cc f_linewidth.cc \
f_openpl.cc f_point.cc f_retrieve.cc

NSRC = n_closepl.cc n_defplot.cc n_ellipse.cc n_endpath.cc n_erase.cc n_openpl.cc \
n_point.cc

ISRC = i_closepl.cc i_color.cc i_defplot.cc i_ellipse.cc i_endpath.cc i_erase.cc \
i_openpl.cc i_point.cc i_rle.cc

PSRC = p_alab_ps.cc p_closepl.cc p_color.cc p_color2.cc p_defplot.cc \
p_ellipse.cc p_endpath.cc p_erase.cc p_openpl.cc p_point.cc

ASRC = a_alab_ps.cc a_attribs.cc a_color.cc a_closepl.cc a_defplot.cc \
a_endpath.cc a_openpl.cc a_point.cc

XSRC = x_alab_x.cc x_attribs.cc x_box.cc x_closepl.cc x_color.cc x_cont.cc \
x_defplot.cc x_ellipse.cc x_endpath.cc x_erase.cc x_flushpl.cc x_openpl.cc \
x_point.cc x_reststate.cc x_retrieve.cc x_savestate.cc

YSRC = y_closepl.cc y_defplot.cc y_erase.cc y_openpl.cc

if NO_X
ALLSRC = $(GSRC) $(MSRC) $(TSRC) $(HSRC) $(FSRC) $(PSRC) \
$(ASRC) $(ISRC) $(NSRC) 
else
ALLSRC = $(GSRC) $(MSRC) $(TSRC) $(HSRC) $(FSRC) $(PSRC) \
$(ASRC) $(ISRC) $(NSRC) $(XSRC) $(YSRC)
endif

ALLHEADERS = extern.h g_arc.h g_colorname.h g_control.h g_cntrlify.h \
g_her_metr.h \
g_mi.h g_mifllarc.h g_mifply.h g_miline.h g_miply.h g_miscanfill.h g_mispans.h \
g_miwidelin.h g_mizerarc.h \
g_jis.h g_pagetype.h g_params.h h_stick.h i_rle.h p_header.h

noinst_HEADERS = $(ALLHEADERS)

CLEANFILES = $(ALLSRC) $(ALLHEADERS)

libplotter_la_SOURCES = $(ALLSRC)

if NO_X
libplotter_la_LIBADD = 
else
libplotter_la_LIBADD = $(X_LIBS) $(X_TOOLKIT_LIBS) $(X_PRE_LIBS) $(X_BASIC_LIBS) $(X_EXTRA_LIBS)
endif

## execute the following command in ../libplot to produce the commands
## that follow
## (for i in ?_*.c; do echo "$i"c: "\$(top_srcdir)/libplot/$i"; echo "	rm -f $i"c" ; if \$(LN_S) \$(top_srcdir)/libplot/$i $i"c" ; then true ; else cp -p \$(top_srcdir)/libplot/$i $i"c" ; fi"; echo; done)

a_alab_ps.cc: $(top_srcdir)/libplot/a_alab_ps.c $(ALLHEADERS)
	@rm -f a_alab_ps.cc ; if $(LN_S) $(top_srcdir)/libplot/a_alab_ps.c a_alab_ps.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_alab_ps.c a_alab_ps.cc ; fi

a_attribs.cc: $(top_srcdir)/libplot/a_attribs.c $(ALLHEADERS)
	@rm -f a_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/a_attribs.c a_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_attribs.c a_attribs.cc ; fi

a_closepl.cc: $(top_srcdir)/libplot/a_closepl.c $(ALLHEADERS)
	@rm -f a_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/a_closepl.c a_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_closepl.c a_closepl.cc ; fi

a_color.cc: $(top_srcdir)/libplot/a_color.c $(ALLHEADERS)
	@rm -f a_color.cc ; if $(LN_S) $(top_srcdir)/libplot/a_color.c a_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_color.c a_color.cc ; fi

a_defplot.cc: $(top_srcdir)/libplot/a_defplot.c $(ALLHEADERS)
	@rm -f a_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/a_defplot.c a_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_defplot.c a_defplot.cc ; fi

a_endpath.cc: $(top_srcdir)/libplot/a_endpath.c $(ALLHEADERS)
	@rm -f a_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/a_endpath.c a_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_endpath.c a_endpath.cc ; fi

a_openpl.cc: $(top_srcdir)/libplot/a_openpl.c $(ALLHEADERS)
	@rm -f a_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/a_openpl.c a_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_openpl.c a_openpl.cc ; fi

a_point.cc: $(top_srcdir)/libplot/a_point.c $(ALLHEADERS)
	@rm -f a_point.cc ; if $(LN_S) $(top_srcdir)/libplot/a_point.c a_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_point.c a_point.cc ; fi

f_alab_ps.cc: $(top_srcdir)/libplot/f_alab_ps.c $(ALLHEADERS)
	@rm -f f_alab_ps.cc ; if $(LN_S) $(top_srcdir)/libplot/f_alab_ps.c f_alab_ps.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_alab_ps.c f_alab_ps.cc ; fi

f_box.cc: $(top_srcdir)/libplot/f_box.c $(ALLHEADERS)
	@rm -f f_box.cc ; if $(LN_S) $(top_srcdir)/libplot/f_box.c f_box.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_box.c f_box.cc ; fi

f_closepl.cc: $(top_srcdir)/libplot/f_closepl.c $(ALLHEADERS)
	@rm -f f_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/f_closepl.c f_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_closepl.c f_closepl.cc ; fi

f_color.cc: $(top_srcdir)/libplot/f_color.c $(ALLHEADERS)
	@rm -f f_color.cc ; if $(LN_S) $(top_srcdir)/libplot/f_color.c f_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_color.c f_color.cc ; fi

f_color2.cc: $(top_srcdir)/libplot/f_color2.c $(ALLHEADERS)
	@rm -f f_color2.cc ; if $(LN_S) $(top_srcdir)/libplot/f_color2.c f_color2.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_color2.c f_color2.cc ; fi

f_defplot.cc: $(top_srcdir)/libplot/f_defplot.c $(ALLHEADERS)
	@rm -f f_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/f_defplot.c f_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_defplot.c f_defplot.cc ; fi

f_ellipse.cc: $(top_srcdir)/libplot/f_ellipse.c $(ALLHEADERS)
	@rm -f f_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/f_ellipse.c f_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_ellipse.c f_ellipse.cc ; fi

f_endpath.cc: $(top_srcdir)/libplot/f_endpath.c $(ALLHEADERS)
	@rm -f f_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/f_endpath.c f_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_endpath.c f_endpath.cc ; fi

f_erase.cc: $(top_srcdir)/libplot/f_erase.c $(ALLHEADERS)
	@rm -f f_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/f_erase.c f_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_erase.c f_erase.cc ; fi

f_linewidth.cc: $(top_srcdir)/libplot/f_linewidth.c $(ALLHEADERS)
	@rm -f f_linewidth.cc ; if $(LN_S) $(top_srcdir)/libplot/f_linewidth.c f_linewidth.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_linewidth.c f_linewidth.cc ; fi

f_openpl.cc: $(top_srcdir)/libplot/f_openpl.c $(ALLHEADERS)
	@rm -f f_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/f_openpl.c f_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_openpl.c f_openpl.cc ; fi

f_point.cc: $(top_srcdir)/libplot/f_point.c $(ALLHEADERS)
	@rm -f f_point.cc ; if $(LN_S) $(top_srcdir)/libplot/f_point.c f_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_point.c f_point.cc ; fi

f_retrieve.cc: $(top_srcdir)/libplot/f_retrieve.c $(ALLHEADERS)
	@rm -f f_retrieve.cc ; if $(LN_S) $(top_srcdir)/libplot/f_retrieve.c f_retrieve.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_retrieve.c f_retrieve.cc ; fi

g_affine.cc: $(top_srcdir)/libplot/g_affine.c $(ALLHEADERS)
	@rm -f g_affine.cc ; if $(LN_S) $(top_srcdir)/libplot/g_affine.c g_affine.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_affine.c g_affine.cc ; fi

g_alab_her.cc: $(top_srcdir)/libplot/g_alab_her.c $(ALLHEADERS)
	@rm -f g_alab_her.cc ; if $(LN_S) $(top_srcdir)/libplot/g_alab_her.c g_alab_her.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_alab_her.c g_alab_her.cc ; fi

g_alabel.cc: $(top_srcdir)/libplot/g_alabel.c $(ALLHEADERS)
	@rm -f g_alabel.cc ; if $(LN_S) $(top_srcdir)/libplot/g_alabel.c g_alabel.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_alabel.c g_alabel.cc ; fi

g_arc.cc: $(top_srcdir)/libplot/g_arc.c $(ALLHEADERS)
	@rm -f g_arc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_arc.c g_arc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_arc.c g_arc.cc ; fi

g_bez.cc: $(top_srcdir)/libplot/g_bez.c $(ALLHEADERS)
	@rm -f g_bez.cc ; if $(LN_S) $(top_srcdir)/libplot/g_bez.c g_bez.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_bez.c g_bez.cc ; fi

g_bgcolor.cc: $(top_srcdir)/libplot/g_bgcolor.c $(ALLHEADERS)
	@rm -f g_bgcolor.cc ; if $(LN_S) $(top_srcdir)/libplot/g_bgcolor.c g_bgcolor.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_bgcolor.c g_bgcolor.cc ; fi

g_box.cc: $(top_srcdir)/libplot/g_box.c $(ALLHEADERS)
	@rm -f g_box.cc ; if $(LN_S) $(top_srcdir)/libplot/g_box.c g_box.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_box.c g_box.cc ; fi

g_capmod.cc: $(top_srcdir)/libplot/g_capmod.c $(ALLHEADERS)
	@rm -f g_capmod.cc ; if $(LN_S) $(top_srcdir)/libplot/g_capmod.c g_capmod.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_capmod.c g_capmod.cc ; fi

g_circ.cc: $(top_srcdir)/libplot/g_circ.c $(ALLHEADERS)
	@rm -f g_circ.cc ; if $(LN_S) $(top_srcdir)/libplot/g_circ.c g_circ.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_circ.c g_circ.cc ; fi

g_clipper.cc: $(top_srcdir)/libplot/g_clipper.c $(ALLHEADERS)
	@rm -f g_clipper.cc ; if $(LN_S) $(top_srcdir)/libplot/g_clipper.c g_clipper.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_clipper.c g_clipper.cc ; fi

g_closepl.cc: $(top_srcdir)/libplot/g_closepl.c $(ALLHEADERS)
	@rm -f g_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/g_closepl.c g_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_closepl.c g_closepl.cc ; fi

g_cntrlify.cc: $(top_srcdir)/libplot/g_cntrlify.c $(ALLHEADERS)
	@rm -f g_cntrlify.cc ; if $(LN_S) $(top_srcdir)/libplot/g_cntrlify.c g_cntrlify.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_cntrlify.c g_cntrlify.cc ; fi

g_colorname.cc: $(top_srcdir)/libplot/g_colorname.c $(ALLHEADERS)
	@rm -f g_colorname.cc ; if $(LN_S) $(top_srcdir)/libplot/g_colorname.c g_colorname.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_colorname.c g_colorname.cc ; fi

g_colors.cc: $(top_srcdir)/libplot/g_colors.c $(ALLHEADERS)
	@rm -f g_colors.cc ; if $(LN_S) $(top_srcdir)/libplot/g_colors.c g_colors.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_colors.c g_colors.cc ; fi

g_concat.cc: $(top_srcdir)/libplot/g_concat.c $(ALLHEADERS)
	@rm -f g_concat.cc ; if $(LN_S) $(top_srcdir)/libplot/g_concat.c g_concat.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_concat.c g_concat.cc ; fi

g_cont.cc: $(top_srcdir)/libplot/g_cont.c $(ALLHEADERS)
	@rm -f g_cont.cc ; if $(LN_S) $(top_srcdir)/libplot/g_cont.c g_cont.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_cont.c g_cont.cc ; fi

g_dash.cc: $(top_srcdir)/libplot/g_dash.c $(ALLHEADERS)
	@rm -f g_dash.cc ; if $(LN_S) $(top_srcdir)/libplot/g_dash.c g_dash.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_dash.c g_dash.cc ; fi

g_dash2.cc: $(top_srcdir)/libplot/g_dash2.c $(ALLHEADERS)
	@rm -f g_dash2.cc ; if $(LN_S) $(top_srcdir)/libplot/g_dash2.c g_dash2.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_dash2.c g_dash2.cc ; fi

g_defplot.cc: $(top_srcdir)/libplot/g_defplot.c $(ALLHEADERS)
	@rm -f g_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/g_defplot.c g_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_defplot.c g_defplot.cc ; fi

g_defstate.cc: $(top_srcdir)/libplot/g_defstate.c $(ALLHEADERS)
	@rm -f g_defstate.cc ; if $(LN_S) $(top_srcdir)/libplot/g_defstate.c g_defstate.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_defstate.c g_defstate.cc ; fi

g_ellipse.cc: $(top_srcdir)/libplot/g_ellipse.c $(ALLHEADERS)
	@rm -f g_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/g_ellipse.c g_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_ellipse.c g_ellipse.cc ; fi

g_endpath.cc: $(top_srcdir)/libplot/g_endpath.c $(ALLHEADERS)
	@rm -f g_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/g_endpath.c g_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_endpath.c g_endpath.cc ; fi

g_erase.cc: $(top_srcdir)/libplot/g_erase.c $(ALLHEADERS)
	@rm -f g_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/g_erase.c g_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_erase.c g_erase.cc ; fi

g_error.cc: $(top_srcdir)/libplot/g_error.c $(ALLHEADERS)
	@rm -f g_error.cc ; if $(LN_S) $(top_srcdir)/libplot/g_error.c g_error.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_error.c g_error.cc ; fi

g_fillcolor.cc: $(top_srcdir)/libplot/g_fillcolor.c $(ALLHEADERS)
	@rm -f g_fillcolor.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fillcolor.c g_fillcolor.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fillcolor.c g_fillcolor.cc ; fi

g_fillrule.cc: $(top_srcdir)/libplot/g_fillrule.c $(ALLHEADERS)
	@rm -f g_fillrule.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fillrule.c g_fillrule.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fillrule.c g_fillrule.cc ; fi

g_filltype.cc: $(top_srcdir)/libplot/g_filltype.c $(ALLHEADERS)
	@rm -f g_filltype.cc ; if $(LN_S) $(top_srcdir)/libplot/g_filltype.c g_filltype.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_filltype.c g_filltype.cc ; fi

g_flushpl.cc: $(top_srcdir)/libplot/g_flushpl.c $(ALLHEADERS)
	@rm -f g_flushpl.cc ; if $(LN_S) $(top_srcdir)/libplot/g_flushpl.c g_flushpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_flushpl.c g_flushpl.cc ; fi

g_fontd2.cc: $(top_srcdir)/libplot/g_fontd2.c $(ALLHEADERS)
	@rm -f g_fontd2.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fontd2.c g_fontd2.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fontd2.c g_fontd2.cc ; fi

g_fontdb.cc: $(top_srcdir)/libplot/g_fontdb.c $(ALLHEADERS)
	@rm -f g_fontdb.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fontdb.c g_fontdb.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fontdb.c g_fontdb.cc ; fi

g_fontname.cc: $(top_srcdir)/libplot/g_fontname.c $(ALLHEADERS)
	@rm -f g_fontname.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fontname.c g_fontname.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fontname.c g_fontname.cc ; fi

g_fontsize.cc: $(top_srcdir)/libplot/g_fontsize.c $(ALLHEADERS)
	@rm -f g_fontsize.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fontsize.c g_fontsize.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fontsize.c g_fontsize.cc ; fi

g_havecap.cc: $(top_srcdir)/libplot/g_havecap.c $(ALLHEADERS)
	@rm -f g_havecap.cc ; if $(LN_S) $(top_srcdir)/libplot/g_havecap.c g_havecap.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_havecap.c g_havecap.cc ; fi

g_her_glyph.cc: $(top_srcdir)/libplot/g_her_glyph.c $(ALLHEADERS)
	@rm -f g_her_glyph.cc ; if $(LN_S) $(top_srcdir)/libplot/g_her_glyph.c g_her_glyph.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_her_glyph.c g_her_glyph.cc ; fi

g_integer.cc: $(top_srcdir)/libplot/g_integer.c $(ALLHEADERS)
	@rm -f g_integer.cc ; if $(LN_S) $(top_srcdir)/libplot/g_integer.c g_integer.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_integer.c g_integer.cc ; fi

g_iso.cc: $(top_srcdir)/libplot/g_iso.c $(ALLHEADERS)
	@rm -f g_iso.cc ; if $(LN_S) $(top_srcdir)/libplot/g_iso.c g_iso.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_iso.c g_iso.cc ; fi

g_joinmod.cc: $(top_srcdir)/libplot/g_joinmod.c $(ALLHEADERS)
	@rm -f g_joinmod.cc ; if $(LN_S) $(top_srcdir)/libplot/g_joinmod.c g_joinmod.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_joinmod.c g_joinmod.cc ; fi

g_label.cc: $(top_srcdir)/libplot/g_label.c $(ALLHEADERS)
	@rm -f g_label.cc ; if $(LN_S) $(top_srcdir)/libplot/g_label.c g_label.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_label.c g_label.cc ; fi

g_line.cc: $(top_srcdir)/libplot/g_line.c $(ALLHEADERS)
	@rm -f g_line.cc ; if $(LN_S) $(top_srcdir)/libplot/g_line.c g_line.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_line.c g_line.cc ; fi

g_linemod.cc: $(top_srcdir)/libplot/g_linemod.c $(ALLHEADERS)
	@rm -f g_linemod.cc ; if $(LN_S) $(top_srcdir)/libplot/g_linemod.c g_linemod.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_linemod.c g_linemod.cc ; fi

g_linewidth.cc: $(top_srcdir)/libplot/g_linewidth.c $(ALLHEADERS)
	@rm -f g_linewidth.cc ; if $(LN_S) $(top_srcdir)/libplot/g_linewidth.c g_linewidth.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_linewidth.c g_linewidth.cc ; fi

g_mark.cc: $(top_srcdir)/libplot/g_mark.c $(ALLHEADERS)
	@rm -f g_mark.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mark.c g_mark.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mark.c g_mark.cc ; fi

g_matrix.cc: $(top_srcdir)/libplot/g_matrix.c $(ALLHEADERS)
	@rm -f g_matrix.cc ; if $(LN_S) $(top_srcdir)/libplot/g_matrix.c g_matrix.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_matrix.c g_matrix.cc ; fi

g_miscmi.cc: $(top_srcdir)/libplot/g_miscmi.c $(ALLHEADERS)
	@rm -f g_miscmi.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miscmi.c g_miscmi.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miscmi.c g_miscmi.cc ; fi

g_miter.cc: $(top_srcdir)/libplot/g_miter.c $(ALLHEADERS)
	@rm -f g_miter.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miter.c g_miter.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miter.c g_miter.cc ; fi

g_mialloc.cc: $(top_srcdir)/libplot/g_mialloc.c $(ALLHEADERS)
	@rm -f g_mialloc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mialloc.c g_mialloc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mialloc.c g_mialloc.cc ; fi

g_miarc.cc: $(top_srcdir)/libplot/g_miarc.c $(ALLHEADERS)
	@rm -f g_miarc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miarc.c g_miarc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miarc.c g_miarc.cc ; fi

g_mifllarc.cc: $(top_srcdir)/libplot/g_mifllarc.c $(ALLHEADERS)
	@rm -f g_mifllarc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mifllarc.c g_mifllarc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mifllarc.c g_mifllarc.cc ; fi

g_mifllrct.cc: $(top_srcdir)/libplot/g_mifllrct.c $(ALLHEADERS)
	@rm -f g_mifllrct.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mifllrct.c g_mifllrct.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mifllrct.c g_mifllrct.cc ; fi

g_mifllspn.cc: $(top_srcdir)/libplot/g_mifllspn.c $(ALLHEADERS)
	@rm -f g_mifllspn.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mifllspn.c g_mifllspn.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mifllspn.c g_mifllspn.cc ; fi

g_mifplycon.cc: $(top_srcdir)/libplot/g_mifplycon.c $(ALLHEADERS)
	@rm -f g_mifplycon.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mifplycon.c g_mifplycon.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mifplycon.c g_mifplycon.cc ; fi

g_miply.cc: $(top_srcdir)/libplot/g_miply.c $(ALLHEADERS)
	@rm -f g_miply.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miply.c g_miply.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miply.c g_miply.cc ; fi

g_miplycon.cc: $(top_srcdir)/libplot/g_miplycon.c $(ALLHEADERS)
	@rm -f g_miplycon.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miplycon.c g_miplycon.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miplycon.c g_miplycon.cc ; fi

g_miplygen.cc: $(top_srcdir)/libplot/g_miplygen.c $(ALLHEADERS)
	@rm -f g_miplygen.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miplygen.c g_miplygen.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miplygen.c g_miplygen.cc ; fi

g_miplypnt.cc: $(top_srcdir)/libplot/g_miplypnt.c $(ALLHEADERS)
	@rm -f g_miplypnt.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miplypnt.c g_miplypnt.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miplypnt.c g_miplypnt.cc ; fi

g_miplyutil.cc: $(top_srcdir)/libplot/g_miplyutil.c $(ALLHEADERS)
	@rm -f g_miplyutil.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miplyutil.c g_miplyutil.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miplyutil.c g_miplyutil.cc ; fi

g_mispans.cc: $(top_srcdir)/libplot/g_mispans.c $(ALLHEADERS)
	@rm -f g_mispans.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mispans.c g_mispans.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mispans.c g_mispans.cc ; fi

g_miwidelin.cc: $(top_srcdir)/libplot/g_miwidelin.c $(ALLHEADERS)
	@rm -f g_miwidelin.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miwidelin.c g_miwidelin.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miwidelin.c g_miwidelin.cc ; fi

g_mizerarc.cc: $(top_srcdir)/libplot/g_mizerarc.c $(ALLHEADERS)
	@rm -f g_mizerarc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mizerarc.c g_mizerarc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mizerarc.c g_mizerarc.cc ; fi

g_mizerolin.cc: $(top_srcdir)/libplot/g_mizerolin.c $(ALLHEADERS)
	@rm -f g_mizerolin.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mizerolin.c g_mizerolin.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mizerolin.c g_mizerolin.cc ; fi

g_move.cc: $(top_srcdir)/libplot/g_move.c $(ALLHEADERS)
	@rm -f g_move.cc ; if $(LN_S) $(top_srcdir)/libplot/g_move.c g_move.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_move.c g_move.cc ; fi

g_openpl.cc: $(top_srcdir)/libplot/g_openpl.c $(ALLHEADERS)
	@rm -f g_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/g_openpl.c g_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_openpl.c g_openpl.cc ; fi

g_outbuf.cc: $(top_srcdir)/libplot/g_outbuf.c $(ALLHEADERS)
	@rm -f g_outbuf.cc ; if $(LN_S) $(top_srcdir)/libplot/g_outbuf.c g_outbuf.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_outbuf.c g_outbuf.cc ; fi

g_outfile.cc: $(top_srcdir)/libplot/g_outfile.c $(ALLHEADERS)
	@rm -f g_outfile.cc ; if $(LN_S) $(top_srcdir)/libplot/g_outfile.c g_outfile.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_outfile.c g_outfile.cc ; fi

g_pagetype.cc: $(top_srcdir)/libplot/g_pagetype.c $(ALLHEADERS)
	@rm -f g_pagetype.cc ; if $(LN_S) $(top_srcdir)/libplot/g_pagetype.c g_pagetype.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_pagetype.c g_pagetype.cc ; fi

g_params.cc: $(top_srcdir)/libplot/g_params.c $(ALLHEADERS)
	@rm -f g_params.cc ; if $(LN_S) $(top_srcdir)/libplot/g_params.c g_params.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_params.c g_params.cc ; fi

g_pencolor.cc: $(top_srcdir)/libplot/g_pencolor.c $(ALLHEADERS)
	@rm -f g_pencolor.cc ; if $(LN_S) $(top_srcdir)/libplot/g_pencolor.c g_pencolor.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_pencolor.c g_pencolor.cc ; fi

g_point.cc: $(top_srcdir)/libplot/g_point.c $(ALLHEADERS)
	@rm -f g_point.cc ; if $(LN_S) $(top_srcdir)/libplot/g_point.c g_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_point.c g_point.cc ; fi

g_range.cc: $(top_srcdir)/libplot/g_range.c $(ALLHEADERS)
	@rm -f g_range.cc ; if $(LN_S) $(top_srcdir)/libplot/g_range.c g_range.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_range.c g_range.cc ; fi

g_relative.cc: $(top_srcdir)/libplot/g_relative.c $(ALLHEADERS)
	@rm -f g_relative.cc ; if $(LN_S) $(top_srcdir)/libplot/g_relative.c g_relative.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_relative.c g_relative.cc ; fi

g_reststate.cc: $(top_srcdir)/libplot/g_reststate.c $(ALLHEADERS)
	@rm -f g_reststate.cc ; if $(LN_S) $(top_srcdir)/libplot/g_reststate.c g_reststate.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_reststate.c g_reststate.cc ; fi

g_retrieve.cc: $(top_srcdir)/libplot/g_retrieve.c $(ALLHEADERS)
	@rm -f g_retrieve.cc ; if $(LN_S) $(top_srcdir)/libplot/g_retrieve.c g_retrieve.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_retrieve.c g_retrieve.cc ; fi

g_savestate.cc: $(top_srcdir)/libplot/g_savestate.c $(ALLHEADERS)
	@rm -f g_savestate.cc ; if $(LN_S) $(top_srcdir)/libplot/g_savestate.c g_savestate.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_savestate.c g_savestate.cc ; fi

g_space.cc: $(top_srcdir)/libplot/g_space.c $(ALLHEADERS)
	@rm -f g_space.cc ; if $(LN_S) $(top_srcdir)/libplot/g_space.c g_space.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_space.c g_space.cc ; fi

g_textangle.cc: $(top_srcdir)/libplot/g_textangle.c $(ALLHEADERS)
	@rm -f g_textangle.cc ; if $(LN_S) $(top_srcdir)/libplot/g_textangle.c g_textangle.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_textangle.c g_textangle.cc ; fi

g_vector.cc: $(top_srcdir)/libplot/g_vector.c $(ALLHEADERS)
	@rm -f g_vector.cc ; if $(LN_S) $(top_srcdir)/libplot/g_vector.c g_vector.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_vector.c g_vector.cc ; fi

g_write.cc: $(top_srcdir)/libplot/g_write.c $(ALLHEADERS)
	@rm -f g_write.cc ; if $(LN_S) $(top_srcdir)/libplot/g_write.c g_write.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_write.c g_write.cc ; fi

g_xmalloc.cc: $(top_srcdir)/libplot/g_xmalloc.c $(ALLHEADERS)
	@rm -f g_xmalloc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_xmalloc.c g_xmalloc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_xmalloc.c g_xmalloc.cc ; fi

g_xstring.cc: $(top_srcdir)/libplot/g_xstring.c $(ALLHEADERS)
	@rm -f g_xstring.cc ; if $(LN_S) $(top_srcdir)/libplot/g_xstring.c g_xstring.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_xstring.c g_xstring.cc ; fi

h_alab_pcl.cc: $(top_srcdir)/libplot/h_alab_pcl.c $(ALLHEADERS)
	@rm -f h_alab_pcl.cc ; if $(LN_S) $(top_srcdir)/libplot/h_alab_pcl.c h_alab_pcl.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_alab_pcl.c h_alab_pcl.cc ; fi

h_attribs.cc: $(top_srcdir)/libplot/h_attribs.c $(ALLHEADERS)
	@rm -f h_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/h_attribs.c h_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_attribs.c h_attribs.cc ; fi

h_box.cc: $(top_srcdir)/libplot/h_box.c $(ALLHEADERS)
	@rm -f h_box.cc ; if $(LN_S) $(top_srcdir)/libplot/h_box.c h_box.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_box.c h_box.cc ; fi

h_circ.cc: $(top_srcdir)/libplot/h_circ.c $(ALLHEADERS)
	@rm -f h_circ.cc ; if $(LN_S) $(top_srcdir)/libplot/h_circ.c h_circ.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_circ.c h_circ.cc ; fi

h_closepl.cc: $(top_srcdir)/libplot/h_closepl.c $(ALLHEADERS)
	@rm -f h_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/h_closepl.c h_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_closepl.c h_closepl.cc ; fi

h_color.cc: $(top_srcdir)/libplot/h_color.c $(ALLHEADERS)
	@rm -f h_color.cc ; if $(LN_S) $(top_srcdir)/libplot/h_color.c h_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_color.c h_color.cc ; fi

h_defplot.cc: $(top_srcdir)/libplot/h_defplot.c $(ALLHEADERS)
	@rm -f h_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/h_defplot.c h_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_defplot.c h_defplot.cc ; fi

h_endpath.cc: $(top_srcdir)/libplot/h_endpath.c $(ALLHEADERS)
	@rm -f h_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/h_endpath.c h_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_endpath.c h_endpath.cc ; fi

h_font.cc: $(top_srcdir)/libplot/h_font.c $(ALLHEADERS)
	@rm -f h_font.cc ; if $(LN_S) $(top_srcdir)/libplot/h_font.c h_font.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_font.c h_font.cc ; fi

h_linewidth.cc: $(top_srcdir)/libplot/h_linewidth.c $(ALLHEADERS)
	@rm -f h_linewidth.cc ; if $(LN_S) $(top_srcdir)/libplot/h_linewidth.c h_linewidth.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_linewidth.c h_linewidth.cc ; fi

h_openpl.cc: $(top_srcdir)/libplot/h_openpl.c $(ALLHEADERS)
	@rm -f h_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/h_openpl.c h_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_openpl.c h_openpl.cc ; fi

h_point.cc: $(top_srcdir)/libplot/h_point.c $(ALLHEADERS)
	@rm -f h_point.cc ; if $(LN_S) $(top_srcdir)/libplot/h_point.c h_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_point.c h_point.cc ; fi

h_position.cc: $(top_srcdir)/libplot/h_position.c $(ALLHEADERS)
	@rm -f h_position.cc ; if $(LN_S) $(top_srcdir)/libplot/h_position.c h_position.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_position.c h_position.cc ; fi

n_closepl.cc: $(top_srcdir)/libplot/n_closepl.c $(ALLHEADERS)
	@rm -f n_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/n_closepl.c n_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_closepl.c n_closepl.cc ; fi

n_defplot.cc: $(top_srcdir)/libplot/n_defplot.c $(ALLHEADERS)
	@rm -f n_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/n_defplot.c n_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_defplot.c n_defplot.cc ; fi

n_ellipse.cc: $(top_srcdir)/libplot/n_ellipse.c $(ALLHEADERS)
	@rm -f n_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/n_ellipse.c n_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_ellipse.c n_ellipse.cc ; fi

n_endpath.cc: $(top_srcdir)/libplot/n_endpath.c $(ALLHEADERS)
	@rm -f n_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/n_endpath.c n_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_endpath.c n_endpath.cc ; fi

n_erase.cc: $(top_srcdir)/libplot/n_erase.c $(ALLHEADERS)
	@rm -f n_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/n_erase.c n_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_erase.c n_erase.cc ; fi

n_openpl.cc: $(top_srcdir)/libplot/n_openpl.c $(ALLHEADERS)
	@rm -f n_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/n_openpl.c n_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_openpl.c n_openpl.cc ; fi

n_point.cc: $(top_srcdir)/libplot/n_point.c $(ALLHEADERS)
	@rm -f n_point.cc ; if $(LN_S) $(top_srcdir)/libplot/n_point.c n_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_point.c n_point.cc ; fi

i_closepl.cc: $(top_srcdir)/libplot/i_closepl.c $(ALLHEADERS)
	@rm -f i_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/i_closepl.c i_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_closepl.c i_closepl.cc ; fi

i_color.cc: $(top_srcdir)/libplot/i_color.c $(ALLHEADERS)
	@rm -f i_color.cc ; if $(LN_S) $(top_srcdir)/libplot/i_color.c i_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_color.c i_color.cc ; fi

i_defplot.cc: $(top_srcdir)/libplot/i_defplot.c $(ALLHEADERS)
	@rm -f i_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/i_defplot.c i_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_defplot.c i_defplot.cc ; fi

i_ellipse.cc: $(top_srcdir)/libplot/i_ellipse.c $(ALLHEADERS)
	@rm -f i_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/i_ellipse.c i_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_ellipse.c i_ellipse.cc ; fi

i_endpath.cc: $(top_srcdir)/libplot/i_endpath.c $(ALLHEADERS)
	@rm -f i_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/i_endpath.c i_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_endpath.c i_endpath.cc ; fi

i_erase.cc: $(top_srcdir)/libplot/i_erase.c $(ALLHEADERS)
	@rm -f i_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/i_erase.c i_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_erase.c i_erase.cc ; fi

i_openpl.cc: $(top_srcdir)/libplot/i_openpl.c $(ALLHEADERS)
	@rm -f i_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/i_openpl.c i_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_openpl.c i_openpl.cc ; fi

i_point.cc: $(top_srcdir)/libplot/i_point.c $(ALLHEADERS)
	@rm -f i_point.cc ; if $(LN_S) $(top_srcdir)/libplot/i_point.c i_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_point.c i_point.cc ; fi

i_rle.cc: $(top_srcdir)/libplot/i_rle.c $(ALLHEADERS)
	@rm -f i_rle.cc ; if $(LN_S) $(top_srcdir)/libplot/i_rle.c i_rle.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_rle.c i_rle.cc ; fi

m_alabel.cc: $(top_srcdir)/libplot/m_alabel.c $(ALLHEADERS)
	@rm -f m_alabel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_alabel.c m_alabel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_alabel.c m_alabel.cc ; fi

m_arc.cc: $(top_srcdir)/libplot/m_arc.c $(ALLHEADERS)
	@rm -f m_arc.cc ; if $(LN_S) $(top_srcdir)/libplot/m_arc.c m_arc.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_arc.c m_arc.cc ; fi

m_arcrel.cc: $(top_srcdir)/libplot/m_arcrel.c $(ALLHEADERS)
	@rm -f m_arcrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_arcrel.c m_arcrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_arcrel.c m_arcrel.cc ; fi

m_bez.cc: $(top_srcdir)/libplot/m_bez.c $(ALLHEADERS)
	@rm -f m_bez.cc ; if $(LN_S) $(top_srcdir)/libplot/m_bez.c m_bez.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_bez.c m_bez.cc ; fi

m_bezrel.cc: $(top_srcdir)/libplot/m_bezrel.c $(ALLHEADERS)
	@rm -f m_bezrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_bezrel.c m_bezrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_bezrel.c m_bezrel.cc ; fi

m_bgcolor.cc: $(top_srcdir)/libplot/m_bgcolor.c $(ALLHEADERS)
	@rm -f m_bgcolor.cc ; if $(LN_S) $(top_srcdir)/libplot/m_bgcolor.c m_bgcolor.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_bgcolor.c m_bgcolor.cc ; fi

m_box.cc: $(top_srcdir)/libplot/m_box.c $(ALLHEADERS)
	@rm -f m_box.cc ; if $(LN_S) $(top_srcdir)/libplot/m_box.c m_box.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_box.c m_box.cc ; fi

m_boxrel.cc: $(top_srcdir)/libplot/m_boxrel.c $(ALLHEADERS)
	@rm -f m_boxrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_boxrel.c m_boxrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_boxrel.c m_boxrel.cc ; fi

m_capmod.cc: $(top_srcdir)/libplot/m_capmod.c $(ALLHEADERS)
	@rm -f m_capmod.cc ; if $(LN_S) $(top_srcdir)/libplot/m_capmod.c m_capmod.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_capmod.c m_capmod.cc ; fi

m_circ.cc: $(top_srcdir)/libplot/m_circ.c $(ALLHEADERS)
	@rm -f m_circ.cc ; if $(LN_S) $(top_srcdir)/libplot/m_circ.c m_circ.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_circ.c m_circ.cc ; fi

m_circrel.cc: $(top_srcdir)/libplot/m_circrel.c $(ALLHEADERS)
	@rm -f m_circrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_circrel.c m_circrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_circrel.c m_circrel.cc ; fi

m_closepl.cc: $(top_srcdir)/libplot/m_closepl.c $(ALLHEADERS)
	@rm -f m_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/m_closepl.c m_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_closepl.c m_closepl.cc ; fi

m_concat.cc: $(top_srcdir)/libplot/m_concat.c $(ALLHEADERS)
	@rm -f m_concat.cc ; if $(LN_S) $(top_srcdir)/libplot/m_concat.c m_concat.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_concat.c m_concat.cc ; fi

m_cont.cc: $(top_srcdir)/libplot/m_cont.c $(ALLHEADERS)
	@rm -f m_cont.cc ; if $(LN_S) $(top_srcdir)/libplot/m_cont.c m_cont.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_cont.c m_cont.cc ; fi

m_dash.cc: $(top_srcdir)/libplot/m_dash.c $(ALLHEADERS)
	@rm -f m_dash.cc ; if $(LN_S) $(top_srcdir)/libplot/m_dash.c m_dash.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_dash.c m_dash.cc ; fi

m_contrel.cc: $(top_srcdir)/libplot/m_contrel.c $(ALLHEADERS)
	@rm -f m_contrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_contrel.c m_contrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_contrel.c m_contrel.cc ; fi

m_defplot.cc: $(top_srcdir)/libplot/m_defplot.c $(ALLHEADERS)
	@rm -f m_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/m_defplot.c m_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_defplot.c m_defplot.cc ; fi

m_earc.cc: $(top_srcdir)/libplot/m_earc.c $(ALLHEADERS)
	@rm -f m_earc.cc ; if $(LN_S) $(top_srcdir)/libplot/m_earc.c m_earc.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_earc.c m_earc.cc ; fi

m_earcrel.cc: $(top_srcdir)/libplot/m_earcrel.c $(ALLHEADERS)
	@rm -f m_earcrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_earcrel.c m_earcrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_earcrel.c m_earcrel.cc ; fi

m_ellipse.cc: $(top_srcdir)/libplot/m_ellipse.c $(ALLHEADERS)
	@rm -f m_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/m_ellipse.c m_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_ellipse.c m_ellipse.cc ; fi

m_emit.cc: $(top_srcdir)/libplot/m_emit.c $(ALLHEADERS)
	@rm -f m_emit.cc ; if $(LN_S) $(top_srcdir)/libplot/m_emit.c m_emit.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_emit.c m_emit.cc ; fi

m_endpath.cc: $(top_srcdir)/libplot/m_endpath.c $(ALLHEADERS)
	@rm -f m_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/m_endpath.c m_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_endpath.c m_endpath.cc ; fi

m_erase.cc: $(top_srcdir)/libplot/m_erase.c $(ALLHEADERS)
	@rm -f m_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/m_erase.c m_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_erase.c m_erase.cc ; fi

m_fillcolor.cc: $(top_srcdir)/libplot/m_fillcolor.c $(ALLHEADERS)
	@rm -f m_fillcolor.cc ; if $(LN_S) $(top_srcdir)/libplot/m_fillcolor.c m_fillcolor.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_fillcolor.c m_fillcolor.cc ; fi

m_fillrule.cc: $(top_srcdir)/libplot/m_fillrule.c $(ALLHEADERS)
	@rm -f m_fillrule.cc ; if $(LN_S) $(top_srcdir)/libplot/m_fillrule.c m_fillrule.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_fillrule.c m_fillrule.cc ; fi

m_filltype.cc: $(top_srcdir)/libplot/m_filltype.c $(ALLHEADERS)
	@rm -f m_filltype.cc ; if $(LN_S) $(top_srcdir)/libplot/m_filltype.c m_filltype.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_filltype.c m_filltype.cc ; fi

m_fontname.cc: $(top_srcdir)/libplot/m_fontname.c $(ALLHEADERS)
	@rm -f m_fontname.cc ; if $(LN_S) $(top_srcdir)/libplot/m_fontname.c m_fontname.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_fontname.c m_fontname.cc ; fi

m_fontsize.cc: $(top_srcdir)/libplot/m_fontsize.c $(ALLHEADERS)
	@rm -f m_fontsize.cc ; if $(LN_S) $(top_srcdir)/libplot/m_fontsize.c m_fontsize.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_fontsize.c m_fontsize.cc ; fi

m_joinmod.cc: $(top_srcdir)/libplot/m_joinmod.c $(ALLHEADERS)
	@rm -f m_joinmod.cc ; if $(LN_S) $(top_srcdir)/libplot/m_joinmod.c m_joinmod.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_joinmod.c m_joinmod.cc ; fi

m_label.cc: $(top_srcdir)/libplot/m_label.c $(ALLHEADERS)
	@rm -f m_label.cc ; if $(LN_S) $(top_srcdir)/libplot/m_label.c m_label.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_label.c m_label.cc ; fi

m_line.cc: $(top_srcdir)/libplot/m_line.c $(ALLHEADERS)
	@rm -f m_line.cc ; if $(LN_S) $(top_srcdir)/libplot/m_line.c m_line.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_line.c m_line.cc ; fi

m_linemod.cc: $(top_srcdir)/libplot/m_linemod.c $(ALLHEADERS)
	@rm -f m_linemod.cc ; if $(LN_S) $(top_srcdir)/libplot/m_linemod.c m_linemod.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_linemod.c m_linemod.cc ; fi

m_linerel.cc: $(top_srcdir)/libplot/m_linerel.c $(ALLHEADERS)
	@rm -f m_linerel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_linerel.c m_linerel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_linerel.c m_linerel.cc ; fi

m_linewidth.cc: $(top_srcdir)/libplot/m_linewidth.c $(ALLHEADERS)
	@rm -f m_linewidth.cc ; if $(LN_S) $(top_srcdir)/libplot/m_linewidth.c m_linewidth.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_linewidth.c m_linewidth.cc ; fi

m_mark.cc: $(top_srcdir)/libplot/m_mark.c $(ALLHEADERS)
	@rm -f m_mark.cc ; if $(LN_S) $(top_srcdir)/libplot/m_mark.c m_mark.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_mark.c m_mark.cc ; fi

m_markrel.cc: $(top_srcdir)/libplot/m_markrel.c $(ALLHEADERS)
	@rm -f m_markrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_markrel.c m_markrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_markrel.c m_markrel.cc ; fi

m_miter.cc: $(top_srcdir)/libplot/m_miter.c $(ALLHEADERS)
	@rm -f m_miter.cc ; if $(LN_S) $(top_srcdir)/libplot/m_miter.c m_miter.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_miter.c m_miter.cc ; fi

m_move.cc: $(top_srcdir)/libplot/m_move.c $(ALLHEADERS)
	@rm -f m_move.cc ; if $(LN_S) $(top_srcdir)/libplot/m_move.c m_move.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_move.c m_move.cc ; fi

m_moverel.cc: $(top_srcdir)/libplot/m_moverel.c $(ALLHEADERS)
	@rm -f m_moverel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_moverel.c m_moverel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_moverel.c m_moverel.cc ; fi

m_openpl.cc: $(top_srcdir)/libplot/m_openpl.c $(ALLHEADERS)
	@rm -f m_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/m_openpl.c m_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_openpl.c m_openpl.cc ; fi

m_pencolor.cc: $(top_srcdir)/libplot/m_pencolor.c $(ALLHEADERS)
	@rm -f m_pencolor.cc ; if $(LN_S) $(top_srcdir)/libplot/m_pencolor.c m_pencolor.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_pencolor.c m_pencolor.cc ; fi

m_point.cc: $(top_srcdir)/libplot/m_point.c $(ALLHEADERS)
	@rm -f m_point.cc ; if $(LN_S) $(top_srcdir)/libplot/m_point.c m_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_point.c m_point.cc ; fi

m_pointrel.cc: $(top_srcdir)/libplot/m_pointrel.c $(ALLHEADERS)
	@rm -f m_pointrel.cc ; if $(LN_S) $(top_srcdir)/libplot/m_pointrel.c m_pointrel.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_pointrel.c m_pointrel.cc ; fi

m_reststate.cc: $(top_srcdir)/libplot/m_reststate.c $(ALLHEADERS)
	@rm -f m_reststate.cc ; if $(LN_S) $(top_srcdir)/libplot/m_reststate.c m_reststate.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_reststate.c m_reststate.cc ; fi

m_savestate.cc: $(top_srcdir)/libplot/m_savestate.c $(ALLHEADERS)
	@rm -f m_savestate.cc ; if $(LN_S) $(top_srcdir)/libplot/m_savestate.c m_savestate.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_savestate.c m_savestate.cc ; fi

m_space.cc: $(top_srcdir)/libplot/m_space.c $(ALLHEADERS)
	@rm -f m_space.cc ; if $(LN_S) $(top_srcdir)/libplot/m_space.c m_space.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_space.c m_space.cc ; fi

m_textangle.cc: $(top_srcdir)/libplot/m_textangle.c $(ALLHEADERS)
	@rm -f m_textangle.cc ; if $(LN_S) $(top_srcdir)/libplot/m_textangle.c m_textangle.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_textangle.c m_textangle.cc ; fi

p_alab_ps.cc: $(top_srcdir)/libplot/p_alab_ps.c $(ALLHEADERS)
	@rm -f p_alab_ps.cc ; if $(LN_S) $(top_srcdir)/libplot/p_alab_ps.c p_alab_ps.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_alab_ps.c p_alab_ps.cc ; fi

p_closepl.cc: $(top_srcdir)/libplot/p_closepl.c $(ALLHEADERS)
	@rm -f p_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/p_closepl.c p_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_closepl.c p_closepl.cc ; fi

p_color.cc: $(top_srcdir)/libplot/p_color.c $(ALLHEADERS)
	@rm -f p_color.cc ; if $(LN_S) $(top_srcdir)/libplot/p_color.c p_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_color.c p_color.cc ; fi

p_color2.cc: $(top_srcdir)/libplot/p_color2.c $(ALLHEADERS)
	@rm -f p_color2.cc ; if $(LN_S) $(top_srcdir)/libplot/p_color2.c p_color2.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_color2.c p_color2.cc ; fi

p_defplot.cc: $(top_srcdir)/libplot/p_defplot.c $(ALLHEADERS)
	@rm -f p_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/p_defplot.c p_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_defplot.c p_defplot.cc ; fi

p_ellipse.cc: $(top_srcdir)/libplot/p_ellipse.c $(ALLHEADERS)
	@rm -f p_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/p_ellipse.c p_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_ellipse.c p_ellipse.cc ; fi

p_endpath.cc: $(top_srcdir)/libplot/p_endpath.c $(ALLHEADERS)
	@rm -f p_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/p_endpath.c p_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_endpath.c p_endpath.cc ; fi

p_erase.cc: $(top_srcdir)/libplot/p_erase.c $(ALLHEADERS)
	@rm -f p_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/p_erase.c p_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_erase.c p_erase.cc ; fi

p_openpl.cc: $(top_srcdir)/libplot/p_openpl.c $(ALLHEADERS)
	@rm -f p_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/p_openpl.c p_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_openpl.c p_openpl.cc ; fi

p_point.cc: $(top_srcdir)/libplot/p_point.c $(ALLHEADERS)
	@rm -f p_point.cc ; if $(LN_S) $(top_srcdir)/libplot/p_point.c p_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_point.c p_point.cc ; fi

t_attribs.cc: $(top_srcdir)/libplot/t_attribs.c $(ALLHEADERS)
	@rm -f t_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/t_attribs.c t_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_attribs.c t_attribs.cc ; fi

t_closepl.cc: $(top_srcdir)/libplot/t_closepl.c $(ALLHEADERS)
	@rm -f t_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/t_closepl.c t_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_closepl.c t_closepl.cc ; fi

t_color.cc: $(top_srcdir)/libplot/t_color.c $(ALLHEADERS)
	@rm -f t_color.cc ; if $(LN_S) $(top_srcdir)/libplot/t_color.c t_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_color.c t_color.cc ; fi

t_color2.cc: $(top_srcdir)/libplot/t_color2.c $(ALLHEADERS)
	@rm -f t_color2.cc ; if $(LN_S) $(top_srcdir)/libplot/t_color2.c t_color2.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_color2.c t_color2.cc ; fi

t_cont.cc: $(top_srcdir)/libplot/t_cont.c $(ALLHEADERS)
	@rm -f t_cont.cc ; if $(LN_S) $(top_srcdir)/libplot/t_cont.c t_cont.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_cont.c t_cont.cc ; fi

t_defplot.cc: $(top_srcdir)/libplot/t_defplot.c $(ALLHEADERS)
	@rm -f t_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/t_defplot.c t_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_defplot.c t_defplot.cc ; fi

t_erase.cc: $(top_srcdir)/libplot/t_erase.c $(ALLHEADERS)
	@rm -f t_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/t_erase.c t_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_erase.c t_erase.cc ; fi

t_openpl.cc: $(top_srcdir)/libplot/t_openpl.c $(ALLHEADERS)
	@rm -f t_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/t_openpl.c t_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_openpl.c t_openpl.cc ; fi

t_point.cc: $(top_srcdir)/libplot/t_point.c $(ALLHEADERS)
	@rm -f t_point.cc ; if $(LN_S) $(top_srcdir)/libplot/t_point.c t_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_point.c t_point.cc ; fi

t_tek_md.cc: $(top_srcdir)/libplot/t_tek_md.c $(ALLHEADERS)
	@rm -f t_tek_md.cc ; if $(LN_S) $(top_srcdir)/libplot/t_tek_md.c t_tek_md.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_tek_md.c t_tek_md.cc ; fi

t_tek_mv.cc: $(top_srcdir)/libplot/t_tek_mv.c $(ALLHEADERS)
	@rm -f t_tek_mv.cc ; if $(LN_S) $(top_srcdir)/libplot/t_tek_mv.c t_tek_mv.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_tek_mv.c t_tek_mv.cc ; fi

t_tek_vec.cc: $(top_srcdir)/libplot/t_tek_vec.c $(ALLHEADERS)
	@rm -f t_tek_vec.cc ; if $(LN_S) $(top_srcdir)/libplot/t_tek_vec.c t_tek_vec.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_tek_vec.c t_tek_vec.cc ; fi

x_alab_x.cc: $(top_srcdir)/libplot/x_alab_x.c $(ALLHEADERS)
	@rm -f x_alab_x.cc ; if $(LN_S) $(top_srcdir)/libplot/x_alab_x.c x_alab_x.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_alab_x.c x_alab_x.cc ; fi

x_attribs.cc: $(top_srcdir)/libplot/x_attribs.c $(ALLHEADERS)
	@rm -f x_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/x_attribs.c x_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_attribs.c x_attribs.cc ; fi

x_box.cc: $(top_srcdir)/libplot/x_box.c $(ALLHEADERS)
	@rm -f x_box.cc ; if $(LN_S) $(top_srcdir)/libplot/x_box.c x_box.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_box.c x_box.cc ; fi

x_closepl.cc: $(top_srcdir)/libplot/x_closepl.c $(ALLHEADERS)
	@rm -f x_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/x_closepl.c x_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_closepl.c x_closepl.cc ; fi

x_color.cc: $(top_srcdir)/libplot/x_color.c $(ALLHEADERS)
	@rm -f x_color.cc ; if $(LN_S) $(top_srcdir)/libplot/x_color.c x_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_color.c x_color.cc ; fi

x_cont.cc: $(top_srcdir)/libplot/x_cont.c $(ALLHEADERS)
	@rm -f x_cont.cc ; if $(LN_S) $(top_srcdir)/libplot/x_cont.c x_cont.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_cont.c x_cont.cc ; fi

x_defplot.cc: $(top_srcdir)/libplot/x_defplot.c $(ALLHEADERS)
	@rm -f x_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/x_defplot.c x_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_defplot.c x_defplot.cc ; fi

x_ellipse.cc: $(top_srcdir)/libplot/x_ellipse.c $(ALLHEADERS)
	@rm -f x_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/x_ellipse.c x_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_ellipse.c x_ellipse.cc ; fi

x_endpath.cc: $(top_srcdir)/libplot/x_endpath.c $(ALLHEADERS)
	@rm -f x_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/x_endpath.c x_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_endpath.c x_endpath.cc ; fi

x_erase.cc: $(top_srcdir)/libplot/x_erase.c $(ALLHEADERS)
	@rm -f x_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/x_erase.c x_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_erase.c x_erase.cc ; fi

x_flushpl.cc: $(top_srcdir)/libplot/x_flushpl.c $(ALLHEADERS)
	@rm -f x_flushpl.cc ; if $(LN_S) $(top_srcdir)/libplot/x_flushpl.c x_flushpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_flushpl.c x_flushpl.cc ; fi

x_openpl.cc: $(top_srcdir)/libplot/x_openpl.c $(ALLHEADERS)
	@rm -f x_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/x_openpl.c x_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_openpl.c x_openpl.cc ; fi

x_point.cc: $(top_srcdir)/libplot/x_point.c $(ALLHEADERS)
	@rm -f x_point.cc ; if $(LN_S) $(top_srcdir)/libplot/x_point.c x_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_point.c x_point.cc ; fi

x_reststate.cc: $(top_srcdir)/libplot/x_reststate.c $(ALLHEADERS)
	@rm -f x_reststate.cc ; if $(LN_S) $(top_srcdir)/libplot/x_reststate.c x_reststate.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_reststate.c x_reststate.cc ; fi

x_retrieve.cc: $(top_srcdir)/libplot/x_retrieve.c $(ALLHEADERS)
	@rm -f x_retrieve.cc ; if $(LN_S) $(top_srcdir)/libplot/x_retrieve.c x_retrieve.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_retrieve.c x_retrieve.cc ; fi

x_savestate.cc: $(top_srcdir)/libplot/x_savestate.c $(ALLHEADERS)
	@rm -f x_savestate.cc ; if $(LN_S) $(top_srcdir)/libplot/x_savestate.c x_savestate.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_savestate.c x_savestate.cc ; fi

y_closepl.cc: $(top_srcdir)/libplot/y_closepl.c $(ALLHEADERS)
	@rm -f y_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/y_closepl.c y_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_closepl.c y_closepl.cc ; fi

y_defplot.cc: $(top_srcdir)/libplot/y_defplot.c $(ALLHEADERS)
	@rm -f y_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/y_defplot.c y_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_defplot.c y_defplot.cc ; fi

y_erase.cc: $(top_srcdir)/libplot/y_erase.c $(ALLHEADERS)
	@rm -f y_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/y_erase.c y_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_erase.c y_erase.cc ; fi

y_openpl.cc: $(top_srcdir)/libplot/y_openpl.c $(ALLHEADERS)
	@rm -f y_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/y_openpl.c y_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_openpl.c y_openpl.cc ; fi


## execute the following command in ../libplot to produce the commands
## that follow
## (for i in extern.h ?_*.h; do echo "$i": "\$(top_srcdir)/libplot/$i"; echo "	@rm -f $i"" ; if \$(LN_S) \$(top_srcdir)/libplot/$i $i"" ; then true ; else cp -p \$(top_srcdir)/libplot/$i $i"" ; fi"; echo; done)

extern.h: $(top_srcdir)/libplot/extern.h
	@rm -f extern.h ; if $(LN_S) $(top_srcdir)/libplot/extern.h extern.h ; then true ; else cp -p $(top_srcdir)/libplot/extern.h extern.h ; fi

g_arc.h: $(top_srcdir)/libplot/g_arc.h
	@rm -f g_arc.h ; if $(LN_S) $(top_srcdir)/libplot/g_arc.h g_arc.h ; then true ; else cp -p $(top_srcdir)/libplot/g_arc.h g_arc.h ; fi

g_cntrlify.h: $(top_srcdir)/libplot/g_cntrlify.h
	@rm -f g_cntrlify.h ; if $(LN_S) $(top_srcdir)/libplot/g_cntrlify.h g_cntrlify.h ; then true ; else cp -p $(top_srcdir)/libplot/g_cntrlify.h g_cntrlify.h ; fi

g_colorname.h: $(top_srcdir)/libplot/g_colorname.h
	@rm -f g_colorname.h ; if $(LN_S) $(top_srcdir)/libplot/g_colorname.h g_colorname.h ; then true ; else cp -p $(top_srcdir)/libplot/g_colorname.h g_colorname.h ; fi

g_control.h: $(top_srcdir)/libplot/g_control.h
	@rm -f g_control.h ; if $(LN_S) $(top_srcdir)/libplot/g_control.h g_control.h ; then true ; else cp -p $(top_srcdir)/libplot/g_control.h g_control.h ; fi

g_her_metr.h: $(top_srcdir)/libplot/g_her_metr.h
	@rm -f g_her_metr.h ; if $(LN_S) $(top_srcdir)/libplot/g_her_metr.h g_her_metr.h ; then true ; else cp -p $(top_srcdir)/libplot/g_her_metr.h g_her_metr.h ; fi

g_jis.h: $(top_srcdir)/libplot/g_jis.h
	@rm -f g_jis.h ; if $(LN_S) $(top_srcdir)/libplot/g_jis.h g_jis.h ; then true ; else cp -p $(top_srcdir)/libplot/g_jis.h g_jis.h ; fi

g_mi.h: $(top_srcdir)/libplot/g_mi.h
	@rm -f g_mi.h ; if $(LN_S) $(top_srcdir)/libplot/g_mi.h g_mi.h ; then true ; else cp -p $(top_srcdir)/libplot/g_mi.h g_mi.h ; fi

g_mifllarc.h: $(top_srcdir)/libplot/g_mifllarc.h
	@rm -f g_mifllarc.h ; if $(LN_S) $(top_srcdir)/libplot/g_mifllarc.h g_mifllarc.h ; then true ; else cp -p $(top_srcdir)/libplot/g_mifllarc.h g_mifllarc.h ; fi

g_mifply.h: $(top_srcdir)/libplot/g_mifply.h
	@rm -f g_mifply.h ; if $(LN_S) $(top_srcdir)/libplot/g_mifply.h g_mifply.h ; then true ; else cp -p $(top_srcdir)/libplot/g_mifply.h g_mifply.h ; fi

g_miline.h: $(top_srcdir)/libplot/g_miline.h
	@rm -f g_miline.h ; if $(LN_S) $(top_srcdir)/libplot/g_miline.h g_miline.h ; then true ; else cp -p $(top_srcdir)/libplot/g_miline.h g_miline.h ; fi

g_miply.h: $(top_srcdir)/libplot/g_miply.h
	@rm -f g_miply.h ; if $(LN_S) $(top_srcdir)/libplot/g_miply.h g_miply.h ; then true ; else cp -p $(top_srcdir)/libplot/g_miply.h g_miply.h ; fi

g_miscanfill.h: $(top_srcdir)/libplot/g_miscanfill.h
	@rm -f g_miscanfill.h ; if $(LN_S) $(top_srcdir)/libplot/g_miscanfill.h g_miscanfill.h ; then true ; else cp -p $(top_srcdir)/libplot/g_miscanfill.h g_miscanfill.h ; fi

g_mispans.h: $(top_srcdir)/libplot/g_mispans.h
	@rm -f g_mispans.h ; if $(LN_S) $(top_srcdir)/libplot/g_mispans.h g_mispans.h ; then true ; else cp -p $(top_srcdir)/libplot/g_mispans.h g_mispans.h ; fi

g_miwidelin.h: $(top_srcdir)/libplot/g_miwidelin.h
	@rm -f g_miwidelin.h ; if $(LN_S) $(top_srcdir)/libplot/g_miwidelin.h g_miwidelin.h ; then true ; else cp -p $(top_srcdir)/libplot/g_miwidelin.h g_miwidelin.h ; fi

g_mizerarc.h: $(top_srcdir)/libplot/g_mizerarc.h
	@rm -f g_mizerarc.h ; if $(LN_S) $(top_srcdir)/libplot/g_mizerarc.h g_mizerarc.h ; then true ; else cp -p $(top_srcdir)/libplot/g_mizerarc.h g_mizerarc.h ; fi

g_pagetype.h: $(top_srcdir)/libplot/g_pagetype.h
	@rm -f g_pagetype.h ; if $(LN_S) $(top_srcdir)/libplot/g_pagetype.h g_pagetype.h ; then true ; else cp -p $(top_srcdir)/libplot/g_pagetype.h g_pagetype.h ; fi

g_params.h: $(top_srcdir)/libplot/g_params.h
	@rm -f g_params.h ; if $(LN_S) $(top_srcdir)/libplot/g_params.h g_params.h ; then true ; else cp -p $(top_srcdir)/libplot/g_params.h g_params.h ; fi

h_stick.h: $(top_srcdir)/libplot/h_stick.h
	@rm -f h_stick.h ; if $(LN_S) $(top_srcdir)/libplot/h_stick.h h_stick.h ; then true ; else cp -p $(top_srcdir)/libplot/h_stick.h h_stick.h ; fi

i_rle.h: $(top_srcdir)/libplot/i_rle.h
	@rm -f i_rle.h ; if $(LN_S) $(top_srcdir)/libplot/i_rle.h i_rle.h ; then true ; else cp -p $(top_srcdir)/libplot/i_rle.h i_rle.h ; fi

p_header.h: $(top_srcdir)/libplot/p_header.h
	@rm -f p_header.h ; if $(LN_S) $(top_srcdir)/libplot/p_header.h p_header.h ; then true ; else cp -p $(top_srcdir)/libplot/p_header.h p_header.h ; fi

##EOF
