/* This file defines the initializations for HPGLPlotter and PCLPlotter
   objects including both private data and public methods.  There is a
   one-to-one correspondence between public methods and user-callable
   functions in the C API. */

/* Originally, the only differences between the two types of Plotter were
   the PCL5 control codes that must be emitted to switch a PCL5 printer
   into HP-GL/2 mode, and back out of it.

   More recently, the two types of Plotter are distinguished by their
   viewport positioning.  A PCL Plotter positions its viewport on the page
   in the same position that a PS, AI, or Fig Plotter does, i.e. it centers
   it.  But a pure HPGL[/2] Plotter doesn't know where on the page the
   origin of the device coordinate system lies.  (Though it's probably
   close to a corner.)  Nor does can it set programmatically whether it's
   plotting in portrait or landscape mode.  (It can flip between them, but
   it doesn't know which is which.)

   So HPGL Plotters use a viewport of the same size as PCL, PS, AI, and Fig
   Plotters.  But they don't position it: the lower left corner of the
   viewport is chosen to be the origin of the device coordinate system:
   what in HP-GL[/2] jargon is called "scaling point P1".

   For this to look reasonably good, the viewport needs to have a size
   appropriate for an HP-GL[/2] device.  And in fact, that's what
   determines our choice of viewport size -- for all Plotters, not just
   HPGLPlotters.  See comments in g_pagetype.h.  */

#include "sys-defines.h"
#include "extern.h"

#define MAX_COLOR_NAME_LEN 32	/* long enough for all known colors */

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a HPGLPlotter struct. */
const Plotter _h_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _h_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _h_endpath, _g_endsubpath, _g_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _h_fbox, _g_fboxrel, _h_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _h_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _h_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _h_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _h_initialize, _h_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_falabel_hershey, _h_falabel_ps, _h_falabel_pcl, _h_falabel_stick, _g_falabel_other,
  _g_flabelwidth_hershey, _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, _g_flabelwidth_other,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _h_set_font,
  /* private low-level `sync line attributes' method */
  _h_set_attributes,
  /* private low-level `sync color' methods */
  _h_set_pen_color,
  _h_set_fill_color,
  _g_set_bg_color,
  /* private low-level `sync position' method */
  _h_set_position,
  /* error handlers */
  _g_warning,
  _g_error,
  /* low-level output routines */
  _g_write_byte,
  _g_write_bytes,
  _g_write_string
};
#endif /* not LIBPLOTTER */

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a PCLPlotter struct.  It is the same as the above except for the
   different initialization and termination routines. */
const Plotter _q_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _h_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _h_endpath, _g_endsubpath, _g_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _h_fbox, _g_fboxrel, _h_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _h_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _h_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _h_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _q_initialize, _q_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_falabel_hershey, _h_falabel_ps, _h_falabel_pcl, _h_falabel_stick, _g_falabel_other,
  _g_flabelwidth_hershey, _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, _g_flabelwidth_other,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _h_set_font,
  /* private low-level `sync line attributes' method */
  _h_set_attributes,
  /* private low-level `sync color' methods */
  _h_set_pen_color,
  _h_set_fill_color,
  _g_set_bg_color,
  /* private low-level `sync position' method */
  _h_set_position,
  /* error handlers */
  _g_warning,
  _g_error,
  /* low-level output routines */
  _g_write_byte,
  _g_write_bytes,
  _g_write_string
};
#endif /* not LIBPLOTTER */

/* The private `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  When this
   is invoked, _plotter points to the Plotter that has just been
   created. */

/* The initializations for HPGL and PCL Plotters are similar.

   For HPGL Plotters, we determine the HP-GL version from the environment
   variable HPGL_VERSION ("1", "1.5", or "2", meaning generic HP-GL,
   HP7550A, and modern HP-GL/2 respectively), and determine the page size
   and the location on the page of the viewport, so that we'll be able to
   work out the map from user coordinates to device coordinates in
   g_space.c.

   We allow the user to shift the location of the viewport by specifying an
   offset vector, since the origin of the HP-GL coordinate system and the
   size of the `hard-clip region' within which graphics can be drawn are
   not known.  (There are so many HP-GL and HP-GL/2 devices.)

   We also work out which pens are available, and whether the device, if an
   HP-GL/2 device, supports the Palette Extension so that new logical pens
   can be defined as RGB triples.  The HPGL_PENS and HPGL_ASSIGN_COLORS
   environment variables are used for this.  (The default is for a generic
   HP-GL device to have exactly 1 pen, #1, and for an HP7550A or HP-GL/2
   device to have 7 pens, #1 through #7, with colors equal to the seven
   non-white vertices of the RGB color cube.  We allow the user to specify
   up to 31 pens, #1 through #31, via HPGL_PENS. */

