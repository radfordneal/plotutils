/* This file defines the initialization for any GIFPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"		/* use libxmi scan conversion module */

/* forward references */
static bool _parse_bitmap_size ____P((const char *bitmap_size_s, int *width, int *height));

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a GIFPlotter struct. */
const Plotter _i_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _i_initialize, _i_terminate,
  /* page manipulation */
  _i_begin_page, _i_erase_page, _i_end_page,
  /* drawing state manipulation */
  _g_push_state, _g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _i_paint_path, _i_paint_paths, _g_path_is_flushable, _g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _i_paint_point,
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
_i_initialize (S___(Plotter *_plotter))
#else
_i_initialize (S___(_plotter))
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
  _plotter->data->type = PL_GIF;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
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

  /* path-related parameters (also internal); note that we
     don't set max_unfilled_path_length, because it was set by the
     superclass initialization */
  _plotter->data->have_mixed_paths = false;
  _plotter->data->allowed_arc_scaling = AS_AXES_PRESERVED;
  _plotter->data->allowed_ellarc_scaling = AS_AXES_PRESERVED;
  _plotter->data->allowed_quad_scaling = AS_NONE;  
  _plotter->data->allowed_cubic_scaling = AS_NONE;  
  _plotter->data->allowed_box_scaling = AS_NONE;
  _plotter->data->allowed_circle_scaling = AS_NONE;
  _plotter->data->allowed_ellipse_scaling = AS_AXES_PRESERVED;

  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_LIBXMI;
  _plotter->data->flipped_y = true;
  _plotter->data->imin = 0;
  _plotter->data->imax = 569;  
  _plotter->data->jmin = 569;
  _plotter->data->jmax = 0;
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 0.0;  
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 0.0;  
  _plotter->data->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->i_xn = _plotter->data->imax + 1;
  _plotter->i_yn = _plotter->data->jmin + 1;
  _plotter->i_num_pixels = (_plotter->i_xn) * (_plotter->i_yn);
  _plotter->i_animation = true;	/* default, can be turned off */
  _plotter->i_iterations = 0;
  _plotter->i_delay = 0;
  _plotter->i_interlace = false;
  _plotter->i_transparent = false;  
  _plotter->i_transparent_color.red = 255; /* dummy */
  _plotter->i_transparent_color.green = 255; /* dummy */
  _plotter->i_transparent_color.blue = 255; /* dummy */
  _plotter->i_transparent_index = 0; /* dummy */
  /* storage used by libxmi's reentrant miDrawArcs_r() function for
     cacheing rasterized ellipses */
  _plotter->i_arc_cache_data = (voidptr_t)miNewEllipseCache ();
  /* dynamic variables */
  _plotter->i_painted_set = (voidptr_t)NULL;
  _plotter->i_canvas = (voidptr_t)NULL;
  /* N.B. _plotter->i_colormap is initialized in i_openpl.c */
  _plotter->i_num_color_indices = 0;
  _plotter->i_bit_depth = 0;
  _plotter->i_frame_nonempty = false;
  _plotter->i_pixels_scanned = 0;
  _plotter->i_pass = 0;
  _plotter->i_hot.x = 0;
  _plotter->i_hot.y = 0;  
  /* N.B. _plotter->i_global_colormap, i_num_global_color_indices are
     copied into later */
  _plotter->i_header_written = false;

  /* initialize certain data members from device driver parameters */

  /* is there a user-specified transparent color? */
  {
    const char *transparent_name_s;
    plColor color;

    transparent_name_s = (const char *)_get_plot_param (_plotter->data, "TRANSPARENT_COLOR");
    if (transparent_name_s 
	&& _string_to_color (transparent_name_s, &color, _plotter->data->color_name_cache))
      /* have 24-bit RGB */
      {
	_plotter->i_transparent = true;
	_plotter->i_transparent_color = color;
      }
  }

  /* produce an interlaced GIF? */
  {
    const char *interlace_s;

    interlace_s = (const char *)_get_plot_param (_plotter->data, "INTERLACE" );
    if (strcasecmp (interlace_s, "yes") == 0)
      _plotter->i_interlace = true;
  }
  
  /* turn off animation? */
  {
    const char *animate_s;

    animate_s = (const char *)_get_plot_param (_plotter->data, "GIF_ANIMATION" );
    if (strcasecmp (animate_s, "no") == 0)
      _plotter->i_animation = false;
  }
  
  /* determine number of iterations to be used (if animating) */
  {
    const char *iteration_s;
    int num_iterations;

    iteration_s = (const char *)_get_plot_param (_plotter->data, "GIF_ITERATIONS" );
    if (sscanf (iteration_s, "%d", &num_iterations) > 0 
	&& num_iterations >= 0 && num_iterations <= 65535)
      _plotter->i_iterations = num_iterations;
  }

  /* determine delay after each frame, in 1/100 sec units */
  {
    const char *delay_s;
    int delay;

    delay_s = (const char *)_get_plot_param (_plotter->data, "GIF_DELAY" );
    if (sscanf (delay_s, "%d", &delay) > 0 
	&& delay >= 0 && delay <= 65535)
      _plotter->i_delay = delay;
  }

  /* determine the range of device coordinates over which the graphics
     display will extend (and hence the transformation from user to device
     coordinates). */
  {
    const char *bitmap_size_s;
    int width = 1, height = 1;
	
    bitmap_size_s = (const char *)_get_plot_param (_plotter->data, "BITMAPSIZE");
    if (bitmap_size_s && _parse_bitmap_size (bitmap_size_s, &width, &height)
	/* insist on range of 1..65535 for GIF format */
	&& width >= 1 && height >= 1
	&& width <= 65535 && height <= 65535)
      /* override defaults above */
      {
	_plotter->data->imax = width - 1;
	_plotter->data->jmin = height - 1;
	_plotter->i_xn = width;
	_plotter->i_yn = height;
	_plotter->i_num_pixels = width * height;
      }
  }

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);
}

static bool 
#ifdef _HAVE_PROTOS
_parse_bitmap_size (const char *bitmap_size_s, int *width, int *height)
#else
_parse_bitmap_size (bitmap_size_s, width, height)
  const char *bitmap_size_s;
  int *width, *height;
#endif
{
  int local_width = 1, local_height = 1;

  if (bitmap_size_s
      /* should parse this better */
      && sscanf (bitmap_size_s, "%dx%d", &local_width, &local_height) == 2
      && local_width > 0 && local_height > 0)
    {
      *width = local_width;
      *height = local_height;
      return true;
    }
  else
    return false;
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_i_terminate (S___(Plotter *_plotter))
#else
_i_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* free storage used by libxmi's reentrant miDrawArcs_r() function */
  miDeleteEllipseCache ((miEllipseCache *)_plotter->i_arc_cache_data);

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
GIFPlotter::GIFPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (ostream& out)
	: Plotter (out)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter ()
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _i_initialize ();
}

GIFPlotter::GIFPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _i_initialize ();
}

GIFPlotter::~GIFPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _i_terminate ();
}
#endif
