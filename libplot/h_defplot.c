/* This file defines the initializations for HPGLPlotter and PCLPlotter
   objects including both private data and public methods.  There is a
   one-to-one correspondence between public methods and user-callable
   functions in the C API. */

/* Currently, only differences between the two types of Plotter are (1) the
   different `type' field in corresponding _*_default_plotter structures,
   and (2) some tests for same, in h_openpl.c (in the openpl and closepl
   methods).  The differences are due to the PCL 5 control codes that must
   be emitted to switch the printer into HP-GL/2 mode, etc. */

#include "sys-defines.h"
#include "extern.h"

/* forward references */
static bool _string_to_inches ____P ((const char *offset_s, double *offset));

#define MAX_COLOR_NAME_LEN 32	/* long enough for all known colors */

/* The size of the graphics display is determined by the PAGESIZE
   environment variable ("usletter", "a4", etc.)  The table of known
   pagetypes is in g_pagetype.h.  The default is "usletter", for which the
   graphics display is a square with side length equal to 80% of 8.5
   inches, so it should fit on an 8.5" by 11" page.

   The origin of the HP-GL[/2] coordinate system is the lower left corner
   of the `hard-clip region', a rectangle that is not the same as the page.
   For this reason we allow the user to shift the position of the graphics
   display via the environment variables HPGL_XOFFSET and HPGL_YOFFSET (see
   below). */

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   the HPGLPlotter struct. */
const Plotter _h_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _h_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _h_endpath, _g_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _h_fbox, _g_fboxrel, _h_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _h_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _h_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _h_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
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
   the PCLPlotter struct.  It is the same as the above except for the
   different initialization and termination routines. */
const Plotter _q_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _h_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _h_endpath, _g_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _h_fbox, _g_fboxrel, _h_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _h_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _h_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _h_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
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
   is invoked, _plotter points (temporarily) to the Plotter that has just
   been created. */

