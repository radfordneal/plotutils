/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* size of a `point' as fraction of diagonal distance between scaling
   points P1 and P2, i.e. as fraction of distance between opposite corners
   of the graphics display */
#define POINT_HPGL_SIZE 0.0001

int
#ifdef _HAVE_PROTOS
_h_fpoint (double x, double y)
#else
_h_fpoint (x, y)
     double x, y;
#endif
{
  double saved_width;
  int saved_join_type, saved_cap_type;

  if (!_plotter->open)
    {
      _plotter->error ("fpoint: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  /* Sync pen color.  This may set the _plotter->bad_pen flag (if optimal
     pen is #0 [white] and we're not allowed to use pen #0 to draw with).
     So we test _plotter->bad_pen before drawing the point (see below). */
  _plotter->set_pen_color ();

  /* temporarily store pen width and line attributes */
  saved_width = _plotter->drawstate->hpgl_pen_width;
  saved_join_type = _plotter->drawstate->join_type;
  saved_cap_type = _plotter->drawstate->cap_type;  
  _plotter->drawstate->hpgl_pen_width = POINT_HPGL_SIZE;
  _plotter->drawstate->join_type = JOIN_ROUND;
  _plotter->drawstate->cap_type = CAP_ROUND;  

  /* sync line attributes, incl. pen width, and sync pen position */
  _plotter->set_attributes();
  _plotter->set_position();

  if (_plotter->pendown == false && _plotter->bad_pen == false)
    /* if pen were down, point would be invisible */
    {
      strcpy (_plotter->page->point, "PD;");
      _update_buffer (_plotter->page);
      _plotter->pendown = true;
    }

  /* restore line attributes, incl. pen width */
  _plotter->drawstate->hpgl_pen_width = saved_width;
  _plotter->drawstate->join_type = saved_join_type;
  _plotter->drawstate->cap_type = saved_cap_type;

  return 0;
}
