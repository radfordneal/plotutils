/* This file is specific to libplot, rather than libplotter.  It defines
   the new (i.e., thread-safe) C API.  The new C API contains wrappers
   around the operations that may be applied to any Plotter object, plus
   two additional functions (pl_newpl_r, pl_deletepl_r) that are specific
   to libplot.

   pl_newpl_r/pl_deletepl_r construct and destroy Plotter instances.  Their
   names resemble the C++ operations `new' and `delete'.  When a new
   Plotter of any type is constructed, the appropriate `default_init'
   structure, which contains function pointers, is copied into it.  Then
   its `initialize' method is invoked.  Before the Plotter is destroyed,
   its `terminate' method is invoked similarly.

   The C API also includes the functions pl_newplparams/pl_deleteplparams,
   which create/destroy PlotterParams instances, and wrappers around the
   methods that may be applied to any PlotterParams object.  A pointer to a
   PlotterParams object is passed to pl_newpl_r().  It specifies the
   device-driver parameters of the Plotter that will be created. */

#include "sys-defines.h"
#include "extern.h"
#include "plot.h"		/* header file for C API */

/* Known Plotter types, indexed into by a short mnemonic case-insensitive
   string: "generic"=generic (i.e. base Plotter class), "meta"=metafile,
   "tek"=Tektronix, "hpgl"=HP-GL/2, "pcl"=PCL 5, "fig"=xfig, "cgm"=CGM,
   "ps"=PS, "ai"="AI", "gif"=GIF, "pnm"=PNM (i.e. PBM/PGM/PPM), "X"=X11,
   "Xdrawable"=X11Drawable.  */

typedef struct 
{
  const char *name;
  const Plotter *default_init;
}
Plotter_data;

/* Initializations for the function-pointer part of each type of Plotter.
   Each of the initializing structures listed here is defined in the
   corresponding ?_defplot.c file. */