void
#ifdef _HAVE_PROTOS
_h_initialize (S___(Plotter *_plotter))
#else
_h_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i;
  double xoffset, yoffset;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize (S___(_plotter));
#endif

  /* override generic initializations (which are appropriate to the base
     Plotter class), as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->type = PL_HPGL;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 1;
  _plotter->have_dash_array = 1;
  _plotter->have_solid_fill = 1;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 0;
  _plotter->have_hershey_fonts = 1;
#ifdef USE_PS_FONTS_IN_PCL
  _plotter->have_ps_fonts = 1;
#else
  _plotter->have_ps_fonts = 0;
#endif
  _plotter->have_pcl_fonts = 1;
  _plotter->have_stick_fonts = 1;
  _plotter->have_extra_stick_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->default_font_type = F_HERSHEY;
  _plotter->pcl_before_ps = true;
  _plotter->have_horizontal_justification = false;
  _plotter->have_vertical_justification = false;
  _plotter->kern_stick_fonts = false; /* in HP-GL/2, anyway (see below) */
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal); note that we
     don't set max_unfilled_polyline_length, because it was set by the
     superclass initialization */
  _plotter->have_mixed_paths = true;
  _plotter->allowed_arc_scaling = AS_UNIFORM;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_model_type = (int)DISP_MODEL_PHYSICAL;
  _plotter->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->flipped_y = false;
  _plotter->imin = 0;
  _plotter->imax = 0;  
  _plotter->jmin = 0;
  _plotter->jmax = 0;  
  _plotter->xmin = 0.0;
  _plotter->xmax = 0.0;  
  _plotter->ymin = 0.0;
  _plotter->ymax = 0.0;  
  _plotter->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->hpgl_version = 2;
  _plotter->hpgl_rotation = 0;
  _plotter->hpgl_p1.x = 0.0;
  _plotter->hpgl_p1.y = 8128.0;  
  _plotter->hpgl_p2.x = 0.0;
  _plotter->hpgl_p2.y = 8128.0;  
  _plotter->hpgl_plot_length = 10668.0;  
  _plotter->hpgl_have_palette = false;
  _plotter->hpgl_use_opaque_mode = true;  
  /* dynamic variables */
  	/* pen_color[] and pen_defined[] arrays also used */
  _plotter->pen = 1;  
  _plotter->hpgl_free_pen = 2;  
  _plotter->hpgl_bad_pen = false;  
  _plotter->hpgl_pendown = false;  
  _plotter->hpgl_pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
  _plotter->hpgl_miter_limit = 5.0; /* default HP-GL/2 value */
  _plotter->hpgl_fill_type = HPGL_FILL_SOLID_BI;
  _plotter->hpgl_shading_level = 0.0;
  _plotter->pcl_symbol_set = PCL_ROMAN_8;  
  _plotter->pcl_spacing = 0;  
  _plotter->pcl_posture = 0;  
  _plotter->pcl_stroke_weight = 0;  
  _plotter->pcl_typeface = STICK_TYPEFACE;  
  _plotter->hpgl_charset_lower = HP_ASCII;
  _plotter->hpgl_charset_upper = HP_ASCII;
  _plotter->hpgl_rel_char_height = 0.0;
  _plotter->hpgl_rel_char_width = 0.0;  
  _plotter->hpgl_rel_label_rise = 0.0;    
  _plotter->hpgl_rel_label_run = 0.0;      
  _plotter->hpgl_tan_char_slant = 0.0;      
  _plotter->hpgl_position_is_unknown = true;
  _plotter->hpgl_pos.x = 0;
  _plotter->hpgl_pos.y = 0;

  /* note: this driver also uses pen_color[], pen_defined[] arrays;
     see initializations below */

  /* initialize certain data members from device driver parameters */
      
  /* determine HP-GL version */
  {
    const char *version_s;
    
    version_s = (const char *)_get_plot_param (R___(_plotter) "HPGL_VERSION");
    /* there are three subcases: "1", "1.5", and "2" (default, see above) */
    if (strcmp (version_s, "1") == 0) /* generic HP-GL, HP7220 or HP7475A */
      {
	_plotter->hpgl_version = 0;
	_plotter->have_wide_lines = 0;
	_plotter->have_dash_array = 0;
	_plotter->have_solid_fill = 0;
	_plotter->have_odd_winding_fill = 1;
	_plotter->have_nonzero_winding_fill = 0;
	_plotter->have_ps_fonts = 0;
	_plotter->have_pcl_fonts = 0;
	_plotter->have_stick_fonts = 0;
	_plotter->have_extra_stick_fonts = 0;
	_plotter->kern_stick_fonts = true; /* not relevant in generic HP-GL */
      }
    else if (strcmp (version_s, "1.5") == 0) /* HP7550A */
      {
	_plotter->hpgl_version = 1;
	_plotter->have_wide_lines = 0;
	_plotter->have_dash_array = 0;
	_plotter->have_solid_fill = 1;
	_plotter->have_odd_winding_fill = 1;
	_plotter->have_nonzero_winding_fill = 0;
	_plotter->have_ps_fonts = 0;
	_plotter->have_pcl_fonts = 0;
	_plotter->have_stick_fonts = 1;
	_plotter->have_extra_stick_fonts = 1;
	_plotter->kern_stick_fonts = true; /* definitely relevant */
      }
  }

  /* determine page type, and user-specified viewport offset if any */
  _set_page_type (R___(_plotter) &xoffset, &yoffset);
  
  /* Determine range of device coordinates over which the viewport will
     extend (and hence the transformation from user to device coordinates;
     see g_space.c). */

  /* We use the corners of the viewport, in device coordinates, as our
     `scaling points' P1 and P2 (see h_openpl.c).  The coordinates we use
     in our output file will be normalized device coordinates, not physical
     device coordinates (for the transformation between them, which is
     accomplished by the HP-GL `SC' instruction, see h_openpl.c). */
  {
    double x_llc, y_llc, x_urc, y_urc, viewport_size;
    
    viewport_size = _plotter->page_data->viewport_size;

    /* by default, viewport lower left corner is (0,0) in HP-GL coordinates */
    x_llc = 0.0 + xoffset;
    y_llc = 0.0 + yoffset;
    x_urc = x_llc + viewport_size;
    y_urc = y_llc + viewport_size;    
    
    _plotter->hpgl_p1.x = HPGL_UNITS_PER_INCH * x_llc;
    _plotter->hpgl_p2.x = HPGL_UNITS_PER_INCH * x_urc;
    _plotter->hpgl_p1.y = HPGL_UNITS_PER_INCH * y_llc;
    _plotter->hpgl_p2.y = HPGL_UNITS_PER_INCH * y_urc;

    _plotter->xmin = HPGL_SCALED_DEVICE_LEFT; 
    _plotter->xmax = HPGL_SCALED_DEVICE_RIGHT;
    _plotter->ymin = HPGL_SCALED_DEVICE_BOTTOM;
    _plotter->ymax = HPGL_SCALED_DEVICE_TOP;

  /* plot length (to be emitted in an HP-GL/2 `PS' instruction, important
     mostly for roll plotters; see h_openpl.c) */
  _plotter->hpgl_plot_length = 
    _plotter->page_data->hpgl2_plot_length * HPGL_UNITS_PER_INCH;
  }

  /* determine whether to rotate the figure (e.g. horizontal instead of
     vertical, see h_openpl.c) */
  {
    const char *rotate_s;

    rotate_s = (const char *)_get_plot_param (R___(_plotter) "HPGL_ROTATE");
    /* four subcases: 0 (default), 90, 180, 270 (latter two only if "2") */
    if (strcasecmp (rotate_s, "yes") == 0
	|| strcmp (rotate_s, "90") == 0)
      _plotter->hpgl_rotation = 90;
    else if (strcmp (rotate_s, "180") == 0 && _plotter->hpgl_version == 2)
      _plotter->hpgl_rotation = 180;
    else if (strcmp (rotate_s, "270") == 0 && _plotter->hpgl_version == 2)
      _plotter->hpgl_rotation = 270;
    else
      _plotter->hpgl_rotation = 0;
  }

  /* Should we avoid emitting the `white is opaque' HP-GL/2 instruction?
     (HP-GL/2 pen plotters may not like it) */
  {
    const char *transparent_s;

    transparent_s = (const char *)_get_plot_param (R___(_plotter) "HPGL_OPAQUE_MODE" );
    if (strcasecmp (transparent_s, "no") == 0)
      _plotter->hpgl_use_opaque_mode = false;
  }
  
  /* do we support the HP-GL/2 palette extension, i.e. can we define new
     logical pens as RGB triples? (user must request this with
     HPGL_ASSIGN_COLORS) */
  if (_plotter->hpgl_version == 2)
    {
      const char *palette_s;
	  
      palette_s = (const char *)_get_plot_param (R___(_plotter) "HPGL_ASSIGN_COLORS");
      if (strcasecmp (palette_s, "yes") == 0)
	_plotter->hpgl_have_palette = true;
    }
      
  /* initialize pen color array, typically 0..31 */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    _plotter->pen_defined[i] = 0; /* pen absent, or at least undefined */
      
  /* pen #0 (white pen, RGB=255,255,255) is always defined */
  _plotter->pen_color[0].red = 255;
  _plotter->pen_color[0].green = 255;
  _plotter->pen_color[0].blue = 255;
  _plotter->pen_defined[0] = 2; /* i.e. hard-defined */
      
  /* determine initial palette, i.e. available pens in 1..31 range */
  {
    const char *pen_s;

    pen_s = (const char *)_get_plot_param (R___(_plotter) "HPGL_PENS");
    
    if (pen_s == NULL 
	|| _parse_pen_string (R___(_plotter) pen_s) == false
	|| (_plotter->hpgl_have_palette == false 
	    && _plotter->pen_defined[1] == 0))
      /* Either user didn't assign a value, or it was bad; use default.
         Note that if no logical pens, we insist on pen #1 being present
         (for backward compatibility?). */
      {
	if (_plotter->hpgl_version == 0) /* i.e. generic HP-GL */
	  pen_s = DEFAULT_HPGL_PEN_STRING;
	else
	  pen_s = DEFAULT_HPGL2_PEN_STRING;
	_parse_pen_string (R___(_plotter) pen_s); /* default is guaranteed to parse */
      }
  }
  
  /* Examine presence or absence of hard-defined pens in 2..31 range.
     0 = undefined, 1 = soft-defined (not yet), 2 = hard-defined. */
  {
    bool undefined_pen_seen = false;
	
    for (i = 2; i < HPGL2_MAX_NUM_PENS; i++)
      {
	if (_plotter->pen_defined[i] == 0)
	  /* at least one pen with number > 1 is not yet defined */
	  {
	    /* record which such was encountered first */
	    _plotter->hpgl_free_pen = i;
	    undefined_pen_seen = true;
	    break;
	  }
      }
    if (!undefined_pen_seen)	
      /* too many pens specified, can't soft-define colors */
      _plotter->hpgl_have_palette = false;
  }
}

