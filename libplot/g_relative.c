/* This file contains the libplot methods that take floating-point relative
   coordinates as arguments. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_farcrel (double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
#else
_g_farcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     double dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->farc ((_plotter->drawstate->pos).x + dxc, 
			 (_plotter->drawstate->pos).y + dyc,
			 (_plotter->drawstate->pos).x + dx0, 
			 (_plotter->drawstate->pos).y + dy0,
			 (_plotter->drawstate->pos).x + dx1, 
			 (_plotter->drawstate->pos).y + dy1);
}

int
#ifdef _HAVE_PROTOS
_g_fbezier2rel (double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
#else
_g_fbezier2rel (dxc, dyc, dx0, dy0, dx1, dy1)
     double dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fbezier2 ((_plotter->drawstate->pos).x + dxc, 
			     (_plotter->drawstate->pos).y + dyc,
			     (_plotter->drawstate->pos).x + dx0, 
			     (_plotter->drawstate->pos).y + dy0,
			     (_plotter->drawstate->pos).x + dx1, 
			     (_plotter->drawstate->pos).y + dy1);
}

int
#ifdef _HAVE_PROTOS
_g_fbezier3rel (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
#else
_g_fbezier3rel (dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3)
     double dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3;
#endif
{
  return _plotter->fbezier3 ((_plotter->drawstate->pos).x + dx0, 
			     (_plotter->drawstate->pos).y + dy0,
			     (_plotter->drawstate->pos).x + dx1, 
			     (_plotter->drawstate->pos).y + dy1,
			     (_plotter->drawstate->pos).x + dx2,
			     (_plotter->drawstate->pos).y + dy2,
			     (_plotter->drawstate->pos).x + dx3, 
			     (_plotter->drawstate->pos).y + dy3);
}

int
#ifdef _HAVE_PROTOS
_g_fellarcrel (double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
#else
_g_fellarcrel (dxc, dyc, dx0, dy0, dx1, dy1)
     double dxc, dyc, dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fellarc (_plotter->drawstate->pos.x + dxc, 
			    _plotter->drawstate->pos.y + dyc,
			    _plotter->drawstate->pos.x + dx0, 
			    _plotter->drawstate->pos.y + dy0,
			    _plotter->drawstate->pos.x + dx1, 
			    _plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_g_fboxrel (double dx0, double dy0, double dx1, double dy1)
#else
_g_fboxrel (dx0, dy0, dx1, dy1)
     double dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fbox (_plotter->drawstate->pos.x + dx0, 
			 _plotter->drawstate->pos.y + dy0,
			 _plotter->drawstate->pos.x + dx1, 
			 _plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_g_fcirclerel (double dx, double dy, double r)
#else
_g_fcirclerel (dx, dy, r)
     double dx, dy, r;
#endif
{
  return _plotter->fcircle (_plotter->drawstate->pos.x + dx, 
			    _plotter->drawstate->pos.y + dy, r);
}

int
#ifdef _HAVE_PROTOS
_g_fellipserel (double dx, double dy, double rx, double ry, double angle)
#else
_g_fellipserel (dx, dy, rx, ry, angle)
     double dx, dy, rx, ry, angle;
#endif
{
  return _plotter->fellipse (_plotter->drawstate->pos.x + dx, 
			     _plotter->drawstate->pos.y + dy, 
			     rx, ry, angle);
}

int
#ifdef _HAVE_PROTOS
_g_fcontrel (double dx, double dy)
#else
_g_fcontrel (dx, dy)
     double dx, dy;
#endif
{
  return _plotter->fcont (_plotter->drawstate->pos.x + dx, 
			  _plotter->drawstate->pos.y + dy);
}

int
#ifdef _HAVE_PROTOS
_g_flinerel (double dx0, double dy0, double dx1, double dy1)
#else
_g_flinerel (dx0, dy0, dx1, dy1)
     double dx0, dy0, dx1, dy1;
#endif
{
  return _plotter->fline (_plotter->drawstate->pos.x + dx0, 
			  _plotter->drawstate->pos.y + dy0,
			  _plotter->drawstate->pos.x + dx1, 
			  _plotter->drawstate->pos.y + dy1);
}

int
#ifdef _HAVE_PROTOS
_g_fmarkerrel (double dx, double dy, int type, double size)
#else
_g_fmarkerrel (dx, dy, type, size)
     double dx, dy;
     int type;
     double size;
#endif
{
  return _plotter->fmarker (_plotter->drawstate->pos.x + dx, 
			    _plotter->drawstate->pos.y + dy, 
			    type, size);
}

int
#ifdef _HAVE_PROTOS
_g_fmoverel (double x, double y)
#else
_g_fmoverel (x, y)
     double x, y;
#endif
{
  return _plotter->fmove (_plotter->drawstate->pos.x + x, 
			  _plotter->drawstate->pos.y + y);
}

int
#ifdef _HAVE_PROTOS
_g_fpointrel (double dx, double dy)
#else
_g_fpointrel (dx, dy)
     double dx, dy;
#endif
{
  return _plotter->fpoint (_plotter->drawstate->pos.x + dx, 
			   _plotter->drawstate->pos.y + dy);
}
