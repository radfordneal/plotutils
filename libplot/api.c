/* This file defines the C binding for GNU libplot.  The C API consists of
   81 functions that are wrappers around the 81 methods that may be applied
   to any Plotter object, plus four additional functions (newpl, selectpl,
   deletepl, parampl) that are specific to the C binding.

   These four functions maintain an array of Plotter objects, and construct
   and destroy them, as requested.  The _plotter pointer, which is
   referenced in the 81 basic functions, is a pointer to one of the objects
   in the array of constructed plotters.  It must be selected by calling
   selectpl.

   By default, _plotter points to a Plotter object that sends output in
   metafile format to standard output.  That is for compatibility with
   pre-GNU versions of libplot.

   Note that the parampl function is used to set device driver parameters,
   i.e., Plotter class variables.  The parameter values in effect at the
   time a Plotter object is created are copied into the object. */

#include "sys-defines.h"
#include <signal.h>		/* for kill() */
#include "plot.h"
#include "extern.h"
#include "g_params.h"

/* global library variables (user-settable error handlers) */
#ifdef _HAVE_PROTOS
int (*libplot_warning_handler)(const char *) = NULL;
int (*libplot_error_handler)(const char *) = NULL;
#else
int (*libplot_warning_handler)() = NULL;
int (*libplot_error_handler)() = NULL;
#endif

/* known plotter types, indexed into by a short mnemonic string:
   "meta"=metafile, "tek"=Tektronix, "hpgl"=HP-GL/2, "fig"=xfig, "ps"=PS,
   "x"=X11.  The `default plotter' structures are simply copied into a new
   Plotter struct at the time it is constructed. */

typedef struct 
{
  const char *name;
  Plotter *init;
}
plotter_initialization;

/* default plotter produces metafile output */
#ifndef DEFAULT_PLOTTER_TYPE
#define DEFAULT_PLOTTER_TYPE "meta"
#endif

static plotter_initialization _plotter_initializations[] = 
{
  {"meta", &_meta_default_plotter},
  {"tek", &_tek_default_plotter},
  {"hpgl", &_hpgl_default_plotter},
  {"fig", &_fig_default_plotter},
  {"ps", &_ps_default_plotter},
#ifndef X_DISPLAY_MISSING
  {"X", &_X_default_plotter},    
  {"Xdrawable", &_X_drawable_default_plotter},    
#endif /* X_DISPLAY_MISSING */
  {(const char *)NULL, (Plotter *)NULL},    
};

/* array of plotter objects, needed by C binding */
#define INITIAL_NUM_PLOTTERS 4
static int _num_plotters = 0;	/* size of array */
static Plotter **_plotters;

/* distinguished plotter object, for functions in the C API to act on */
Plotter *_plotter = NULL;

/* forward references (first two are used in x_closepl.c) */
extern void _close_other_plotter_fds __P((Plotter *plotter));
extern void _flush_plotter_outstreams __P((void));
extern void _process_other_plotter_events __P((Plotter *plotter));
static bool _string_to_plotter_init __P((const char *type, Plotter **pptr));
static void _api_warning __P((const char *msg));
static void _init_plotter_array __P((void));
static void _copy_params_to_plotter __P((Plotter *plotter));

/* Initialize the array of plotters (on entry, _num_plotters should be 0).
   This is invoked when the user calls, for the very first time, one of the
   81 basic libplot functions. */
static void
#ifdef _HAVE_PROTOS
_init_plotter_array (void)
#else
_init_plotter_array ()
#endif
{
  int i;
  bool found;
  Plotter *plotter;

  /* set up plotter array */
  _plotters = (Plotter **)_plot_xmalloc (INITIAL_NUM_PLOTTERS * sizeof(Plotter *));
  for (i = 0; i < INITIAL_NUM_PLOTTERS; i++)
    _plotters[i] = (Plotter *)NULL;
  _num_plotters = INITIAL_NUM_PLOTTERS;
  
  /* determine initialization for the default plotter type */
  found = _string_to_plotter_init (DEFAULT_PLOTTER_TYPE, &plotter);
  if (!found)
    {
      fprintf (stderr, "libplot: panic: cannot create default plotter type \"%s\"\n",
	       DEFAULT_PLOTTER_TYPE);
      exit (1);
    }
  
  /* copy initialization for default type into array slot #0 */
  _plotters[0] = (Plotter *)_plot_xmalloc (sizeof(Plotter));
  memcpy (_plotters[0], plotter, sizeof(Plotter));
  _plotters[0]->instream = stdin;
  _plotters[0]->outstream = stdout;
  _plotters[0]->errstream = stderr;

  /* copy in the current values of class variables from _plot_params[] */
  _copy_params_to_plotter (_plotters[0]);

  /* select the just-created instance of the default plotter type */
  selectpl (0);
}

