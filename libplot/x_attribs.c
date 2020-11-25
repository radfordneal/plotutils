/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes in our X graphics context (line type, cap type,
   join type, line width) to what they should be. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_x_set_attributes (void)
#else
_x_set_attributes ()
#endif
{
  XGCValues gcv;

  /* the canonical four non-solid line patterns */

#define DOTTED_LENGTH 2
#define DOTDASHED_LENGTH 4
#define SHORTDASHED_LENGTH 2
#define LONGDASHED_LENGTH 2
  
  /* these on/off bit patterns are those used by the xterm Tektronix
     emulator, except that the emulator seems incorrectly to have on and
     off interchanged */
  static char dotted[DOTTED_LENGTH]		= { 1, 3 };
  static char dotdashed[DOTDASHED_LENGTH] 	= { 1, 3, 4, 3 };  
  static char shortdashed[SHORTDASHED_LENGTH] 	= { 4, 4 };  
  static char longdashed[LONGDASHED_LENGTH] 	= { 7, 4 };    
  
  /* N.B. ps4014, the Tek->PS translator in Adobe's Transcript package,
     uses { 1, 2 }, { 1, 2, 8, 2 }, { 2, 2 }, { 12, 2 } instead. */

  /* N.B. a genuine Tektronix 4014 (with Enhanced Graphics Module) uses
     { 1, 1 }, { 1, 1, 5, 1 }, { 3, 1 }, { 6, 2 }.  See the Tektronix 4014
     Service Instruction Manual (dated Aug. 1974) for the diode array that
     produces these patterns. */

  switch (_plotter->drawstate->line_type)
    {
    case L_SOLID:
    default:
      gcv.line_style = LineSolid;
      break;
    case L_DOTTED:
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc, 
		  0, dotted, DOTTED_LENGTH);
      gcv.line_style = LineOnOffDash;
      break;
    case L_DOTDASHED:
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc, 
		  0, dotdashed, DOTDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      break;
    case L_SHORTDASHED:
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc, 
		  0, shortdashed, SHORTDASHED_LENGTH);
      gcv.line_style = LineOnOffDash;
      break;
    case L_LONGDASHED:
      XSetDashes (_plotter->dpy, _plotter->drawstate->gc, 
		  0, longdashed, LONGDASHED_LENGTH);
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

  XChangeGC (_plotter->dpy, _plotter->drawstate->gc, 
	     GCLineStyle | GCCapStyle | GCJoinStyle | GCLineWidth, &gcv);
  
  _handle_x_events();

  return;
}