/* The initializations for both HPGL and PCL Plotters are almost identical.

   For HPGL Plotters, we determine the HP-GL version from the environment
   variable HPGL_VERSION ("1", "1.5", or "2", meaning generic HP-GL,
   HP7550A, and modern HP-GL/2 respectively), and determine the page size
   and the location on the page of the graphics display, so that we'll be
   able to work out the map from user coordinates to device coordinates in
   space.c.

   We allow the user to shift the location of the graphics display by
   specifying an offset vector, since the origin of the HP-GL coordinate
   system and the size of the `hard-clip region' within which graphics can
   be drawn are not known.  (There are so many HP-GL and HP-GL/2
   devices.)

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
_h_initialize (void)
#else
_h_initialize ()
#endif
{
  int i;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize ();
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
  _plotter->have_justification = false;
  _plotter->kern_stick_fonts = false;
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal) */
  _plotter->max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;
  _plotter->have_mixed_paths = true;
  _plotter->allowed_arc_scaling = AS_UNIFORM;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_type = DISP_PHYSICAL;
  _plotter->integer_device_coors = false;
  _plotter->imin = 0;
  _plotter->imax = 0;  
  _plotter->jmin = 0;
  _plotter->jmax = 0;  
  _plotter->display_coors.left = 0.0;
  _plotter->display_coors.right = 8.0;
  _plotter->display_coors.bottom = 0.0;
  _plotter->display_coors.top = 8.0;
  _plotter->display_coors.extra = 10.5;  
  _plotter->page_type = NULL;
  _plotter->device_units_per_inch = (double)HPGL_UNITS_PER_INCH;
  _plotter->use_metric = false;
  _plotter->flipped_y = false;

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->hpgl_version = 2;
  _plotter->rotation = 0;
  _plotter->p1.x = 0.0;
  _plotter->p1.y = 8128.0;  
  _plotter->p2.x = 0.0;
  _plotter->p2.y = 8128.0;  
  _plotter->plot_length = 10668.0;  
  _plotter->palette = false;
  _plotter->opaque_mode = true;  
  /* dynamic variables */
  	/* pen_color[] and pen_defined[] arrays also used */
  _plotter->pen = 1;  
  _plotter->free_pen = 2;  
  _plotter->bad_pen = false;  
  _plotter->pendown = false;  
  _plotter->pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
  _plotter->hpgl_miter_limit = 5.0; /* default HP-GL/2 value */
  _plotter->fill_type = HPGL_FILL_SOLID_BI;
  _plotter->shading_level = 0.0;
  _plotter->pcl_symbol_set = PCL_ROMAN_8;  
  _plotter->pcl_spacing = 0;  
  _plotter->pcl_posture = 0;  
  _plotter->pcl_stroke_weight = 0;  
  _plotter->pcl_typeface = STICK_TYPEFACE;  
  _plotter->hpgl_charset_lower = HP_ASCII;
  _plotter->hpgl_charset_upper = HP_ASCII;
  _plotter->relative_char_height = 0.0;
  _plotter->relative_char_width = 0.0;  
  _plotter->relative_label_rise = 0.0;    
  _plotter->relative_label_run = 0.0;      
  _plotter->char_slant_tangent = 0.0;      
  _plotter->hpgl_position_is_unknown = true;
  _plotter->hpgl_pos.x = 0;
  _plotter->hpgl_pos.y = 0;

  /* note: this driver also uses pen_color[], pen_defined[] arrays;
     see initializations below */

  /* initialize certain data members from device driver parameters */
      
  /* determine HP-GL version */
  {
    const char *version_s;
    
    version_s = (const char *)_get_plot_param ("HPGL_VERSION");
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
	_plotter->kern_stick_fonts = true; /* shouldn't be relevant */
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
	_plotter->kern_stick_fonts = true;
      }
  }

  /* determine page type i.e. determine the range of device coordinates
     over which the graphics display will extend (and hence the
     transformation from user to device coordinates). */
  {
    const char *pagesize;
    const Pagedata *pagedata;

    pagesize = (const char *)_get_plot_param ("PAGESIZE");
    pagedata = _pagetype(pagesize);
    if (pagedata == NULL)
      {
	pagesize = (const char *)_get_default_plot_param ("PAGESIZE");
	pagedata = _pagetype(pagesize);
      }
    _plotter->display_coors = pagedata->hpgl;
    _plotter->use_metric = pagedata->metric;
    _plotter->page_type = pagedata->name;
  }
      
  /* Origin of HP-GL[/2] coordinates is the lower left corner of the
     hard-clip region, which is not the same as the lower left corner
     of the page.  So we allow the user to specify an offset for the
     location of the graphics display, using HPGL_?OFFSET. */
  {
    const char *xoffset_s, *yoffset_s;
    double xoffset, yoffset;
	
    xoffset_s = (const char *)_get_plot_param ("HPGL_XOFFSET");
    if (_string_to_inches (xoffset_s, &xoffset))
      {
	_plotter->display_coors.left += xoffset;
	_plotter->display_coors.right += xoffset;
      }
	
    yoffset_s = (const char *)_get_plot_param ("HPGL_YOFFSET");  
    if (_string_to_inches (yoffset_s, &yoffset))
      {
	_plotter->display_coors.bottom += yoffset;
	_plotter->display_coors.top += yoffset;
      }
  }
      
  /* At this point we stash the coordinates for later use.  They'll become
     the coordinates of our `scaling points' P1 and P2 (see h_openpl.c).
     The numbers in our output file will be normalized device coordinates,
     not physical device coordinates (for the transformation between them,
     which is accomplished by the HP-GL `SC' instruction, see h_openpl.c) */
  _plotter->p1.x = _plotter->display_coors.left * HPGL_UNITS_PER_INCH;
  _plotter->p2.x = _plotter->display_coors.right * HPGL_UNITS_PER_INCH;
  _plotter->p1.y = _plotter->display_coors.bottom * HPGL_UNITS_PER_INCH;
  _plotter->p2.y = _plotter->display_coors.top * HPGL_UNITS_PER_INCH;
  _plotter->display_coors.left = HPGL_SCALED_DEVICE_LEFT; 
  _plotter->display_coors.right = HPGL_SCALED_DEVICE_RIGHT;
  _plotter->display_coors.bottom = HPGL_SCALED_DEVICE_BOTTOM;
  _plotter->display_coors.top = HPGL_SCALED_DEVICE_TOP;
  _plotter->device_units_per_inch = 1.0;
      
  /* plot length (to be emitted in an HP-GL/2 `PS' instruction,
     important mostly for roll plotters) */
  _plotter->plot_length = 
    _plotter->display_coors.extra * HPGL_UNITS_PER_INCH;

  /* determine whether to rotate the figure (e.g. horizontal instead of
     vertical, see h_openpl.c) */
  {
    const char *rotate_s;

    rotate_s = (const char *)_get_plot_param ("HPGL_ROTATE");
    /* four subcases: 0 (default), 90, 180, 270 (latter two only if "2") */
    if (strcasecmp (rotate_s, "yes") == 0
	|| strcmp (rotate_s, "90") == 0)
      _plotter->rotation = 90;
    else if (strcmp (rotate_s, "180") == 0 && _plotter->hpgl_version == 2)
      _plotter->rotation = 180;
    else if (strcmp (rotate_s, "270") == 0 && _plotter->hpgl_version == 2)
      _plotter->rotation = 270;
    else
      _plotter->rotation = 0;
  }

  /* Should we avoid emitting the `white is opaque' HP-GL/2 instruction?
     (HP-GL/2 pen plotters may not like it) */
  {
    const char *transparent_s;

    transparent_s = (const char *)_get_plot_param ( "HPGL_OPAQUE_MODE" );
    if (strcasecmp (transparent_s, "no") == 0)
      _plotter->opaque_mode = false;
  }
  
  /* do we support the HP-GL/2 palette extension, i.e. can we define new
     logical pens as RGB triples? (user must request this with
     HPGL_ASSIGN_COLORS) */
  if (_plotter->hpgl_version == 2)
    {
      const char *palette_s;
	  
      palette_s = (const char *)_get_plot_param ("HPGL_ASSIGN_COLORS");
      if (strcasecmp (palette_s, "yes") == 0)
	_plotter->palette = true;
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

    pen_s = (const char *)_get_plot_param ("HPGL_PENS");
    
    if (pen_s == NULL || _parse_pen_string (pen_s) == false
	|| (_plotter->palette == false && _plotter->pen_defined[1] == 0))
      /* Either user didn't assign a value, or it was bad; use default.
         Note that if no logical pens, we insist on pen #1 being present
         (for backward compatibility?). */
      {
	if (_plotter->hpgl_version == 0) /* i.e. generic HP-GL */
	  pen_s = DEFAULT_HPGL_PEN_STRING;
	else
	  pen_s = DEFAULT_HPGL2_PEN_STRING;
	_parse_pen_string (pen_s); /* default is guaranteed to parse */
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
	    _plotter->free_pen = i;
	    undefined_pen_seen = true;
	    break;
	  }
      }
    if (!undefined_pen_seen)	
      /* too many pens specified, can't soft-define colors */
      _plotter->palette = false;
  }
}