/* These are the user-callable functions that are specific to the C
   binding: newpl, selectpl, deletepl; also parampl. */

/* user-callable */
int 
#ifdef _HAVE_PROTOS
newpl (const char *type, FILE *instream, FILE *outstream, FILE *errstream)
#else
newpl (type, instream, outstream, errstream)
     const char *type;
     FILE *instream, *outstream, *errstream;
#endif
{
  bool open_slot = false;
  int i = 0;
  int j;
  Plotter *plotter;
  bool found;
  
  /* initialize plotter array if necessary */
  if (_num_plotters == 0)
    _init_plotter_array ();

  /* determine initialization for specified plotter type */
  found = _string_to_plotter_init (type, &plotter);
  if (!found)
    {
      _api_warning ("ignoring request to create unknown plotter type");
      return -1;
    }

  /* be sure there is an open slot */
  for (i = 0; i < _num_plotters; i++)
    if (_plotters[i] == NULL)
      {
	open_slot = true;
	break;
      }
  if (!open_slot)
    /* expand array, clearing upper half */
    {
      i = _num_plotters;
      _plotters = 
	(Plotter **)_plot_xrealloc (_plotters, 
				    2 * _num_plotters * sizeof (Plotter *));
      for (j = _num_plotters; j < 2 * _num_plotters; j++)
	_plotters[j] = (Plotter *)NULL;
      _num_plotters *= 2;
    }
  
  /* copy initialization for specified plotter type into open slot */
  _plotters[i] = (Plotter *)_plot_xmalloc (sizeof(Plotter));
  memcpy (_plotters[i], plotter, sizeof(Plotter));
  _plotters[i]->instream = instream;  
  _plotters[i]->outstream = outstream;
  _plotters[i]->errstream = errstream;

  /* copy in the current values of class variables from _plot_params[] */
  _copy_params_to_plotter (_plotters[i]);
  
  return i;
}

/* utility function, used above */
static bool
#ifdef _HAVE_PROTOS
_string_to_plotter_init (const char *type, Plotter **pptr)
#else
_string_to_plotter_init (type, pptr)
     const char *type;
     Plotter **pptr;
#endif
{
  plotter_initialization *p = _plotter_initializations;
  bool found = false;
  
  /* search table of known plotter type mnemonics */
  while (p->name)
    {
      if (strcasecmp ((char *)type, (char *)p->name) == 0)
	{
	  found = true;
	  break;
	}
      p++;
    }
  /* return pointer to plotter initialization through pointer */
  *pptr = p->init;
  return found;
}

/* utility function, used above */
static void 
#ifdef _HAVE_PROTOS
_copy_params_to_plotter (Plotter *plotter)
#else
_copy_params_to_plotter (plotter)
     Plotter *plotter;
#endif
{
  int j;
  char *envs;

  for (j = 0; j < NUM_DEVICE_DRIVER_PARAMETERS; j++)
    {
      if (!_plot_params[j].is_string)
	/* not a string, just copy the void pointer into the plotter */
	plotter->params[j] = _plot_params[j].value;

      else
	/* parameter value is a string, so use malloc and strcpy */
	{
	  if (_plot_params[j].value)
	    /* have user-specified value */
	    {
	      plotter->params[j] = 
		(char *)_plot_xmalloc (strlen ((char *)_plot_params[j].value) + 1);
	      strcpy ((char *)plotter->params[j], 
		      (char *)_plot_params[j].value);
	    }
	  else if ((envs = getenv (_plot_params[j].parameter)) != NULL)
	    /* have value of environment variable */
	    {
	      plotter->params[j] = 
		(char *)_plot_xmalloc (strlen (envs) + 1);
	      strcpy ((char *)plotter->params[j], envs);
	    }
	  else if (_plot_params[j].default_value)
	    /* have default libplot value */
	    {
	      plotter->params[j] = 
		(char *)_plot_xmalloc (strlen ((char *)_plot_params[j].default_value) + 1);
	      strcpy ((char *)plotter->params[j], 
		      (char *)_plot_params[j].default_value);
	    }
	  else			/* punt */
	    plotter->params[j] = NULL;
	}
    }
}

