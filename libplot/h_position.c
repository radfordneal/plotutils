/* This file contains a low-level method for moving the pen position of an
   HP-GL plotter to agree with an HPGLPlotter's notion of what the graphics
   cursor position should be.

   The state of the pen (up vs. down) after calling this function is not
   uniquely determined.  */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_h_set_position (S___(Plotter *_plotter))
#else
_h_set_position (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int xnew, ynew;
  
  /* if plotter's pen position doesn't agree with what it should be,
     adjust it */

  xnew = IROUND(XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y));
  ynew = IROUND(YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y));  

  if (_plotter->hpgl_position_is_unknown == true
      || xnew != _plotter->hpgl_pos.x || ynew != _plotter->hpgl_pos.y)
    {
      if (_plotter->hpgl_pendown == true)
	{
	  sprintf (_plotter->page->point, "PU;PA%d,%d;", xnew, ynew);
	  _plotter->hpgl_pendown = false;
	}
      else
	sprintf (_plotter->page->point, "PA%d,%d;", xnew, ynew);
      _update_buffer (_plotter->page);

      /* update our knowledge of pen position */
      _plotter->hpgl_position_is_unknown = false;
      _plotter->hpgl_pos.x = xnew;
      _plotter->hpgl_pos.y = ynew;
    }
}
