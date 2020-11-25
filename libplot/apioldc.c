/* This file is specific to libplot, rather than libplotter.  It defines
   the `old' (i.e. non-thread-safe) C API.  The old C API consists of
   wrappers around the methods that may be applied to any Plotter object.

   The old API also contains pl_newpl/pl_selectpl/pl_deletepl, which
   construct and destroy Plotter instances, and maintain the global
   variable `_old_api_plotter'.  This is a pointer to a `currently
   selected' Plotter instance.  It is positioned by calling pl_selectpl.
   
   Because of this global variable, the old C API is not thread-safe.  In
   the new C API, which is thread-safe, each function in the API is passed
   a pointer to a Plotter as first argument.  Even in the absence of
   multithreading, this is a cleaner approach.

   By convention, _old_api_plotter is initialized to point to a Plotter
   instance that sends output in metafile format to standard output.
   Initialization takes place when the first function in the old API is
   invoked.  This convention is for compatibility with pre-GNU versions of
   libplot, which did not support pl_newpl/pl_select/pl_deletepl.

   The old C API also contains the pl_parampl function.  This function is
   held in common with the old (non-thread-safe) C++ API, in which it is
   simply called parampl.  It is defined in apioldcc.c.

   pl_parampl sets parameters in a global PlotterParams struct, a pointer
   to which is kept in the global variable
   `_old_api_global_plotter_params'.  (See its definition and
   initialization in g_defplot.c.)  This PlotterParams is used when any
   Plotter is created by pl_newpl.  The presence of this global state is
   another reason why the old C API is not thread-safe.  */

#include "sys-defines.h"
#include "extern.h"
#include "plot.h"		/* header file for C API's */

/* Sparse array of pointers to the old API's Plotter instances, and the
   array size; also a distinguished Plotter pointer, through which the old
   API will act. */
static Plotter **_old_api_plotters = NULL;
static int _old_api_plotters_len = 0;
static Plotter *_old_api_plotter = NULL;

/* initial size of _old_api_plotters[] */
#define INITIAL_PLOTTERS_LEN 4

/* default Plotter type (see list of supported types in the file devoted to
   the new C API) */
#ifndef DEFAULT_PLOTTER_TYPE
#define DEFAULT_PLOTTER_TYPE "meta"
#endif

/* forward references */
static void _api_warning ____P((const char *msg));
static void _create_and_select_default_plotter ____P((void));

/* Expand the local array of Plotters to include a single Plotter, of
   default type; also, select that Plotter.  When this is invoked, the
   array has zero size.  */
static void
#ifdef _HAVE_PROTOS
_create_and_select_default_plotter (void)
#else
_create_and_select_default_plotter ()
#endif
{
  int i;
  Plotter *default_plotter;

  /* create the default Plotter by invoking function in new API (make sure
     global PlotterParams struct, used by the old API, is set up first) */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = pl_newplparams();
  default_plotter = pl_newpl_r (DEFAULT_PLOTTER_TYPE, stdin, stdout, stderr,
				_old_api_global_plotter_params);

  /* initialize local array of Plotters */
  _old_api_plotters = (Plotter **)_plot_xmalloc (INITIAL_PLOTTERS_LEN * sizeof(Plotter *));
  for (i = 0; i < INITIAL_PLOTTERS_LEN; i++)
    _old_api_plotters[i] = (Plotter *)NULL;
  _old_api_plotters_len = INITIAL_PLOTTERS_LEN;

  /* place default Plotter in local array, and select it */
  _old_api_plotters[0] = default_plotter;
  _old_api_plotter = default_plotter;
}

/* These are the 3 user-callable functions that are specific to the old C
   binding: newpl, selectpl, deletepl. */

/* user-callable */
int 
#ifdef _HAVE_PROTOS
pl_newpl (const char *type, FILE *infile, FILE *outfile, FILE *errfile)
#else
pl_newpl (type, infile, outfile, errfile)
     const char *type;
     FILE *infile, *outfile, *errfile;
#endif
{
  Plotter *new_plotter;
  bool open_slot;
  int i, j;

  if (_old_api_plotters_len == 0)
    /* initialize local array of Plotters, and install default Plotter as
       Plotter #0 */
    _create_and_select_default_plotter ();

  /* create the default Plotter by invoking function in new API (make sure
     global PlotterParams struct, used by the old API, is set up first) */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = pl_newplparams();
  new_plotter = pl_newpl_r (type, infile, outfile, errfile,
			    _old_api_global_plotter_params);

  /* ensure local array has an open slot (slot i) */
  open_slot = false;
  for (i = 0; i < _old_api_plotters_len; i++)
    if (_old_api_plotters[i] == NULL)
      {
	open_slot = true;
	break;
      }

  if (!open_slot)
    /* expand array, clearing upper half */
    {
      i = _old_api_plotters_len;
      _old_api_plotters = 
	(Plotter **)_plot_xrealloc (_old_api_plotters, 
				    2 * _old_api_plotters_len * sizeof (Plotter *));
      for (j = _old_api_plotters_len; j < 2 * _old_api_plotters_len; j++)
	_old_api_plotters[j] = (Plotter *)NULL;
      _old_api_plotters_len *= 2;
    }
  
  /* place newly created Plotter in open slot */
  _old_api_plotters[i] = new_plotter;

  /* return index of newly created Plotter */
  return i;
}

