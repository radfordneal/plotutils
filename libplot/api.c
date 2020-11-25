/* This file is specific to libplot, rather than libplotter.  It defines
   the C API.  The C API consists of wrappers around the operations that
   may be applied to any Plotter object, plus three additional functions
   (newpl, selectpl, deletepl) that are specific to libplot, and parampl.

   newpl/selectpl/deletepl maintain the libplot/libplotter sparse array of
   Plotter instances ("_plotters"), constructing and destroying them as
   requested.  The "_plotter" pointer is positioned by calling selectpl.
   It is a pointer to one of the instances in the array.

   By convention, _plotter is initialized to point to a Plotter instance
   that sends output in metafile format to standard output.  Initialization
   takes place when the first Plotter operation is invoked, provided that
   no Plotters have previously been created by calling newpl.  This
   convention is for compatibility with the device-dependent pre-GNU
   versions of libplot, which did not support newpl/select/deletepl.

   The parampl function is not specific to the libplot API.  It is defined
   in g_params.c. */

#include "sys-defines.h"
#include "extern.h"

/* Known Plotter types, indexed into by a short mnemonic case-insensitive
   string: "generic"=generic (i.e. base Plotter class), "meta"=metafile,
   "tek"=Tektronix, "hpgl"=HP-GL/2, "fig"=xfig, "pcl"=PCL 5, "ps"=PS,
   "ai"="AI", "gif"=GIF, "pnm"=PNM (PBM/PGM/PPM), "X"=X11,
   "Xdrawable"=X11Drawable.

   When a new Plotter of any type is constructed, the appropriate
   `default_init' structure is copied into it, and (temporarily) "_plotter"
   is changed to point to it.  Then the appropriate `initialize' routine is
   invoked.  Before the Plotter is destroyed, the appropriate `terminate'
   routine is invoked similarly. */

typedef struct 
{
  const char *name;
  const Plotter *default_init;
}
Plotter_data;

/* Initializations for the function-pointer part of each type of Plotter.
   Each of the initializing structures listed here is located in the
   corresponding ?_defplot.c file. */
static const Plotter_data _plotter_data[] = 
{
  {"generic", &_g_default_plotter},
  {"meta", &_m_default_plotter},
  {"tek", &_t_default_plotter},
  {"hpgl", &_h_default_plotter},
  {"pcl", &_q_default_plotter},
  {"fig", &_f_default_plotter},
  {"ps", &_p_default_plotter},
  {"ai", &_a_default_plotter},
  {"gif", &_i_default_plotter},
  {"pnm", &_n_default_plotter},
#ifndef X_DISPLAY_MISSING
  {"Xdrawable", &_x_default_plotter},
  {"X", &_y_default_plotter},
#endif /* X_DISPLAY_MISSING */
  {(const char *)NULL, (const Plotter *)NULL}
};

/* default plotter type (must be one of the preceding) */
#ifndef DEFAULT_PLOTTER_TYPE
#define DEFAULT_PLOTTER_TYPE "meta"
#endif

/* distinguished Plotter instance, for libplot functions to act on (in
   libplotter, _plotter is an alias for `this') */
Plotter *_plotter = NULL;

/* forward references */
static bool _string_to_plotter_data ____P((const char *type, int *position));
static void _api_error ____P((const char *msg));
static void _api_warning ____P((const char *msg));
static void _create_and_select_default_plotter ____P((void));

/* Initialize the array of plotters to include a single Plotter, of default
   type, in location zero; also, select that Plotter.  This is invoked when
   the user calls, for the very first time, any of the Plotter methods. */
static void
#ifdef _HAVE_PROTOS
_create_and_select_default_plotter (void)
#else
_create_and_select_default_plotter ()
#endif
{
  bool found;
  int position;

  found = _string_to_plotter_data (DEFAULT_PLOTTER_TYPE, &position);
  if (!found)
    /* shouldn't happen (library installed wrong?), treat as fatal */
    _api_error ("cannot create plotter of default type");
  
  /* create new Plotter to serve as default Plotter */
  _plotter = (Plotter *)_plot_xmalloc (sizeof(Plotter));
  memcpy (_plotter, _plotter_data[position].default_init, sizeof(Plotter));
  _plotter->infp = stdin;  
  _plotter->outfp = stdout;
  _plotter->errfp = stderr;

  /* Do any needed initializiations of _plotter (e.g., copy in the current
     values of class variables from _plot_params[], initialize data members
     from the class variables).  Also, add _plotter to _plotters[], as 0'th
     element (this will create the _plotters[] array, as a default size). */
  _plotter->initialize ();
}

