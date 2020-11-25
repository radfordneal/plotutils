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

/* initial size for "_xplotters", the sparse array of XPlotter instances */
#define INITIAL_XPLOTTERS_LEN 4

/* Sparse array of current XPlotter instances, and its size.  In libplotter
   these are not global variables: in extern.h, they are #define'd to be
   static data members of the XPlotter class. */
XPlotter **_xplotters = NULL;
int _xplotters_len = 0;		/* size of array */

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   the XPlotter struct. */
const Plotter _y_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _y_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _x_endpath,  _y_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _x_fbox, _g_fboxrel, _g_fcircle, _g_fcirclerel, _g_fconcat, _x_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _x_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _x_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _x_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _y_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _x_restorestate, _x_savestate, _g_space, _g_space2, _g_textangle,
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
   is invoked, _plotter points (temporarily) to the Plotter that has just
   been created. */

void
#ifdef _HAVE_PROTOS
_y_initialize (void)
#else
_y_initialize ()
#endif
{
  bool open_slot = false;
  int i, j;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _x_initialize ();
#endif

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
  _plotter->y_vanish_on_delete = false;
  _plotter->y_pids = (pid_t *)NULL;
  _plotter->y_num_pids = 0;

  /* initialize certain data members from device driver parameters */

  /* determine whether windows vanish on Plotter deletion */
  {
    const char *vanish_s;

    vanish_s = (const char *)_get_plot_param ("VANISH_ON_DELETE");
    if (strcasecmp (vanish_s, "yes") == 0)
      _plotter->y_vanish_on_delete = true;
    else
      _plotter->y_vanish_on_delete = false;
  }
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_y_terminate (void)
#else
_y_terminate ()
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

  /* remove XPlotter from XPlotter array */
  for (i = 0; i < _xplotters_len; i++)
    if (_xplotters[i] == _plotter)
      {
	_xplotters[i] = (XPlotter *)NULL;
	break;
      }

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _x_terminate ();
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
}

XPlotter::XPlotter (ostream& out)
	: XDrawablePlotter (out)
{
  _y_initialize ();
}

XPlotter::XPlotter ()
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
