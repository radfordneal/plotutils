/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* For HPGLPlotter objects, we determine the HP-GL version from the
   environment variable "HPGL_VERSION" ("1", "1.5", or "2", meaning generic
   HP-GL, HP7550A, and modern HP-GL/2 respectively), and determine the page
   size and the location on the page of the graphics display, so that we'll
   be able to work out the map from user coordinates to device coordinates
   in space.c.

   We allow the user to shift the location of the graphics display by
   specifying an offset vector, since the origin of the HP-GL coordinate
   system and the size of the `hard-clip region' within which graphics can
   be drawn are not known.  (There are so many HP-GL and HP-GL/2
   devices.)

   We also work out which pens are available, and whether the device, if an
   HP-GL/2 device, supports the Palette Extension so that new logical pens
   can be defined as RGB triples.  The HPGL_PENS and HPGL_ASSIGN_COLORS
   environment variables are used for this.  (The default is for a
   pre-HP-GL/2 device to have exactly 1 pen, #1, and for an HP-GL/2 device
   to have 7 pens, #1 through #7, with colors equal to the seven non-white
   vertices of the RGB color cube.  We allow the user to specify up to 8
   pens, #1 through #8, via HPGL_PENS. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* forward references */
static bool _parse_pen_string __P ((const char *pen_s, Plotter *state_ptr));
static bool _string_to_inches __P ((const char *offset_s, double *offset));

#define MAX_COLOR_NAME_LEN 32	/* long enough for all known colors */

int
#ifdef _HAVE_PROTOS
_h_openpl (void)
#else
_h_openpl ()
#endif
{
  const char *version_s, *rotate_s, *xoffset_s, *yoffset_s, *pen_s, *transparent_s, *length_s, *pagesize;
  const Pagedata *pagedata;
  int i;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* initialize certain data members from values of relevant class variables */

  length_s = (const char *)_get_plot_param ("MAX_LINE_LENGTH");
  {
    int local_length;
    
    if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
      {
	_plotter->error ("bad MAX_LINE_LENGTH parameter, can't initialize");
	return -1;
      }
    else
      _plotter->max_unfilled_polyline_length = local_length;
  }

  version_s = (const char *)_get_plot_param ("HPGL_VERSION");
  {
    if (strcmp (version_s, "1") == 0)	/* generic HP-GL, HP7220 or HP7475A */
      {
	_plotter->hpgl_version = 0;
	_plotter->have_wide_lines = 0;
	_plotter->have_solid_fill = 0;
	_plotter->have_pcl_fonts = 0;
      }
    else if (strcmp (version_s, "1.5") == 0) /* HP7550A */
      {
	_plotter->hpgl_version = 1;
	_plotter->have_wide_lines = 0;
	_plotter->have_solid_fill = 1;
	_plotter->have_pcl_fonts = 0;
      }
    else if (strcmp (version_s, "2") == 0) /* HP-GL/2 */
      {
	_plotter->hpgl_version = 2;
	_plotter->have_wide_lines = 1;
	_plotter->have_solid_fill = 1;
	_plotter->have_pcl_fonts = 1;
      }
    else 
      {
	_plotter->error ("bad HPGL_VERSION variable, can't initialize");
	return -1;
      }
  }

  /* determine page type i.e. determine the range of device coordinates
   over which the graphics display will extend (and hence the
   transformation from user to device coordinates). */
  pagesize = (const char *)_get_plot_param ("PAGESIZE");
  pagedata = _pagetype(pagesize);
  if (pagedata == NULL)
    {
      _plotter->error ("bad PAGESIZE variable, can't initialize");
      return -1;
    }
  _plotter->display_coors = pagedata->hpgl;

  /* Origin of HP-GL[/2] coordinates is the lower left corner of the
     hard-clip region, which is not the same as the lower left corner of
     the page.  So we allow the user to specify an offset for the location
     of the graphics display. */
  xoffset_s = (const char *)_get_plot_param ("HPGL_XOFFSET");
  yoffset_s = (const char *)_get_plot_param ("HPGL_YOFFSET");  
  {
    double xoffset;
    
    if (_string_to_inches (xoffset_s, &xoffset))
      {
	_plotter->display_coors.left += xoffset;
	_plotter->display_coors.right += xoffset;
      }
    else
      {
	_plotter->error ("bad HPGL_XOFFSET variable, can't initialize");
	return -1;
      }
  }
  {
    double yoffset;
    
    if (_string_to_inches (yoffset_s, &yoffset))
      {
	_plotter->display_coors.bottom += yoffset;
	_plotter->display_coors.top += yoffset;
      }
    else
      {
	_plotter->error ("bad HPGL_YOFFSET variable, can't initialize");
	return -1;
      }
  }
  
  /* At this point we stash the coordinates for later use.  They'll become
     the coordinates of our `scaling points' P1 and P2 (see closepl.c).
     The numbers in our output file will be normalized device coordinates,
     not physical device coordinates (for the transformation between them,
     which is accomplished by the HP-GL `SC' instruction, see closepl.c) */
  _plotter->p1x = _plotter->display_coors.left * HPGL_UNITS_PER_INCH;
  _plotter->p2x = _plotter->display_coors.right * HPGL_UNITS_PER_INCH;
  _plotter->p1y = _plotter->display_coors.bottom * HPGL_UNITS_PER_INCH;
  _plotter->p2y = _plotter->display_coors.top * HPGL_UNITS_PER_INCH;
  _plotter->display_coors.left = HPGL_SCALED_DEVICE_LEFT; 
  _plotter->display_coors.right = HPGL_SCALED_DEVICE_RIGHT;
  _plotter->display_coors.bottom = HPGL_SCALED_DEVICE_BOTTOM;
  _plotter->display_coors.top = HPGL_SCALED_DEVICE_TOP;
  _plotter->device_units_per_inch = 1.0;

  /* plot length (to be emitted in an HP-GL/2 `PS' instruction, important
     mostly for roll plotters) */
  _plotter->plot_length = 
    _plotter->display_coors.extra * HPGL_UNITS_PER_INCH;

  /* determine whether to rotate the figure (e.g. horizontal instead of
     vertical, see closepl.c) */
  rotate_s = (const char *)_get_plot_param ("ROTATE");
  if (strcasecmp (rotate_s, "yes") == 0
      || strcmp (rotate_s, "90") == 0)
    _plotter->rotation = 90;
  else if (strcmp (rotate_s, "180") == 0)
    _plotter->rotation = 180;
  else if (strcmp (rotate_s, "270") == 0)
    _plotter->rotation = 270;
  else
    _plotter->rotation = 0;

  /* should we avoid emitting the `white is opaque' HP-GL/2 instruction?
     (HP-GL/2 pen plotters do not like it) */
  transparent_s = (const char *)_get_plot_param ("HPGL_OPAQUE_MODE");
  if (strcasecmp (transparent_s, "yes") == 0)
      _plotter->opaque_white = true;
  else
      _plotter->opaque_white = false;

  /* do we support the HP-GL/2 palette extension, i.e. can we define new
     logical pens as RGB triples? (user must request this) */
  _plotter->palette = false;
  if (_plotter->hpgl_version == 2)
    {
      const char *palette_s;
      
      palette_s = (const char *)_get_plot_param ("HPGL_ASSIGN_COLORS");
      if (strcasecmp (palette_s, "yes") == 0)
	_plotter->palette = true;
    }

  /* initialize pen color array, typically 0..31 */
  for (i = 0; i < MAX_NUM_PENS; i++)
    _plotter->pen_defined[i] = 0; /* pen absent */

  /* pen #0 (white pen, RGB=255,255,255) is always defined */
  _plotter->pen_color[0].red = 255;
  _plotter->pen_color[0].green = 255;
  _plotter->pen_color[0].blue = 255;
  _plotter->pen_defined[0] = 2; /* i.e. hard-defined */

  /* determine initial palette, i.e. available pens in 1..31 range */
  if ((pen_s = (const char *)_get_plot_param ("HPGL_PENS")) == NULL)
    /* since no value is assigned to HPGL_PENS by default, user must not
       have assigned a value to it; we'll choose a value based on version */
    {
      if (_plotter->hpgl_version < 2)
	pen_s = DEFAULT_HPGL_PEN_STRING;
      else
	pen_s = DEFAULT_HPGL2_PEN_STRING;
    }
  if (_parse_pen_string (pen_s, _plotter) == false)
    {
      _plotter->error ("bad HPGL_PENS variable, can't initialize");
      return -1;
    }
  /* if no logical pens, insist on pen #1 being present (backward
     compatibility?) */
  if (_plotter->palette == false 
      && _plotter->pen_defined[1] == 0)
    {
      _plotter->error ("pen #1 not defined in HPGL_PENS variable, can't initialize");
      return -1;
    }
  /* examine presence or absence of hard-defined pens in 2..31 range */
  {
    bool have_more_than_one_pen = false;
    bool have_free_pens = false;
    
    for (i = 2; i < MAX_NUM_PENS; i++)
      {
	if (_plotter->pen_defined[i] == 2)
	  /* at least one pen with number > 1 is hard-defined */
	  have_more_than_one_pen = true;
	else
	  /* at least one pen with number > 1 is not hard-defined */
	  {
	    /* record which such was encountered first */
	    if (have_free_pens == false)
	      _plotter->free_pen = i;
	    have_free_pens = true;
	  }
      }
    if (_plotter->palette == false
	&& have_more_than_one_pen == false)
      /* only have pen #1, all non-white objects will be drawn with it */
      _plotter->monochrome = true;
    else
      _plotter->monochrome = false;
    if (!have_free_pens)	
      /* luser specified too many pens, can't soft-define colors */
      _plotter->palette = false;
  }
  
  /* prepare buffer in which we'll cache all HP-GL code */
  _initialize_buffer (&_plotter->outbuf);

  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  return 0;
}