/* These are the 3 user-callable functions that are specific to the C
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
  bool found;
  int i, position;
  Plotter *current_plotter;
  
  /* determine initialization for specified plotter type */
  found = _string_to_plotter_data (type, &position);
  if (!found)
    {
      _api_warning ("ignoring request to create plotter of unknown type");
      return -1;
    }

  if (_plotters_len == 0)
    _create_and_select_default_plotter ();

  /* stash _plotter */
  current_plotter = _plotter;

  /* create Plotter */
  _plotter = (Plotter *)_plot_xmalloc (sizeof(Plotter));
  memcpy (_plotter, _plotter_data[position].default_init, sizeof(Plotter));
  _plotter->infp = infile;  
  _plotter->outfp = outfile;
  _plotter->errfp = errfile;

  /* do any needed initializiations of _plotter (e.g., copy in the current
     values of class variables from _plot_params[], initialize data members
     from the class variables), and also add _plotter to _plotters[] */
  _plotter->initialize ();

  /* determine index of new Plotter in _plotters[] (not very efficiently) */
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i] == _plotter)
      break;

  /* restore _plotter */
  _plotter = current_plotter;

  /* return index of just-created Plotter */
  return i;
}

/* utility function, used above; keys into table of Plotter types by a
   short mnemonic string */
static bool
#ifdef _HAVE_PROTOS
_string_to_plotter_data (const char *type, int *position)
#else
_string_to_plotter_data (type, position)
     const char *type;
     int *position;
#endif
{
  const Plotter_data *p = _plotter_data;
  bool found = false;
  int i = 0;
  
  /* search table of known plotter type mnemonics */
  while (p->name)
    {
      if (strcasecmp ((char *)type, (char *)p->name) == 0)
	{
	  found = true;
	  break;
	}
      p++;
      i++;
    }
  /* return pointer to plotter data through pointer */
  if (found)
    *position = i;
  return found;
}

/* user-callable, returns index of previously selected Plotter */
int
#ifdef _HAVE_PROTOS
pl_selectpl (int handle)
#else
pl_selectpl (handle)
     int handle;