static const Plotter_data _plotter_data[] = 
{
  {"generic", &_g_default_plotter},
  {"meta", &_m_default_plotter},
  {"tek", &_t_default_plotter},
  {"hpgl", &_h_default_plotter},
  {"pcl", &_q_default_plotter},
  {"fig", &_f_default_plotter},
  {"cgm", &_c_default_plotter},
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

/* forward references */
static bool _string_to_plotter_data ____P((const char *type, int *position));
static void _api_warning ____P((const char *msg));

/* These are two user-callable functions that are specific to the new
   (i.e., thread-safe) C binding: pl_newpl_r, pl_deletepl_r. */

Plotter *
#ifdef _HAVE_PROTOS
pl_newpl_r (const char *type, FILE *infile, FILE *outfile, FILE *errfile, const PlotterParams *plotter_params)
#else
pl_newpl_r (type, infile, outfile, errfile, plotter_params)
     const char *type;
     FILE *infile, *outfile, *errfile;
     const PlotterParams *plotter_params;
#endif
{
  bool found;
  int position;
  Plotter *_plotter;
  
  /* determine initialization for specified plotter type */
  found = _string_to_plotter_data (type, &position);
  if (!found)
    {
      _api_warning ("ignoring request to create plotter of unknown type");
      return NULL;
    }

  /* create Plotter, copy function pointers to it */
  _plotter = (Plotter *)_plot_xmalloc (sizeof(Plotter));
  memcpy (_plotter, _plotter_data[position].default_init, sizeof(Plotter));

  /* copy parameters to it */
  _plotter->infp = infile;  
  _plotter->outfp = outfile;
  _plotter->errfp = errfile;
  _copy_params_to_plotter (_plotter, plotter_params);

  /* do any needed initializiations of the Plotter (e.g., initialize data
     members from the class variables, and also add the Plotter to the
     _plotters[] array */
  _plotter->initialize (_plotter);

  return _plotter;
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

int
#ifdef _HAVE_PROTOS
pl_deletepl_r (Plotter *_plotter)
#else
pl_deletepl_r (_plotter)
     Plotter *_plotter;
#endif
{
  if (_plotter == NULL)
    {
      _api_warning ("ignoring request to delete a null Plotter");
      return -1;
    }

  /* Invoke an internal Plotter method before deletion.  At a minimum, this
     private `terminate' method closes the Plotter (if open), frees
     instance-specific copies of class parameters, and also removes the
     pointer to the Plotter instance from the _plotters[] array.

     Also, it writes any unwritten graphics to the Plotter's output stream.
     This is the case for PSPlotters in particular, which write graphics
     only when they are deleted.  For a PSPlotter, the terminate method
     emits the Plotter's pages of graphics to its output stream and then
     deallocates associated storage.  For an XPlotter, this method kills
     the forked-off processes that are maintaining its popped-up windows
     (if any), provided that the VANISH_ON_DELETE parameter is set.  */
  _plotter->terminate (_plotter);

  /* tear down the Plotter */
  free (_plotter);

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


/* The following are C wrappers around the public functions in the Plotter
   class.  Together with the two functions above (pl_newpl_r,
   pl_deletepl_r), they are part of the new (thread-safe) libplot C API. */

int 
#ifdef _HAVE_PROTOS
pl_alabel_r (Plotter *plotter, int x_justify, int y_justify, const char *s)
#else
pl_alabel_r (plotter, x_justify, y_justify, s)
     Plotter *plotter;
     int x_justify, y_justify;
     const char *s;
#endif
{
  return plotter->alabel (plotter, x_justify, y_justify, s);
}

int
#ifdef _HAVE_PROTOS
pl_arc_r (Plotter *plotter, int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_arc_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->arc (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_arcrel_r (Plotter *plotter, int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_arcrel_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->arcrel (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier2_r (Plotter *plotter, int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_bezier2_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->bezier2 (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier2rel_r (Plotter *plotter, int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_bezier2rel_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->bezier2rel (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_bezier3_r (Plotter *plotter, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
pl_bezier3_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3)
     Plotter *plotter;
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return plotter->bezier3 (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_bezier3rel_r (Plotter *plotter, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
pl_bezier3rel_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3)
     Plotter *plotter;
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return plotter->bezier3rel (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_bgcolor_r (Plotter *plotter, int red, int green, int blue)
#else
pl_bgcolor_r (plotter, red, green, blue)
     Plotter *plotter;
     int red, green, blue;
#endif
{
  return plotter->bgcolor (plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_bgcolorname_r (Plotter *plotter, const char *s)
#else
pl_bgcolorname_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->bgcolorname (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_box_r (Plotter *plotter, int x0, int y0, int x1, int y1)
#else
pl_box_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     int x0, y0, x1, y1;
#endif
{
  return plotter->box (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_boxrel_r (Plotter *plotter, int x0, int y0, int x1, int y1)
#else
pl_boxrel_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     int x0, y0, x1, y1;
#endif
{
  return plotter->boxrel (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_capmod_r (Plotter *plotter, const char *s)
#else
pl_capmod_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->capmod (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_circle_r (Plotter *plotter, int x, int y, int r)
#else
pl_circle_r (plotter, x, y, r)
     Plotter *plotter;
     int x, y, r;
#endif
{
  return plotter->circle (plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_circlerel_r (Plotter *plotter, int x, int y, int r)
#else
pl_circlerel_r (plotter, x, y, r)
     Plotter *plotter;
     int x, y, r;
#endif
{
  return plotter->circlerel (plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_closepl_r (Plotter *plotter)
#else
pl_closepl_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->closepl (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_color_r (Plotter *plotter, int red, int green, int blue)
#else
pl_color_r (plotter, red, green, blue)
     Plotter *plotter;
     int red, green, blue;
#endif
{
  return plotter->color (plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_colorname_r (Plotter *plotter, const char *s)
#else
pl_colorname_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->colorname (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_cont_r (Plotter *plotter, int x, int y)
#else
pl_cont_r (plotter, x, y)
     Plotter *plotter;
     int x, y;
#endif
{
  return plotter->cont (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_contrel_r (Plotter *plotter, int x, int y)
#else
pl_contrel_r (plotter, x, y)
     Plotter *plotter;
     int x, y;
#endif
{
  return plotter->contrel (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_ellarc_r (Plotter *plotter, int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_ellarc_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->ellarc (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_ellarcrel_r (Plotter *plotter, int xc, int yc, int x0, int y0, int x1, int y1)
#else
pl_ellarcrel_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->ellarcrel (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_ellipse_r (Plotter *plotter, int x, int y, int rx, int ry, int angle)
#else
pl_ellipse_r (plotter, x, y, rx, ry, angle)
     Plotter *plotter;
     int x, y, rx, ry, angle;
#endif
{
  return plotter->ellipse (plotter, x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_ellipserel_r (Plotter *plotter, int x, int y, int rx, int ry, int angle)
#else
pl_ellipserel_r (plotter, x, y, rx, ry, angle)
     Plotter *plotter;
     int x, y, rx, ry, angle;
#endif
{
  return plotter->ellipserel (plotter, x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_endpath_r (Plotter *plotter)
#else
pl_endpath_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->endpath (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_endsubpath_r (Plotter *plotter)
#else
pl_endsubpath_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->endsubpath (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_erase_r (Plotter *plotter)
#else
pl_erase_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->erase (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_farc_r (Plotter *plotter, double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_farc_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     double xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->farc (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_farcrel_r (Plotter *plotter, double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_farcrel_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     double xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->farcrel (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier2_r (Plotter *plotter, double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fbezier2_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     double xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->fbezier2 (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier2rel_r (Plotter *plotter, double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fbezier2rel_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     double xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->fbezier2rel (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier3_r (Plotter *plotter, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
pl_fbezier3_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3)
     Plotter *plotter;
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return plotter->fbezier3 (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_fbezier3rel_r (Plotter *plotter, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
#else
pl_fbezier3rel_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3)
     Plotter *plotter;
     double x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return plotter->fbezier3rel (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
#ifdef _HAVE_PROTOS
pl_fbox_r (Plotter *plotter, double x0, double y0, double x1, double y1)
#else
pl_fbox_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     double x0, y0, x1, y1;
#endif
{
  return plotter->fbox (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fboxrel_r (Plotter *plotter, double x0, double y0, double x1, double y1)
#else
pl_fboxrel_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     double x0, y0, x1, y1;
#endif
{
  return plotter->fboxrel (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fcircle_r (Plotter *plotter, double x, double y, double r)
#else
pl_fcircle_r (plotter, x, y, r)
     Plotter *plotter;
     double x, y, r;
#endif
{
  return plotter->fcircle (plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_fcirclerel_r (Plotter *plotter, double x, double y, double r)
#else
pl_fcirclerel_r (plotter, x, y, r)
     Plotter *plotter;
     double x, y, r;
#endif
{
  return plotter->fcirclerel (plotter, x, y, r);
}

int
#ifdef _HAVE_PROTOS
pl_fconcat_r (Plotter *plotter, double m0, double m1, double m2, double m3, double m4, double m5)
#else
pl_fconcat_r (plotter, m0, m1, m2, m3, m4, m5)
     Plotter *plotter;
     double m0, m1, m2, m3, m4, m5;
#endif
{
  return plotter->fconcat (plotter, m0, m1, m2, m3, m4, m5);
}

int
#ifdef _HAVE_PROTOS
pl_fcont_r (Plotter *plotter, double x, double y)
#else
pl_fcont_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->fcont (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fcontrel_r (Plotter *plotter, double x, double y)
#else
pl_fcontrel_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->fcontrel (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fellarc_r (Plotter *plotter, double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fellarc_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     double xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->fellarc (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fellarcrel_r (Plotter *plotter, double xc, double yc, double x0, double y0, double x1, double y1)
#else
pl_fellarcrel_r (plotter, xc, yc, x0, y0, x1, y1)
     Plotter *plotter;
     double xc, yc, x0, y0, x1, y1;
#endif
{
  return plotter->fellarcrel (plotter, xc, yc, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fellipse_r (Plotter *plotter, double x, double y, double rx, double ry, double angle)
#else
pl_fellipse_r (plotter, x, y, rx, ry, angle)
     Plotter *plotter;
     double x, y, rx, ry, angle;
#endif
{
  return plotter->fellipse (plotter, x, y, rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
pl_fellipserel_r (Plotter *plotter, double x, double y, double rx, double ry, double angle)
#else
pl_fellipserel_r (plotter, x, y, rx, ry, angle)
     Plotter *plotter;
     double x, y, rx, ry, angle;
#endif
{
  return plotter->fellipserel (plotter, x, y, rx, ry, angle);
}

double
#ifdef _HAVE_PROTOS
pl_ffontname_r (Plotter *plotter, const char *s)
#else
pl_ffontname_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->ffontname (plotter, s);
}

double
#ifdef _HAVE_PROTOS
pl_ffontsize_r (Plotter *plotter, double size)
#else
pl_ffontsize_r (plotter, size)
     Plotter *plotter;
     double size;
#endif
{
  return plotter->ffontsize (plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_fillcolor_r (Plotter *plotter, int red, int green, int blue)
#else
pl_fillcolor_r (plotter, red, green, blue)
     Plotter *plotter;
     int red, green, blue;
#endif
{
  return plotter->fillcolor (plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_fillcolorname_r (Plotter *plotter, const char *s)
#else
pl_fillcolorname_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->fillcolorname (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_fillmod_r (Plotter *plotter, const char *s)
#else
pl_fillmod_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->fillmod (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_filltype_r (Plotter *plotter, int level)
#else
pl_filltype_r (plotter, level)
     Plotter *plotter;
     int level;
#endif
{
  return plotter->filltype (plotter, level);
}

double
#ifdef _HAVE_PROTOS
pl_flabelwidth_r (Plotter *plotter, const char *s)
#else
pl_flabelwidth_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->flabelwidth (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_fline_r (Plotter *plotter, double x0, double y0, double x1, double y1)
#else
pl_fline_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     double x0, y0, x1, y1;
#endif
{
  return plotter->fline (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_flinedash_r (Plotter *plotter, int n, const double *dashes, double offset)
#else
pl_flinedash_r (plotter, n, dashes, offset)
     Plotter *plotter;
     int n;
     const double *dashes;
     double offset;
#endif
{
  return plotter->flinedash (plotter, n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
pl_flinerel_r (Plotter *plotter, double x0, double y0, double x1, double y1)
#else
pl_flinerel_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     double x0, y0, x1, y1;
#endif
{
  return plotter->flinerel (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_flinewidth_r (Plotter *plotter, double size)
#else
pl_flinewidth_r (plotter, size)
     Plotter *plotter;
     double size;
#endif
{
  return plotter->flinewidth (plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_flushpl_r (Plotter *plotter)
#else
pl_flushpl_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->flushpl (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_fmarker_r (Plotter *plotter, double x, double y, int type, double size)
#else
pl_fmarker_r (plotter, x, y, type, size)
     Plotter *plotter;
     double x, y;
     int type;
     double size;
#endif
{
  return plotter->fmarker (plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_fmarkerrel_r (Plotter *plotter, double x, double y, int type, double size)
#else
pl_fmarkerrel_r (plotter, x, y, type, size)
     Plotter *plotter;
     double x, y;
     int type;
     double size;
#endif
{
  return plotter->fmarkerrel (plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_fmiterlimit_r (Plotter *plotter, double limit)
#else
pl_fmiterlimit_r (plotter, limit)
     Plotter *plotter;
     double limit;
#endif
{
  return plotter->fmiterlimit (plotter, limit);
}

int
#ifdef _HAVE_PROTOS
pl_fmove_r (Plotter *plotter, double x, double y)
#else
pl_fmove_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->fmove (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fmoverel_r (Plotter *plotter, double x, double y)
#else
pl_fmoverel_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->fmoverel (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fontname_r (Plotter *plotter, const char *s)
#else
pl_fontname_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->fontname (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_fontsize_r (Plotter *plotter, int size)
#else
pl_fontsize_r (plotter, size)
     Plotter *plotter;
     int size;
#endif
{
  return plotter->fontsize (plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_fpoint_r (Plotter *plotter, double x, double y)
#else
pl_fpoint_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->fpoint (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fpointrel_r (Plotter *plotter, double x, double y)
#else
pl_fpointrel_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->fpointrel (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_frotate_r (Plotter *plotter, double theta)
#else
pl_frotate_r (plotter, theta)
     Plotter *plotter;
     double theta;
#endif
{
  return plotter->frotate (plotter, theta);
}

int
#ifdef _HAVE_PROTOS
pl_fscale_r (Plotter *plotter, double x, double y)
#else
pl_fscale_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->fscale (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_fspace_r (Plotter *plotter, double x0, double y0, double x1, double y1)
#else
pl_fspace_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     double x0, y0, x1, y1;
#endif
{
  return plotter->fspace (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_fspace2_r (Plotter *plotter, double x0, double y0, double x1, double y1, double x2, double y2)
#else
pl_fspace2_r (plotter, x0, y0, x1, y1, x2, y2)
     Plotter *plotter;
     double x0, y0, x1, y1, x2, y2;
#endif
{
  return plotter->fspace2 (plotter, x0, y0, x1, y1, x2, y2);
}

double
#ifdef _HAVE_PROTOS
pl_ftextangle_r (Plotter *plotter, double angle)
#else
pl_ftextangle_r (plotter, angle)
     Plotter *plotter;
     double angle;
#endif
{
  return plotter->ftextangle (plotter, angle);
}

int
#ifdef _HAVE_PROTOS
pl_ftranslate_r (Plotter *plotter, double x, double y)
#else
pl_ftranslate_r (plotter, x, y)
     Plotter *plotter;
     double x, y;
#endif
{
  return plotter->ftranslate (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_havecap_r (Plotter *plotter, const char *s)
#else
pl_havecap_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->havecap (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_joinmod_r (Plotter *plotter, const char *s)
#else
pl_joinmod_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->joinmod (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_label_r (Plotter *plotter, const char *s)
#else
pl_label_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->label (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_labelwidth_r (Plotter *plotter, const char *s)
#else
pl_labelwidth_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->labelwidth (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_line_r (Plotter *plotter, int x0, int y0, int x1, int y1)
#else
pl_line_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     int x0, y0, x1, y1;
#endif
{
  return plotter->line (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_linerel_r (Plotter *plotter, int x0, int y0, int x1, int y1)
#else
pl_linerel_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     int x0, y0, x1, y1;
#endif
{
  return plotter->linerel (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_linewidth_r (Plotter *plotter, int size)
#else
pl_linewidth_r (plotter, size)
     Plotter *plotter;
     int size;
#endif
{
  return plotter->linewidth (plotter, size);
}

int
#ifdef _HAVE_PROTOS
pl_linedash_r (Plotter *plotter, int n, const int *dashes, int offset)
#else
pl_linedash_r (plotter, n, dashes, offset)
     Plotter *plotter;
     int n;
     const int *dashes;
     int offset;
#endif
{
  return plotter->linedash (plotter, n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
pl_linemod_r (Plotter *plotter, const char *s)
#else
pl_linemod_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->linemod (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_marker_r (Plotter *plotter, int x, int y, int type, int size)
#else
pl_marker_r (plotter, x, y, type, size)
     Plotter *plotter;
     int x, y, type, size;
#endif
{
  return plotter->marker (plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_markerrel_r (Plotter *plotter, int x, int y, int type, int size)
#else
pl_markerrel_r (plotter, x, y, type, size)
     Plotter *plotter;
     int x, y, type, size;
#endif
{
  return plotter->markerrel (plotter, x, y, type, size);
}

int
#ifdef _HAVE_PROTOS
pl_move_r (Plotter *plotter, int x, int y)
#else
pl_move_r (plotter, x, y)
     Plotter *plotter;
     int x, y;
#endif
{
  return plotter->move (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_moverel_r (Plotter *plotter, int x, int y)
#else
pl_moverel_r (plotter, x, y)
     Plotter *plotter;
     int x, y;
#endif
{
  return plotter->moverel (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_openpl_r (Plotter *plotter)
#else
pl_openpl_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->openpl (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_orientation_r (Plotter *plotter, int direction)
#else
pl_orientation_r (plotter, direction)
     Plotter *plotter;
     int direction;
#endif
{
  return plotter->orientation (plotter, direction);
}

FILE *
#ifdef _HAVE_PROTOS
pl_outfile_r (Plotter *plotter, FILE *outfile)
#else
pl_outfile_r (plotter, outfile)
     Plotter *plotter;
     FILE *outfile;
#endif
{
  return plotter->outfile (plotter, outfile);
}

int
#ifdef _HAVE_PROTOS
pl_pencolor_r (Plotter *plotter, int red, int green, int blue)
#else
pl_pencolor_r (plotter, red, green, blue)
     Plotter *plotter;
     int red, green, blue;
#endif
{
  return plotter->pencolor (plotter, red, green, blue);
}

int
#ifdef _HAVE_PROTOS
pl_pencolorname_r (Plotter *plotter, const char *s)
#else
pl_pencolorname_r (plotter, s)
     Plotter *plotter;
     const char *s;
#endif
{
  return plotter->pencolorname (plotter, s);
}

int
#ifdef _HAVE_PROTOS
pl_pentype_r (Plotter *plotter, int level)
#else
pl_pentype_r (plotter, level)
     Plotter *plotter;
     int level;
#endif
{
  return plotter->pentype (plotter, level);
}

int
#ifdef _HAVE_PROTOS
pl_point_r (Plotter *plotter, int x, int y)
#else
pl_point_r (plotter, x, y)
     Plotter *plotter;
     int x, y;
#endif
{
  return plotter->point (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_pointrel_r (Plotter *plotter, int x, int y)
#else
pl_pointrel_r (plotter, x, y)
     Plotter *plotter;
     int x, y;
#endif
{
  return plotter->pointrel (plotter, x, y);
}

int
#ifdef _HAVE_PROTOS
pl_restorestate_r (Plotter *plotter)
#else
pl_restorestate_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->restorestate (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_savestate_r (Plotter *plotter)
#else
pl_savestate_r (plotter)
     Plotter *plotter;
#endif
{
  return plotter->savestate (plotter);
}

int
#ifdef _HAVE_PROTOS
pl_space_r (Plotter *plotter, int x0, int y0, int x1, int y1)
#else
pl_space_r (plotter, x0, y0, x1, y1)
     Plotter *plotter;
     int x0, y0, x1, y1;
#endif
{
  return plotter->space (plotter, x0, y0, x1, y1);
}

int
#ifdef _HAVE_PROTOS
pl_space2_r (Plotter *plotter, int x0, int y0, int x1, int y1, int x2, int y2)
#else
pl_space2_r (plotter, x0, y0, x1, y1, x2, y2)
     Plotter *plotter;
     int x0, y0, x1, y1, x2, y2;
#endif
{
  return plotter->space2 (plotter, x0, y0, x1, y1, x2, y2);
}

int
#ifdef _HAVE_PROTOS
pl_textangle_r (Plotter *plotter, int angle)
#else
pl_textangle_r (plotter, angle)
     Plotter *plotter;
     int angle;
#endif
{
  return plotter->textangle (plotter, angle);
}

/* The following four functions are an undocumented part of the libplot
   API.  Each returns a pointer to the head of a list of fonts in
   g_fontdb.c.  They allow applications to extract information about the
   fonts that libplot's Plotters support.

   These functions should be replaced by a formal `font API' for querying
   font names, font metrics, etc. */

#ifdef NO_VOID_SUPPORT
char * 
#else
void *
#endif
#ifdef _HAVE_PROTOS
pl_get_hershey_font_info (Plotter *plotter)
#else
pl_get_hershey_font_info (plotter)
     Plotter *plotter;
#endif
{
  return get_hershey_font_info (plotter);
}

#ifdef NO_VOID_SUPPORT
char * 
#else
void *
#endif
#ifdef _HAVE_PROTOS
pl_get_ps_font_info (Plotter *plotter)
#else
pl_get_ps_font_info (plotter)
     Plotter *plotter;
#endif
{
  return get_ps_font_info (plotter);
}

#ifdef NO_VOID_SUPPORT
char * 
#else
void *
#endif
#ifdef _HAVE_PROTOS
pl_get_pcl_font_info (Plotter *plotter)
#else
pl_get_pcl_font_info (plotter)
     Plotter *plotter;
#endif
{
  return get_pcl_font_info (plotter);
}

#ifdef NO_VOID_SUPPORT
char * 
#else
void *
#endif
#ifdef _HAVE_PROTOS
pl_get_stick_font_info (Plotter *plotter)
#else
pl_get_stick_font_info (plotter)
     Plotter *plotter;
#endif
{
  return get_stick_font_info (plotter);
}



/* END OF WRAPPERS AROUND PLOTTER METHODS */


/* These are two user-callable functions that are specific to the new
   (i.e., thread-safe) C binding: pl_newplparams, pl_deleteplparams,
   pl_copyplparams. */

PlotterParams *
#ifdef _HAVE_PROTOS
pl_newplparams (void)
#else
pl_newplparams ()
#endif
{
  int i;
  PlotterParams *_plotter_params_p;
  
  /* create PlotterParams, copy function pointers to it */
  _plotter_params_p = (PlotterParams *)_plot_xmalloc (sizeof(PlotterParams));
  memcpy (_plotter_params_p, &_default_plotter_params, sizeof(PlotterParams));

  /* null out all parameters */
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    _plotter_params_p->plparams[i] = (voidptr_t)NULL;

  return _plotter_params_p;
}

int
#ifdef _HAVE_PROTOS
pl_deleteplparams (PlotterParams *_plotter_params_p)
#else
pl_deleteplparams (_plotter_params_p)
     PlotterParams *_plotter_params_p;
#endif
{
  int i;
  
  /* free all copied strings, and the structure itself */
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    if (_known_params[i].is_string && _plotter_params_p->plparams[i] != NULL)
      free (_plotter_params_p->plparams[i]);
  free (_plotter_params_p);

  return 0;
}

PlotterParams *
#ifdef _HAVE_PROTOS
pl_copyplparams (const PlotterParams *_plotter_params_p)
#else
pl_copyplparams (_plotter_params_p)
     const PlotterParams *_plotter_params_p;
#endif
{
  int i;
  PlotterParams *new_plotter_params_p;
  
  /* create PlotterParams, copy function pointers to it */
  new_plotter_params_p = (PlotterParams *)_plot_xmalloc (sizeof(PlotterParams));
  memcpy (new_plotter_params_p, &_default_plotter_params, sizeof(PlotterParams));

  /* copy all parameters */
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    new_plotter_params_p->plparams[i] = _plotter_params_p->plparams[i];

  return new_plotter_params_p;
}

/* The following are C wrappers around the public functions in the
   PlotterParams class.  Together with the preceding functions, they are
   part of the new (i.e., thread-safe) C API. */

int
#ifdef _HAVE_PROTOS
pl_setplparam (PlotterParams *plotter_params, const char *parameter, voidptr_t value)
#else
pl_setplparam (plotter_params, parameter, value)
     PlotterParams *plotter_params;
     const char *parameter;
     voidptr_t value;
#endif
{
  return plotter_params->setplparam (plotter_params, parameter, value);
}

/* END OF WRAPPERS AROUND PLOTTERPARAMS METHODS */
