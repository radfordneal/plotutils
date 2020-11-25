/* This file defines the initialization for any XDrawablePlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   an XDrawablePlotter struct. */
const Plotter _x_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _x_initialize, _x_terminate,
  /* page manipulation */
  _x_begin_page, _x_erase_page, _x_end_page,
  /* drawing state manipulation */
  _x_push_state, _x_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _x_paint_path, _x_paint_paths, _x_path_is_flushable, _x_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _x_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_paint_text_string_with_escapes, _x_paint_text_string,
  _x_get_text_width,
  /* private low-level `retrieve font' method */
  _x_retrieve_font,
  /* `flush output' method, called only if Plotter handles its own output */
  _x_flush_output,
  /* internal error handlers */
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
_x_initialize (S___(Plotter *_plotter))
#else
_x_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  Colormap *x_cmap_ptr;
  Drawable *drawable_p1, *drawable_p2;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_X11_DRAWABLE;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES_TO_NON_STREAM;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 1;
  _plotter->data->have_escaped_string_support = 0;
  _plotter->data->have_ps_fonts = 1;
#ifdef USE_LJ_FONTS_IN_X
  _plotter->data->have_pcl_fonts = 1;
#else
  _plotter->data->have_pcl_fonts = 0;
#endif
  _plotter->data->have_stick_fonts = 0;
  _plotter->data->have_extra_stick_fonts = 0;
  _plotter->data->have_other_fonts = 1;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->data->default_font_type = F_POSTSCRIPT;
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
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_LIBXMI; /* X != NeWS, alas */
  _plotter->data->flipped_y = true;
  _plotter->data->imin = 0;
  _plotter->data->imax = 569;  
  _plotter->data->jmin = 569;
  _plotter->data->jmax = 0;		/* flipped y */
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 0.0;  
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 0.0;  
  _plotter->data->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  _plotter->x_dpy = (Display *)NULL;
  _plotter->x_visual = (Visual *)NULL;
  _plotter->x_drawable1 = (Drawable)0;
  _plotter->x_drawable2 = (Drawable)0;  
  _plotter->x_drawable3 = (Drawable)0;
  _plotter->x_double_buffering = DBL_NONE;
  _plotter->x_max_polyline_len = INT_MAX; /* reduced in openpl() */
  _plotter->x_fontlist = (plFontRecord *)NULL;
  _plotter->x_colorlist = (plColorRecord *)NULL;  
  _plotter->x_cmap = (Colormap)0;
  _plotter->x_cmap_type = CMAP_ORIG;
  _plotter->x_colormap_warning_issued = false;
  _plotter->x_bg_color_warning_issued = false;
  _plotter->x_paint_pixel_count = 0;

  /* initialize certain data members from device driver parameters */

  /* if this is NULL, won't be able to open Plotter */
  _plotter->x_dpy = (Display *)_get_plot_param (_plotter->data, "XDRAWABLE_DISPLAY");

  /* we allow the visual to be NULL, i.e., not set, since we use it only
     for determining the visual class of the colormap (see below); since if
     it's Truecolor, that means we can avoid calling XAllocColor() */
  _plotter->x_visual = (Visual *)_get_plot_param (_plotter->data, "XDRAWABLE_VISUAL");

  /* we allow either or both of the drawables to be NULL, i.e. not set */
  drawable_p1 = (Drawable *)_get_plot_param (_plotter->data, "XDRAWABLE_DRAWABLE1");
  drawable_p2 = (Drawable *)_get_plot_param (_plotter->data, "XDRAWABLE_DRAWABLE2");
  _plotter->x_drawable1 = drawable_p1 ? *drawable_p1 : 0;
  _plotter->x_drawable2 = drawable_p2 ? *drawable_p2 : 0;

  /* allow user to specify a non-default colormap */
  x_cmap_ptr = (Colormap *)_get_plot_param (_plotter->data, "XDRAWABLE_COLORMAP");
  if (x_cmap_ptr != NULL)
    /* user-specified colormap */
    {
      _plotter->x_cmap = *x_cmap_ptr;
      if (_plotter->x_dpy)
	/* have a display, so is this the default colormap? */
	{
	  int screen;		/* screen number */
	  Screen *screen_struct; /* screen structure */
	  
	  screen = DefaultScreen (_plotter->x_dpy);
	  screen_struct = ScreenOfDisplay (_plotter->x_dpy, screen);
	  if (_plotter->x_cmap == DefaultColormapOfScreen (screen_struct))
	    /* it is, so as visual, use visual of default screen */
	  _plotter->x_visual = DefaultVisualOfScreen (screen_struct);
	}
    }
  else 
    /* default colormap */
    {
      if (_plotter->x_dpy)
	/* have a display, so as default, use colormap of its default screen */
	{
	  int screen;		/* screen number */
	  Screen *screen_struct; /* screen structure */
	  
	  screen = DefaultScreen (_plotter->x_dpy);
	  screen_struct = ScreenOfDisplay (_plotter->x_dpy, screen);
	  _plotter->x_cmap = DefaultColormapOfScreen (screen_struct);
	  
	  /* also, as visual, use visual of its default screen */
	  _plotter->x_visual = DefaultVisualOfScreen (screen_struct);
	}
    }

  /* colormap type will always be `original' (unlike XPlotters, XDrawable
     Plotters never switch to a private colormap) */
  _plotter->x_cmap_type = CMAP_ORIG;
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
#ifdef _HAVE_PROTOS
_x_terminate (S___(Plotter *_plotter))
#else
_x_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
XDrawablePlotter::XDrawablePlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (FILE *outfile)
	:Plotter (outfile)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (ostream& out)
	: Plotter (out)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter ()
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _x_initialize ();
}

