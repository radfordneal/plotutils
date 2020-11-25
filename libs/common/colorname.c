/* This file contains the pencolorname and fillcolorname routines, which
   are GNU extensions to libplot.  They search a database of known names
   (stored in colorname.h) for a specified color name.  If the name is
   found, its interpretation as a 48-bit RGB color is determined, and
   pencolor or fillcolor is called to set the color.  If the name is not
   found, a default color (black) is substituted. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "colorname.h"

int 
pencolorname (name)
     char *name;
{
  Colornameinfo *info = colornames;
  Boolean found = FALSE;
  char *squeezed_name, *optr, *nptr;
  int intred, intgreen, intblue;

  /* RGB values for defaul color: black */
  intred = 0;
  intgreen = 0;
  intblue = 0;

  /* copy string, removing spaces */
  squeezed_name = (char *)_plot_xmalloc (strlen (name) + 1);
  optr = name, nptr = squeezed_name;
  while (*optr)
    {
      if (*optr == '\0')
	break;
      if (*optr != ' ')
	*nptr++ = *optr;
      optr++;
    }
  *nptr = '\0';

  do
    {
      if (strcasecmp (info->name, squeezed_name) == 0)
	{
	  found = TRUE;
	  break;
	}
      info++;
    }
  while (info->name);
  
  if (found)
    {
      unsigned int red, green, blue;
      
      red = info->red;
      green = info->green;
      blue = info->blue;
      /* to convert from 24-bit to 48-bit color, double bytes */
      intred = (red << 8) | red;
      intgreen = (green << 8) | green;
      intblue = (blue << 8) | blue;

    }

  pencolor (intred, intgreen, intblue);
  free (squeezed_name);

  return 0;
}

int 
fillcolorname (name)
     char *name;
{
  Colornameinfo *info = colornames;
  Boolean found = FALSE;
  char *squeezed_name, *optr, *nptr;
  int intred, intgreen, intblue;

  /* RGB values for defaul color: black */
  intred = 0;
  intgreen = 0;
  intblue = 0;

  /* copy string, removing spaces */
  squeezed_name = (char *)_plot_xmalloc (strlen (name) + 1);
  optr = name, nptr = squeezed_name;
  while (*optr)
    {
      if (*optr == '\0')
	break;
      if (*optr != ' ')
	*nptr++ = *optr;
      optr++;
    }
  *nptr = '\0';

  do
    {
      if (strcasecmp (info->name, squeezed_name) == 0)
	{
	  found = TRUE;
	  break;
	}
      info++;
    }
  while (info->name);
  
  if (found)
    {
      unsigned int red, green, blue;

      red = info->red;
      green = info->green;
      blue = info->blue;
      /* to convert from 24-bit to 48-bit color, double bytes */
      intred = (red << 8) | red;
      intgreen = (green << 8) | green;
      intblue = (blue << 8) | blue;

    }

  fillcolor (intred, intgreen, intblue);
  free (squeezed_name);

  return 0;
}