void
#ifdef _HAVE_PROTOS
_q_initialize (void)
#else
_q_initialize ()
#endif
{
  int i;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _h_initialize ();
#endif

  /* Superclass initialization (i.e., of an HPGLPlotter) may well have
     screwed things up, since e.g. for a PCLPlotter, hpgl_version should
     always be equal to 2, irrespective of what HPGL_VERSION is.  So we
     redo a large part of the initialization, most of which is redundant
     (FIXME). */

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
  _plotter->default_font_type = F_HERSHEY;
  _plotter->pcl_before_ps = true;
  _plotter->have_justification = false;
  _plotter->kern_stick_fonts = false;
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal) */
  _plotter->max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;
  _plotter->have_mixed_paths = true;
  _plotter->allowed_arc_scaling = AS_UNIFORM;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions, differ in derived classes */
  _plotter->integer_device_coors = false;
  _plotter->imin = 0;
  _plotter->imax = 0;  
  _plotter->jmin = 0;
  _plotter->jmax = 0;  
  _plotter->display_coors.left = 0.0;
  _plotter->display_coors.right = 8.0;
  _plotter->display_coors.bottom = 0.0;
  _plotter->display_coors.top = 8.0;
  _plotter->display_coors.extra = 10.5;  
  _plotter->page_type = NULL;
  _plotter->device_units_per_inch = (double)HPGL_UNITS_PER_INCH;
  _plotter->use_metric = false;
  _plotter->flipped_y = false;

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->hpgl_version = 2;
  _plotter->rotation = 0;
  _plotter->p1.x = 0.0;
  _plotter->p1.y = 8128.0;  
  _plotter->p2.x = 0.0;
  _plotter->p2.y = 8128.0;  
  _plotter->plot_length = 10668.0;  
  _plotter->palette = false;
  _plotter->opaque_mode = true;  
  /* dynamic variables */
  	/* pen_color[] and pen_defined[] arrays also used */
  _plotter->pen = 1;  
  _plotter->free_pen = 2;  
  _plotter->bad_pen = false;  
  _plotter->pendown = false;  
  _plotter->pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
