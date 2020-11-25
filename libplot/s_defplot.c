/* This file defines the initialization for any SVGPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a SVGPlotter struct. */
const Plotter _s_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _s_initialize, _s_terminate,
  /* page manipulation */
  _s_begin_page, _s_erase_page, _s_end_page,
  /* drawing state manipulation */
  _g_push_state, _g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _s_paint_path, _s_paint_paths, _g_path_is_flushable, _g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _s_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_paint_text_string_with_escapes, _s_paint_text_string,
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
_s_initialize (S___(Plotter *_plotter))
#else
_s_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double xoffset, yoffset;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize (S___(_plotter));
#endif

  /* override generic initializations (which are appropriate to the base
     Plotter class), as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_SVG;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_ONE_PAGE;
  
  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 1;
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
  _plotter->data->have_horizontal_justification = true;
  _plotter->data->have_vertical_justification = true;
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal); note that we
     don't set max_unfilled_path_length, because it was set by the
     superclass initialization */
  _plotter->data->have_mixed_paths = false;
  _plotter->data->allowed_arc_scaling = AS_ANY;
  _plotter->data->allowed_ellarc_scaling = AS_ANY;
  _plotter->data->allowed_quad_scaling = AS_ANY;
  _plotter->data->allowed_cubic_scaling = AS_ANY;
  _plotter->data->allowed_box_scaling = AS_ANY;
  _plotter->data->allowed_circle_scaling = AS_ANY;
  _plotter->data->allowed_ellipse_scaling = AS_ANY;

  /* color-related parameters (also internal) */
  _plotter->data->emulate_color = false;
  
  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_REAL;
  _plotter->data->flipped_y = true;
  _plotter->data->imin = 0;
  _plotter->data->imax = 0;  
  _plotter->data->jmin = 0;
  _plotter->data->jmax = 0;  
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 1.0;
  _plotter->data->ymin = 1.0;	/* note flipped y coordinate */
  _plotter->data->ymax = 0.0;
  _plotter->data->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  _plotter->s_matrix[0] = 1.0;/* dummy matrix values */
  _plotter->s_matrix[1] = 0.0;
  _plotter->s_matrix[2] = 0.0;
  _plotter->s_matrix[3] = 1.0;
  _plotter->s_matrix[4] = 0.0;
  _plotter->s_matrix[5] = 0.0;
  _plotter->s_matrix_is_unknown = true;
  _plotter->s_matrix_is_bogus = false;
  _plotter->s_bgcolor.red = -1;	/* initialized in s_begin_page */
  _plotter->s_bgcolor.green = -1;
  _plotter->s_bgcolor.blue = -1;
  _plotter->s_bgcolor_suppressed = false;

  /* Note: xmin,xmax,ymin,ymax above determine the range of device
     coordinates over which the viewport will extend (and hence the
     transformation from user to device coordinates; see g_space.c).

     For an SVG Plotter, `device coordinates' are basically the same as
     libplot's NDC coordinates, on account of the way we wrap a
     transformation matrix around each page in the output file; see
     s_closepl.c.  However, SVG uses a flipped-y convention: ymin,ymax are
     1 and 0 respectively (see above). */

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* initialize certain data members from device driver parameters */
      
  /* determine page type, and hence nominal viewport size (returned xoffset
     and yoffset make no sense in an SVG context, so we'll ignore them) */
  _set_page_type (_plotter->data, &xoffset, &yoffset);
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
#ifdef _HAVE_PROTOS
_s_terminate (S___(Plotter *_plotter))
#else
_s_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
}

#ifdef LIBPLOTTER
SVGPlotter::SVGPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (ostream& out)
	: Plotter (out)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter ()
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _s_initialize ();
}

SVGPlotter::SVGPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _s_initialize ();
}

SVGPlotter::~SVGPlotter ()
{
  _s_terminate ();
}
#endif
