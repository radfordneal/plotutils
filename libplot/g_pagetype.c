/* This file contains a utility function, _set_page_type(), which searches
   the database of known pagetypes for the one specified in the PAGESIZE
   parameter, sets page-related data (dimensions, size of viewport) in the
   Plotter, and also passes back the viewport offset vector.

   The PAGESIZE parameter should be of a form resembling "letter", or "a4",
   or "letter,xoffset=-1.2in", or "a4,yoffset=0.5cm,xoffset = 2mm". */

#include "sys-defines.h"
#include "extern.h"
#include "g_pagetype.h"

/* forward references */
static bool _parse_page_type ____P((const char *pagesize, const plPageData **pagedata, double *xoffset, double *yoffset));
static bool _string_to_inches ____P ((const char *offset_s, double *offset));

void
#ifdef _HAVE_PROTOS
_set_page_type(R___(Plotter *_plotter) double *xoffset, double *yoffset)
#else
_set_page_type(R___(_plotter) xoffset, yoffset)
     S___(Plotter *_plotter;)
     double *xoffset, *yoffset;
#endif
{
  const char *pagesize;
  const plPageData *pagedata;
  double local_xoffset, local_yoffset;
  
  /* examine user-specified value for PAGESIZE parameter, or the default
     value if we can't parse the user-specified value */
  pagesize = (const char *)_get_plot_param (R___(_plotter) "PAGESIZE");
  if (!_parse_page_type (pagesize, &pagedata, 
			 &local_xoffset, &local_yoffset))
    {
      pagesize = (const char *)_get_default_plot_param ("PAGESIZE");
      _parse_page_type (pagesize, &pagedata, 
			&local_xoffset, &local_yoffset);
    }

  /* set page data in Plotter; pass back viewport offset vector */
  _plotter->page_data = pagedata;
  *xoffset = local_xoffset;
  *yoffset = local_yoffset;
}

static bool
#ifdef _HAVE_PROTOS
_parse_page_type (const char *pagesize, const plPageData **pagedata, double *xoffset, double *yoffset)
#else
_parse_page_type (pagesize, pagedata, xoffset, yoffset)
     const char *pagesize;
     const plPageData **pagedata;
     double *xoffset, *yoffset;
#endif
{
  const plPageData *local_pagedata = _pagedata;
  char *local_pagesize, *first, *next;
  char xoffset_s[32], yoffset_s[32]; /* each field should have length <=31 */
  bool anotherfield, success, got_xoffset = false, got_yoffset = false;
  int i;

  local_pagesize = (char *)_plot_xmalloc (strlen (pagesize) + 1);  
  strcpy (local_pagesize, pagesize);
  first = local_pagesize;
  next = strchr (local_pagesize, (int)',');
  if (next)
    {
      anotherfield = true;
      *next = '\0';
      next++;
    }
  else
    anotherfield = false;

  /* try to match page size to a page size on our list */
  success = false;
  for (i = 0; i < NUM_PAGESIZES; i++, local_pagedata++)
    if (strcasecmp (local_pagedata->name, local_pagesize) == 0
	|| 
	(local_pagedata->alt_name 
	 && strcasecmp (local_pagedata->alt_name, local_pagesize) == 0))
      {
	success = true;
	break;
      }
  free (local_pagesize);

  if (success)
    /* matched the page size, at least */
    {
      /* pass back pointer to page data via pointer */
      *pagedata = local_pagedata;

      while (anotherfield && *next) /* i.e. while there's a nonempty field */
	{
	  first = next;
	  next = strchr (next, (int)',');
	  if (next)
	    {
	      anotherfield = true;
	      *next = '\0';
	      next++;
	    }
	  else
	    anotherfield = false;

	  /* try to parse field */
	  if (sscanf (first, "xoffset = %31s", xoffset_s) == 1)
	    got_xoffset = true;
	  else if (sscanf (first, "yoffset = %31s", yoffset_s) == 1)
	    got_yoffset = true;	      
	}
      
      /* pass back viewpoint offset vector */
      {
	double local_xoffset, local_yoffset;

	if (got_xoffset && _string_to_inches (xoffset_s, &local_xoffset))
	  *xoffset = local_xoffset;
	else
	  *xoffset = 0.0;
	
	if (got_yoffset && _string_to_inches (yoffset_s, &local_yoffset))
	  *yoffset = local_yoffset;
	else
	  *yoffset = 0.0;
      }
    }

  /* indicate whether we were able to match at least the page size */
  return success;
}

/* convert a string representing a distance measurement to inches; units
   `in', `cm', `mm' are supported */

static bool 
#ifdef _HAVE_PROTOS
_string_to_inches (const char *string, double *inches)
#else
_string_to_inches (string, inches)
     const char *string; 
     double *inches;
#endif
{
  double val;
  char s[4];
  
  if (sscanf (string, "%lf %3s" , &val, s) == 2)
    {
      if (strlen (s) > 2)
	return false;
      if (strcmp (s, "in") == 0)
	{
	  *inches = val;
	  return true;
	}
      else if (strcmp (s, "cm") == 0)
	{
	  *inches = val / 2.54;
	  return true;
	}
      else if (strcmp (s, "mm") == 0)      
	{
	  *inches = val / 25.4;
	  return true;
	}
    }
    return false;
}
