/* This file defines the initialization for any XPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

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
  /* initialization (after creation) and termination (before deletion) */
  _y_initialize, _y_terminate,
  /* page manipulation */
  _y_begin_page, _y_erase_page, _y_end_page,
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
  _plotter->data->type = PL_X11;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES_TO_NON_STREAM;

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

    vanish_s = (const char *)_get_plot_param (_plotter->data,
					      "X_AUTO_FLUSH");
    if (strcasecmp (vanish_s, "no") == 0)
      _plotter->y_auto_flush = false;
    else
      _plotter->y_auto_flush = true;
  }

  /* determine whether windows vanish on Plotter deletion */
  {
    const char *vanish_s;

    vanish_s = (const char *)_get_plot_param (_plotter->data,
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
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _y_terminate ();

  /* remove from _xplotters sparse array */
}
#endif