/* user-callable */
int
#ifdef _HAVE_PROTOS
selectpl (int handle)
#else
selectpl (handle)
     int handle;
#endif
{
  if (handle < 0 || handle >= _num_plotters 
      || _plotters[handle] == NULL)
    {
      _api_warning ("ignoring request to select a nonexistent plotter");
      return -1;
    }

  /* alter value of the global _plotter pointer */
  _plotter = _plotters[handle];

  return 0;
}

/* user-callable */
int
#ifdef _HAVE_PROTOS
deletepl (int handle)
#else
deletepl (handle)
     int handle;
#endif
{
  Plotter *current_plotter;
  int j;
  
  if (handle < 0 || handle >= _num_plotters 
      || _plotters[handle] == NULL)
    {
      _api_warning ("ignoring request to delete a nonexistent plotter");
      return -1;
    }

  if (_plotters[handle] == _plotter)
    {
      _api_warning ("ignoring request to delete currently selected plotter");
      return -1;
    }

  /* set _plotter to point to specified plotter */
  current_plotter = _plotter;

  /* if this plotter is open, close it */
  _plotter = _plotters[handle];
  if (_plotter->open)
    _plotter->closepl ();

#ifndef X_DISPLAY_MISSING
  /* if an X Plotter, kill forked-off processes that are maintaining its
     popped-up windows, provided that the VANISH_ON_DELETE parameter was
     set to "yes" at Plotter creation time */
  if (_plotter->vanish_on_delete)
    {
      for (j = 0; j < _plotter->num_pids; j++)
	kill (_plotter->pids[j], SIGKILL);
      if (_plotter->num_pids > 0)
	{
	  free (_plotter->pids);
	  _plotter->pids = (pid_t *)NULL;
	}
    }
#endif /* not X_DISPLAY_MISSING */

  /* tear down the plotter */
  for (j = 0; j < NUM_DEVICE_DRIVER_PARAMETERS; j++)
    if (_plot_params[j].is_string && _plotter->params[j] != NULL)
      /* stored parameter is a previously allocated string, so deallocate */
      free (_plotter->params[j]);
    /* Could do some additional deallocations here.  For an XPlotter, the
       `fontlist' member of the XPlotter object, which is a list of
       retrieved X fonts, should be freed.  FIXME. */
  free (_plotter);
  _plotters[handle] = (Plotter *)NULL;

  /* restore _plotter pointer */
  _plotter = current_plotter;
  
  return 0;
}

/* User-callable, affects only the _plot_params[] array, which implements
   the C counterpart of class variables (see g_params.h). */
int
#ifdef _HAVE_PROTOS
parampl (const char *parameter, void *value)
#else
#ifdef NO_VOID_SUPPORT
parampl (parameter, value)
     const char *parameter;
     char *value;
#else
parampl (parameter, value)
     const char *parameter;
     void *value;
#endif /* NO_VOID_SUPPORT */
#endif
{
  int j;

  for (j = 0; j < NUM_DEVICE_DRIVER_PARAMETERS; j++)
    {
      if (strcmp (_plot_params[j].parameter, parameter) == 0)
	{
	  if (_plot_params[j].is_string)
	    /* parameter value is a string, so treat specially */
	    {
	      if (_plot_params[j].value)
		free (_plot_params[j].value);
	      if (value != NULL)
		{
		  _plot_params[j].value = 
		    (char *)_plot_xmalloc (strlen ((char *)value) + 1);
		  strcpy ((char *)_plot_params[j].value, (char *)value);
		}
	      else
		_plot_params[j].value = NULL;
	    }
	  else
	    /* parameter value is a void *, so just copy user-spec'd value */
	    _plot_params[j].value = value;
	  
	  /* matched, so return happily */
	  return 0;
	}
    }
  _api_warning ("ignoring request to set an unknown parameter");
  return -1;
}

/* called in each openpl() method, to retrieve value of a class variable,
   in order to initialize the appropriate data field of a Plotter object */

void *
#ifdef _HAVE_PROTOS
_get_plot_param (const char *parameter_name)
#else
_get_plot_param (parameter_name)
     const char *parameter_name;
#endif
{
  int j;

  for (j = 0; j < NUM_DEVICE_DRIVER_PARAMETERS; j++)
    if (strcmp (_plot_params[j].parameter, parameter_name) == 0)
      return _plotter->params[j];

  return (void *)NULL;		/* name not matched */
}

