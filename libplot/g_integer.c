/* This file contains the versions of the Plotter methods that (i) take as
   arguments, and/or (ii) return integers rather than doubles. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_arc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
_g_arc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _plotter->farc ((double)xc, (double)yc, 
				    (double)x0, (double)y0, 
				    (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_arcrel (int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_g_arcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->farcrel ((double)dxc, (double)dyc, 
				       (double)dx0, (double)dy0, 
				       (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_box (int x0, int y0, int x1, int y1)
#else
_g_box (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  return _plotter->fbox ((double)x0, (double)y0, (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_boxrel (int dx0, int dy0, int dx1, int dy1)
#else
_g_boxrel (dx0, dy0, dx1, dy1)
     int dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fboxrel ((double)dx0, (double)dy0, 
				       (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_circle (int x, int y, int r)
#else
_g_circle (x, y, r)
     int x, y, r;
#endif
{
  return _plotter->fcircle ((double)x, (double)y, (double)r);
}

int
#ifdef _HAVE_PROTOS
_g_circlerel (int dx, int dy, int r)
#else
_g_circlerel (dx, dy, r)
     int dx, dy, r;
#endif
{
  return _plotter->fcirclerel ((double)dx, (double)dy, (double)r);
}

int
#ifdef _HAVE_PROTOS
_g_cont (int x, int y)
#else
_g_cont (x, y)
     int x, y;
#endif
{
  return _plotter->fcont ((double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_contrel (int dx, int dy)
#else
_g_contrel (dx, dy)
     int dx, dy;
#endif
{
  return _plotter->fcontrel ((double)dx, (double)dy);
}

#ifdef _HAVE_PROTOS
int
_g_ellarc (int xc, int yc, int x0, int y0, int x1, int y1)
#else
_g_ellarc (xc, yc, x0, y0, x1, y1)
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _plotter->fellarc ((double)xc, (double)yc, 
				       (double)x0, (double)y0, 
				       (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_ellarcrel (int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_g_ellarcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fellarcrel ((double)dxc, (double)dyc, 
					  (double)dx0, (double)dy0, 
					  (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_ellipse (int x, int y, int rx, int ry, int angle)
#else
_g_ellipse (x, y, rx, ry, angle)
     int x, y, rx, ry, angle;
#endif
{
  return _plotter->fellipse ((double)x, (double)y, 
					(double)rx, (double)ry, (double)angle);
}

int
#ifdef _HAVE_PROTOS
_g_ellipserel (int dx, int dy, int rx, int ry, int angle)
#else
_g_ellipserel (dx, dy, rx, ry, angle)
     int dx, dy, rx, ry, angle;
#endif
{
  return _plotter->fellipserel ((double)dx, (double)dy, 
					   (double)rx, (double)ry, 
					   (double)angle);
}

int 
#ifdef _HAVE_PROTOS
_g_fontname (const char *s)
#else
_g_fontname (s)
     const char *s;
#endif
{
  double new_size = _plotter->ffontname (s);
  
  return IROUND(new_size);
}

int
#ifdef _HAVE_PROTOS
_g_fontsize (int size)
#else
_g_fontsize (size)
     int size;
#endif
{
  double new_size = _plotter->ffontsize ((double)size);

  return IROUND(new_size);
}

int
#ifdef _HAVE_PROTOS
_g_line (int x0, int y0, int x1, int y1)
#else
_g_line (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  return _plotter->fline ((double)x0, (double)y0, 
				     (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_linerel (int dx0, int dy0, int dx1, int dy1)
#else
_g_linerel (dx0, dy0, dx1, dy1)
     int dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->flinerel ((double)dx0, (double)dy0, 
					(double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_labelwidth (const char *s)
#else
_g_labelwidth (s)
     const char *s;
#endif
{
  double width = _plotter->flabelwidth (s);
  
  return IROUND(width);
}

int
#ifdef _HAVE_PROTOS
_g_linewidth (int new_line_width)
#else
_g_linewidth (new_line_width)
     int new_line_width;
#endif
{
  return _plotter->flinewidth ((double)new_line_width);
}

int
#ifdef _HAVE_PROTOS
_g_marker (int x, int y, int type, int size)
#else
_g_marker (x, y, type, size)
     int x, y;
     int type;
     int size;
#endif
{
  return _plotter->fmarker ((double)x, (double)y, 
				       type, (double)size);
}

int
#ifdef _HAVE_PROTOS
_g_markerrel (int dx, int dy, int type, int size)
#else
_g_markerrel (dx, dy, type, size)
     int dx, dy;
     int type;
     int size;
#endif
{
  return _plotter->fmarkerrel ((double)dx, (double)dy, 
					  type, (double)size);
}

int
#ifdef _HAVE_PROTOS
_g_move (int x, int y)
#else
_g_move (x, y)
     int x,y;
#endif
{
  return _plotter->fmove ((double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_moverel (int x, int y)
#else
_g_moverel (x, y)
     int x, y;
#endif
{
  return _plotter->fmoverel ((double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_point (int x, int y)
#else
_g_point (x, y)
     int x, y;
#endif
{
  return _plotter->fpoint ((double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_pointrel (int dx, int dy)
#else
_g_pointrel (dx, dy)
     int dx, dy;
#endif
{
  return _plotter->fpointrel ((double)dx, (double)dy);
}

int
#ifdef _HAVE_PROTOS
_g_space (int x0, int y0, int x1, int y1)
#else
_g_space (x0, y0, x1, y1)
     int x0, y0, x1, y1;
#endif
{
  return _plotter->fspace ((double)x0, (double)y0, 
				      (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_space2 (int x0, int y0, int x1, int y1, int x2, int y2)
#else
_g_space2 (x0, y0, x1, y1, x2, y2)
     int x0, y0, x1, y1, x2, y2;
#endif
{
  return _plotter->fspace2 ((double)x0, (double)y0, 
				       (double)x1, (double)y1, 
				       (double)x2, (double)y2);
}

int
#ifdef _HAVE_PROTOS
_g_textangle (int angle)
#else
_g_textangle (angle)
     int angle;
#endif
{
  double new_size = _plotter->ftextangle ((double)angle);

  return IROUND(new_size);
}