/* user-callable, alters selected Plotter and returns index of the one that
   was previously selected */
int
#ifdef _HAVE_PROTOS
pl_selectpl (int handle)
#else
pl_selectpl (handle)
     int handle;
#endif
{
  int i;

  if (handle < 0 || handle >= _old_api_plotters_len 
      || _old_api_plotters[handle] == NULL)
    {
      _api_warning ("ignoring request to select a nonexistent plotter");
      return -1;
    }

  /* determine index of currently selected Plotter in _old_api_plotters[] */
  for (i = 0; i < _old_api_plotters_len; i++)
    if (_old_api_plotters[i] == _old_api_plotter)
      break;

  /* select specified Plotter: alter value of the _old_api_plotter pointer */
  _old_api_plotter = _old_api_plotters[handle];

  /* return index of previously selected Plotter */
  return i;
}

/* user-callable */
int
#ifdef _HAVE_PROTOS
pl_deletepl (int handle)
#else
pl_deletepl (handle)
     int handle;
#endif
{
  if (handle < 0 || handle >= _old_api_plotters_len 
      || _old_api_plotters[handle] == NULL)
    {
      _api_warning ("ignoring request to delete a nonexistent plotter");
      return -1;
    }

  if (_old_api_plotters[handle] == _old_api_plotter)
    {
      _api_warning ("ignoring request to delete currently selected plotter");
      return -1;
    }

  /* delete Plotter by invoking function in new API */
  pl_deletepl_r (_old_api_plotters[handle]);

  /* remove now-invalid pointer from local array */
  _old_api_plotters[handle] = NULL;

  return 0;
}


/* function used in this file to print warning messages */
static void
#ifdef _HAVE_PROTOS
_api_warning (const char *msg)
#else
_api_warning (msg)
     const char *msg;
#endif
{
  if (libplot_warning_handler != NULL)
    (*libplot_warning_handler)(msg);
  else
    fprintf (stderr, "libplot: %s\n", msg);
}


/* The following are the C wrappers around the public functions in the
   Plotter class.  Together with the three functions above (pl_newpl,
   pl_selectpl, pl_deletepl), and pl_parampl, they make up the old
   (non-thread-safe) libplot C API.

   Each binding tests whether _old_api_plotter is non-NULL, which determines
   whether the array of Plotter instances has been initialized.  That is
   because it makes no sense to call these functions before the
   _old_api_plotter pointer points to a Plotter object.

   In fact, of the below functions, it really only makes sense to call
   openpl, havecap, or outfile [deprecated] before the Plotter array is
   initialized.  Calling any other of the below functions before the
   Plotter array is initialized will generate an error message because even
   though the call to _create_and_select_default_plotter will initialize
   the Plotter array and select a default Plotter instance, the Plotter
   will not be open.  No operation in the Plotter class, with the exception
   of the just-mentioned ones, may be invoked unless the Plotter that is
   being acted on is open. */

int 
#ifdef _HAVE_PROTOS
pl_alabel (int x_justify, int y_justify, const char *s)
#else
pl_alabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->alabel (_old_api_plotter, x_justify, y_justify, s);
}

