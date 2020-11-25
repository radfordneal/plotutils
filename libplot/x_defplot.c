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
   the XDrawablePlotter struct. */
const Plotter _x_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _x_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _x_endpath,  _x_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _x_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _x_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _x_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _x_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _x_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _x_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _x_restorestate, _x_savestate, _g_space, _g_space2, _g_textangle,
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
   is invoked, _plotter points (temporarily) to the Plotter that has just
   been created. */

void
#ifdef _HAVE_PROTOS
_x_initialize (void)
#else
_x_initialize ()
#endif
{
  Colormap *x_cmap_ptr;
  Drawable *drawable_p1, *drawable_p2;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize ();
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

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->default_font_type = F_POSTSCRIPT;
  _plotter->pcl_before_ps = false;
  _plotter->have_justification = false;
  _plotter->kern_stick_fonts = false;
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal) */
  _plotter->max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;
  _plotter->have_mixed_paths = false;
  _plotter->allowed_arc_scaling = AS_AXES_PRESERVED;
  _plotter->allowed_ellarc_scaling = AS_AXES_PRESERVED;
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;  
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_type = DISP_INTEGER;
  _plotter->integer_device_coors = true; /* X != NeWS, alas */
  _plotter->imin = 0;
  _plotter->imax = 569;  
  _plotter->jmin = 569;
  _plotter->jmax = 0;  
  _plotter->display_coors.left = 0.0;
  _plotter->display_coors.right = 0.0;
  _plotter->display_coors.bottom = 0.0;
  _plotter->display_coors.top = 0.0;
  _plotter->display_coors.extra = 0.0;  
  _plotter->page_type = NULL;
  _plotter->device_units_per_inch = 0.0;
  _plotter->use_metric = false;
  _plotter->flipped_y = true;

  /* initialize data members specific to this derived class */
  _plotter->x_dpy = (Display *)NULL;
  _plotter->x_drawable1 = (Drawable)0;
  _plotter->x_drawable2 = (Drawable)0;  
  _plotter->x_drawable3 = (Drawable)0;
  _plotter->x_double_buffering = DBL_NONE;
  _plotter->x_fontlist = (Fontrecord *)NULL;
  _plotter->x_colorlist = (Colorrecord *)NULL;  
  _plotter->x_cmap = (Colormap)0;
  _plotter->x_cmap_type = CMAP_ORIG;
  _plotter->x_color_warning_issued = false;

  /* initialize certain data members from device driver parameters */

  /* if this is NULL, won't be able to open Plotter */
  _plotter->x_dpy = (Display *)_get_plot_param ("XDRAWABLE_DISPLAY");

  /* we allow either or both of the drawables to be NULL, i.e. not set */
  drawable_p1 = (Drawable *)_get_plot_param ("XDRAWABLE_DRAWABLE1");
  drawable_p2 = (Drawable *)_get_plot_param ("XDRAWABLE_DRAWABLE2");
  _plotter->x_drawable1 = drawable_p1 ? *drawable_p1 : 0;
  _plotter->x_drawable2 = drawable_p2 ? *drawable_p2 : 0;

  /* allow user to specify a non-default colormap */
  x_cmap_ptr = (Colormap *)_get_plot_param ("XDRAWABLE_COLORMAP");
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
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_x_terminate (void)
#else
_x_terminate ()
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate ();
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

XDrawablePlotter::~XDrawablePlotter ()
{
  _x_terminate ();
}
#endif
