/* This file defines the initialization for any XDrawablePlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API.

   This is identical to the initialization for an XPlotter, except that it
   has _x_openpl, _x_erase, _x_closepl instead of _y_openpl, _y_erase,
   _y_closepl. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   an XDrawablePlotter struct. */
const Plotter _x_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _x_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _x_endpath,  _g_endsubpath, _x_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _g_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _x_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _x_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _x_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _x_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _x_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _x_restorestate, _x_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _x_initialize, _x_terminate,
  /* internal methods that plot strings in non-Hershey fonts */
  _g_falabel_hershey, _x_falabel_ps, _x_falabel_pcl, _g_falabel_stick, _x_falabel_other,
  _g_flabelwidth_hershey, _x_flabelwidth_ps, _x_flabelwidth_pcl, _g_flabelwidth_stick, _x_flabelwidth_other,
  /* private low-level `retrieve font' method */
  _x_retrieve_font,
  /* private low-level `sync font' method */
  _g_set_font,
  /* private low-level `sync line attributes' method */
  _x_set_attributes,
  /* private low-level `sync color' methods */
  _x_set_pen_color,
  _x_set_fill_color,
  _x_set_bg_color,
  /* private low-level `sync position' method */
  _g_set_position,
  /* internal error handlers */
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
  _plotter->type = PL_X11_DRAWABLE;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 1;
  _plotter->have_dash_array = 1;
  _plotter->have_solid_fill = 1;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 1;
  _plotter->have_hershey_fonts = 1;
  _plotter->have_ps_fonts = 1;
#ifdef USE_LJ_FONTS_IN_X
  _plotter->have_pcl_fonts = 1;
#else
  _plotter->have_pcl_fonts = 0;
#endif
  _plotter->have_stick_fonts = 0;
  _plotter->have_extra_stick_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->default_font_type = F_POSTSCRIPT;
  _plotter->pcl_before_ps = false;
  _plotter->have_horizontal_justification = false;
  _plotter->have_vertical_justification = false;
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal); note that we
     don't set max_unfilled_polyline_length, because it was set by the
     superclass initialization */
  _plotter->have_mixed_paths = false;
  _plotter->allowed_arc_scaling = AS_AXES_PRESERVED;
  _plotter->allowed_ellarc_scaling = AS_AXES_PRESERVED;
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;  
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_LIBXMI; /* X != NeWS, alas */
  _plotter->flipped_y = true;
  _plotter->imin = 0;
  _plotter->imax = 569;  
  _plotter->jmin = 569;
  _plotter->jmax = 0;  
  _plotter->xmin = 0.0;
  _plotter->xmax = 0.0;  
  _plotter->ymin = 0.0;
  _plotter->ymax = 0.0;  
  _plotter->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  _plotter->x_dpy = (Display *)NULL;
  _plotter->x_drawable1 = (Drawable)0;
  _plotter->x_drawable2 = (Drawable)0;  
  _plotter->x_drawable3 = (Drawable)0;
  _plotter->x_double_buffering = DBL_NONE;
  _plotter->x_fontlist = (plFontRecord *)NULL;
  _plotter->x_colorlist = (plColorRecord *)NULL;  
  _plotter->x_cmap = (Colormap)0;
  _plotter->x_cmap_type = CMAP_ORIG;
  _plotter->x_color_warning_issued = false;
  _plotter->x_paint_pixel_count = 0;

  /* initialize certain data members from device driver parameters */

  /* if this is NULL, won't be able to open Plotter */
  _plotter->x_dpy = (Display *)_get_plot_param (R___(_plotter) "XDRAWABLE_DISPLAY");

  /* we allow either or both of the drawables to be NULL, i.e. not set */
  drawable_p1 = (Drawable *)_get_plot_param (R___(_plotter) "XDRAWABLE_DRAWABLE1");
  drawable_p2 = (Drawable *)_get_plot_param (R___(_plotter) "XDRAWABLE_DRAWABLE2");
  _plotter->x_drawable1 = drawable_p1 ? *drawable_p1 : 0;
  _plotter->x_drawable2 = drawable_p2 ? *drawable_p2 : 0;

  /* allow user to specify a non-default colormap */
  x_cmap_ptr = (Colormap *)_get_plot_param (R___(_plotter) "XDRAWABLE_COLORMAP");
  if (x_cmap_ptr != NULL)
    /* user-specified colormap */
    _plotter->x_cmap = *x_cmap_ptr;
  else if (_plotter->x_dpy)
    /* have a display, so as default, use colormap of its default screen */
    {
      int screen;		/* screen number */
      Screen *screen_struct;	/* screen structure */

      screen = DefaultScreen (_plotter->x_dpy);
      screen_struct = ScreenOfDisplay (_plotter->x_dpy, screen);
      _plotter->x_cmap = DefaultColormapOfScreen (screen_struct);
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
  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

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
  if (_plotter->type == PL_X11_DRAWABLE)
    _x_maybe_get_new_colormap (_plotter);
  else if (_plotter->type == PL_X11)
    _y_maybe_get_new_colormap (_plotter);
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
  if (_plotter->type == PL_X11_DRAWABLE)
    _x_maybe_handle_x_events (_plotter);
  else if (_plotter->type == PL_X11)
    _y_maybe_handle_x_events (_plotter);
}
#endif /* not LIBPLOTTER */
