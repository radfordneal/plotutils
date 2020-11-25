/* This file defines the initialization for any generic Plotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

/* additional include files for DJGPP */
#ifdef MSDOS
#include <io.h>			/* for setmode() */
#include <fcntl.h>		/* for O_BINARY */
#include <unistd.h>		/* for isatty() */
#endif

/* global library variables (user-settable error handlers) */
#ifndef LIBPLOTTER
#ifdef _HAVE_PROTOS
int (*libplot_warning_handler)(const char *) = NULL;
int (*libplot_error_handler)(const char *) = NULL;
#else
int (*libplot_warning_handler)() = NULL;
int (*libplot_error_handler)() = NULL;
#endif
#else  /* LIBPLOTTER */
int (*libplotter_warning_handler)(const char *) = NULL;
int (*libplotter_error_handler)(const char *) = NULL;
#endif /* LIBPLOTTER */

/* The following variables (_plotters, _plotters_len) are global variables
   in libplot, but static data members of the Plotter class in libplotter.
   That's arranged by #ifdefs's in extern.h. */

/* Sparse array of pointers to Plotter instances, and its size.
   Initialized to a NULL pointer, and 0, respectively.  

   Accessed by _g_initialize() and _g_terminate() in this file, and by
   g_flush() in g_flushpl.c. */

Plotter **_plotters = NULL;
int _plotters_len = 0;

/* initial size for _plotters[], the sparse array of pointers to Plotter
   instances */
#define INITIAL_PLOTTERS_LEN 4

/* Mutex for locking _plotters[] and _plotters_len.  A global variable in
   both libplot and libplotter. */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
pthread_mutex_t _plotters_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

/* Pointer to distinguished (global) PlotterParams object, used by the old
   (non-thread-safe) bindings.  In libplotter, this is a static data member
   of the Plotter class.  That's arranged by an #ifdef in extern.h. */
PlotterParams *_old_api_global_plotter_params = NULL;

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a Plotter struct. */
const Plotter _g_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _g_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _g_endpath, _g_endsubpath, _g_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _g_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _g_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _g_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _g_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _g_initialize, _g_terminate,
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
  _g_set_pen_color,
  _g_set_fill_color,
  _g_set_bg_color,
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

/* The private `initialize' method, invoked when a Plotter is created.  It
   does such things as initializing capability flags from the values of
   class variables, allocating storage, etc.  In the C binding, when this
   is invoked _plotter points to the Plotter that has just been created.
   In the C++ binding, this is a function member of the Plotter class, and
   _plotter is an alias for `this'. */

void
#ifdef _HAVE_PROTOS
_g_initialize (S___(Plotter *_plotter))
#else
_g_initialize (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  bool open_slot = false;
  int i, j;

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the global variables _plotters[] and _plotters_len */
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif

  /* ensure plotter instance array is set up */
  if (_plotters_len == 0)
    {
      _plotters = (Plotter **)_plot_xmalloc (INITIAL_PLOTTERS_LEN * sizeof(Plotter *));
      for (i = 0; i < INITIAL_PLOTTERS_LEN; i++)
	_plotters[i] = (Plotter *)NULL;
      _plotters_len = INITIAL_PLOTTERS_LEN;
    }

  /* be sure there is an open slot (slot i) */
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i] == NULL)
      {
	open_slot = true;
	break;
      }

  if (!open_slot)
    /* expand array, clearing upper half */
    {
      i = _plotters_len;
      _plotters = 
	(Plotter **)_plot_xrealloc (_plotters, 
				    2 * _plotters_len * sizeof (Plotter *));
      for (j = _plotters_len; j < 2 * _plotters_len; j++)
	_plotters[j] = (Plotter *)NULL;
      _plotters_len *= 2;
    }
  
  /* place just-created Plotter in open slot */
  _plotters[i] = _plotter;

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the global variables _plotters[] and _plotters_len */
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif

  /* Initialize all data members (except in/out/err streams and device
     driver parameters). */

#ifndef LIBPLOTTER
  /* tag field, will differ in derived classes */
  _plotter->type = PL_GENERIC;