XDrawablePlotter::XDrawablePlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _x_initialize ();
}

XDrawablePlotter::~XDrawablePlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _x_terminate ();
}
#endif

#ifndef LIBPLOTTER
/* The following forwarding functions provide special support in libplot
   for deriving the XPlotter class from the XDrawablePlotter class.  In
   libplotter, forwarding is implemented by a virtual function; see
   plotter.h. */

/* Forwarding function called by any XDrawablePlotter/XPlotter in
   x_color.c, if the original colormap fills up.  See x_openpl.c and
   y_openpl.c for the two forwarded-to functions
   _x_maybe_get_new_colormap() and _y_maybe_get_new_colormap(),
   respectively.  The former is a no-op, but the latter tries to switch to
   a new colormap. */
void
#ifdef _HAVE_PROTOS
_maybe_get_new_colormap (Plotter *_plotter)
#else
_maybe_get_new_colormap (_plotter)
     Plotter *_plotter;
#endif
{
  switch ((int)_plotter->data->type)
    {
    case (int)PL_X11_DRAWABLE:
    default:
      _x_maybe_get_new_colormap (_plotter); /* no-op */
      break;
    case (int)PL_X11:
      _y_maybe_get_new_colormap (_plotter);
      break;
    }
}

/* Forwarding function called by any XDrawablePlotter at the conclusion of
   most drawing operations.  See x_openpl.c and y_openpl.c for the two
   forwarded-to functions _x_maybe_handle_x_events() and
   _y_maybe_handle_x_events(), respectively.  The former is a no-op, but
   the latter is processes pending X events. */
void
#ifdef _HAVE_PROTOS
_maybe_handle_x_events (Plotter *_plotter)
#else
_maybe_handle_x_events (_plotter)
     Plotter *_plotter;
#endif
{
  switch ((int)_plotter->data->type)
    {
    case (int)PL_X11_DRAWABLE:
    default:
      _x_maybe_handle_x_events (_plotter); /* no-op */
      break;
    case (int)PL_X11:
      _y_maybe_handle_x_events (_plotter);
      break;
    }
}
#endif /* not LIBPLOTTER */
