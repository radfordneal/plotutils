/* This file defines the initialization for any MetaPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a MetaPlotter struct. */
const Plotter _m_default_plotter = 
{
  /* methods */
  _m_alabel, _m_arc, _m_arcrel, _m_bezier2, _m_bezier2rel, _m_bezier3, _m_bezier3rel, _m_bgcolor, _g_bgcolorname, _m_box, _m_boxrel, _m_capmod, _m_circle, _m_circlerel, _m_closepl, _g_color, _g_colorname, _m_cont, _m_contrel, _m_ellarc, _m_ellarcrel, _m_ellipse, _m_ellipserel, _m_endpath, _m_endsubpath, _m_erase, _m_farc, _m_farcrel, _m_fbezier2, _m_fbezier2rel, _m_fbezier3, _m_fbezier3rel, _m_fbox, _m_fboxrel, _m_fcircle, _m_fcirclerel, _m_fconcat, _m_fcont, _m_fcontrel, _m_fellarc, _m_fellarcrel, _m_fellipse, _m_fellipserel, _m_ffontname, _m_ffontsize, _m_fillcolor, _g_fillcolorname, _m_fillmod, _m_filltype, _g_flabelwidth, _m_fline, _m_flinedash, _m_flinerel, _m_flinewidth, _g_flushpl, _m_fmarker, _m_fmarkerrel, _m_fmiterlimit, _m_fmove, _m_fmoverel, _m_fontname, _m_fontsize, _m_fpoint, _m_fpointrel, _g_frotate, _g_fscale, _m_fspace, _m_fspace2, _m_ftextangle, _g_ftranslate, _g_havecap, _m_joinmod, _m_label, _g_labelwidth, _m_line, _m_linedash, _m_linemod, _m_linerel, _m_linewidth, _m_marker, _m_markerrel, _m_move, _m_moverel, _m_openpl, _m_orientation, _g_outfile, _m_pencolor, _g_pencolorname, _m_pentype, _m_point, _m_pointrel, _m_restorestate, _m_savestate, _m_space, _m_space2, _m_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _m_initialize, _m_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_falabel_hershey, _g_falabel_ps, _g_falabel_pcl, _g_falabel_stick, _g_falabel_other,
  _g_flabelwidth_hershey, _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, _g_flabelwidth_other,
  /* internal `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _g_set_font,
  /* private low-level `sync line attributes' method */
  _g_set_attributes,
  /* private low-level `sync color' methods */
  _g_set_pen_color,
  _g_set_fill_color,
  _g_set_bg_color,
  /* private low-level `sync position' method */
  _g_set_position,
  /* internal `error handler' methods */
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
_m_initialize (S___(Plotter *_plotter))
#else
_m_initialize (S___(_plotter))
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
  _plotter->type = PL_META;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 2;
  _plotter->have_dash_array = 2;
  _plotter->have_solid_fill = 2;
  _plotter->have_odd_winding_fill = 2;
  _plotter->have_nonzero_winding_fill = 2;
  _plotter->have_settable_bg = 2;
  _plotter->have_hershey_fonts = 1;
  _plotter->have_ps_fonts = 1;
  _plotter->have_pcl_fonts = 1;
  _plotter->have_stick_fonts = 1;
  _plotter->have_extra_stick_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->default_font_type = F_HERSHEY;
  _plotter->pcl_before_ps = false;
  _plotter->have_horizontal_justification = true;
  _plotter->have_vertical_justification = false;
  _plotter->issue_font_warning = false;

  /* path and polyline-related parameters (also internal) */
  _plotter->max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;
  _plotter->have_mixed_paths = false;
  _plotter->allowed_arc_scaling = AS_NONE;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;  
  _plotter->flush_long_polylines = false; /* avoid spurious endpath()'s */
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_model_type = (int)DISP_MODEL_NONE;
  _plotter->display_coors_type = (int)DISP_DEVICE_COORS_REAL;
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
  _plotter->meta_portable_output = false;

  /* initialize certain data members from device driver parameters */
      
  /* determine version of metafile format */
  {
    const char *portable_s;
    
    portable_s = (const char *)_get_plot_param (R___(_plotter) 
						"META_PORTABLE");
    if (strcasecmp (portable_s, "yes") == 0)
      _plotter->meta_portable_output = true;
    else
      _plotter->meta_portable_output = false; /* default value */
  }
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
#ifdef _HAVE_PROTOS
_m_terminate (S___(Plotter *_plotter))
#else
_m_terminate (S___(_plotter))
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

#ifdef LIBPLOTTER
MetaPlotter::MetaPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	: Plotter (infile, outfile, errfile)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (FILE *outfile)
	: Plotter (outfile)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (ostream& out)
	: Plotter (out)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter ()
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _m_initialize ();
}

MetaPlotter::MetaPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _m_initialize ();
}

MetaPlotter::~MetaPlotter ()
{
  _m_terminate ();
}
#endif