#endif

  /* I/O, will not differ in derived classes */
  _plotter->page = (plOutbuf *)NULL;
  _plotter->first_page = (plOutbuf *)NULL;  

  /* basic data members, will not differ in derived classes */
  _plotter->open = false;
  _plotter->opened = false;
  _plotter->page_number = 0;
  _plotter->space_invoked = false;
  _plotter->frame_number = 0;

  /* drawing state stack (initially empty; same in derived classes) */
  _plotter->drawstate = (plDrawState *)NULL;  

  /* warnings, will not differ in derived classes */
  _plotter->font_warning_issued = false;
  _plotter->pen_color_warning_issued = false;
  _plotter->fill_color_warning_issued = false;
  _plotter->bg_color_warning_issued = false;
  
  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 1;
  _plotter->have_dash_array = 1;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 1;
  _plotter->have_hershey_fonts = 1;
  _plotter->have_ps_fonts = 1;
  _plotter->have_pcl_fonts = 1;
  _plotter->have_stick_fonts = 1;
  _plotter->have_extra_stick_fonts = 0;	/* specific to HP-GL version "1.5" */

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->default_font_type = F_HERSHEY;
  _plotter->pcl_before_ps = false;
  _plotter->have_horizontal_justification = false;
  _plotter->have_vertical_justification = false;
  _plotter->kern_stick_fonts = false;
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal) */
  _plotter->max_unfilled_polyline_length = MAX_UNFILLED_POLYLINE_LENGTH;
  _plotter->have_mixed_paths = false;
  _plotter->allowed_arc_scaling = AS_NONE;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;  
  _plotter->flush_long_polylines = true;
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

  /* cached color name -> RGB correspondences */
  _plotter->cached_colors = NULL;

  /* initialize certain data members from values of relevant device
     driver parameters */

  /* set maximum polyline length (relevant to most Plotters, esp. those
     that do not do real time output) */
  {
    const char *length_s;
    int local_length;
	
    length_s = (const char *)_get_plot_param (R___(_plotter) 
					      "MAX_LINE_LENGTH");

    if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
      {
	length_s = (const char *)_get_default_plot_param ("MAX_LINE_LENGTH");
	sscanf (length_s, "%d", &local_length);
      }
    _plotter->max_unfilled_polyline_length = local_length;
  }
      
  /* Ensure widths of labels rendered in the Stick fonts are correctly
     computed.  This is a kludge (in pre-HP-GL/2, Stick fonts were kerned;
     see g_alabel.c.)  */
  {
    const char *version_s;

    version_s = (const char *)_get_plot_param (R___(_plotter) 
					       "HPGL_VERSION");
    if (strcmp (version_s, "2") == 0) /* modern HP-GL/2 (default) */
      _plotter->kern_stick_fonts = false;
    else if (strcmp (version_s, "1.5") == 0) /* HP7550A */
      _plotter->kern_stick_fonts = true;
    else if (strcmp (version_s, "1") == 0) /* generic HP-GL */
      _plotter->kern_stick_fonts = true; /* meaningless (no stick fonts) */
  }

#ifdef MSDOS
  /* A DJGPP enhancement (thanks, Michel de Ruiter).  If the output fp is
     standard output, and standard output has been redirected to a file,
     then set the output mode to binary.  This turns off character mapping,
     which is a necessity when writing binary output formats such as GIF or
     Tektronix.  If the output fp is a (FILE *) other than standard output,
     then whoever opened it is responsible for having opened it with mode
     "wb" instead of "wt" or "w". */
  if (_plotter->outfp
      && (_plotter->outfp == stdout)
      && O_BINARY
      && !isatty (fileno (stdout)))
    {
      fflush (stdout);
      setmode (fileno (stdout), O_BINARY);
    }
#endif

}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  It may do such things as write
   to an output stream from internal storage, deallocate storage, etc. */

void
#ifdef _HAVE_PROTOS
_g_terminate (S___(Plotter *_plotter))
#else
_g_terminate (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  int i;
  plCachedColorNameInfo *colorptr;

  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

  /* free instance-specific copies of class parameters */
  _free_params_in_plotter (S___(_plotter));

  /* free color name cache */
  colorptr = _plotter->cached_colors;
  while (colorptr != NULL)
    {
      plCachedColorNameInfo *next_colorptr;
      
      next_colorptr = colorptr->next;
      free (colorptr);
      colorptr = next_colorptr;
    }

  /* remove Plotter from sparse Plotter array */

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the global variables _plotters[] and _plotters_len */
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i] == _plotter)
      {
	_plotters[i] = (Plotter *)NULL;
	break;
      }
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the global variables _plotters[] and _plotters_len */
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif
}

/* These low-level methods, in a generic (base class) Plotter, do nothing
   and are never invoked.  Any derived class that wants to use them for
   something should override them. */

void
#ifdef _HAVE_PROTOS
_g_set_font (S___(Plotter *_plotter))
#else
_g_set_font (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}

void
#ifdef _HAVE_PROTOS
_g_set_attributes (S___(Plotter *_plotter))
#else
_g_set_attributes (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}

void
#ifdef _HAVE_PROTOS
_g_set_pen_color (S___(Plotter *_plotter))
#else
_g_set_pen_color (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}

void
#ifdef _HAVE_PROTOS
_g_set_fill_color (S___(Plotter *_plotter))
#else
_g_set_fill_color (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}