/* Initialization for the PCLPlotter class, which is subclassed from the
   HPGLPlotter class. */

void
#ifdef _HAVE_PROTOS
_q_initialize (S___(Plotter *_plotter))
#else
_q_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i;
  double xoffset, yoffset;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _h_initialize (S___(_plotter));
#endif

  /* Superclass initialization (i.e., of an HPGLPlotter) may well have
     screwed things up, since e.g. for a PCLPlotter, hpgl_version should
     always be equal to 2, irrespective of what HPGL_VERSION is; also the
     viewport positioning is different.  So we redo a large part of the
     initialization, most of which is redundant (FIXME). */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->type = PL_PCL;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 1;
  _plotter->have_dash_array = 1;
  _plotter->have_solid_fill = 1;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 0;
  _plotter->have_hershey_fonts = 1;
#ifdef USE_PS_FONTS_IN_PCL
  _plotter->have_ps_fonts = 1;
#else
  _plotter->have_ps_fonts = 0;
#endif
  _plotter->have_pcl_fonts = 1;
  _plotter->have_stick_fonts = 1;
  _plotter->have_extra_stick_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->default_font_type = F_PCL;
  _plotter->pcl_before_ps = true;
  _plotter->have_horizontal_justification = false;
  _plotter->have_vertical_justification = false;
  _plotter->kern_stick_fonts = false; /* in HP-GL/2 */
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal); note that we
     don't set max_unfilled_polyline_length, because it was set by the
     superclass initialization */
  _plotter->have_mixed_paths = true;
  _plotter->allowed_arc_scaling = AS_UNIFORM;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions, differ in derived classes */
  _plotter->display_model_type = (int)DISP_MODEL_PHYSICAL;
  _plotter->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->flipped_y = false;
  _plotter->imin = 0;
  _plotter->imax = 0;  
  _plotter->jmin = 0;
  _plotter->jmax = 0;  
  _plotter->xmin = 0.0;
  _plotter->xmax = 0.0;  
  _plotter->ymin = 0.0;
  _plotter->ymax = 0.0;  
  _plotter->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->hpgl_version = 2;
  _plotter->hpgl_rotation = 0;
  _plotter->hpgl_p1.x = 0.0;
  _plotter->hpgl_p1.y = 8128.0;  
  _plotter->hpgl_p2.x = 0.0;
  _plotter->hpgl_p2.y = 8128.0;  
  _plotter->hpgl_plot_length = 10668.0;  
  _plotter->hpgl_have_palette = false;
  _plotter->hpgl_use_opaque_mode = true;  
  /* dynamic variables */
  	/* pen_color[] and pen_defined[] arrays also used */
  _plotter->pen = 1;  
  _plotter->hpgl_free_pen = 2;  
  _plotter->hpgl_bad_pen = false;  
  _plotter->hpgl_pendown = false;  
  _plotter->hpgl_pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
