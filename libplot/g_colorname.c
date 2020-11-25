/* This file contains the pencolorname and fillcolorname methods, which are
   GNU extensions to libplot.  They search a database of known names
   (stored in colorname.h) for a specified color name.  If the name is
   found, its interpretation as a 48-bit RGB color is determined, and
   pencolor or fillcolor is called to set the color.  If the name is not
   found, a default color (black) is substituted.

   This file also contains the lower-level routine _string_to_color(). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_colorname.h"

int 
#ifdef _HAVE_PROTOS
_g_pencolorname (const char *name)
#else
_g_pencolorname (name)
     const char *name;
#endif
{
  const Colornameinfo *info;
  int intred, intgreen, intblue;

  /* RGB values for default color: black */
  intred = 0;
  intgreen = 0;
  intblue = 0;

  if (_string_to_color (name, &info))
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

  _plotter->pencolor (intred, intgreen, intblue);

  return 0;
}

int 
#ifdef _HAVE_PROTOS
_g_fillcolorname (const char *name)
#else
_g_fillcolorname (name)
     const char *name;
#endif
{
  const Colornameinfo *info;
  int intred, intgreen, intblue;

  /* RGB values for default color: black */
  intred = 0;
  intgreen = 0;
  intblue = 0;

  if (_string_to_color (name, &info))
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

  _plotter->fillcolor (intred, intgreen, intblue);

  return 0;
}

int 
#ifdef _HAVE_PROTOS
_g_bgcolorname (const char *name)
#else
_g_bgcolorname (name)
     const char *name;
#endif
{
  const Colornameinfo *info;
  int intred, intgreen, intblue;

  /* RGB values for default color: white */
  intred = 0xffff;
  intgreen = 0xffff;
  intblue = 0xffff;

  if (_string_to_color (name, &info))
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

  _plotter->bgcolor (intred, intgreen, intblue);

  return 0;
}

/* _string_to_color() searches a database of known color names, in
   colorname.h, for a specified string.  Matches are case-insensitive and
   ignore spaces.  A pointer to the retrieved color name info is returned,
   via a pointer. */
bool
#ifdef _HAVE_PROTOS
_string_to_color (const char *name, const Colornameinfo **info_p)
#else
_string_to_color (name, info_p)
     const char *name;
     const Colornameinfo **info_p;
#endif
{
  const Colornameinfo *info = _colornames; /* see colorname.h */
  const char *optr;
  char *squeezed_name, *nptr;
  bool found = false;

  if (name == NULL)		/* avoid core dumps */
    return false;
  
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

  /* search table */
  do
    {
      if (strcasecmp (info->name, squeezed_name) == 0)
	{
	  found = true;
	  break;
	}
      info++;
    }
  while (info->name);
  
  free (squeezed_name);
  if (found)
    *info_p = info;

  return found;
}
