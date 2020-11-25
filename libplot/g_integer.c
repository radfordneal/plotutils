/* This file contains the versions of the Plotter methods that (i) take as
   arguments, and/or (ii) return integers rather than doubles. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_API_arc (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_API_arc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _API_farc (R___(_plotter) 
		    (double)xc, (double)yc, 
		    (double)x0, (double)y0, 
		    (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_API_arcrel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_API_arcrel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _API_farcrel (R___(_plotter) 
		       (double)dxc, (double)dyc, 
		       (double)dx0, (double)dy0, 
		       (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_API_bezier2 (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_API_bezier2 (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _API_fbezier2 (R___(_plotter) 
			(double)xc, (double)yc, 
			(double)x0, (double)y0, 
			(double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_API_bezier2rel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_API_bezier2rel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _API_fbezier2rel (R___(_plotter) 
			   (double)dxc, (double)dyc, 
			   (double)dx0, (double)dy0, 
			   (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_API_bezier3 (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
_API_bezier3 (R___(_plotter) x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return _API_fbezier3 (R___(_plotter) 
			(double)x0, (double)y0, 
			(double)x1, (double)y1, 
			(double)x2, (double)y2, 
			(double)x3, (double)y3);
}

int
#ifdef _HAVE_PROTOS
_API_bezier3rel (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
#else
_API_bezier3rel (R___(_plotter) x0, y0, x1, y1, x2, y2, x3, y3)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1, x2, y2, x3, y3;
#endif
{
  return _API_fbezier3rel (R___(_plotter) 
			   (double)x0, (double)y0, 
			   (double)x1, (double)y1, 
			   (double)x2, (double)y2, 
			   (double)x3, (double)y3);
}

int
#ifdef _HAVE_PROTOS
_API_box (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_API_box (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  return _API_fbox (R___(_plotter) 
		    (double)x0, (double)y0, (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_API_boxrel (R___(Plotter *_plotter) int dx0, int dy0, int dx1, int dy1)
#else
_API_boxrel (R___(_plotter) dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dx0, dy0, dx1, dy1;
#endif
{
  return _API_fboxrel (R___(_plotter) 
		       (double)dx0, (double)dy0, 
		       (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_API_circle (R___(Plotter *_plotter) int x, int y, int r)
#else
_API_circle (R___(_plotter) x, y, r)
     S___(Plotter *_plotter;) 
     int x, y, r;
#endif
{
  return _API_fcircle (R___(_plotter) 
		       (double)x, (double)y, (double)r);
}

int
#ifdef _HAVE_PROTOS
_API_circlerel (R___(Plotter *_plotter) int dx, int dy, int r)
#else
_API_circlerel (R___(_plotter) dx, dy, r)
     S___(Plotter *_plotter;) 
     int dx, dy, r;
#endif
{
  return _API_fcirclerel (R___(_plotter) 
			  (double)dx, (double)dy, (double)r);
}

int
#ifdef _HAVE_PROTOS
_API_cont (R___(Plotter *_plotter) int x, int y)
#else
_API_cont (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  return _API_fcont (R___(_plotter) 
		     (double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_API_contrel (R___(Plotter *_plotter) int dx, int dy)
#else
_API_contrel (R___(_plotter) dx, dy)
     S___(Plotter *_plotter;) 
     int dx, dy;
#endif
{
  return _API_fcontrel (R___(_plotter) 
			(double)dx, (double)dy);
}

int
#ifdef _HAVE_PROTOS
_API_ellarc (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
#else
_API_ellarc (R___(_plotter) xc, yc, x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int xc, yc, x0, y0, x1, y1;
#endif
{
  return _API_fellarc (R___(_plotter) 
		       (double)xc, (double)yc, 
		       (double)x0, (double)y0, 
		       (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_API_ellarcrel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
#else
_API_ellarcrel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _API_fellarcrel (R___(_plotter) 
			  (double)dxc, (double)dyc, 
			  (double)dx0, (double)dy0, 
			  (double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_API_ellipse (R___(Plotter *_plotter) int x, int y, int rx, int ry, int angle)
#else
_API_ellipse (R___(_plotter) x, y, rx, ry, angle)
     S___(Plotter *_plotter;) 
     int x, y, rx, ry, angle;
#endif
{
  return _API_fellipse (R___(_plotter) 
			(double)x, (double)y, 
			(double)rx, (double)ry, (double)angle);
}

int
#ifdef _HAVE_PROTOS
_API_ellipserel (R___(Plotter *_plotter) int dx, int dy, int rx, int ry, int angle)
#else
_API_ellipserel (R___(_plotter) dx, dy, rx, ry, angle)
     S___(Plotter *_plotter;) 
     int dx, dy, rx, ry, angle;
#endif
{
  return _API_fellipserel (R___(_plotter) 
			   (double)dx, (double)dy, 
			   (double)rx, (double)ry, 
			   (double)angle);
}

int 
#ifdef _HAVE_PROTOS
_API_fontname (R___(Plotter *_plotter) const char *s)
#else
_API_fontname (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  double new_size = _API_ffontname (R___(_plotter) s);
  
  return IROUND(new_size);
}

int
#ifdef _HAVE_PROTOS
_API_fontsize (R___(Plotter *_plotter) int size)
#else
_API_fontsize (R___(_plotter) size)
     S___(Plotter *_plotter;) 
     int size;
#endif
{
  double new_size = _API_ffontsize (R___(_plotter) (double)size);

  return IROUND(new_size);
}

int
#ifdef _HAVE_PROTOS
_API_line (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_API_line (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  return _API_fline (R___(_plotter) 
		     (double)x0, (double)y0, 
		     (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_API_linedash (R___(Plotter *_plotter) int n, const int *dashes, int offset)
#else
_API_linedash (R___(_plotter) n, dashes, offset)
     S___(Plotter *_plotter;) 
     int n;
     const int *dashes;
     int offset;
#endif
{
  double *idashes;
  int i, retval;

  if (!_plotter->data->open)
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
  retval = _API_flinedash (R___(_plotter) 
			   n, idashes, (double)offset);
  free (idashes);

  return retval;
}

int
#ifdef _HAVE_PROTOS
_API_linerel (R___(Plotter *_plotter) int dx0, int dy0, int dx1, int dy1)
#else
_API_linerel (R___(_plotter) dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;) 
     int dx0, dy0, dx1, dy1;
#endif
{
  return _API_flinerel (R___(_plotter) 
			(double)dx0, (double)dy0, 
			(double)dx1, (double)dy1);
}

int
#ifdef _HAVE_PROTOS
_API_labelwidth (R___(Plotter *_plotter) const char *s)
#else
_API_labelwidth (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  double width = _API_flabelwidth (R___(_plotter) s);
  
  return IROUND(width);
}

int
#ifdef _HAVE_PROTOS
_API_linewidth (R___(Plotter *_plotter) int new_line_width)
#else
_API_linewidth (R___(_plotter) new_line_width)
     S___(Plotter *_plotter;) 
     int new_line_width;
#endif
{
  return _API_flinewidth (R___(_plotter) (double)new_line_width);
}

int
#ifdef _HAVE_PROTOS
_API_marker (R___(Plotter *_plotter) int x, int y, int type, int size)
#else
_API_marker (R___(_plotter) x, y, type, size)
     S___(Plotter *_plotter;) 
     int x, y;
     int type;
     int size;
#endif
{
  return _API_fmarker (R___(_plotter) 
		       (double)x, (double)y, 
		       type, (double)size);
}

int
#ifdef _HAVE_PROTOS
_API_markerrel (R___(Plotter *_plotter) int dx, int dy, int type, int size)
#else
_API_markerrel (R___(_plotter) dx, dy, type, size)
     S___(Plotter *_plotter;) 
     int dx, dy;
     int type;
     int size;
#endif
{
  return _API_fmarkerrel (R___(_plotter) 
			  (double)dx, (double)dy, 
			  type, (double)size);
}

int
#ifdef _HAVE_PROTOS
_API_move (R___(Plotter *_plotter) int x, int y)
#else
_API_move (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x,y;
#endif
{
  return _API_fmove (R___(_plotter) 
		     (double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_API_moverel (R___(Plotter *_plotter) int x, int y)
#else
_API_moverel (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  return _API_fmoverel (R___(_plotter) 
			(double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_API_point (R___(Plotter *_plotter) int x, int y)
#else
_API_point (R___(_plotter) x, y)
     S___(Plotter *_plotter;) 
     int x, y;
#endif
{
  return _API_fpoint (R___(_plotter) 
		      (double)x, (double)y);
}

int
#ifdef _HAVE_PROTOS
_API_pointrel (R___(Plotter *_plotter) int dx, int dy)
#else
_API_pointrel (R___(_plotter) dx, dy)
     S___(Plotter *_plotter;) 
     int dx, dy;
#endif
{
  return _API_fpointrel (R___(_plotter) 
			 (double)dx, (double)dy);
}

int
#ifdef _HAVE_PROTOS
_API_space (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_API_space (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  return _API_fspace (R___(_plotter) 
		      (double)x0, (double)y0, 
		      (double)x1, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_API_space2 (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2)
#else
_API_space2 (R___(_plotter) x0, y0, x1, y1, x2, y2)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1, x2, y2;
#endif
{
  return _API_fspace2 (R___(_plotter) 
		       (double)x0, (double)y0, 
		       (double)x1, (double)y1, 
		       (double)x2, (double)y2);
}

int
#ifdef _HAVE_PROTOS
_API_textangle (R___(Plotter *_plotter) int angle)
#else
_API_textangle (R___(_plotter) angle)
     S___(Plotter *_plotter;) 
     int angle;
#endif
{
  double new_size = _API_ftextangle (R___(_plotter) 
				     (double)angle);

  return IROUND(new_size);
}
