/* This file contains the cont() method, which is a standard part of the
   libplot.  It continues a line from the current position of the graphics
   cursor to the point specified by x and y.

   This method is used in the construction of paths.  By repeatedly
   invoking cont(), the user may construct a polyline of arbitrary length.
   arc() and ellarc() may also be invoked, to add circular or elliptic arc
   elements to the path.  The path will terminate when the user either

     (1) explicitly invokes the endpath() method, or 
     (2) changes the value of one of the relevant drawing attributes, 
          by invoking move(), linemod(), linewidth(), color(), fillcolor(),
          or filltype(), or 
     (3) draws some non-path object, by invoking box(), 
           circle(), point(), label(), alabel(), etc., or 
     (4) invokes restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "extern.h"

#define DATAPOINTS_BUFSIZ 64	/* initial size */

/* for Cohen-Sutherland clipper */
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

int
#ifdef _HAVE_PROTOS
_t_fcont (double x, double y)
#else
_t_fcont (x, y)
     double x, y;
#endif
{
  Point start, end;		/* endpoints of line seg. (in device coors) */
  IntPoint istart, iend;	/* same, quantized to integer Tek coors */
  bool force;
  int oldindex, newindex;
  int clipval;
  int retval;

  if (!_plotter->open)
    {
      _plotter->error ("fcont: invalid operation");
      return -1;
    }

  /* Invoke generic method.  This builds up the path by storing the point
     internally, with the storage deleted when endpath() is invoked.
     However, in any TekPlotter, the value of the `flush_long_polylines'
     data member is false.  This keeps the generic method from calling
     endpath() when the stored polyline gets too long. */
  retval = _g_fcont (x, y);

  /* Draw new line segment on Tektronix display, in real time */

  /* Skip drawing it if the pen color is white.  Since our TekPlotter class
     doesn't support filling, this is ok to do if the Tektronix isn't a
     kermit emulator (the kermit emulator supports color). */
  if (_plotter->tek_display_type != D_KERMIT 
      && _plotter->drawstate->fgcolor.red == 0xffff
      && _plotter->drawstate->fgcolor.green == 0xffff
      && _plotter->drawstate->fgcolor.blue == 0xffff)
    return retval;

  /* nominal starting point and ending point for new line segment, in
     floating point device coordinates */
  oldindex = _plotter->drawstate->points_in_path - 2;
  newindex = oldindex + 1;
  start.x = XD(_plotter->drawstate->datapoints[oldindex].x,
	       _plotter->drawstate->datapoints[oldindex].y);
  start.y = YD(_plotter->drawstate->datapoints[oldindex].x,
	       _plotter->drawstate->datapoints[oldindex].y);
  end.x = XD(_plotter->drawstate->datapoints[newindex].x,
	     _plotter->drawstate->datapoints[newindex].y);
  end.y = YD(_plotter->drawstate->datapoints[newindex].x,
	     _plotter->drawstate->datapoints[newindex].y);

  /* clip line segment to rectangular clipping region in device frame */
  clipval = _clip_line (&start.x, &start.y, &end.x, &end.y,
			TEK_DEVICE_X_MIN_CLIP, TEK_DEVICE_X_MAX_CLIP,
			TEK_DEVICE_Y_MIN_CLIP, TEK_DEVICE_Y_MAX_CLIP);
  if (!(clipval & ACCEPTED))	/* line segment is OOB */
    return retval;

  /* convert clipped starting point, ending point to integer Tek coors */
  istart.x = IROUND (start.x);
  istart.y = IROUND (start.y);
  iend.x = IROUND (end.x);
  iend.y = IROUND (end.y);

  if (oldindex == 0)
    /* New polyline is beginning, so start to draw it on the display: move
     to starting point of the first line segment, in Tek space.  As a
     side-effect, the escape sequence emitted by _tek_move() will shift the
     Tekronix to the desired mode, either PLOT or POINT.  Note that if we
     are already in the desired mode, emitting the escape sequence will
     prevent a line being drawn at the time of the move (the "dark vector"
     concept).  That is of course what we want. */
    _tek_move (istart.x, istart.y);
  else
    /* A polyline is underway, >=1 line segments already.  So check whether
       the position on the Tektronix is the same as the starting point of
       the new line segment; if it differs, move to the latter.  Such a
       difference can occur on account of clipping.  Also the Tektronix
       position could have changed on us if a savestate()...restorestate()
       occurred since the last call to cont(). */
    {
      int correct_tek_mode = 
	_plotter->drawstate->points_are_connected ? MODE_PLOT : MODE_POINT;

      if (_plotter->tek_position_is_unknown
	  || (_plotter->tek_pos.x != istart.x)
	  || (_plotter->tek_pos.y != istart.y)
	  || (_plotter->tek_mode_is_unknown)
	  || (_plotter->tek_mode != correct_tek_mode))
	/* Move to desired position.  This automatically shifts the
	   Tektronix to correct mode, PLOT or POINT; see comment above. */
	_tek_move (istart.x, istart.y);
    }

  /* Sync Tek's linestyle with ours; an escape sequence is emitted only if
     necessary.  Linestyle could have changed on us if a
     savestate()...restorestate() occurred since the last call to cont().
     Sync Tek's color too (significant only for kermit Tek emulator). */
  _plotter->set_attributes();  
  _plotter->set_pen_color();

  /* Be sure the background color is correct too.  A background color may
     be set in openpl() from the BG_COLOR parameter, but unless we do the
     following, it won't take effect until erase() is invoked. */
  _plotter->set_bg_color();     

  /* If this is the initial line segment of a polyline, force output of a
  vector even if the line segment has zero length, so that something
  visible will show up on the Tek display.  We do this only if the cap mode
  isn't "butt"; if it is, we don't draw anything. */
  if (oldindex == 0 && _plotter->drawstate->cap_type != CAP_BUTT)
    force = true;
  else 
    force = false;

  /* continue polyline by drawing vector on Tek display */
  _tek_vector_compressed (iend.x, iend.y, istart.x, istart.y, force);
      
  /* update our notion of Tek's notion of position */
  _plotter->tek_pos.x = iend.x;
  _plotter->tek_pos.y = iend.y;

  return retval;
}
