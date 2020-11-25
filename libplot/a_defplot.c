/* This file defines the initialization for any AIPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   the AIPlotter struct. */
const Plotter _a_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _a_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _a_endpath, _g_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _g_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _a_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _a_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _a_initialize, _a_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_falabel_hershey, _a_falabel_ps, _a_falabel_pcl, _g_falabel_stick, _g_falabel_other,
  _g_flabelwidth_hershey, _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, _g_flabelwidth_other,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _g_set_font,
  /* private low-level `sync line attributes' method */
  _a_set_attributes,
  /* private low-level `sync color' methods */
  _a_set_pen_color,
  _a_set_fill_color,
  _g_set_bg_color,
  /* private low-level `sync position' method */
  _g_set_position,
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

/* For AI Plotter objects, we determine the page size and the location on
   the page of the graphics display, so that we'll be able to work out the
   map from user coordinates to device coordinates in space.c.  Also
   we determine which version of Illustrator file format we'll emit. */

void
#ifdef _HAVE_PROTOS
_a_initialize (void)
#else
_a_initialize ()
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize ();
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->type = PL_AI;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 1;
  _plotter->have_dash_array = 1;
  _plotter->have_solid_fill = 1;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 0;
  _plotter->have_hershey_fonts = 1;
  _plotter->have_ps_fonts = 1;
  _plotter->have_pcl_fonts = 1;
  _plotter->have_stick_fonts = 0;
  _plotter->have_extra_stick_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->default_font_type = F_POSTSCRIPT;
  _plotter->pcl_before_ps = false;
  _plotter->issue_font_warning = true;
  _plotter->kern_stick_fonts = false;
  _plotter->have_justification = true;

  /* path and polyline-related parameters (also internal) */
  _plotter->max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;
  _plotter->have_mixed_paths = true;
  _plotter->allowed_arc_scaling = AS_ANY;
  _plotter->allowed_ellarc_scaling = AS_ANY;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_ANY;  
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_type = DISP_PHYSICAL;
  _plotter->integer_device_coors = false;
  _plotter->imin = 0;
  _plotter->imax = 0;  
  _plotter->jmin = 0;
  _plotter->jmax = 0;  
  _plotter->display_coors.left = 0.25;
  _plotter->display_coors.right = 8.25;
  _plotter->display_coors.bottom = 1.5;
  _plotter->display_coors.top = 9.5;
  _plotter->display_coors.extra = 0.0;  
  _plotter->page_type = NULL;
  _plotter->device_units_per_inch = 72.0;
  _plotter->use_metric = false;
  _plotter->flipped_y = false;

  /* initialize data members specific to this derived class */
  _plotter->ai_version = AI_VERSION_5;
  _plotter->ai_pen_cyan = 0.0;
  _plotter->ai_pen_magenta = 0.0;
  _plotter->ai_pen_yellow = 0.0;
  _plotter->ai_pen_black = 1.0;  
  _plotter->ai_fill_cyan = 0.0;
  _plotter->ai_fill_magenta = 0.0;
  _plotter->ai_fill_yellow = 0.0;
  _plotter->ai_fill_black = 1.0;  
  _plotter->ai_cyan_used = false;
  _plotter->ai_magenta_used = false;
  _plotter->ai_yellow_used = false;
  _plotter->ai_black_used = false;
  _plotter->ai_cap_style = PS_CAP_BUTT;
  _plotter->ai_join_style = PS_JOIN_MITER;  
/* Maximum value the cosecant of the half-angle between any two line
   segments can have, if the join is to be mitered rather than beveled.
   Default value for AI is 4.0. */
  _plotter->ai_miter_limit = 4.0;
  _plotter->ai_line_type = L_SOLID;  
  _plotter->ai_line_width = 1.0;    
  _plotter->ai_fill_rule_type = 0; /* i.e. nonzero winding number rule */

  /* initialize certain data members from device driver parameters */

  /* determine which version of AI format we'll emit (obsolescent) */
  {
    const char *version_s;
    
    version_s = (const char *)_get_plot_param ("AI_VERSION");
    if (strcmp (version_s, "3") == 0)
      _plotter->ai_version = AI_VERSION_3;
    else if (strcmp (version_s, "5") == 0)
      _plotter->ai_version = AI_VERSION_5;
    else      
      {
	version_s = (const char *)_get_default_plot_param ("AI_VERSION");
	if (strcmp (version_s, "3") == 0)
	  _plotter->ai_version = AI_VERSION_3;
	else if (strcmp (version_s, "5") == 0)
	  _plotter->ai_version = AI_VERSION_5;
      }
  }
  /* AI didn't support even-odd fill until version 5 */
  if (_plotter->ai_version == AI_VERSION_3)
    _plotter->have_odd_winding_fill = 0;

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
    _plotter->display_coors = pagedata->ps;
    _plotter->use_metric = pagedata->metric;
    _plotter->page_type = pagedata->name;
  }
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_a_terminate (void)
#else
_a_terminate ()
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate ();
#endif
}

#ifdef LIBPLOTTER
AIPlotter::AIPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _a_initialize ();
}

AIPlotter::AIPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _a_initialize ();
}

AIPlotter::AIPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _a_initialize ();
}

AIPlotter::AIPlotter (ostream& out)
	: Plotter (out)
{
  _a_initialize ();
}

AIPlotter::AIPlotter ()
{
  _a_initialize ();
}

AIPlotter::~AIPlotter ()
{
  _a_terminate ();
}
#endif
