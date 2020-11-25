/* This file defines the initialization for any AIPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   an AIPlotter struct. */
const Plotter _a_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _a_initialize, _a_terminate,
  /* page manipulation */
  _a_begin_page, _a_erase_page, _a_end_page,
  /* drawing state manipulation */
  _g_push_state, _g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _a_paint_path, _a_paint_paths, _g_path_is_flushable, _g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _a_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_paint_text_string_with_escapes, _a_paint_text_string,
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

/* For AI Plotter objects, we determine the page size and the location of
   the viewport on the page, so that we'll be able to work out the map from
   user coordinates to device coordinates in space.c.  Also we determine
   which version of Illustrator file format we'll emit. */

void
#ifdef _HAVE_PROTOS
_a_initialize (S___(Plotter *_plotter))
#else
_a_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double xoffset, yoffset;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_AI;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_ONE_PAGE;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 0;
  _plotter->data->have_escaped_string_support = 0;
  _plotter->data->have_ps_fonts = 1;
  _plotter->data->have_pcl_fonts = 1;
  _plotter->data->have_stick_fonts = 0;
  _plotter->data->have_extra_stick_fonts = 0;
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->data->default_font_type = F_POSTSCRIPT;
  _plotter->data->pcl_before_ps = false;
  _plotter->data->issue_font_warning = true;
  _plotter->data->have_horizontal_justification = true;
  _plotter->data->have_vertical_justification = false;

  /* path-related parameters (also internal); note that we
     don't set max_unfilled_path_length, because it was set by the
     superclass initialization */
  _plotter->data->have_mixed_paths = true;
  _plotter->data->allowed_arc_scaling = AS_NONE;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;
  _plotter->data->allowed_quad_scaling = AS_NONE;
  _plotter->data->allowed_cubic_scaling = AS_ANY;
  _plotter->data->allowed_box_scaling = AS_NONE;
  _plotter->data->allowed_circle_scaling = AS_NONE;
  _plotter->data->allowed_ellipse_scaling = AS_NONE;

  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_PHYSICAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_REAL;
  _plotter->data->flipped_y = false;
  _plotter->data->imin = 0;
  _plotter->data->imax = 0;  
  _plotter->data->jmin = 0;
  _plotter->data->jmax = 0;  
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 0.0;  
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 0.0;  
  _plotter->data->page_data = (plPageData *)NULL;

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
  _plotter->ai_cap_style = PS_LINE_CAP_BUTT;
  _plotter->ai_join_style = PS_LINE_JOIN_MITER;  
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
    
    version_s = (const char *)_get_plot_param (_plotter->data, "AI_VERSION");
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
    _plotter->data->have_odd_winding_fill = 0;

  /* determine page type, and user-specified viewport offset if any */
  _set_page_type (_plotter->data, &xoffset, &yoffset);
  
  /* Determine range of device coordinates over which the viewport will
     extend (and hence the transformation from user to device coordinates;
     see g_space.c). */
  {
    double xmid, ymid, viewport_size;
    
    viewport_size = _plotter->data->page_data->viewport_size;
    xmid = 0.5 * _plotter->data->page_data->xsize + xoffset;
    ymid = 0.5 * _plotter->data->page_data->ysize + yoffset;

    _plotter->data->xmin = 72 * (xmid - 0.5 * viewport_size);
    _plotter->data->xmax = 72 * (xmid + 0.5 * viewport_size);    
    _plotter->data->ymin = 72 * (ymid - 0.5 * viewport_size);
    _plotter->data->ymax = 72 * (ymid + 0.5 * viewport_size);    
  }

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
#ifdef _HAVE_PROTOS
_a_terminate (S___(Plotter *_plotter))
#else
_a_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate (S___(_plotter));
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

AIPlotter::AIPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _a_initialize ();
}

AIPlotter::AIPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _a_initialize ();
}

AIPlotter::AIPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _a_initialize ();
}

AIPlotter::AIPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _a_initialize ();
}

AIPlotter::AIPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _a_initialize ();
}

AIPlotter::~AIPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _a_terminate ();
}
#endif
