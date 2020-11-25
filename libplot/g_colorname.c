/* This file contains the pencolorname, fillcolorname, and bgcolorname
   methods, which are GNU extensions to libplot.  They search a database of
   known names (stored in g_colorname.h) for a specified color name.  If
   the name is found, its interpretation as a 48-bit RGB color is
   determined, and pencolor, fillcolor, or bgcolor is called to set the
   color.  If the name is not found, a default color (black for pen and
   fill, white for bg) is substituted.

   This file also contains the lower-level routine _string_to_color(). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "g_colorname.h"

typedef struct lib_cached_colornameinfo
{
  const Colornameinfo *info;
  struct lib_cached_colornameinfo *next;
} Cached_Colornameinfo;

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
  else if (_plotter->pen_color_warning_issued == false)
    {
      char *buf;
		
      buf = (char *)_plot_xmalloc (strlen (name) + 100);
      sprintf (buf, "substituting \"black\" for undefined pen color \"%s\"", 
	       name);
      _plotter->warning (buf);
      free (buf);
      _plotter->pen_color_warning_issued = true;
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
  else if (_plotter->fill_color_warning_issued == false)
    {
      char *buf;
		
      buf = (char *)_plot_xmalloc (strlen (name) + 100);
      sprintf (buf, "substituting \"black\" for undefined fill color \"%s\"", 
	       name);
      _plotter->warning (buf);
      free (buf);
      _plotter->fill_color_warning_issued = true;
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
  else if (_plotter->bg_color_warning_issued == false)
    {
      char *buf;
		
      buf = (char *)_plot_xmalloc (strlen (name) + 100);
      sprintf (buf, "substituting \"white\" for undefined background color \"%s\"", 
	       name);
      _plotter->warning (buf);
      free (buf);
      _plotter->bg_color_warning_issued = true;
    }

  _plotter->bgcolor (intred, intgreen, intblue);

  return 0;
}

/* _string_to_color() searches a database of known color names, in
   g_colorname.h, for a specified string.  Matches are case-insensitive and
   ignore spaces.  A pointer to the retrieved color name info is returned
   via a pointer.

   We maintain a linked list of previously found colors, so we don't need
   to search through the entire (long) color database.  (It contains 600+
   color name strings.)  */

bool
#ifdef _HAVE_PROTOS
_string_to_color (const char *name, const Colornameinfo **info_p)
#else
_string_to_color (name, info_p)
     const char *name;
     const Colornameinfo **info_p;
#endif
{
  bool found = false;
  char *squeezed_name, *nptr;
  const Colornameinfo *info, *found_info = NULL;
  const char *optr;
  Cached_Colornameinfo *cached_info;
  static Cached_Colornameinfo *cached_list = NULL;

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

  /* Search our list of cached, previously used color names, doing string
     comparison.  If this were only for use by the X11 driver, we'd use
     XrmPermStringToQuark to get a faster-compared representation. */
  cached_info = cached_list;
  while (cached_info)
    {
      if (strcasecmp (cached_info->info->name, squeezed_name) == 0)
	{
	  found = true;
	  found_info = cached_info->info;
	  break;
	}
      cached_info = cached_info->next;
    }

  if (!found)
    /* not previously used, so search master colorname table (this is slower) */
    {
      info = _colornames;	/* start at head of list in g_colorname.h */
      while (info->name)
	{
	  if (strcasecmp (info->name, squeezed_name) == 0)
	    {
	      found = true;
	      found_info = info;
	      break;
	    }
	  info++;
	}

      if (found)
	/* copy to head of cache list */
	{
	  Cached_Colornameinfo *old_cached_list;

	  old_cached_list = cached_list;
	  cached_list = 
	    (Cached_Colornameinfo *)_plot_xmalloc (sizeof (Cached_Colornameinfo));
	  cached_list->next = old_cached_list;
	  cached_list->info = found_info;
	}
    }
  
  free (squeezed_name);
  if (found)
    *info_p = found_info;

  return found;
}