/* Maximum value the cosecant of the half-angle between any two line
   segments can have, if the join is to be mitered rather than beveled.
   Default HP-GL/2 value is 5.0. */
  _plotter->hpgl_miter_limit = 5.0;
  _plotter->hpgl_fill_type = HPGL_FILL_SOLID_BI;
  _plotter->hpgl_shading_level = 0.0;
  _plotter->pcl_symbol_set = PCL_ROMAN_8;  
  _plotter->pcl_spacing = 0;  
  _plotter->pcl_posture = 0;  
  _plotter->pcl_stroke_weight = 0;  
  _plotter->pcl_typeface = STICK_TYPEFACE;  
  _plotter->hpgl_charset_lower = HP_ASCII;
  _plotter->hpgl_charset_upper = HP_ASCII;
  _plotter->hpgl_rel_char_height = 0.0;
  _plotter->hpgl_rel_char_width = 0.0;  
  _plotter->hpgl_rel_label_rise = 0.0;    
  _plotter->hpgl_rel_label_run = 0.0;      
  _plotter->hpgl_tan_char_slant = 0.0;      

  /* note: this driver also uses pen_color[], pen_defined[] arrays;
     see initializations below */

  /* initialize certain data members from device driver parameters */
      
  /* determine page type, and user-specified viewport offset if any */
  _set_page_type (R___(_plotter) &xoffset, &yoffset);
  
  /* Determine range of device coordinates over which the viewport will
     extend (and hence the transformation from user to device coordinates;
     see g_space.c). */

  /* We use the corners of the viewport, in device coordinates, as our
     `scaling points' P1 and P2 (see h_openpl.c).  The coordinates we use
     in our output file will be normalized device coordinates, not physical
     device coordinates (for the transformation between them, which is
     accomplished by the HP-GL `SC' instruction, see h_openpl.c). */
  {
    double xmid, ymid, viewport_size;
    
    viewport_size = _plotter->page_data->viewport_size;
    xmid = 0.5 * _plotter->page_data->xsize + xoffset;
    ymid = 0.5 * _plotter->page_data->ysize + yoffset;
    
    /* origin of HP-GL/2 coordinate system used by a PCL5 device is not at
       lower left corner of page; compensate */
    xmid -= _plotter->page_data->pcl_hpgl2_xorigin;
    ymid -= _plotter->page_data->pcl_hpgl2_yorigin;

    _plotter->hpgl_p1.x = HPGL_UNITS_PER_INCH * (xmid - 0.5 * viewport_size);
    _plotter->hpgl_p2.x = HPGL_UNITS_PER_INCH * (xmid + 0.5 * viewport_size);  
    _plotter->hpgl_p1.y = HPGL_UNITS_PER_INCH * (ymid - 0.5 * viewport_size);
    _plotter->hpgl_p2.y = HPGL_UNITS_PER_INCH * (ymid + 0.5 * viewport_size);  

    _plotter->xmin = HPGL_SCALED_DEVICE_LEFT; 
    _plotter->xmax = HPGL_SCALED_DEVICE_RIGHT;
    _plotter->ymin = HPGL_SCALED_DEVICE_BOTTOM;
    _plotter->ymax = HPGL_SCALED_DEVICE_TOP;

  /* plot length (to be emitted in an HP-GL/2 `PS' instruction, important
     mostly for roll plotters; see h_openpl.c) */
  _plotter->hpgl_plot_length = 
    _plotter->page_data->hpgl2_plot_length * HPGL_UNITS_PER_INCH;
  }

  /* don't make use of HP-GL/2's plotting-area rotation facility; if we
     wish to switch between portrait and landscape modes we'll do so from
     within PCL5 */
  _plotter->hpgl_rotation = 0;

  /* do we support the HP-GL/2 palette extension, i.e. can we define new
     logical pens as RGB triples? (user must request this with
     PCL_ASSIGN_COLORS) */
  _plotter->hpgl_have_palette = false;
  {
    const char *palette_s;
    
    palette_s = (const char *)_get_plot_param (R___(_plotter) "PCL_ASSIGN_COLORS");
    if (strcasecmp (palette_s, "yes") == 0)
      _plotter->hpgl_have_palette = true;
  }
      
  /* do we use the HP-GL/2 `BZ' instruction for drawing Beziers?  (the
     LaserJet III did not support it) */
  {
    const char *bezier_s;
    
    bezier_s = (const char *)_get_plot_param (R___(_plotter) "PCL_BEZIERS");

    if (_plotter->hpgl_version == 2 && strcasecmp (bezier_s, "yes") == 0)
      _plotter->allowed_cubic_scaling = AS_ANY;
  }

  /* initialize pen color array, typically 0..31 */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    _plotter->pen_defined[i] = 0; /* pen absent, or at least undefined */
      
  /* pen #0 (white pen, RGB=255,255,255) is always defined */
  _plotter->pen_color[0].red = 255;
  _plotter->pen_color[0].green = 255;
  _plotter->pen_color[0].blue = 255;
  _plotter->pen_defined[0] = 2; /* i.e. hard-defined */
      
  /* determine initial palette, i.e. available pens in 1..31 range; for a
     PCLPlotter we use the default HP-GL/2 pen string */
  {
    const char *pen_s;

    pen_s = DEFAULT_HPGL2_PEN_STRING;
    _parse_pen_string (R___(_plotter) pen_s); /* default is guaranteed to parse */
  }
  
  /* Examine presence or absence of hard-defined pens in 2..31 range.
     0 = undefined, 1 = soft-defined (not yet), 2 = hard-defined. */
  {
    bool undefined_pen_seen = false;
	
    for (i = 2; i < HPGL2_MAX_NUM_PENS; i++)
      {
	if (_plotter->pen_defined[i] == 0)
	  /* at least one pen with number > 1 is not yet defined */
	  {
	    /* record which such was encountered first */
	    _plotter->hpgl_free_pen = i;
	    undefined_pen_seen = true;
	    break;
	  }
      }
    if (!undefined_pen_seen)	
      /* too many pens specified, can't soft-define colors */
      _plotter->hpgl_have_palette = false;
  }
}

