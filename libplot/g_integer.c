/* This file contains the versions of the Plotter methods that (i) take as
   arguments, and/or (ii) return integers rather than doubles. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_arc (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_g_arc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _plotter->farc (R___(_plotter) 
			 (double)xc, (double)yc, 
			 (double)x0, (double)y0, 
			 (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_arcrel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_g_arcrel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->farcrel (R___(_plotter) 
			    (double)dxc, (double)dyc, 
			    (double)dx0, (double)dy0, 
			    (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_bezier2 (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_g_bezier2 (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _plotter->fbezier2 (R___(_plotter) 
			     (double)xc, (double)yc, 
			     (double)x0, (double)y0, 
			     (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_bezier2rel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_g_bezier2rel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fbezier2rel (R___(_plotter) 
				(double)dxc, (double)dyc, 
				(double)dx0, (double)dy0, 
				(double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_bezier3 (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
_g_bezier3 (R___(_plotter) x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return _plotter->fbezier3 (R___(_plotter) 
			     (double)x0, (double)y0, 
			     (double)x1, (double)y1, 
			     (double)x2, (double)y2, 
			     (double)x3, (double)y3);
}

int
#ifdef _HAVE_PROTOS
_g_bezier3rel (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
_g_bezier3rel (R___(_plotter) x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return _plotter->fbezier3rel (R___(_plotter) 
				(double)x0, (double)y0, 
				(double)x1, (double)y1, 
				(double)x2, (double)y2, 
				(double)x3, (double)y3);
}

int
#ifdef _HAVE_PROTOS
_g_box (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_g_box (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  return _plotter->fbox (R___(_plotter) 
			 (double)x0, (double)y0, (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_boxrel (R___(Plotter *_plotter) int dx0, int dy0, int dx1, int dy1)
#else
_g_boxrel (R___(_plotter) dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fboxrel (R___(_plotter) 
			    (double)dx0, (double)dy0, 
			    (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_circle (R___(Plotter *_plotter) int x, int y, int r)
#else
_g_circle (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;) 
     int x, y, r;
#endif
{
  return _plotter->fcircle (R___(_plotter) 
			    (double)x, (double)y, (double)r);
}

int
#ifdef _HAVE_PROTOS
_g_circlerel (R___(Plotter *_plotter) int dx, int dy, int r)
#else
_g_circlerel (R___(_plotter) dx, dy, r)
     S___(Plotter *_plotter;) 
     int dx, dy, r;
#endif
{
  return _plotter->fcirclerel (R___(_plotter) 
			       (double)dx, (double)dy, (double)r);
}

int
#ifdef _HAVE_PROTOS
_g_cont (R___(Plotter *_plotter) int x, int y)
#else
_g_cont (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  return _plotter->fcont (R___(_plotter) 
			  (double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_contrel (R___(Plotter *_plotter) int dx, int dy)
#else
_g_contrel (R___(_plotter) dx, dy)
     S___(Plotter *_plotter;) 
     int dx, dy;
#endif
{
  return _plotter->fcontrel (R___(_plotter) 
			     (double)dx, (double)dy);
}

int
#ifdef _HAVE_PROTOS
_g_ellarc (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_g_ellarc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _plotter->fellarc (R___(_plotter) 
			    (double)xc, (double)yc, 
			    (double)x0, (double)y0, 
			    (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_ellarcrel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_g_ellarcrel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fellarcrel (R___(_plotter) 
			       (double)dxc, (double)dyc, 
			       (double)dx0, (double)dy0, 
			       (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_ellipse (R___(Plotter *_plotter) int x, int y, int rx, int ry, int angle)
#else
_g_ellipse (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;) 
     int x, y, rx, ry, angle;
#endif
{
  return _plotter->fellipse (R___(_plotter) 
			     (double)x, (double)y, 
			     (double)rx, (double)ry, (double)angle);
}

int
#ifdef _HAVE_PROTOS
_g_ellipserel (R___(Plotter *_plotter) int dx, int dy, int rx, int ry, int angle)
#else
_g_ellipserel (R___(_plotter) dx, dy, rx, ry, angle)
     S___(Plotter *_plotter;) 
     int dx, dy, rx, ry, angle;
#endif
{
  return _plotter->fellipserel (R___(_plotter) 
				(double)dx, (double)dy, 
				(double)rx, (double)ry, 
				(double)angle);
}

int 
#ifdef _HAVE_PROTOS
_g_fontname (R___(Plotter *_plotter) const char *s)
#else
_g_fontname (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  double new_size = _plotter->ffontname (R___(_plotter) s);
  
  return IROUND(new_size);
}

int
#ifdef _HAVE_PROTOS
_g_fontsize (R___(Plotter *_plotter) int size)
#else
_g_fontsize (R___(_plotter) size)
     S___(Plotter *_plotter;) 
     int size;
#endif
{
  double new_size = _plotter->ffontsize (R___(_plotter) (double)size);

  return IROUND(new_size);
}

int
#ifdef _HAVE_PROTOS
_g_line (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_g_line (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  return _plotter->fline (R___(_plotter) 
			  (double)x0, (double)y0, 
			  (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_linedash (R___(Plotter *_plotter) int n, const int *dashes, int offset)
#else
_g_linedash (R___(_plotter) n, dashes, offset)
     S___(Plotter *_plotter;) 
     int n;
     const int *dashes;
     int offset;
#endif
{
  double *idashes;
  int i, retval;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "linedash: invalid operation");
      return -1;
    }

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0)
      return -1;

  idashes = (double *)_plot_xmalloc ((unsigned int)n * sizeof(double));
  for (i = 0; i < n; i++)
    idashes[i] = dashes[i];
  retval = _plotter->flinedash (R___(_plotter) 
				n, idashes, (double)offset);
  free (idashes);

  return retval;
}

int
#ifdef _HAVE_PROTOS
_g_linerel (R___(Plotter *_plotter) int dx0, int dy0, int dx1, int dy1)
#else
_g_linerel (R___(_plotter) dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->flinerel (R___(_plotter) 
			     (double)dx0, (double)dy0, 
			     (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_g_labelwidth (R___(Plotter *_plotter) const char *s)
#else
_g_labelwidth (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  double width = _plotter->flabelwidth (R___(_plotter) s);
  
  return IROUND(width);
}

int
#ifdef _HAVE_PROTOS
_g_linewidth (R___(Plotter *_plotter) int new_line_width)
#else
_g_linewidth (R___(_plotter) new_line_width)
     S___(Plotter *_plotter;) 
     int new_line_width;
#endif
{
  return _plotter->flinewidth (R___(_plotter) (double)new_line_width);
}

int
#ifdef _HAVE_PROTOS
_g_marker (R___(Plotter *_plotter) int x, int y, int type, int size)
#else
_g_marker (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;) 
     int x, y;
     int type;
     int size;
#endif
{
  return _plotter->fmarker (R___(_plotter) 
			    (double)x, (double)y, 
			    type, (double)size);
}

int
#ifdef _HAVE_PROTOS
_g_markerrel (R___(Plotter *_plotter) int dx, int dy, int type, int size)
#else
_g_markerrel (R___(_plotter) dx, dy, type, size)
     S___(Plotter *_plotter;) 
     int dx, dy;
     int type;
     int size;
#endif
{
  return _plotter->fmarkerrel (R___(_plotter) 
			       (double)dx, (double)dy, 
			       type, (double)size);
}

int
#ifdef _HAVE_PROTOS
_g_move (R___(Plotter *_plotter) int x, int y)
#else
_g_move (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x,y;
#endif
{
  return _plotter->fmove (R___(_plotter) 
			  (double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_moverel (R___(Plotter *_plotter) int x, int y)
#else
_g_moverel (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  return _plotter->fmoverel (R___(_plotter) 
			     (double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_point (R___(Plotter *_plotter) int x, int y)
#else
_g_point (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  return _plotter->fpoint (R___(_plotter) 
			   (double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_g_pointrel (R___(Plotter *_plotter) int dx, int dy)
#else
_g_pointrel (R___(_plotter) dx, dy)
     S___(Plotter *_plotter;) 
     int dx, dy;
#endif
{
  return _plotter->fpointrel (R___(_plotter) 
			      (double)dx, (double)dy);
}

int
#ifdef _HAVE_PROTOS
_g_space (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_g_space (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  return _plotter->fspace (R___(_plotter) 
			   (double)x0, (double)y0, 
			   (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_g_space2 (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2)
#else
_g_space2 (R___(_plotter) x0, y0, x1, y1, x2, y2)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1, x2, y2;
#endif
{
  return _plotter->fspace2 (R___(_plotter) 
			    (double)x0, (double)y0, 
			    (double)x1, (double)y1, 
			    (double)x2, (double)y2);
}

int
#ifdef _HAVE_PROTOS
_g_textangle (R___(Plotter *_plotter) int angle)
#else
_g_textangle (R___(_plotter) angle)
     S___(Plotter *_plotter;) 
     int angle;
#endif
{
  double new_size = _plotter->ftextangle (R___(_plotter) 
					  (double)angle);

  return IROUND(new_size);
}