static bool 
#ifdef _HAVE_PROTOS
_string_to_inches(const char *string, double *inches)
#else
_string_to_inches(string, inches)
     const char *string; 
     double *inches;
#endif
{
  double val;
  char s[4];
  
  if (sscanf (string, "%lf %3s", &val, s) == 2)
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

/* Parse a pen string, e.g. a user-specified HPGL_PENS environment
   variable, specifying which pens are available.  Result is stored in a
   specified device structure.  More pens (logical pens) may be added later
   to the array of available pens, if the plotter is an HP-GL/2 device and
   supports the palette extension.  User specifies this by setting the
   HPGL_ASSIGN_COLORS environment variable to "yes"; see above. */
static bool
#ifdef _HAVE_PROTOS
_parse_pen_string (const char *pen_s, Plotter *state_ptr)
#else
_parse_pen_string (pen_s, state_ptr)
     const char *pen_s;
     Plotter *state_ptr;
#endif
{
  const char *charp;
  char name[MAX_COLOR_NAME_LEN];
  int i;

  charp = pen_s;
  while (*charp)
    {
      int pen_num;
      bool got_digit;
      const char *tmp;
      const Colornameinfo *info;

      if (*charp == ':')	/* skip any ':' */
	{
	  charp++;
	  continue;		/* back to top of while loop */
	}
      pen_num = 0;
      got_digit = false;
      while (*charp >= '0' && *charp <= '9')
	{
	  pen_num = 10 * pen_num + (int)*charp - (int)'0';
	  got_digit = true;
	  charp++;
	}
      if (!got_digit || pen_num < 1 || pen_num >= MAX_NUM_PENS)
	return false;
      if (*charp != '=')
	return false;
      charp++;
      for (tmp = charp, i = 0; i < MAX_COLOR_NAME_LEN; tmp++, i++)
	{
	  if (*tmp == ':') /* end of color name string */
	    {
	      name[i] = '\0';
	      charp = tmp + 1;
	      break;
	    }
	  else if (*tmp == '\0') /* end of name string and env var also */
	    {
	      name[i] = '\0';
	      charp = tmp;
	      break;
	    }
	  else
	    name[i] = *tmp;
	}

      /* got color name string, parse it */
      if (_string_to_color (name, &info))
	{
	  state_ptr->pen_color[pen_num].red = info->red;
	  state_ptr->pen_color[pen_num].green = info->green;
	  state_ptr->pen_color[pen_num].blue = info->blue;
	  state_ptr->pen_defined[pen_num] = 2; /* hard-defined */
	}
      else			/* couldn't match color name string */
	return false;
    }

  return true;
}  
