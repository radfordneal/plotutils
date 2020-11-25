/* This file defines the initialization for any TekPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

/* Note that Tektronix displays are rectangular, and wider than they are
   high: the aspect ratio is approximately 4:3.  In terms of integer
   Tektronix coordinates the Tektronix display is a [0..4095]x[0..3119]
   rectangle, and we choose our viewport to be the square
   [488..3607]x[0..3119].  I.e. we define it to be a square, occupying the
   entire height of the display, and centered on the display. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a TekPlotter struct. */
const Plotter _t_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _t_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _g_endpath, _g_endsubpath, _t_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _g_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _t_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _t_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _t_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _t_initialize, _t_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_falabel_hershey, _g_falabel_ps, _g_falabel_pcl, _g_falabel_stick, _g_falabel_other,
  _g_flabelwidth_hershey, _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, _g_flabelwidth_other,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _g_set_font,
  /* private low-level `sync line attributes' method */
  _t_set_attributes,
  /* private low-level `sync color' methods */
  _t_set_pen_color,
  _g_set_fill_color,
  _t_set_bg_color,
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
   is invoked, _plotter points to the Plotter that has just been
   created. */

void
#ifdef _HAVE_PROTOS
_t_initialize (S___(Plotter *_plotter))
#else
_t_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->type = PL_TEK;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 0;
  _plotter->have_dash_array = 0;
  _plotter->have_solid_fill = 0;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 0;
  _plotter->have_hershey_fonts = 1;
  _plotter->have_ps_fonts = 0;
  _plotter->have_pcl_fonts = 0;
  _plotter->have_stick_fonts = 0;
  _plotter->have_extra_stick_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->default_font_type = F_HERSHEY;
  _plotter->pcl_before_ps = false;
  _plotter->have_horizontal_justification = false;
  _plotter->have_vertical_justification = false;
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal) */
  _plotter->max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;
  _plotter->have_mixed_paths = false;
  _plotter->allowed_arc_scaling = AS_NONE;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;  
  _plotter->flush_long_polylines = false; /* avoid unneeded endpath()'s */
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->flipped_y = false;
  _plotter->imin = 488;
  _plotter->imax = 3607;  
  _plotter->jmin = 0;
  _plotter->jmax = 3119;  
  _plotter->xmin = 0.0;
  _plotter->xmax = 0.0;  
  _plotter->ymin = 0.0;
  _plotter->ymax = 0.0;  
  _plotter->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  _plotter->tek_display_type = D_GENERIC;
  _plotter->tek_mode = MODE_ALPHA;
  _plotter->tek_line_type = L_SOLID;
  _plotter->tek_mode_is_unknown = true;
  _plotter->tek_line_type_is_unknown = true;
  _plotter->tek_kermit_fgcolor = -1; /* nonsensical value; means `unknown' */
  _plotter->tek_kermit_bgcolor = -1; /* same */
  _plotter->tek_position_is_unknown = true;
  _plotter->tek_pos.x = 0;
  _plotter->tek_pos.y = 0;

  /* initialize certain data members from device driver parameters */

  /* determine Tek display type (xterm(1) / kermit(1) / generic Tektronix) */
  {
    const char* term_type;
    
    term_type = (const char *)_get_plot_param (R___(_plotter) "TERM");
    if (term_type != NULL)
      {
	if ((strcmp (term_type, "xterm") == 0)
	    || (strcmp (term_type, "xterms") == 0)
	    || (strcmp (term_type, "kterm") == 0))
	  _plotter->tek_display_type = D_XTERM;
	else if ((strcmp (term_type, "ansi.sys") == 0)
		 || (strcmp (term_type, "ansi.sysk") == 0)
		 || (strcmp (term_type, "ansisys") == 0)
		 || (strcmp (term_type, "ansisysk") == 0)
		 || (strcmp (term_type, "kermit") == 0))
	  _plotter->tek_display_type = D_KERMIT;
	else
	  _plotter->tek_display_type = D_GENERIC;
      }
    else
      _plotter->tek_display_type = D_GENERIC; /* default value */
  }      
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  It may do such things as write
   to an output stream from internal storage, deallocate storage, etc.
   When this is invoked, _plotter points to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_t_terminate (S___(Plotter *_plotter))
#else
_t_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

#ifndef LIBPLOTTER
  /* invoke generic method, e.g. to deallocate instance-specific copies
     of class variables */
  _g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
TekPlotter::TekPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (ostream& out)
	: Plotter (out)
{
  _t_initialize ();
}

TekPlotter::TekPlotter ()
{
  _t_initialize ();
}

TekPlotter::TekPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _t_initialize ();
}

TekPlotter::~TekPlotter ()
{
  _t_terminate ();
}
#endif