/* Parse a pen string, e.g. a user-specified HPGL_PENS environment
   variable, specifying which pens are available.  Result is stored in the
   Plotter.  More pens (logical pens) may be added later to the array of
   available pens, if the plotter is an HP-GL/2 device and supports the
   palette extension.  User specifies this by setting the
   HPGL_ASSIGN_COLORS environment variable to "yes"; see above. */
bool
#ifdef _HAVE_PROTOS
_parse_pen_string (R___(Plotter *_plotter) const char *pen_s)
#else
_parse_pen_string (R___(_plotter) pen_s)
     S___(Plotter *_plotter;)
     const char *pen_s;
#endif
{
  const char *charp;
  char name[MAX_COLOR_NAME_LEN];
  int i;

  charp = pen_s;
  while (*charp)
    {
      int pen_num;
      bool got_digit;
      const char *tmp;
      const plColorNameInfo *info;

      if (*charp == ':')	/* skip any ':' */
	{
	  charp++;
	  continue;		/* back to top of while loop */
	}
      pen_num = 0;
      got_digit = false;
      while (*charp >= '0' && *charp <= '9')
	{
	  pen_num = 10 * pen_num + (int)*charp - (int)'0';
	  got_digit = true;
	  charp++;
	}
      if (!got_digit || pen_num < 1 || pen_num >= HPGL2_MAX_NUM_PENS)
	return false;
      if (*charp != '=')
	return false;
      charp++;
      for (tmp = charp, i = 0; i < MAX_COLOR_NAME_LEN; tmp++, i++)
	{
	  if (*tmp == ':') /* end of color name string */
	    {
	      name[i] = '\0';
	      charp = tmp + 1;
	      break;
	    }
	  else if (*tmp == '\0') /* end of name string and env var also */
	    {
	      name[i] = '\0';
	      charp = tmp;
	      break;
	    }
	  else
	    name[i] = *tmp;
	}

      /* got color name string, parse it */
      if (_string_to_color (R___(_plotter) name, &info))
	{
	  _plotter->pen_color[pen_num].red = info->red;
	  _plotter->pen_color[pen_num].green = info->green;
	  _plotter->pen_color[pen_num].blue = info->blue;
	  _plotter->pen_defined[pen_num] = 2; /* hard-defined */
	}
      else			/* couldn't match color name string */
	return false;
    }

  return true;
}  

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
#ifdef _HAVE_PROTOS
_h_terminate (S___(Plotter *_plotter))
#else
_h_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate (S___(_plotter));
#endif
}