int
#ifdef _HAVE_PROTOS
pl_arc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->arc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_arcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_arcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->arcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier2 (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_bezier2 (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->bezier2 (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier2rel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_bezier2rel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->bezier2rel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier3 (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
pl_bezier3 (x0, y0, x1, y1, x2, y2, x3, y3)
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->bezier3 (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_bezier3rel (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
pl_bezier3rel (x0, y0, x1, y1, x2, y2, x3, y3)
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->bezier3rel (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_bgcolor (int red, int green, int blue)
#else
pl_bgcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->bgcolor (_old_api_plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_bgcolorname (const char *s)
#else
pl_bgcolorname (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->bgcolorname (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_box (int x0, int y0, int x1, int y1)
#else
pl_box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->box (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_boxrel (int x0, int y0, int x1, int y1)
#else
pl_boxrel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->boxrel (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_capmod (const char *s)
#else
pl_capmod (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->capmod (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_circle (int x, int y, int r)
#else
pl_circle (x, y, r)
     int x, y, r;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->circle (_old_api_plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_circlerel (int x, int y, int r)
#else
pl_circlerel (x, y, r)
     int x, y, r;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->circlerel (_old_api_plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_closepl (void)
#else
pl_closepl ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->closepl (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_color (int red, int green, int blue)
#else
pl_color (red, green, blue)
     int red, green, blue;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->color (_old_api_plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_colorname (const char *s)
#else
pl_colorname (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->colorname (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_cont (int x, int y)
#else
pl_cont (x, y)
     int x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->cont (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_contrel (int x, int y)
#else
pl_contrel (x, y)
     int x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->contrel (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_ellarc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_ellarc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ellarc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_ellarcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_ellarcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ellarcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_ellipse (int x, int y, int rx, int ry, int angle)
#else
pl_ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ellipse (_old_api_plotter, x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_ellipserel (int x, int y, int rx, int ry, int angle)
#else
pl_ellipserel (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ellipserel (_old_api_plotter, x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_endpath (void)
#else
pl_endpath ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->endpath (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_endsubpath (void)
#else
pl_endsubpath ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->endsubpath (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_erase (void)
#else
pl_erase ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->erase (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->farc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_farcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_farcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->farcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier2 (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fbezier2 (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fbezier2 (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier2rel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fbezier2rel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fbezier2rel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier3 (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
pl_fbezier3 (x0, y0, x1, y1, x2, y2, x3, y3)
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fbezier3 (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier3rel (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
pl_fbezier3rel (x0, y0, x1, y1, x2, y2, x3, y3)
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fbezier3rel (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_fbox (double x0, double y0, double x1, double y1)
#else
pl_fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fbox (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fboxrel (double x0, double y0, double x1, double y1)
#else
pl_fboxrel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fboxrel (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fcircle (double x, double y, double r)
#else
pl_fcircle (x, y, r)
     double x, y, r;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fcircle (_old_api_plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_fcirclerel (double x, double y, double r)
#else
pl_fcirclerel (x, y, r)
     double x, y, r;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fcirclerel (_old_api_plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_fconcat (double m0, double m1, double m2, double m3, double m4, double m5)
#else
pl_fconcat (m0, m1, m2, m3, m4, m5)
     double m0, m1, m2, m3, m4, m5;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fconcat (_old_api_plotter, m0, m1, m2, m3, m4, m5);
}

int
#ifdef _HAVE_PROTOS
pl_fcont (double x, double y)
#else
pl_fcont (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fcont (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fcontrel (double x, double y)
#else
pl_fcontrel (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fcontrel (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fellarc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fellarc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fellarc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fellarcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fellarcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fellarcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fellipse (double x, double y, double rx, double ry, double angle)
#else
pl_fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fellipse (_old_api_plotter, x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_fellipserel (double x, double y, double rx, double ry, double angle)
#else
pl_fellipserel (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fellipserel (_old_api_plotter, x, y, rx, ry, angle);
}

double
#ifdef _HAVE_PROTOS
pl_ffontname (const char *s)
#else
pl_ffontname (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ffontname (_old_api_plotter, s);
}

double
#ifdef _HAVE_PROTOS
pl_ffontsize (double size)
#else
pl_ffontsize (size)
     double size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ffontsize (_old_api_plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_fillcolor (int red, int green, int blue)
#else
pl_fillcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fillcolor (_old_api_plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_fillcolorname (const char *s)
#else
pl_fillcolorname (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fillcolorname (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_fillmod (const char *s)
#else
pl_fillmod (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fillmod (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_filltype (int level)
#else
pl_filltype (level)
     int level;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->filltype (_old_api_plotter, level);
}

double
#ifdef _HAVE_PROTOS
pl_flabelwidth (const char *s)
#else
pl_flabelwidth (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->flabelwidth (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_fline (double x0, double y0, double x1, double y1)
#else
pl_fline (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fline (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_flinedash (int n, const double *dashes, double offset)
#else
pl_flinedash (n, dashes, offset)
     int n;
     const double *dashes;
     double offset;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->flinedash (_old_api_plotter, n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
pl_flinerel (double x0, double y0, double x1, double y1)
#else
pl_flinerel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->flinerel (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_flinewidth (double size)
#else
pl_flinewidth (size)
     double size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->flinewidth (_old_api_plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_flushpl (void)
#else
pl_flushpl ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->flushpl (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_fmarker (double x, double y, int type, double size)
#else
pl_fmarker (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fmarker (_old_api_plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_fmarkerrel (double x, double y, int type, double size)
#else
pl_fmarkerrel (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fmarkerrel (_old_api_plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_fmiterlimit (double limit)
#else
pl_fmiterlimit (limit)
     double limit;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fmiterlimit (_old_api_plotter, limit);
}

int
#ifdef _HAVE_PROTOS
pl_fmove (double x, double y)
#else
pl_fmove (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fmove (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fmoverel (double x, double y)
#else
pl_fmoverel (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fmoverel (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fontname (const char *s)
#else
pl_fontname (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fontname (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_fontsize (int size)
#else
pl_fontsize (size)
     int size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fontsize (_old_api_plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_fpoint (double x, double y)
#else
pl_fpoint (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fpoint (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fpointrel (double x, double y)
#else
pl_fpointrel (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fpointrel (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_frotate (double theta)
#else
pl_frotate (theta)
     double theta;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->frotate (_old_api_plotter, theta);
}

int
#ifdef _HAVE_PROTOS
pl_fscale (double x, double y)
#else
pl_fscale (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fscale (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fspace (double x0, double y0, double x1, double y1)
#else
pl_fspace (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fspace (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fspace2 (double x0, double y0, double x1, double y1, double x2, double y2)
#else
pl_fspace2 (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->fspace2 (_old_api_plotter, x0, y0, x1, y1, x2, y2);
}

double
#ifdef _HAVE_PROTOS
pl_ftextangle (double angle)
#else
pl_ftextangle (angle)
     double angle;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ftextangle (_old_api_plotter, angle);
}

int
#ifdef _HAVE_PROTOS
pl_ftranslate (double x, double y)
#else
pl_ftranslate (x, y)
     double x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->ftranslate (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_havecap (const char *s)
#else
pl_havecap (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->havecap (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_joinmod (const char *s)
#else
pl_joinmod (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->joinmod (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_label (const char *s)
#else
pl_label (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->label (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_labelwidth (const char *s)
#else
pl_labelwidth (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->labelwidth (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_line (int x0, int y0, int x1, int y1)
#else
pl_line (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->line (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_linerel (int x0, int y0, int x1, int y1)
#else
pl_linerel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->linerel (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_linewidth (int size)
#else
pl_linewidth (size)
     int size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->linewidth (_old_api_plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_linedash (int n, const int *dashes, int offset)
#else
pl_linedash (n, dashes, offset)
     int n;
     const int *dashes;
     int offset;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->linedash (_old_api_plotter, n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
pl_linemod (const char *s)
#else
pl_linemod (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->linemod (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_marker (int x, int y, int type, int size)
#else
pl_marker (x, y, type, size)
     int x, y, type, size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->marker (_old_api_plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_markerrel (int x, int y, int type, int size)
#else
pl_markerrel (x, y, type, size)
     int x, y, type, size;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->markerrel (_old_api_plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_move (int x, int y)
#else
pl_move (x, y)
     int x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->move (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_moverel (int x, int y)
#else
pl_moverel (x, y)
     int x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->moverel (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_openpl (void)
#else
pl_openpl ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->openpl (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_orientation (int direction)
#else
pl_orientation (direction)
     int direction;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->orientation (_old_api_plotter, direction);
}

FILE *
#ifdef _HAVE_PROTOS
pl_outfile (FILE *outfile)
#else
pl_outfile (outfile)
     FILE *outfile;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->outfile (_old_api_plotter, outfile);
}

int
#ifdef _HAVE_PROTOS
pl_pencolor (int red, int green, int blue)
#else
pl_pencolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->pencolor (_old_api_plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_pencolorname (const char *s)
#else
pl_pencolorname (s)
     const char *s;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->pencolorname (_old_api_plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_pentype (int level)
#else
pl_pentype (level)
     int level;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->pentype (_old_api_plotter, level);
}

int
#ifdef _HAVE_PROTOS
pl_point (int x, int y)
#else
pl_point (x, y)
     int x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->point (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_pointrel (int x, int y)
#else
pl_pointrel (x, y)
     int x, y;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->pointrel (_old_api_plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_restorestate (void)
#else
pl_restorestate ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->restorestate (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_savestate (void)
#else
pl_savestate ()
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->savestate (_old_api_plotter);
}

int
#ifdef _HAVE_PROTOS
pl_space (int x0, int y0, int x1, int y1)
#else
pl_space (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->space (_old_api_plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_space2 (int x0, int y0, int x1, int y1, int x2, int y2)
#else
pl_space2 (x0, y0, x1, y1, x2, y2)
     int x0, y0, x1, y1, x2, y2;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->space2 (_old_api_plotter, x0, y0, x1, y1, x2, y2);
}

int
#ifdef _HAVE_PROTOS
pl_textangle (int angle)
#else
pl_textangle (angle)
     int angle;
#endif
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _old_api_plotter->textangle (_old_api_plotter, angle);
}

/* END OF WRAPPERS */