#ifndef X_DISPLAY_MISSING

/* function called in x_closepl.c, before forking off a process to
   manage a window */
void
#ifdef _HAVE_PROTOS
_flush_plotter_outstreams (void)
#else
_flush_plotter_outstreams ()
#endif
{
  int i;

  for (i = 0; i < _num_plotters; i++)
    if (_plotters[i] && _plotters[i]->outstream)
      fflush (_plotters[i]->outstream);

#ifdef HAVE_NULL_FLUSH
  /* do more: flush _all_ output streams before forking */
  fflush ((FILE *)NULL);
#endif /* HAVE_NULL_FLUSH */
}

/* function called in x_closepl.c, by a child process (forked off to manage
   a window) */
void
#ifdef _HAVE_PROTOS
_close_other_plotter_fds (Plotter *plotter)
#else
_close_other_plotter_fds (plotter)
     Plotter *plotter;
#endif
{
  int i;

  for (i = 0; i < _num_plotters; i++)
    if (_plotters[i] != NULL
	&& _plotters[i] != plotter
	&& _plotters[i]->type == PL_X11
	&& _plotters[i]->opened
	&& _plotters[i]->open
	&& close (ConnectionNumber (_plotters[i]->dpy)) < 0)
      {
	plotter->error ("couldn't close connection to X display");
	exit (1);
      }
}

/* function called in x_xevents.c, to process X events associated with
   other open XPlotters */
void
#ifdef _HAVE_PROTOS
_process_other_plotter_events (Plotter *plotter)
#else
_process_other_plotter_events (plotter)
     Plotter *plotter;
#endif
{
  int i;

  for (i = 0; i < _num_plotters; i++)
    if (_plotters[i] != NULL
	&& _plotters[i] != plotter
	&& _plotters[i]->type == PL_X11
	&& _plotters[i]->opened
	&& _plotters[i]->open
	&& _plotters[i]->app_con != NULL) /* paranoia */
      {
	while (XtAppPending (_plotters[i]->app_con))
	  XtAppProcessEvent (_plotters[i]->app_con, XtIMAll);
      }
}
  
#endif /* not X_DISPLAY_MISSING */

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
    (*libplot_warning_handler)((char *)msg);
  else
    fprintf (stderr, "libplot: warning: %s\n", msg);
}

/* The following are the C bindings for the 81 methods that operate on
   objects in the Plotter class.  Together with the four functions above
   (newpl, selectpl, deletepl; also parampl), they make up the 
   libplot C API.

   Each binding tests whether _num_plotters > 0, which determines whether
   the array of plotters has been initialized.  That is because it makes no
   sense to call these functions before the _plotter pointer points to a
   non-NULL plotter object.

   In fact, of the below functions, it really only makes sense to call
   openpl, havecap, or outfile [deprecated], before the plotter array is
   initialized.  Calling any other of the below functions, before the
   plotter array is initialized, will generate an error message because
   even though the call to _init_plotter_array will initialize the plotter
   array and select a default plotter instance, the plotter will not be
   open.  No operation in the plotter class, with the exception of the
   just-mentioned ones, may be invoked unless the plotter that is being
   acted on is open. */

int 
#ifdef _HAVE_PROTOS
alabel (int x_justify, int y_justify, const char *s)
#else
alabel (x_justify, y_justify, s)
     int x_justify, y_justify;
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->alabel (x_justify, y_justify, s);
}

int
#ifdef _HAVE_PROTOS
arc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->arc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
arcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
arcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->arcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
bgcolor (int red, int green, int blue)
#else
bgcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->bgcolor (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
bgcolorname (const char *s)
#else
bgcolorname (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->bgcolorname (s);
}

int
#ifdef _HAVE_PROTOS
box (int x0, int y0, int x1, int y1)
#else
box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->box (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
boxrel (int x0, int y0, int x1, int y1)
#else
boxrel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->boxrel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
capmod (const char *s)
#else
capmod (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->capmod (s);
}

int
#ifdef _HAVE_PROTOS
circle (int x, int y, int r)
#else
circle (x, y, r)
     int x, y, r;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->circle (x, y, r);
}

int
#ifdef _HAVE_PROTOS
circlerel (int x, int y, int r)
#else
circlerel (x, y, r)
     int x, y, r;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->circlerel (x, y, r);
}