#endif
{
  int i;

  if (handle < 0 || handle >= _plotters_len 
      || _plotters[handle] == NULL)
    {
      _api_warning ("ignoring request to select a nonexistent plotter");
      return -1;
    }

  /* determine index of previously selected Plotter in _plotters[] (not
     very efficiently) */
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i] == _plotter)
      break;

  /* alter value of the global _plotter pointer */
  _plotter = _plotters[handle];

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
  Plotter *current_plotter;
  
  if (handle < 0 || handle >= _plotters_len 
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

  /* save _plotter, set _plotter to point to specified plotter */
  current_plotter = _plotter;
  _plotter = _plotters[handle];

  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl ();

  /* Invoke an internal Plotter method before deletion.  For most Plotters
     this private method merely frees any stored class variables.  For a PS
     Plotter, it first emits the Plotter's pages of graphics to its output
     stream and deallocates associated storage.  For an XPlotter, it kills
     the forked-off processes that are maintaining its popped-up windows,
     if any.  It also removes Plotter from _plotters[]; see g_defplot.c. */
  _plotter->terminate ();

  /* tear down the plotter */
  free (_plotter);

  /* restore _plotter pointer */
  _plotter = current_plotter;
  
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

/* function used in this file to print error messages */
static void
#ifdef _HAVE_PROTOS
_api_error (const char *msg)
#else
_api_error (msg)
     const char *msg;
#endif
{
  if (libplot_error_handler != NULL)
    (*libplot_error_handler)(msg);
  else
    {
      fprintf (stderr, "libplot: error: %s\n", msg);
      exit (EXIT_FAILURE);
    }
}


/* The following are the C wrappers around the public functions in the
   Plotter class.  Together with the three functions above (newpl,
   selectpl, deletepl), and parampl, they make up the libplot C API.

   Each binding tests whether _plotters_len > 0, which determines whether
   the array of Plotter instances has been initialized.  That is because it
   makes no sense to call these functions before the _plotter pointer is
   non-NULL, i.e., points to a Plotter object.

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
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->alabel (x_justify, y_justify, s);
}

int
#ifdef _HAVE_PROTOS
pl_arc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->arc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_arcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_arcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->arcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier2 (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_bezier2 (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->bezier2 (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier2rel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_bezier2rel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->bezier2rel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier3 (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
pl_bezier3 (x0, y0, x1, y1, x2, y2, x3, y3)
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->bezier3 (x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_bezier3rel (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
pl_bezier3rel (x0, y0, x1, y1, x2, y2, x3, y3)
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->bezier3rel (x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_bgcolor (int red, int green, int blue)
#else
pl_bgcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->bgcolor (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_bgcolorname (const char *s)
#else
pl_bgcolorname (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->bgcolorname (s);
}

int
#ifdef _HAVE_PROTOS
pl_box (int x0, int y0, int x1, int y1)
#else
pl_box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->box (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_boxrel (int x0, int y0, int x1, int y1)
#else
pl_boxrel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->boxrel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_capmod (const char *s)
#else
pl_capmod (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->capmod (s);
}

int
#ifdef _HAVE_PROTOS
pl_circle (int x, int y, int r)
#else
pl_circle (x, y, r)
     int x, y, r;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->circle (x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_circlerel (int x, int y, int r)
#else
pl_circlerel (x, y, r)
     int x, y, r;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->circlerel (x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_closepl (void)
#else
pl_closepl ()
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->closepl ();
}

int
#ifdef _HAVE_PROTOS
pl_color (int red, int green, int blue)
#else
pl_color (red, green, blue)
     int red, green, blue;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->color (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_colorname (const char *s)
#else
pl_colorname (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->colorname (s);
}

int
#ifdef _HAVE_PROTOS
pl_cont (int x, int y)
#else
pl_cont (x, y)
     int x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->cont (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_contrel (int x, int y)
#else
pl_contrel (x, y)
     int x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->contrel (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_ellarc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_ellarc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ellarc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_ellarcrel (int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_ellarcrel (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ellarcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_ellipse (int x, int y, int rx, int ry, int angle)
#else
pl_ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ellipse (x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_ellipserel (int x, int y, int rx, int ry, int angle)
#else
pl_ellipserel (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ellipserel (x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_endpath (void)
#else
pl_endpath ()
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->endpath ();
}

int
#ifdef _HAVE_PROTOS
pl_erase (void)
#else
pl_erase ()
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->erase ();
}

int
#ifdef _HAVE_PROTOS
pl_farc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_farc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->farc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_farcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_farcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->farcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier2 (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fbezier2 (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fbezier2 (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier2rel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fbezier2rel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fbezier2rel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier3 (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
pl_fbezier3 (x0, y0, x1, y1, x2, y2, x3, y3)
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fbezier3 (x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier3rel (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
pl_fbezier3rel (x0, y0, x1, y1, x2, y2, x3, y3)
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fbezier3rel (x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_fbox (double x0, double y0, double x1, double y1)
#else
pl_fbox (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fbox (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fboxrel (double x0, double y0, double x1, double y1)
#else
pl_fboxrel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fboxrel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fcircle (double x, double y, double r)
#else
pl_fcircle (x, y, r)
     double x, y, r;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fcircle (x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_fcirclerel (double x, double y, double r)
#else
pl_fcirclerel (x, y, r)
     double x, y, r;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fcirclerel (x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_fconcat (double m0, double m1, double m2, double m3, double m4, double m5)
#else
pl_fconcat (m0, m1, m2, m3, m4, m5)
     double m0, m1, m2, m3, m4, m5;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fconcat (m0, m1, m2, m3, m4, m5);
}

int
#ifdef _HAVE_PROTOS
pl_fcont (double x, double y)
#else
pl_fcont (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fcont (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fcontrel (double x, double y)
#else
pl_fcontrel (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fcontrel (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fellarc (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fellarc (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fellarc (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fellarcrel (double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fellarcrel (xc, yc, x0, y0, x1, y1)
     double xc, yc, x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fellarcrel (xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fellipse (double x, double y, double rx, double ry, double angle)
#else
pl_fellipse (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fellipse (x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_fellipserel (double x, double y, double rx, double ry, double angle)
#else
pl_fellipserel (x, y, rx, ry, angle)
     double x, y, rx, ry, angle;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fellipserel (x, y, rx, ry, angle);
}

double
#ifdef _HAVE_PROTOS
pl_ffontname (const char *s)
#else
pl_ffontname (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ffontname (s);
}

double
#ifdef _HAVE_PROTOS
pl_ffontsize (double size)
#else
pl_ffontsize (size)
     double size;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ffontsize (size);
}

int
#ifdef _HAVE_PROTOS
pl_fillcolor (int red, int green, int blue)
#else
pl_fillcolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fillcolor (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_fillcolorname (const char *s)
#else
pl_fillcolorname (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fillcolorname (s);
}

int
#ifdef _HAVE_PROTOS
pl_fillmod (const char *s)
#else
pl_fillmod (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fillmod (s);
}

int
#ifdef _HAVE_PROTOS
pl_filltype (int level)
#else
pl_filltype (level)
     int level;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->filltype (level);
}

double
#ifdef _HAVE_PROTOS
pl_flabelwidth (const char *s)
#else
pl_flabelwidth (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->flabelwidth (s);
}

int
#ifdef _HAVE_PROTOS
pl_fline (double x0, double y0, double x1, double y1)
#else
pl_fline (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fline (x0, y0, x1, y1);
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
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->flinedash (n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
pl_flinerel (double x0, double y0, double x1, double y1)
#else
pl_flinerel (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->flinerel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_flinewidth (double size)
#else
pl_flinewidth (size)
     double size;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->flinewidth (size);
}

int
#ifdef _HAVE_PROTOS
pl_flushpl (void)
#else
pl_flushpl ()
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->flushpl ();
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
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fmarker (x, y, type, size);
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
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fmarkerrel (x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_fmiterlimit (double limit)
#else
pl_fmiterlimit (limit)
     double limit;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fmiterlimit (limit);
}

int
#ifdef _HAVE_PROTOS
pl_fmove (double x, double y)
#else
pl_fmove (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fmove (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fmoverel (double x, double y)
#else
pl_fmoverel (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fmoverel (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fontname (const char *s)
#else
pl_fontname (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fontname (s);
}

int
#ifdef _HAVE_PROTOS
pl_fontsize (int size)
#else
pl_fontsize (size)
     int size;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fontsize (size);
}

int
#ifdef _HAVE_PROTOS
pl_fpoint (double x, double y)
#else
pl_fpoint (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fpoint (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fpointrel (double x, double y)
#else
pl_fpointrel (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fpointrel (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_frotate (double theta)
#else
pl_frotate (theta)
     double theta;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->frotate (theta);
}

int
#ifdef _HAVE_PROTOS
pl_fscale (double x, double y)
#else
pl_fscale (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fscale (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fspace (double x0, double y0, double x1, double y1)
#else
pl_fspace (x0, y0, x1, y1)
     double x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fspace (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fspace2 (double x0, double y0, double x1, double y1, double x2, double y2)
#else
pl_fspace2 (x0, y0, x1, y1, x2, y2)
     double x0, y0, x1, y1, x2, y2;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->fspace2 (x0, y0, x1, y1, x2, y2);
}

double
#ifdef _HAVE_PROTOS
pl_ftextangle (double angle)
#else
pl_ftextangle (angle)
     double angle;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ftextangle (angle);
}

int
#ifdef _HAVE_PROTOS
pl_ftranslate (double x, double y)
#else
pl_ftranslate (x, y)
     double x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->ftranslate (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_havecap (const char *s)
#else
pl_havecap (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->havecap (s);
}

int
#ifdef _HAVE_PROTOS
pl_joinmod (const char *s)
#else
pl_joinmod (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->joinmod (s);
}

int
#ifdef _HAVE_PROTOS
pl_label (const char *s)
#else
pl_label (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->label (s);
}

int
#ifdef _HAVE_PROTOS
pl_labelwidth (const char *s)
#else
pl_labelwidth (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->labelwidth (s);
}

int
#ifdef _HAVE_PROTOS
pl_line (int x0, int y0, int x1, int y1)
#else
pl_line (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->line (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_linerel (int x0, int y0, int x1, int y1)
#else
pl_linerel (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->linerel (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_linewidth (int size)
#else
pl_linewidth (size)
     int size;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->linewidth (size);
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
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->linedash (n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
pl_linemod (const char *s)
#else
pl_linemod (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->linemod (s);
}

int
#ifdef _HAVE_PROTOS
pl_marker (int x, int y, int type, int size)
#else
pl_marker (x, y, type, size)
     int x, y, type, size;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->marker (x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_markerrel (int x, int y, int type, int size)
#else
pl_markerrel (x, y, type, size)
     int x, y, type, size;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->markerrel (x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_move (int x, int y)
#else
pl_move (x, y)
     int x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->move (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_moverel (int x, int y)
#else
pl_moverel (x, y)
     int x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->moverel (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_openpl (void)
#else
pl_openpl ()
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->openpl ();
}

FILE *
#ifdef _HAVE_PROTOS
pl_outfile (FILE *outfile)
#else
pl_outfile (outfile)
     FILE *outfile;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->outfile (outfile);
}

int
#ifdef _HAVE_PROTOS
pl_pencolor (int red, int green, int blue)
#else
pl_pencolor (red, green, blue)
     int red, green, blue;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->pencolor (red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_pencolorname (const char *s)
#else
pl_pencolorname (s)
     const char *s;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->pencolorname (s);
}

int
#ifdef _HAVE_PROTOS
pl_point (int x, int y)
#else
pl_point (x, y)
     int x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->point (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_pointrel (int x, int y)
#else
pl_pointrel (x, y)
     int x, y;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->pointrel (x, y);
}

int
#ifdef _HAVE_PROTOS
pl_restorestate (void)
#else
pl_restorestate ()
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->restorestate ();
}

int
#ifdef _HAVE_PROTOS
pl_savestate (void)
#else
pl_savestate ()
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->savestate ();
}

int
#ifdef _HAVE_PROTOS
pl_space (int x0, int y0, int x1, int y1)
#else
pl_space (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->space (x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_space2 (int x0, int y0, int x1, int y1, int x2, int y2)
#else
pl_space2 (x0, y0, x1, y1, x2, y2)
     int x0, y0, x1, y1, x2, y2;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->space2 (x0, y0, x1, y1, x2, y2);
}

int
#ifdef _HAVE_PROTOS
pl_textangle (int angle)
#else
pl_textangle (angle)
     int angle;
#endif
{
  if (_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _plotter->textangle (angle);
}

/* END OF WRAPPERS */


/* The following forwarding function provides special support in libplot
   for deriving Plotter classes from other Plotter classes.  This file is
   the only reasonable place to put them. */

/* Support deriving MetaPlotter class from Plotter class */

/* Recompute device-frame line width, without (if the Plotter is a
   MetaPlotter) emitting an op code.  See comments in g_concat.c and
   m_concat.c. */
void
#ifdef _HAVE_PROTOS
_recompute_device_line_width (void)
#else
_recompute_device_line_width ()
#endif
{
  if (_plotter->type == PL_META)
    _m_recompute_device_line_width ();
  else
    _g_recompute_device_line_width ();
}

/* Support deriving PCLPlotter class from HPGLPlotter class */

/* Eject page (if page number > 1) and switch from PCL 5 mode to HP-GL/2
   mode, if a PCL 5 printer (otherwise it's a no-op) */
void
#ifdef _HAVE_PROTOS
_maybe_switch_to_hpgl (void)
#else
_maybe_switch_to_hpgl ()
#endif
{
  if (_plotter->type == PL_HPGL)
    _h_maybe_switch_to_hpgl ();
  else if (_plotter->type == PL_PCL)
    _q_maybe_switch_to_hpgl ();
}

/* Switch back to PCL 5 mode from HP-GL/2 mode, if a PCL 5 printer 
   (otherwise it's a no-op) */
void
#ifdef _HAVE_PROTOS
_maybe_switch_from_hpgl (void)
#else
_maybe_switch_from_hpgl ()
#endif
{
  if (_plotter->type == PL_HPGL)
    _h_maybe_switch_from_hpgl ();
  else if (_plotter->type == PL_PCL)
    _q_maybe_switch_from_hpgl ();
}

#ifndef X_DISPLAY_MISSING
/* Support deriving XPlotter class from XDrawablePlotter class */

/* Internal function for XDrawablePlotters/XPlotters (called in x_color.c,
   if the original colormap fills up).  For XDrawablePlotters, it's a
   no-op, but for XPlotters, we try to switch to a new colormap.  This
   forwarding function is used only by libplot; in libplotter,
   _maybe_get_new_colormap is a virtual function. */
void
#ifdef _HAVE_PROTOS
_maybe_get_new_colormap (void)
#else
_maybe_get_new_colormap ()
#endif
{
  if (_plotter->type == PL_X11_DRAWABLE)
    _x_maybe_get_new_colormap ();
  else if (_plotter->type == PL_X11)
    _y_maybe_get_new_colormap ();
}

/* Internal function for XDrawablePlotters/XPlotters (called after most
   drawing operations).  For XDrawablePlotters, it's a no-op, but for
   XPlotters, we manually process all pending X events.  This forwarding
   function is used only by libplot; in libplotter, _maybe_handle_x_events
   is a virtual function. */
void
#ifdef _HAVE_PROTOS
_maybe_handle_x_events (void)
#else
_maybe_handle_x_events ()
#endif
{
  if (_plotter->type == PL_X11_DRAWABLE)
    _x_maybe_handle_x_events ();
  else if (_plotter->type == PL_X11)
    _y_maybe_handle_x_events ();
}
#endif /* not X_DISPLAY_MISSING */
