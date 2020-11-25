/* This file contains the pencolorname, fillcolorname, and bgcolorname
   methods, which are GNU extensions to libplot.  They search a database of
   known names (stored in g_colorname.h) for a specified color name.  If
   the name is found, its interpretation as a 48-bit RGB color is
   determined, and pencolor, fillcolor, or bgcolor is called to set the
   color.  If the name is not found, a default color (black for pen and
   fill, white for bg) is substituted.

   This file also contains the lower-level routine _string_to_color(). */

#include "sys-defines.h"
#include "extern.h"
#include "g_colorname.h"

int 
#ifdef _HAVE_PROTOS
_g_pencolorname (R___(Plotter *_plotter) const char *name)
#else
_g_pencolorname (R___(_plotter) name)
     S___(Plotter *_plotter;)
     const char *name;
#endif
{
  const plColorNameInfo *info;
  int intred, intgreen, intblue;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "pencolorname: invalid operation");
      return -1;
    }

  /* null pointer ignored */
  if (!name)
    return 0;

  /* RGB values for default pen color */
  intred = _default_drawstate.fgcolor.red;
  intgreen = _default_drawstate.fgcolor.green;
  intblue = _default_drawstate.fgcolor.blue;

  if (_string_to_color (R___(_plotter) name, &info))
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
      _plotter->warning (R___(_plotter) buf);
      free (buf);
      _plotter->pen_color_warning_issued = true;
    }

  _plotter->pencolor (R___(_plotter) intred, intgreen, intblue);

  return 0;
}

int 
#ifdef _HAVE_PROTOS
_g_fillcolorname (R___(Plotter *_plotter) const char *name)
#else
_g_fillcolorname (R___(_plotter) name)
     S___(Plotter *_plotter;) 
     const char *name;
#endif
{
  const plColorNameInfo *info;
  int intred, intgreen, intblue;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fillcolorname: invalid operation");
      return -1;
    }

  /* null pointer ignored */
  if (!name)
    return 0;

  /* RGB values for default fill color */
  intred = _default_drawstate.fillcolor.red;
  intgreen = _default_drawstate.fillcolor.green;
  intblue = _default_drawstate.fillcolor.blue;

  if (_string_to_color (R___(_plotter) name, &info))
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
      _plotter->warning (R___(_plotter) buf);
      free (buf);
      _plotter->fill_color_warning_issued = true;
    }

  _plotter->fillcolor (R___(_plotter) intred, intgreen, intblue);

  return 0;
}

int 
#ifdef _HAVE_PROTOS
_g_bgcolorname (R___(Plotter *_plotter) const char *name)
#else
_g_bgcolorname (R___(_plotter) name)
     S___(Plotter *_plotter;) 
     const char *name;
#endif
{
  const plColorNameInfo *info;
  int intred, intgreen, intblue;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "bgcolorname: invalid operation");
      return -1;
    }

  /* null pointer ignored */
  if (!name)
    return 0;

  /* RGB values for default color */
  intred = _default_drawstate.bgcolor.red;
  intgreen = _default_drawstate.bgcolor.green;
  intblue = _default_drawstate.bgcolor.blue;

  if (_string_to_color (R___(_plotter) name, &info))
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
      _plotter->warning (R___(_plotter) buf);
      free (buf);
      _plotter->bg_color_warning_issued = true;
    }

  _plotter->bgcolor (R___(_plotter) intred, intgreen, intblue);

  return 0;
}

/* _string_to_color() searches a database of known color names, in
   g_colorname.h, for a specified string.  Matches are case-insensitive and
   ignore spaces.  A pointer to the retrieved color name info is returned
   via a pointer.

   Any Plotter maintains a linked list of previously found colors, so we
   don't need to search through the entire (long) color database.  (It
   contains 600+ color name strings.)  */

bool
#ifdef _HAVE_PROTOS
_string_to_color (R___(Plotter *_plotter) const char *name, const plColorNameInfo **info_p)
#else
_string_to_color (R___(_plotter) name, info_p)
     S___(Plotter *_plotter;)
     const char *name;
     const plColorNameInfo **info_p;
#endif
{
  bool found = false;
  char *squeezed_name, *nptr;
  const plColorNameInfo *info, *found_info = NULL;
  const char *optr;
  plCachedColorNameInfo *cached_info;

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
  cached_info = _plotter->cached_colors;
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
	/* copy to head of cached color list */
	{
	  plCachedColorNameInfo *old_cached_colors;

	  old_cached_colors = _plotter->cached_colors;
	  _plotter->cached_colors = 
	    (plCachedColorNameInfo *)_plot_xmalloc (sizeof (plCachedColorNameInfo));
	  _plotter->cached_colors->next = old_cached_colors;
	  _plotter->cached_colors->info = found_info;
	}
    }
  
  free (squeezed_name);
  if (found)
    *info_p = found_info;

  return found;
}