/* Maximum value the cosecant of the half-angle between any two line
   segments can have, if the join is to be mitered rather than beveled.
   Default HP-GL/2 value is 5.0. */
  _plotter->hpgl_miter_limit = 5.0;
  _plotter->fill_type = HPGL_FILL_SOLID_BI;
  _plotter->shading_level = 0.0;
  _plotter->pcl_symbol_set = PCL_ROMAN_8;  
  _plotter->pcl_spacing = 0;  
  _plotter->pcl_posture = 0;  
  _plotter->pcl_stroke_weight = 0;  
  _plotter->pcl_typeface = STICK_TYPEFACE;  
  _plotter->hpgl_charset_lower = HP_ASCII;
  _plotter->hpgl_charset_upper = HP_ASCII;
  _plotter->relative_char_height = 0.0;
  _plotter->relative_char_width = 0.0;  
  _plotter->relative_label_rise = 0.0;    
  _plotter->relative_label_run = 0.0;      
  _plotter->char_slant_tangent = 0.0;      
  /* note: this driver also uses pen_color[], pen_defined[] arrays;
     see initialization below */

  /* superclass initialization may have looked at HPGL_?OFFSET instead of
     PCL_?OFFSET, so we now redo the offset computations */

  /* determine page type i.e. determine the range of device coordinates
     over which the graphics display will extend (and hence the
     transformation from user to device coordinates). */
  {
    const char *pagesize;
    const Pagedata *pagedata;

    pagesize = (const char *)_get_plot_param ("PAGESIZE");
    pagedata = _pagetype(pagesize);
    if (pagedata == NULL)
      {
	pagesize = (const char *)_get_default_plot_param ("PAGESIZE");
	pagedata = _pagetype(pagesize);
      }
    _plotter->display_coors = pagedata->hpgl;
    _plotter->use_metric = pagedata->metric;
    _plotter->page_type = pagedata->name;
  }
      
  /* Origin of HP-GL[/2] coordinates is the lower left corner of the
     hard-clip region, which is not the same as the lower left corner
     of the page.  So we allow the user to specify an offset for the
     location of the graphics display, using PCL_?OFFSET. */
  {
    const char *xoffset_s, *yoffset_s;
    double xoffset, yoffset;
	
    xoffset_s = (const char *)_get_plot_param ("PCL_XOFFSET");
    if (_string_to_inches (xoffset_s, &xoffset))
      {
	_plotter->display_coors.left += xoffset;
	_plotter->display_coors.right += xoffset;
      }
	
    yoffset_s = (const char *)_get_plot_param ("PCL_YOFFSET");  
    if (_string_to_inches (yoffset_s, &yoffset))
      {
	_plotter->display_coors.bottom += yoffset;
	_plotter->display_coors.top += yoffset;
      }
  }
      
  /* At this point we stash the coordinates for later use.  They'll become
     the coordinates of our `scaling points' P1 and P2 (see h_openpl.c).
     The numbers in our output file will be normalized device coordinates,
     not physical device coordinates (for the transformation between them,
     which is accomplished by the HP-GL `SC' instruction, see h_openpl.c) */
  _plotter->p1.x = _plotter->display_coors.left * HPGL_UNITS_PER_INCH;
  _plotter->p2.x = _plotter->display_coors.right * HPGL_UNITS_PER_INCH;
  _plotter->p1.y = _plotter->display_coors.bottom * HPGL_UNITS_PER_INCH;
  _plotter->p2.y = _plotter->display_coors.top * HPGL_UNITS_PER_INCH;
  _plotter->display_coors.left = HPGL_SCALED_DEVICE_LEFT; 
  _plotter->display_coors.right = HPGL_SCALED_DEVICE_RIGHT;
  _plotter->display_coors.bottom = HPGL_SCALED_DEVICE_BOTTOM;
  _plotter->display_coors.top = HPGL_SCALED_DEVICE_TOP;
  _plotter->device_units_per_inch = 1.0;
      
  /* plot length (to be emitted in an HP-GL/2 `PS' instruction,
     important mostly for roll plotters) */
  _plotter->plot_length = 
    _plotter->display_coors.extra * HPGL_UNITS_PER_INCH;

  /* determine whether to rotate the figure (e.g. horizontal instead of
     vertical, see h_openpl.c), using PCL_ROTATE rather than HPGL_ROTATE */
  {
    const char *rotate_s;

    rotate_s = (const char *)_get_plot_param ("PCL_ROTATE");
    /* four subcases: 0 (default), 90, 180, 270 (latter two only if "2") */
    if (strcasecmp (rotate_s, "yes") == 0
	|| strcmp (rotate_s, "90") == 0)
      _plotter->rotation = 90;
    else if (strcmp (rotate_s, "180") == 0 && _plotter->hpgl_version == 2)
      _plotter->rotation = 180;
    else if (strcmp (rotate_s, "270") == 0 && _plotter->hpgl_version == 2)
      _plotter->rotation = 270;
    else
      _plotter->rotation = 0;
  }

  /* do we support the HP-GL/2 palette extension, i.e. can we define new
     logical pens as RGB triples? (user must request this with
     PCL_ASSIGN_COLORS) */
  _plotter->palette = false;
  {
    const char *palette_s;
    
    palette_s = (const char *)_get_plot_param ("PCL_ASSIGN_COLORS");
    if (strcasecmp (palette_s, "yes") == 0)
      _plotter->palette = true;
  }
      
  /* do we support the HP-GL/2 `BZ' instruction for drawing Beziers? */
  {
    const char *bezier_s;
    
    bezier_s = (const char *)_get_plot_param ("PCL_BEZIERS");

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
    _parse_pen_string (pen_s); /* default is guaranteed to parse */
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
	    _plotter->free_pen = i;
	    undefined_pen_seen = true;
	    break;
	  }
      }
    if (!undefined_pen_seen)	
      /* too many pens specified, can't soft-define colors */
      _plotter->palette = false;
  }
}