int
#ifdef _HAVE_PROTOS
closepl (void)
#else
closepl ()
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->closepl ();
}

int
#ifdef _HAVE_PROTOS
color (int red, int green, int blue)
#else
color (red, green, blue)
     int red, green, blue;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->color (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
colorname (const char *s)
#else
colorname (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->colorname (s);
}

int
#ifdef _HAVE_PROTOS
cont (int x, int y)
#else
cont (x, y)
     int x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->cont (x, y);
}

int
#ifdef _HAVE_PROTOS
contrel (int x, int y)
#else
contrel (x, y)
     int x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->contrel (x, y);
}

int
#ifdef _HAVE_PROTOS
ellarc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
ellarc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ellarc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
ellarcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
ellarcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ellarcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
ellipse (int x, int y, int rx, int ry, int angle)
#else
ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ellipse (x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
ellipserel (int x, int y, int rx, int ry, int angle)
#else
ellipserel (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ellipserel (x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
endpath (void)
#else
endpath ()
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->endpath ();
}

int
#ifdef _HAVE_PROTOS
erase (void)
#else
erase ()
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->erase ();
}

int
#ifdef _HAVE_PROTOS
farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->farc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
farcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
farcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->farcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
fbox (double x0, double y0, double x1, double y1)
#else
fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fbox (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
fboxrel (double x0, double y0, double x1, double y1)
#else
fboxrel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fboxrel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
fcircle (double x, double y, double r)
#else
fcircle (x, y, r)
     double x, y, r;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fcircle (x, y, r);
}

int
#ifdef _HAVE_PROTOS
fcirclerel (double x, double y, double r)
#else
fcirclerel (x, y, r)
     double x, y, r;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fcirclerel (x, y, r);
}

int
#ifdef _HAVE_PROTOS
fconcat (double m0, double m1, double m2, double m3, double m4, double m5)
#else
fconcat (m0, m1, m2, m3, m4, m5)
     double m0, m1, m2, m3, m4, m5;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fconcat (m0, m1, m2, m3, m4, m5);
}

int
#ifdef _HAVE_PROTOS
fcont (double x, double y)
#else
fcont (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fcont (x, y);
}

int
#ifdef _HAVE_PROTOS
fcontrel (double x, double y)
#else
fcontrel (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fcontrel (x, y);
}

int
#ifdef _HAVE_PROTOS
fellarc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
fellarc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fellarc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
fellarcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
fellarcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fellarcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
fellipse (double x, double y, double rx, double ry, double angle)
#else
fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fellipse (x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
fellipserel (double x, double y, double rx, double ry, double angle)
#else
fellipserel (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fellipserel (x, y, rx, ry, angle);
}

double
#ifdef _HAVE_PROTOS
ffontname (const char *s)
#else
ffontname (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ffontname (s);
}

double
#ifdef _HAVE_PROTOS
ffontsize (double size)
#else
ffontsize (size)
     double size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ffontsize (size);
}

int
#ifdef _HAVE_PROTOS
fillcolor (int red, int green, int blue)
#else
fillcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fillcolor (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
fillcolorname (const char *s)
#else
fillcolorname (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fillcolorname (s);
}

int
#ifdef _HAVE_PROTOS
filltype (int level)
#else
filltype (level)
     int level;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->filltype (level);
}

double
#ifdef _HAVE_PROTOS
flabelwidth (const char *s)
#else
flabelwidth (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->flabelwidth (s);
}

int
#ifdef _HAVE_PROTOS
fline (double x0, double y0, double x1, double y1)
#else
fline (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fline (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
flinerel (double x0, double y0, double x1, double y1)
#else
flinerel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->flinerel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
flinewidth (double size)
#else
flinewidth (size)
     double size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->flinewidth (size);
}

int
#ifdef _HAVE_PROTOS
flushpl (void)
#else
flushpl ()
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->flushpl ();
}

