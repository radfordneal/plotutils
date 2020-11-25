/* This file defines the initialization for any ReGISPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

/* Note that ReGIS displays are rectangular, and wider than they are high.
   In terms of integer ReGIS coordinates a ReGIS display is a
   [0..767]x[0..479] rectangle, and we choose our viewport to be the square
   [144..623]x[0..479].  I.e. we define it to be a square, occupying the
   entire height of the display, and centered on the display. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a ReGISPlotter struct. */
const Plotter _r_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _r_initialize, _r_terminate,
  /* page manipulation */
  _r_begin_page, _r_erase_page, _r_end_page,
  /* drawing state manipulation */
  _g_push_state, _g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _r_paint_path, _r_paint_paths, _r_path_is_flushable, _r_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _r_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_paint_text_string_with_escapes, _g_paint_text_string,
  _g_get_text_width,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* `flush output' method, called only if Plotter handles its own output */
  _g_flush_output,
  /* error handlers */
  _g_warning,
  _g_error,
};
#endif /* not LIBPLOTTER */

/* The private `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  When this
   is invoked, _plotter points to the Plotter that has just been
   created. */

void
#ifdef _HAVE_PROTOS
_r_initialize (S___(Plotter *_plotter))
#else
_r_initialize (S___(_plotter))
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
  _plotter->data->type = PL_REGIS;
#endif

  /* output model */
  _plotter->data->output_model =  PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME;
  
  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 0;
  _plotter->data->have_dash_array = 0;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 0;
  _plotter->data->have_settable_bg = 1;
  _plotter->data->have_escaped_string_support = 0;
  _plotter->data->have_ps_fonts = 0;
  _plotter->data->have_pcl_fonts = 0;
  _plotter->data->have_stick_fonts = 0;
  _plotter->data->have_extra_stick_fonts = 0;
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->data->default_font_type = F_HERSHEY;
  _plotter->data->pcl_before_ps = false;
  _plotter->data->have_horizontal_justification = false;
  _plotter->data->have_vertical_justification = false;
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal) */
  _plotter->data->max_unfilled_path_length = MAX_UNFILLED_PATH_LENGTH;
  _plotter->data->have_mixed_paths = false;
  _plotter->data->allowed_arc_scaling = AS_NONE;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;  
  _plotter->data->allowed_quad_scaling = AS_NONE;  
  _plotter->data->allowed_cubic_scaling = AS_NONE;  
  _plotter->data->allowed_box_scaling = AS_NONE;
  _plotter->data->allowed_circle_scaling = AS_UNIFORM;
  _plotter->data->allowed_ellipse_scaling = AS_NONE;

  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->data->flipped_y = true;
  _plotter->data->imin = 144;
  _plotter->data->imax = 623;  
  _plotter->data->jmin = 479;
  _plotter->data->jmax = 0;		/* flipped y */
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 0.0;  
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 0.0;  
  _plotter->data->page_data = (plPageData *)NULL;

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* initialize data members specific to this derived class */
  _plotter->regis_pos.x = 0;	/* dummy */
  _plotter->regis_pos.y = 0;	/* dummy */
  _plotter->regis_position_is_unknown = true;  
  _plotter->regis_line_type = L_SOLID; /* dummy */
  _plotter->regis_line_type_is_unknown = true;  
  _plotter->regis_fgcolor = 0; /* dummy */
  _plotter->regis_bgcolor = 0; /* dummy */
  _plotter->regis_fgcolor_is_unknown = true;
  _plotter->regis_bgcolor_is_unknown = true;

  /* initialize certain data members from device driver parameters */

}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  It may do such things as write
   to an output stream from internal storage, deallocate storage, etc.
   When this is invoked, _plotter points to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_r_terminate (S___(Plotter *_plotter))
#else
_r_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* invoke generic method, e.g. to deallocate instance-specific copies
     of class variables */
  _g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
ReGISPlotter::ReGISPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (ostream& out)
	: Plotter (out)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter ()
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _r_initialize ();
}

ReGISPlotter::ReGISPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _r_initialize ();
}

ReGISPlotter::~ReGISPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _r_terminate ();
}
#endif
