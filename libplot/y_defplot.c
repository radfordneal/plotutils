/* This file defines the initialization for any XPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API.

   This is identical to the initialization for an XDrawablePlotter, except
   that it has _y_openpl, _y_erase, _y_closepl instead of _x_openpl,
   _x_erase, _x_closepl. */

#include "sys-defines.h"
#include <signal.h>		/* for kill() */
#include "extern.h"

/* Sparse array of pointers to XPlotter instances, and its size.  Should be
   initialized to a NULL pointer, and 0, respectively.  

   In libplotter these are not global variables: in extern.h, they are
   #define'd to be static data members of the XPlotter class.

   Accessed by _y_initialize() and _y_terminate() in this file, by
   _y_maybe_handle_x_events() in y_openpl.c, and by _y_closepl() in
   y_closepl.c. */

XPlotter **_xplotters = NULL;
int _xplotters_len = 0;

/* initial size for "_xplotters", the sparse array of XPlotter instances */
#define INITIAL_XPLOTTERS_LEN 4

/* Mutex for locking _xplotters[] and _xplotters_len. */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
pthread_mutex_t _xplotters_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   an XPlotter struct. */
const Plotter _y_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _y_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _x_endpath,  _g_endsubpath, _y_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _g_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _x_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _x_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _x_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _x_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _y_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _x_restorestate, _x_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _y_initialize, _y_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
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
_y_initialize (S___(Plotter *_plotter))
#else
_y_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  bool open_slot = false;
  int i, j;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _x_initialize (S___(_plotter));
#endif

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the global variables _xplotters[] and _xplotters_len */
  pthread_mutex_lock (&_xplotters_mutex);
#endif
#endif

  /* If this is the first XPlotter to be created, initialize Xt library.
     At least in X11R6, it's OK to initialize it more than once, but we're
     careful. */
  if (_xplotters_len == 0)
    {
      /* first initialize Xlib and Xt thread support if any */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
#ifdef X_THREAD_SUPPORT
      XInitThreads ();
      XtToolkitThreadInitialize ();
#endif
#endif
#endif
      /* initialize Xt library itself */
      XtToolkitInitialize ();
    }

  /* ensure XPlotter array is set up */
  if (_xplotters_len == 0)
    {
      _xplotters = (XPlotter **)_plot_xmalloc (INITIAL_XPLOTTERS_LEN * sizeof(XPlotter *));
      for (i = 0; i < INITIAL_XPLOTTERS_LEN; i++)
	_xplotters[i] = (XPlotter *)NULL;
      _xplotters_len = INITIAL_XPLOTTERS_LEN;
    }
      
  /* be sure there is an open slot (slot i) */
  for (i = 0; i < _xplotters_len; i++)
    if (_xplotters[i] == NULL)
      {
	open_slot = true;
	break;
      }
  if (!open_slot)
    /* expand array, clearing upper half */
    {
      i = _xplotters_len;
      _xplotters = 
	(XPlotter **)_plot_xrealloc (_xplotters, 
				    2 * _xplotters_len * sizeof (XPlotter *));
      for (j = _xplotters_len; j < 2 * _xplotters_len; j++)
	_xplotters[j] = (XPlotter *)NULL;
      _xplotters_len *= 2;
    }
  
  /* place just-created Plotter in open slot */
  _xplotters[i] = _plotter;

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the global variables _xplotters[] and _xplotters_len */
  pthread_mutex_unlock (&_xplotters_mutex);
#endif
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->type = PL_X11;
#endif

  /* initialize data members specific to this derived class */
  _plotter->y_app_con = (XtAppContext)NULL;
  _plotter->y_toplevel = (Widget)NULL;
  _plotter->y_canvas = (Widget)NULL;
  _plotter->y_drawable4 = (Drawable)0;
  _plotter->y_auto_flush = true;
  _plotter->y_vanish_on_delete = false;
  _plotter->y_pids = (pid_t *)NULL;
  _plotter->y_num_pids = 0;
  _plotter->y_event_handler_count = 0;

  /* initialize certain data members from device driver parameters */

  /* determine whether to do an XFlush() after each drawing operation */
  {
    const char *vanish_s;

    vanish_s = (const char *)_get_plot_param (R___(_plotter)
					      "X_AUTO_FLUSH");
    if (strcasecmp (vanish_s, "no") == 0)
      _plotter->y_auto_flush = false;
    else
      _plotter->y_auto_flush = true;
  }

  /* determine whether windows vanish on Plotter deletion */
  {
    const char *vanish_s;

    vanish_s = (const char *)_get_plot_param (R___(_plotter)
					      "VANISH_ON_DELETE");
    if (strcasecmp (vanish_s, "yes") == 0)
      _plotter->y_vanish_on_delete = true;
    else
      _plotter->y_vanish_on_delete = false;
  }
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
#ifdef _HAVE_PROTOS
_y_terminate (S___(Plotter *_plotter))
#else
_y_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i, j;

  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

  /* kill forked-off processes that are maintaining XPlotter's popped-up
     windows, provided that the VANISH_ON_DELETE parameter was set to "yes"
     at creation time */
  if (_plotter->y_vanish_on_delete)
    {
      for (j = 0; j < _plotter->y_num_pids; j++)
	kill (_plotter->y_pids[j], SIGKILL);
      if (_plotter->y_num_pids > 0)
	{
	  free (_plotter->y_pids);
	  _plotter->y_pids = (pid_t *)NULL;
	}
    }

  /* remove XPlotter from sparse XPlotter array */

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the global variables _xplotters[] and _xplotters_len */
  pthread_mutex_lock (&_xplotters_mutex);
#endif
#endif
  for (i = 0; i < _xplotters_len; i++)
    if (_xplotters[i] == _plotter)
      {
	_xplotters[i] = (XPlotter *)NULL;
	break;
      }
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the global variables _xplotters[] and _xplotters_len */
  pthread_mutex_unlock (&_xplotters_mutex);
#endif
#endif

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _x_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
XPlotter::XPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:XDrawablePlotter (infile, outfile, errfile)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (FILE *outfile)
	:XDrawablePlotter (outfile)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (istream& in, ostream& out, ostream& err)
	: XDrawablePlotter (in, out, err)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (ostream& out)
	: XDrawablePlotter (out)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter ()
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:XDrawablePlotter (infile, outfile, errfile, parameters)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (FILE *outfile, PlotterParams &parameters)
	:XDrawablePlotter (outfile, parameters)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: XDrawablePlotter (in, out, err, parameters)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (ostream& out, PlotterParams &parameters)
	: XDrawablePlotter (out, parameters)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::XPlotter (PlotterParams &parameters)
	: XDrawablePlotter (parameters)
{
  _y_initialize ();

  /* add to _xplotters sparse array */
}

XPlotter::~XPlotter ()
{
  _y_terminate ();

  /* remove from _xplotters sparse array */
}
#endif