static bool 
#ifdef _HAVE_PROTOS
_string_to_inches(const char *string, double *inches)
#else
_string_to_inches(string, inches)
     const char *string; 
     double *inches;
#endif
{
  double val;
  char s[4];
  
  if (sscanf (string, "%lf %3s" , &val, s) == 2)
    {
      if (strlen (s) > 2)
	return false;
      if (strcmp (s, "in") == 0)
	{
	  *inches = val;
	  return true;
	}
      else if (strcmp (s, "cm") == 0)
	{
	  *inches = val / 2.54;
	  return true;
	}
      else if (strcmp (s, "mm") == 0)      
	{
	  *inches = val / 25.4;
	  return true;
	}
    }
    return false;
}

/* Parse a pen string, e.g. a user-specified HPGL_PENS environment
   variable, specifying which pens are available.  Result is stored in the
   Plotter.  More pens (logical pens) may be added later to the array of
   available pens, if the plotter is an HP-GL/2 device and supports the
   palette extension.  User specifies this by setting the
   HPGL_ASSIGN_COLORS environment variable to "yes"; see above. */
bool
#ifdef _HAVE_PROTOS
_parse_pen_string (const char *pen_s)
#else
_parse_pen_string (pen_s)
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
      const Colornameinfo *info;

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
      if (_string_to_color (name, &info))
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
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_h_terminate (void)
#else
_h_terminate ()
#endif
{
#ifndef LIBPLOTTER
  /* invoke generic method, e.g. to deallocate instance-specific copies
     of class variables */
  _g_terminate ();
#endif
}

void
#ifdef _HAVE_PROTOS
_q_terminate (void)
#else
_q_terminate ()
#endif
{
#ifndef LIBPLOTTER
  /* invoke generic method, e.g. to deallocate instance-specific copies
     of class variables */
  _g_terminate ();
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

PCLPlotter::~PCLPlotter ()
{
  _q_terminate ();
}
#endif
