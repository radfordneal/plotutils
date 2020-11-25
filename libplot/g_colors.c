/* This file contains the color method, a convenience method which is a GNU
   extension to libplot.  It simply invokes pencolor() and fillcolor(),
   setting two attributes: the pen color (``foreground color'') and fill
   color (``background color'') of objects created in the drawing
   operations that follow.  Whether filling actually takes place depends on
   whether filltype() is called, and with what arguments.

   This file also contains the colorname method, which similarly invokes
   pencolorname and fillcolorname.  */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_color (R___(Plotter *_plotter) int red, int green, int blue)
#else
_g_color (R___(_plotter) red, green, blue)
     S___(Plotter *_plotter;) 
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "color: invalid operation");
      return -1;
    }

  _plotter->pencolor (R___(_plotter) red, green, blue);
  _plotter->fillcolor (R___(_plotter) red, green, blue);  

  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_colorname (R___(Plotter *_plotter) const char *name)
#else
_g_colorname (R___(_plotter) name)
     S___(Plotter *_plotter;)
     const char *name;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "colorname: invalid operation");
      return -1;
    }

  _plotter->pencolorname (R___(_plotter) name);
  _plotter->fillcolorname (R___(_plotter) name);
  
  return 0;
}
