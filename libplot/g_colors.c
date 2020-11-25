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
_g_color (int red, int green, int blue)
#else
_g_color (red, green, blue)
     int red, green, blue;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("color: invalid operation");
      return -1;
    }

  _plotter->pencolor (red, green, blue);
  _plotter->fillcolor (red, green, blue);  

  return 0;
}

int
#ifdef _HAVE_PROTOS
_g_colorname (const char *name)
#else
_g_colorname (name)
     const char *name;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("colorname: invalid operation");
      return -1;
    }

  _plotter->pencolorname (name);
  _plotter->fillcolorname (name);
  
  return 0;
}