int
#ifdef _HAVE_PROTOS
fmarker (double x, double y, int type, double size)
#else
fmarker (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fmarker (x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
fmarkerrel (double x, double y, int type, double size)
#else
fmarkerrel (x, y, type, size)
     double x, y;
     int type;
     double size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fmarkerrel (x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
fmove (double x, double y)
#else
fmove (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fmove (x, y);
}

int
#ifdef _HAVE_PROTOS
fmoverel (double x, double y)
#else
fmoverel (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fmoverel (x, y);
}

int
#ifdef _HAVE_PROTOS
fontname (const char *s)
#else
fontname (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fontname (s);
}

int
#ifdef _HAVE_PROTOS
fontsize (int size)
#else
fontsize (size)
     int size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fontsize (size);
}

int
#ifdef _HAVE_PROTOS
fpoint (double x, double y)
#else
fpoint (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fpoint (x, y);
}

int
#ifdef _HAVE_PROTOS
fpointrel (double x, double y)
#else
fpointrel (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fpointrel (x, y);
}

int
#ifdef _HAVE_PROTOS
frotate (double theta)
#else
frotate (theta)
     double theta;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->frotate (theta);
}

int
#ifdef _HAVE_PROTOS
fscale (double x, double y)
#else
fscale (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fscale (x, y);
}

int
#ifdef _HAVE_PROTOS
fspace (double x0, double y0, double x1, double y1)
#else
fspace (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fspace (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
fspace2 (double x0, double y0, double x1, double y1, double x2, double y2)
#else
fspace2 (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->fspace2 (x0, y0, x1, y1, x2, y2);
}

double
#ifdef _HAVE_PROTOS
ftextangle (double angle)
#else
ftextangle (angle)
     double angle;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ftextangle (angle);
}

int
#ifdef _HAVE_PROTOS
ftranslate (double x, double y)
#else
ftranslate (x, y)
     double x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->ftranslate (x, y);
}

int
#ifdef _HAVE_PROTOS
havecap (const char *s)
#else
havecap (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->havecap (s);
}

int
#ifdef _HAVE_PROTOS
joinmod (const char *s)
#else
joinmod (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->joinmod (s);
}

int
#ifdef _HAVE_PROTOS
label (const char *s)
#else
label (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->label (s);
}

int
#ifdef _HAVE_PROTOS
labelwidth (const char *s)
#else
labelwidth (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->labelwidth (s);
}

int
#ifdef _HAVE_PROTOS
line (int x0, int y0, int x1, int y1)
#else
line (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->line (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
linerel (int x0, int y0, int x1, int y1)
#else
linerel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->linerel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
linewidth (int size)
#else
linewidth (size)
     int size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->linewidth (size);
}

int
#ifdef _HAVE_PROTOS
linemod (const char *s)
#else
linemod (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->linemod (s);
}

int
#ifdef _HAVE_PROTOS
marker (int x, int y, int type, int size)
#else
marker (x, y, type, size)
     int x, y, type, size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->marker (x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
markerrel (int x, int y, int type, int size)
#else
markerrel (x, y, type, size)
     int x, y, type, size;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->markerrel (x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
move (int x, int y)
#else
move (x, y)
     int x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->move (x, y);
}

int
#ifdef _HAVE_PROTOS
moverel (int x, int y)
#else
moverel (x, y)
     int x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->moverel (x, y);
}

int
#ifdef _HAVE_PROTOS
openpl (void)
#else
openpl ()
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->openpl ();
}

FILE *
#ifdef _HAVE_PROTOS
outfile (FILE *newstream)
#else
outfile (newstream)
     FILE *newstream;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->outfile (newstream);
}

int
#ifdef _HAVE_PROTOS
pencolor (int red, int green, int blue)
#else
pencolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->pencolor (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pencolorname (const char *s)
#else
pencolorname (s)
     const char *s;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->pencolorname (s);
}

int
#ifdef _HAVE_PROTOS
point (int x, int y)
#else
point (x, y)
     int x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->point (x, y);
}

int
#ifdef _HAVE_PROTOS
pointrel (int x, int y)
#else
pointrel (x, y)
     int x, y;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->pointrel (x, y);
}

int
#ifdef _HAVE_PROTOS
restorestate (void)
#else
restorestate ()
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->restorestate ();
}

int
#ifdef _HAVE_PROTOS
savestate (void)
#else
savestate ()
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->savestate ();
}

int
#ifdef _HAVE_PROTOS
space (int x0, int y0, int x1, int y1)
#else
space (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->space (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
space2 (int x0, int y0, int x1, int y1, int x2, int y2)
#else
space2 (x0, y0, x1, y1, x2, y2)
     int x0, y0, x1, y1, x2, y2;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->space2 (x0, y0, x1, y1, x2, y2);
}

int
#ifdef _HAVE_PROTOS
textangle (int angle)
#else
textangle (angle)
     int angle;
#endif
{
  if (_num_plotters == 0)
    _init_plotter_array ();
  return _plotter->textangle (angle);
}