void
#ifdef _HAVE_PROTOS
_q_terminate (S___(Plotter *_plotter))
#else
_q_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _h_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
HPGLPlotter::HPGLPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (ostream& out)
	: Plotter (out)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter ()
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _h_initialize ();
}

HPGLPlotter::HPGLPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _h_initialize ();
}

HPGLPlotter::~HPGLPlotter ()
{
  _h_terminate ();
}
#endif

#ifdef LIBPLOTTER
PCLPlotter::PCLPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:HPGLPlotter (infile, outfile, errfile)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (FILE *outfile)
	:HPGLPlotter (outfile)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (istream& in, ostream& out, ostream& err)
	: HPGLPlotter (in, out, err)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (ostream& out)
	: HPGLPlotter (out)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter ()
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:HPGLPlotter (infile, outfile, errfile, parameters)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (FILE *outfile, PlotterParams &parameters)
	:HPGLPlotter (outfile, parameters)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: HPGLPlotter (in, out, err, parameters)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (ostream& out, PlotterParams &parameters)
	: HPGLPlotter (out, parameters)
{
  _q_initialize ();
}

PCLPlotter::PCLPlotter (PlotterParams &parameters)
	: HPGLPlotter (parameters)
{
  _q_initialize ();
}

PCLPlotter::~PCLPlotter ()
{
  _q_terminate ();
}
#endif

