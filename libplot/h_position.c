/* This file contains a low-level method for moving the pen position of an
   HP-GL plotter to agree with an HPGLPlotter's notion of what the graphics
   cursor position should be.

   The state of the pen (up vs. down) after calling this function is not
   uniquely determined.  */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_h_set_position (void)
#else
_h_set_position ()
#endif
{
  int xnew, ynew;
  
  /* if plotter's pen position doesn't agree with what it should be,
     adjust it */

  xnew = IROUND(XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y));
  ynew = IROUND(YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y));  

  if (_plotter->position_is_unknown == true
      || xnew != _plotter->pos.x || ynew != _plotter->pos.y)
    {
      if (_plotter->pendown == true)
	{
	  sprintf (_plotter->outbuf.current, "PU;PA%d,%d;", xnew, ynew);
	  _plotter->pendown = false;
	}
      else
	sprintf (_plotter->outbuf.current, "PA%d,%d;", xnew, ynew);
      _update_buffer (&_plotter->outbuf);

      /* update our knowledge of pen position */
      _plotter->position_is_unknown = false;
      _plotter->pos.x = xnew;
      _plotter->pos.y = ynew;
    }
}
