/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes in our X graphics contexts (line type, cap type,
   join type, line width) to what they should be. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* the canonical four non-solid line patterns */

#define DOTTED_LENGTH 2
#define DOTDASHED_LENGTH 4
#define SHORTDASHED_LENGTH 2
#define LONGDASHED_LENGTH 2

#define MAX_DASH_LENGTH 4	/* max of preceding pattern lengths */

/* these on/off bit patterns are those used by the xterm Tektronix
   emulator, except that the emulator seems incorrectly to have on and
   off interchanged */
static const char dotted[DOTTED_LENGTH]		  = { 1, 3 };
static const char dotdashed[DOTDASHED_LENGTH] 	  = { 1, 3, 4, 3 };  
static const char shortdashed[SHORTDASHED_LENGTH] = { 4, 4 };  
static const char longdashed[LONGDASHED_LENGTH]   = { 7, 4 };    

/* N.B. ps4014, the Tek->PS translator in Adobe's Transcript package, uses
   { 1, 2 }, { 1, 2, 8, 2 }, { 2, 2 }, { 12, 2 } instead. */

/* N.B. a genuine Tektronix 4014 (with Enhanced Graphics Module) uses { 1,
   1 }, { 1, 1, 5, 1 }, { 3, 1 }, { 6, 2 }.  See the Tektronix 4014 Service
   Instruction Manual (dated Aug. 1974) for the diode array that produces
   these patterns. */

/* We scale the above bit patterns by (essentially) the line width, but we
   can't scale by too great a factor, because each entry needs to be <= 255
   to fit in a char.  We make the entries <= 127 to be on the safe side. */
#define MAX_SCALE 18

void
#ifdef _HAVE_PROTOS
_x_set_attributes (void)
#else
_x_set_attributes ()
#endif
{
  XGCValues gcv;
  char dashbuf[MAX_DASH_LENGTH];
  int scale, i;

  /* scale by line width in terms of pixels, if nonzero */
  scale = _plotter->drawstate->quantized_device_line_width;
  if (scale <= 0)
    scale = 1;
  if (scale > MAX_SCALE)
    scale = MAX_SCALE;

  switch (_plotter->drawstate->line_type)
    {
    case L_SOLID:
    default:
      gcv.line_style = LineSolid;
      break;
    case L_DOTTED:
      for (i = 0; i < DOTTED_LENGTH; i++)
	dashbuf[i] = scale * dotted[i];
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc_fg, 
		  0, dashbuf, DOTTED_LENGTH);
      gcv.line_style = LineOnOffDash;
      break;
    case L_DOTDASHED:
      for (i = 0; i < DOTDASHED_LENGTH; i++)
	dashbuf[i] = scale * dotdashed[i];
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc_fg, 
		  0, dashbuf, DOTDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      break;
    case L_SHORTDASHED:
      for (i = 0; i < SHORTDASHED_LENGTH; i++)
	dashbuf[i] = scale * shortdashed[i];
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc_fg, 
		  0, dashbuf, SHORTDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      break;
    case L_LONGDASHED:
      for (i = 0; i < LONGDASHED_LENGTH; i++)
	dashbuf[i] = scale * longdashed[i];
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc_fg, 
		  0, dashbuf, LONGDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      break;
    }

  /* in GC, set line cap type */
  switch (_plotter->drawstate->cap_type)
    {
    case CAP_BUTT:
    default:
      gcv.cap_style = CapButt;
      break;
    case CAP_ROUND:
      gcv.cap_style = CapRound;
      break;
    case CAP_PROJECT:
      gcv.cap_style = CapProjecting;
      break;
    }

  /* in GC, set line join type */
  switch (_plotter->drawstate->join_type)
    {
    case JOIN_MITER:
    default:
      gcv.join_style = JoinMiter;
      break;
    case JOIN_ROUND:
      gcv.join_style = JoinRound;
      break;
    case JOIN_BEVEL:
      gcv.join_style = JoinBevel;
      break;
    }

  /* in GC, set line width in device coors (pixels) */
  gcv.line_width = _plotter->drawstate->quantized_device_line_width;

  /* change both our GC's: the one used for drawing, and the one used for
     filling */
  XChangeGC (_plotter->dpy, _plotter->drawstate->gc_fg, 
	     GCLineStyle | GCCapStyle | GCJoinStyle | GCLineWidth, &gcv);
  XChangeGC (_plotter->dpy, _plotter->drawstate->gc_fill, 
	     GCLineStyle | GCCapStyle | GCJoinStyle | GCLineWidth, &gcv);
  
  _handle_x_events();

  return;
}