void
#ifdef _HAVE_PROTOS
_g_set_bg_color (S___(Plotter *_plotter))
#else
_g_set_bg_color (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}

void
#ifdef _HAVE_PROTOS
_g_set_position (S___(Plotter *_plotter))
#else
_g_set_position (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return;
}

#ifdef LIBPLOTTER

/* OLD API; not thread-safe (these ctors take values for Plotter
   parameters from the global PlotterParams struct) */

Plotter::Plotter (FILE *infile, FILE *outfile, FILE *errfile)
{
  _plotter->infp = infile;
  _plotter->outfp = outfile;  
  _plotter->errfp = errfile;
  _plotter->instream = NULL;
  _plotter->outstream = NULL;  
  _plotter->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _copy_params_to_plotter (_old_api_global_plotter_params);

  _g_initialize ();
}

Plotter::Plotter (FILE *outfile)
{
  _plotter->infp = NULL;
  _plotter->outfp = outfile;  
  _plotter->errfp = NULL;
  _plotter->instream = NULL;
  _plotter->outstream = NULL;  
  _plotter->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _copy_params_to_plotter (_old_api_global_plotter_params);

  _g_initialize ();
}

Plotter::Plotter (istream& in, ostream& out, ostream& err)
{
  _plotter->infp = NULL;
  _plotter->outfp = NULL;  
  _plotter->errfp = NULL;
  if (in.rdbuf())
    _plotter->instream = &in;
  else
    _plotter->instream = NULL;
  if (out.rdbuf())
    _plotter->outstream = &out;  
  else
    _plotter->outstream = NULL;
  if (err.rdbuf())
    _plotter->errstream = &err;
  else
    _plotter->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _copy_params_to_plotter (_old_api_global_plotter_params);

  _g_initialize ();
}

Plotter::Plotter (ostream& out)
{
  _plotter->infp = NULL;
  _plotter->outfp = NULL;
  _plotter->errfp = NULL;
  _plotter->instream = NULL;
  if (out.rdbuf())
    _plotter->outstream = &out;
  else
    _plotter->outstream = NULL;
  _plotter->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _copy_params_to_plotter (_old_api_global_plotter_params);

  _g_initialize ();
}

Plotter::Plotter ()
{
  _plotter->infp = NULL;
  _plotter->outfp = NULL;
  _plotter->errfp = NULL;
  _plotter->instream = NULL;
  _plotter->outstream = NULL;  
  _plotter->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _copy_params_to_plotter (_old_api_global_plotter_params);

  _g_initialize ();
}

/* NEW API; thread-safe (since user can specify a local PlotterParams
   struct from which parameters should be taken) */

Plotter::Plotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &plotter_params)
{
  _plotter->infp = infile;
  _plotter->outfp = outfile;  
  _plotter->errfp = errfile;
  _plotter->instream = NULL;
  _plotter->outstream = NULL;  
  _plotter->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _copy_params_to_plotter (&plotter_params);

  _g_initialize ();
}

Plotter::Plotter (FILE *outfile, PlotterParams &plotter_params)
{
  _plotter->infp = NULL;
  _plotter->outfp = outfile;  
  _plotter->errfp = NULL;
  _plotter->instream = NULL;
  _plotter->outstream = NULL;  
  _plotter->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _copy_params_to_plotter (&plotter_params);

  _g_initialize ();
}

Plotter::Plotter (istream& in, ostream& out, ostream& err, PlotterParams &plotter_params)
{
  _plotter->infp = NULL;
  _plotter->outfp = NULL;  
  _plotter->errfp = NULL;
  if (in.rdbuf())
    _plotter->instream = &in;
  else
    _plotter->instream = NULL;
  if (out.rdbuf())
    _plotter->outstream = &out;  
  else
    _plotter->outstream = NULL;
  if (err.rdbuf())
    _plotter->errstream = &err;
  else
    _plotter->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _copy_params_to_plotter (&plotter_params);

  _g_initialize ();
}

Plotter::Plotter (ostream& out, PlotterParams &plotter_params)
{
  _plotter->infp = NULL;
  _plotter->outfp = NULL;
  _plotter->errfp = NULL;
  _plotter->instream = NULL;
  if (out.rdbuf())
    _plotter->outstream = &out;
  else
    _plotter->outstream = NULL;
  _plotter->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _copy_params_to_plotter (&plotter_params);

  _g_initialize ();
}

Plotter::Plotter (PlotterParams &plotter_params)
{
  _plotter->infp = NULL;
  _plotter->outfp = NULL;
  _plotter->errfp = NULL;
  _plotter->instream = NULL;
  _plotter->outstream = NULL;  
  _plotter->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _copy_params_to_plotter (&plotter_params);

  _g_initialize ();
}

Plotter::~Plotter ()
{
  _g_terminate ();
}
#endif
