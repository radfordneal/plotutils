/* This file defines the initialization for any Bitmap Plotter object,
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
   a BitmapPlotter struct. */
const Plotter _b_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _b_initialize, _b_terminate,
  /* page manipulation */
  _b_begin_page, _b_erase_page, _b_end_page,
  /* drawing state manipulation */
  _g_push_state, _g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _b_paint_path, _b_paint_paths, _g_path_is_flushable, _g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _b_paint_point,
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
_b_initialize (S___(Plotter *_plotter))
#else
_b_initialize (S___(_plotter))
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
  _plotter->data->type = PL_BITMAP;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_NONE;

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
  _plotter->b_xn = _plotter->data->imax + 1;
  _plotter->b_yn = _plotter->data->jmin + 1;
  _plotter->b_painted_set = (voidptr_t)NULL;
  _plotter->b_canvas = (voidptr_t)NULL;

  /* initialize storage used by libxmi's reentrant miDrawArcs_r() function
     for cacheing rasterized ellipses */
  _plotter->b_arc_cache_data = (voidptr_t)miNewEllipseCache ();

  /* determine the range of device coordinates over which the graphics
     display will extend (and hence the transformation from user to device
     coordinates). */
  {
    const char *bitmap_size_s;
    int width = 1, height = 1;
	
    bitmap_size_s = (const char *)_get_plot_param (_plotter->data, "BITMAPSIZE");
    if (bitmap_size_s && _parse_bitmap_size (bitmap_size_s, &width, &height)
	/* insist on >=1 */
	&& width >= 1 && height >= 1)
      /* override defaults above */
      {
	_plotter->data->imax = width - 1;
	_plotter->data->jmin = height - 1;
	_plotter->b_xn = width;
	_plotter->b_yn = height;
      }
  }

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* initialize certain data members from device driver parameters */
      
  /* for this class, there are none */
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
_b_terminate (S___(Plotter *_plotter))
#else
_b_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* free storage used by libxmi's reentrant miDrawArcs_r() function */
  miDeleteEllipseCache ((miEllipseCache *)_plotter->b_arc_cache_data);

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
BitmapPlotter::BitmapPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (ostream& out)
	: Plotter (out)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter ()
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _b_initialize ();
}

BitmapPlotter::BitmapPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _b_initialize ();
}

BitmapPlotter::~BitmapPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _b_terminate ();
}
#endif

#ifndef LIBPLOTTER
/* The following forwarding functions provide special support in libplot
   for deriving classes such as the PNMPlotter and PNGPlotter classes from
   the BitmapPlotter class.  In libplotter, forwarding is implemented by a
   virtual function; see plotter.h. */

/* Forwarding function called by any BitmapPlotter in closepl.  See
   b_closepl.c, n_write.c, z_write.c for the forwarded-to functions.  The
   first is currently a no-op. */
int
#ifdef _HAVE_PROTOS
_maybe_output_image (Plotter *_plotter)
#else
_maybe_output_image (_plotter)
     Plotter *_plotter;
#endif
{
  int retval;

  switch ((int)(_plotter->data->type))
    {
    case (int)PL_BITMAP:
    default:
      retval = _b_maybe_output_image (_plotter);
      break;
    case (int)PL_PNM:
      retval = _n_maybe_output_image (_plotter);
      break;
#ifdef INCLUDE_PNG_SUPPORT
    case (int)PL_PNG:
      retval = _z_maybe_output_image (_plotter);
      break;
#endif
    }

  return retval;
}
#endif /* not LIBPLOTTER */