#ifndef LIBPLOTTER
/* The following forwarding functions provide special support in libplot
   for deriving the PCLPlotter class from the HPGLPlotter class.  In
   libplotter, forwarding is implemented by a virtual function; see
   plotter.h. */

/* Two forwarding functions called by any HPGLPlotter/PCLPlotter in
   h_openpl.c and h_closepl.c, respectively.  See h_openpl.c and
   h_closepl.c for the forwarded-to functions _h_maybe_switch_to_hpgl(),
   _q_maybe_switch_to_hpgl(), _h_maybe_switch_from_hpgl(),
   _q_maybe_switch_from_hpgl().  The HPGLPlotter versions are no-ops, but
   the PCLPlotter versions switch the printer to HP-GL/2 mode from PCL 5
   mode, and back to PCL 5 mode from HP-GL/2 mode. */
   
/* Eject page (if page number > 1) and switch from PCL 5 mode to HP-GL/2
   mode, if a PCL 5 printer (otherwise it's a no-op) */
void
#ifdef _HAVE_PROTOS
_maybe_switch_to_hpgl (Plotter *_plotter)
#else
_maybe_switch_to_hpgl (_plotter)
     Plotter *_plotter;
#endif
{
  if (_plotter->type == PL_HPGL)
    _h_maybe_switch_to_hpgl (_plotter);
  else if (_plotter->type == PL_PCL)
    _q_maybe_switch_to_hpgl (_plotter);
}

/* Switch back to PCL 5 mode from HP-GL/2 mode, if a PCL 5 printer 
   (otherwise it's a no-op) */
void
#ifdef _HAVE_PROTOS
_maybe_switch_from_hpgl (Plotter *_plotter)
#else
_maybe_switch_from_hpgl (_plotter)
     Plotter *_plotter;
#endif
{
  if (_plotter->type == PL_HPGL)
    _h_maybe_switch_from_hpgl (_plotter);
  else if (_plotter->type == PL_PCL)
    _q_maybe_switch_from_hpgl (_plotter);
}
#endif /* not LIBPLOTTER */
