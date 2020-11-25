/* This file contains the color routine, a convenience routine which is a
   GNU extension to libplot.  It simply calls pencolor() and fillcolor(),
   setting two attributes: the pen color (``foreground color'') and fill
   color (``background color'') of objects created in the drawing
   operations that follow.  Whether filling actually takes place depends on
   whether fill() is called, and with what arguments.

   This file also contains the colorname routine, which similarly calls
   pencolorname and fillcolorname.  */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
color (red, green, blue)
     int red, green, blue;
{
  pencolor (red, green, blue);
  fillcolor (red, green, blue);  

  return 0;
}

int
colorname (name)
     char *name;
{
  pencolorname (name);
  fillcolorname (name);
  
  return 0;
}
