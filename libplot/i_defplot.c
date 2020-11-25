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
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _i_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _i_endpath, _g_endsubpath, _i_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _g_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _i_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _i_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _i_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _i_initialize, _i_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_falabel_hershey, _g_falabel_ps, _g_falabel_pcl, _g_falabel_stick, _g_falabel_other,
  _g_flabelwidth_hershey, _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, _g_flabelwidth_other,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _g_set_font,
  /* private low-level `sync line attributes' method */
  _g_set_attributes,
  /* private low-level `sync color' methods */
  _i_set_pen_color,
  _i_set_fill_color,
  _i_set_bg_color,
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
  _plotter->type = PL_GIF;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 1;
  _plotter->have_dash_array = 1;
  _plotter->have_solid_fill = 1;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 1;
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

  /* path and polyline-related parameters (also internal); note that we
     don't set max_unfilled_polyline_length, because it was set by the
     superclass initialization */
  _plotter->have_mixed_paths = false;
  _plotter->allowed_arc_scaling = AS_NONE;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;  
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_LIBXMI;
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
  /* parameters */
  _plotter->i_xn = _plotter->imax + 1;
  _plotter->i_yn = _plotter->jmin + 1;
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

  /* produce an interlaced GIF? */
  {
    const char *interlace_s;

    interlace_s = (const char *)_get_plot_param ( R___(_plotter) "INTERLACE" );
    if (strcasecmp (interlace_s, "yes") == 0)
      _plotter->i_interlace = true;
  }
  
  /* turn off animation? */
  {
    const char *animate_s;

    animate_s = (const char *)_get_plot_param ( R___(_plotter) "GIF_ANIMATION" );
    if (strcasecmp (animate_s, "no") == 0)
      _plotter->i_animation = false;
  }
  
  /* determine number of iterations to be used (if animating) */
  {
    const char *iteration_s;
    int num_iterations;

    iteration_s = (const char *)_get_plot_param ( R___(_plotter) "GIF_ITERATIONS" );
    if (sscanf (iteration_s, "%d", &num_iterations) > 0 
	&& num_iterations >= 0 && num_iterations <= 65535)
      _plotter->i_iterations = num_iterations;
  }

  /* determine delay after each frame, in 1/100 sec units */
  {
    const char *delay_s;
    int delay;

    delay_s = (const char *)_get_plot_param ( R___(_plotter) "GIF_DELAY" );
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
	
    bitmap_size_s = (const char *)_get_plot_param (R___(_plotter) "BITMAPSIZE");
    if (bitmap_size_s && _parse_bitmap_size (bitmap_size_s, &width, &height)
	/* insist on range of 1..65535 for GIF format */
	&& width >= 1 && height >= 1
	&& width <= 65535 && height <= 65535)
      /* override defaults above */
      {
	_plotter->imax = width - 1;
	_plotter->jmin = height - 1;
	_plotter->i_xn = width;
	_plotter->i_yn = height;
	_plotter->i_num_pixels = width * height;
      }
  }
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
  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

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
  _i_terminate ();
}
#endif
