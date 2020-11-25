/* This file contains the cont() routine, which is a standard part of
   libplot.  It continues a line from the last point plotted to the point
   specified by x and y.  If no point was just plotted, it plots one before
   continuing the line.

   This routine is used in the construction of polyline objects.  By
   repeatedly calling cont(), the user may construct a polyline of
   arbitrary length.  The polyline will terminate when the user either

     (1) makes an explict call to the endpoly() routine, or 
     (2) changes the value of one of the relevant drawing attributes, 
          by calling move(), linemod(), linewidth(), color(), fillcolor(),
          or fill(), or 
     (3) draws some non-polyline object, by calling arc(), box(), 
           circle(), point(), label(), or alabel(), or 
     (4) calls restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define DATAPOINTS_BUFSIZ 64	/* initial size */

/* for Cohen-Sutherland clipper */
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

/* ignored in this version of libplot, but some apps assume this var exists */
int _libplot_max_unfilled_polyline_length = 0;

int
fcont (x, y)
     double x, y;
{
  Point start, end;		/* endpoints of line seg. (in device coors) */
  IntPoint istart, iend;	/* same, quantized to integer Tek coors */
  int clipval;

  if (!_grdevice_open)
    {
      fprintf (stderr, "libplot: fcont() called when graphics device not open\n");
      return -1;
    }

  /* create or adjust size of polyline buffer, as needed */

  if (_drawstate->datapoints_len == 0)
    {
      _drawstate->datapoints = (Point *) 
	_plot_xmalloc (DATAPOINTS_BUFSIZ * sizeof(Point));
      _drawstate->datapoints_len = DATAPOINTS_BUFSIZ;
    }
  
  if (_drawstate->PointsInLine == _drawstate->datapoints_len)
    {
      _drawstate->datapoints = (Point *) 
	_plot_xrealloc (_drawstate->datapoints, 
			2 * _drawstate->datapoints_len * sizeof(Point));
      _drawstate->datapoints_len *= 2;
    }
  
  /* analyse the present situation */

  if (_drawstate->PointsInLine == 0)
    /* no polyline in progress, so begin one (at current position) */
    {
      _drawstate->datapoints[0].x = _drawstate->pos.x;
      _drawstate->datapoints[0].y = _drawstate->pos.y;
      _drawstate->PointsInLine++;
    }

  /* add new point to internal polyline buffer, so that PointsInLine >= 2 */
  _drawstate->datapoints[_drawstate->PointsInLine].x = x;
  _drawstate->datapoints[_drawstate->PointsInLine].y = y;
  _drawstate->PointsInLine++;
  
  /* update our notion of position */
  (_drawstate->pos).x = x;
  (_drawstate->pos).y = y;

  /* Skip drawing a polyline on the (monochrome) Tektronix display if the
     pen color is white.  Since libplottek doesn't support filling, this is
     a legitimate thing to do if the Tektronix isn't a kermit emulator
     (the kermit emulator supports color). */

  if (_tek_display_type != D_KERMIT 
      && _drawstate->fgcolor.red == 0xffff
      && _drawstate->fgcolor.green == 0xffff
      && _drawstate->fgcolor.blue == 0xffff)
    return 0;

  /* nominal starting point and ending point for new line segment, in
     floating point device coordinates */
  start.x = XD(_drawstate->datapoints[_drawstate->PointsInLine - 2].x,_drawstate->datapoints[_drawstate->PointsInLine - 2].y);
  start.y = YD(_drawstate->datapoints[_drawstate->PointsInLine - 2].x,_drawstate->datapoints[_drawstate->PointsInLine - 2].y);;
  end.x = XD(_drawstate->datapoints[_drawstate->PointsInLine - 1].x,_drawstate->datapoints[_drawstate->PointsInLine - 1].y);;
  end.y = YD(_drawstate->datapoints[_drawstate->PointsInLine - 1].x,_drawstate->datapoints[_drawstate->PointsInLine - 1].y);;

  /* clip line segment to rectangular clipping region in device frame */
  clipval = clip_line (&start.x, &start.y, &end.x, &end.y);
  if (!(clipval & ACCEPTED))	/* line segment is OOB */
    return 0;

  /* convert clipped starting point, ending point to integer Tek coors */
  istart.x = IROUND (start.x);
  istart.y = IROUND (start.y);
  iend.x = IROUND (end.x);
  iend.y = IROUND (end.y);

  if (_drawstate->PointsInLine == 2)
    /* New polyline is beginning, so can start to draw it on the display:
       move to starting point of the first line segment, in Tek space.
       Also shift Tek to correct mode, either PLOT or POINT.  Third arg
       TRUE means always issue a change-mode command, which will break any
       Tek polyline in progress. */
      _tek_move (istart.x, istart.y, TRUE);
  
  else
    /* A polyline is being drawn, >=1 line segments already.  So check
       whether Tek's notion of position is the same as the starting point
       of the new line segment; if it differs, move to the latter.  Such a
       difference can occur on account of clipping.  Tek's notion of
       position could also have changed on us if we did a
       savestate()...restorestate() since the last call to cont(). */
    {
      int correct_tek_mode = _drawstate->points_are_connected ? MODE_PLOT : MODE_POINT;

      if (_tekstate.position_is_unknown
	  || (_tekstate.pos.x != istart.x)
	  || (_tekstate.pos.y != istart.y)
	  || (_tekstate.mode_is_unknown)
	  || (_tekstate.mode != correct_tek_mode))
	{
	  /* Sync Tek's notion of position with ours. */
	  /* Also shift Tek to correct mode, PLOT or POINT. */
	  /* Note: we emit a mode-change sequence only if 
	     necessary (see _tek_move.c), or if need_break is set. */
	  _tek_move (istart.x, istart.y, TRUE);
	}
    }

  /* sync Tek's linestyle with ours if necessary (could have changed on us) */
  _tek_linemod();

  /* continue polyline by drawing vector on Tek display */
  _putcode_compressed (iend.x, iend.y, istart.x, istart.y, _outstream);
      
  /* update our notion of Tek's notion of position */
  _tekstate.pos.x = iend.x;
  _tekstate.pos.y = iend.y;

  return 0;
}

int
cont (x, y)
     int x, y;
{
  return fcont ((double)x, (double)y);
}
