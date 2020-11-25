/* This file contains the libplot methods that take floating-point relative
   coordinates as arguments. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_API_farcrel (R___(Plotter *_plotter) double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
#else
_API_farcrel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;)
     double dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _API_farc (R___(_plotter) 
		    _plotter->drawstate->pos.x + dxc, 
		    _plotter->drawstate->pos.y + dyc,
		    _plotter->drawstate->pos.x + dx0, 
		    _plotter->drawstate->pos.y + dy0,
		    _plotter->drawstate->pos.x + dx1, 
		    _plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_API_fbezier2rel (R___(Plotter *_plotter) double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
#else
_API_fbezier2rel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;)
     double dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _API_fbezier2 (R___(_plotter) 
			_plotter->drawstate->pos.x + dxc, 
			_plotter->drawstate->pos.y + dyc,
			_plotter->drawstate->pos.x + dx0, 
			_plotter->drawstate->pos.y + dy0,
			_plotter->drawstate->pos.x + dx1, 
			_plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_API_fbezier3rel (R___(Plotter *_plotter) double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
#else
_API_fbezier3rel (R___(_plotter) dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3)
     S___(Plotter *_plotter;)
     double dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3;
#endif
{
  return _API_fbezier3 (R___(_plotter) 
			_plotter->drawstate->pos.x + dx0, 
			_plotter->drawstate->pos.y + dy0,
			_plotter->drawstate->pos.x + dx1, 
			_plotter->drawstate->pos.y + dy1,
			_plotter->drawstate->pos.x + dx2,
			_plotter->drawstate->pos.y + dy2,
			_plotter->drawstate->pos.x + dx3, 
			_plotter->drawstate->pos.y + dy3);
}

int
#ifdef _HAVE_PROTOS
_API_fellarcrel (R___(Plotter *_plotter) double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
#else
_API_fellarcrel (R___(_plotter) dxc, dyc, dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;)
     double dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _API_fellarc (R___(_plotter) 
		       _plotter->drawstate->pos.x + dxc, 
		       _plotter->drawstate->pos.y + dyc,
		       _plotter->drawstate->pos.x + dx0, 
		       _plotter->drawstate->pos.y + dy0,
		       _plotter->drawstate->pos.x + dx1, 
		       _plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_API_fboxrel (R___(Plotter *_plotter) double dx0, double dy0, double dx1, double dy1)
#else
_API_fboxrel (R___(_plotter) dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;)
     double dx0, dy0, dx1, dy1;
#endif
{
  return _API_fbox (R___(_plotter) 
		    _plotter->drawstate->pos.x + dx0, 
		    _plotter->drawstate->pos.y + dy0,
		    _plotter->drawstate->pos.x + dx1, 
		    _plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_API_fcirclerel (R___(Plotter *_plotter) double dx, double dy, double r)
#else
_API_fcirclerel (R___(_plotter) dx, dy, r)
     S___(Plotter *_plotter;)
     double dx, dy, r;
#endif
{
  return _API_fcircle (R___(_plotter) 
		       _plotter->drawstate->pos.x + dx, 
		       _plotter->drawstate->pos.y + dy, r);
}

int
#ifdef _HAVE_PROTOS
_API_fellipserel (R___(Plotter *_plotter) double dx, double dy, double rx, double ry, double angle)
#else
_API_fellipserel (R___(_plotter) dx, dy, rx, ry, angle)
     S___(Plotter *_plotter;)
     double dx, dy, rx, ry, angle;
#endif
{
  return _API_fellipse (R___(_plotter) 
			_plotter->drawstate->pos.x + dx, 
			_plotter->drawstate->pos.y + dy, 
			rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
_API_fcontrel (R___(Plotter *_plotter) double dx, double dy)
#else
_API_fcontrel (R___(_plotter) dx, dy)
     S___(Plotter *_plotter;)
     double dx, dy;
#endif
{
  return _API_fcont (R___(_plotter) 
		     _plotter->drawstate->pos.x + dx, 
		     _plotter->drawstate->pos.y + dy);
}

int
#ifdef _HAVE_PROTOS
_API_flinerel (R___(Plotter *_plotter) double dx0, double dy0, double dx1, double dy1)
#else
_API_flinerel (R___(_plotter) dx0, dy0, dx1, dy1)
     S___(Plotter *_plotter;)
     double dx0, dy0, dx1, dy1;
#endif
{
  return _API_fline (R___(_plotter) 
		     _plotter->drawstate->pos.x + dx0, 
		     _plotter->drawstate->pos.y + dy0,
		     _plotter->drawstate->pos.x + dx1, 
		     _plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_API_fmarkerrel (R___(Plotter *_plotter) double dx, double dy, int type, double size)
#else
_API_fmarkerrel (R___(_plotter) dx, dy, type, size)
     S___(Plotter *_plotter;)
     double dx, dy;
     int type;
     double size;
#endif
{
  return _API_fmarker (R___(_plotter) 
		       _plotter->drawstate->pos.x + dx, 
		       _plotter->drawstate->pos.y + dy, 
		       type, size);
}

int
#ifdef _HAVE_PROTOS
_API_fmoverel (R___(Plotter *_plotter) double x, double y)
#else
_API_fmoverel (R___(_plotter) x, y)
     S___(Plotter *_plotter;)
     double x, y;
#endif
{
  return _API_fmove (R___(_plotter) 
		     _plotter->drawstate->pos.x + x, 
		     _plotter->drawstate->pos.y + y);
}

int
#ifdef _HAVE_PROTOS
_API_fpointrel (R___(Plotter *_plotter) double dx, double dy)
#else
_API_fpointrel (R___(_plotter) dx, dy)
     S___(Plotter *_plotter;)
     double dx, dy;
#endif
{
  return _API_fpoint (R___(_plotter) 
		      _plotter->drawstate->pos.x + dx, 
		      _plotter->drawstate->pos.y + dy);
}
