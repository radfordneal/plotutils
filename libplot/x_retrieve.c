/* This file contains the internal _retrieve_font method, which is called
   when the font_name, font_size, and textangle fields of the current
   drawing state have been filled in.  It retrieves the specified font, and
   fills in the font_type, typeface_index, font_index, true_font_size,
   font_ascent, font_descent, and font_is_iso8859_1 fields of the drawing
   state. */

/* This version is for XDrawablePlotters (and XPlotters).  It also fills in
   the x_font_pixmatrix and x_native_positioning fields of the drawing
   state, which are X-specific.  If the retrieved font is a non-Hershey
   font, i.e. a font retrieved from the X server, it sets the X-specific
   field x_font_struct also. */

/* _retrieve_X_font_internal() is what actually retrieves the font from the
   X server, or from a local font cache.  We run through various
   possibilities: first we try to interpret `name' as the name of a
   built-in Hershey font (e.g. "HersheySerif"), then as the name of a
   built-in PS font (e.g. "Times-Roman"), then as a base XLFD name
   (e.g. "bembo-medium-r-normal"), etc.  We do support retrieval of fonts
   with non-XLFD names that are aliases or even pre-XLFD (e.g. "fixed" or
   "vtsingle").

   The `true_font_size' field is a bit difficult to compute for an X font.
   Ideally it would be a minimum interline spacing, because that is what
   must be returned (in user units) by fontname(), fontsize, and
   textangle().  But even for non-rotated, non-sheared fonts, there are
   several possibilities for this: (1) the so-called pixel size (which can
   be extracted only if the font has an XLFD name, or if the font has a
   PIXEL_SIZE or POINT_SIZE property), (2) the sum of the `logical'
   font_ascent and font_descent (from the font structure), (3) the sum of
   the maximum (over characters) ascent and the maximum descent, (4) the
   sum of the cap height and the maximum descender length (which may both
   be difficult to extract), etc.

   I decided to use (1).  The analogue for affinely transformed fonts is
   easy to compute (from the pixel matrix).  However, for old non-XLFD (and
   hence non-rotated, non-sheared) fonts, there is the possibility that the
   PIXEL_SIZE property may not be available.  For such fonts, the
   POINT_SIZE and RESOLUTION_Y properties are used to compute it.  If they
   also are not available, we fall back on definition (2).

   The {font_ascent,font_descent} fields of the drawing state are important
   for vertical positioning.  For modern XLFD fonts at least, we obtain
   them from the RAW_ASCENT and RAW_DESCENT properties.  See comments in
   the code. */

/* NOTE: _retrieve_X_font_internal() and the routines that it calls now
   keep careful track of which elements of the X11R6-style font pixel
   matrix, if any, are zero.  This is because the true font size, as
   supplied by the X server, may not be quite the size we request.  If a
   font with pixel matrix containing one or more zeroes is requested from
   the X server, what comes back may have the zeroes replaced by absurdly
   small quantities (e.g. 1e-311).  Believe it or not, floating point
   numbers that small cannot be manipulated arithmetically on a DEC Alpha.
   If you try, you get a SIGFPE and a program crash!  We make sure that our
   internal representation for the returned font pixel matrix sets such
   elements to zero (exactly).  This problem was first noticed when `graph'
   on a DEC Alpha crashed when trying to plot a y-axis label (rotated 90
   degrees, so two of the four elements of the pixel matrix were zero). */

#include "sys-defines.h"
#include "extern.h"
#include "g_her_metr.h"

/* max length user-level font name we'll accept; this may be either an XLFD
   base name (a string with exactly three hyphens in it), or something else
   (an alias, a pre-XLFD font name, etc.). */
#define MAX_USER_FONT_NAME_LENGTH 200

/* length of buffer for constructing an X font name; must be large enough
   to handle XFLD template, user-level font name */
#define MAX_FONT_NAME_LENGTH 255

/* An XLFD template, with holes into which we can punch the base name (a
   string with exactly three hyphens in it) and the integer size in terms
   of pixels */
static const char * const _xlfd_template = "-*-%s-*-%d-*-*-*-*-*-*-*";

/* An XLFD template for an affinely transformed font, with holes for the
   base name and for four dimensions in terms of pixels.  Supported as of
   X11R6.  (And by HP for several years before that?) */
static const char * const _xlfd_template_with_scaling = "-*-%s-*-[%s %s %s %s]-*-*-*-*-*-*-*";

/* length of buffer for constructing an X11R6-style list-of-charset-ranges
   string, e.g. "[32 48_57 65_90]" would represent the non-contiguous set
   of characters [ 0-9A-Z]. */
#define MAX_CHARSET_SUBSET_LIST_LENGTH 767

#define NUM_XLFD_FIELDS 14
#define FIELD_FOUNDRY 0
#define FIELD_FAMILY 1
#define FIELD_WEIGHT 2
#define FIELD_SLANT 3
#define FIELD_SET_WIDTH 4
#define FIELD_ADDITIONAL_STYLE 5
#define FIELD_PIXELS 6
#define FIELD_POINTS 7
#define FIELD_HORIZONTAL_RESOLUTION 8
#define FIELD_VERTICAL_RESOLUTION 9
#define FIELD_SPACING 10
#define FIELD_AVERAGE_WIDTH 11
#define FIELD_CHARACTER_SET_MAJOR 12
#define FIELD_CHARACTER_SET_MINOR 13 /* in X11R6 may include char subset too */

/* forward references */
static bool _is_a_subset ____P((unsigned char set1[32], unsigned char set2[32]));
static char *_xlfd_field ____P ((const char *name, int field));
static void _parse_pixmatrix ____P ((const char *pixel_string, double d[4], bool *x_native_positioning, bool is_zero[4]));
static void _print_bitvector ____P ((unsigned char v[32], char *s));
static void _string_to_bitvector ____P ((const unsigned char *s, unsigned char v[8]));

void
#ifdef _HAVE_PROTOS
_x_retrieve_font (S___(Plotter *_plotter))
#else
_x_retrieve_font (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int i;
  
#ifdef DEBUG
  fprintf (stderr, "_x_retrieve_font (%s,%f,%f), with x_label = %s\n",
	   _plotter->drawstate->font_name, 
	   _plotter->drawstate->font_size,
	   _plotter->drawstate->text_rotation,
	   _plotter->drawstate->x_label);
#endif

  if (_plotter->have_hershey_fonts) /* should always be true... */
    {
      /* if font is a Hershey font, retrieve it and return */
      i = -1;
      while (_hershey_font_info[++i].name)
	{
	  if (_hershey_font_info[i].visible) /* i.e. font not internal-only */
	    if (strcasecmp (_hershey_font_info[i].name, 
			    _plotter->drawstate->font_name) == 0
		|| (_hershey_font_info[i].othername
		    && strcasecmp (_hershey_font_info[i].othername, 
				   _plotter->drawstate->font_name) == 0))
	      {
		_plotter->drawstate->font_type = F_HERSHEY;
		_plotter->drawstate->typeface_index = 
		  _hershey_font_info[i].typeface_index;
		_plotter->drawstate->font_index = 
		  _hershey_font_info[i].font_index;
		_plotter->drawstate->font_is_iso8859_1 = 
		  _hershey_font_info[i].iso8859_1;
		_plotter->drawstate->true_font_size = 
		  _plotter->drawstate->font_size;
		/* N.B. this macro uses true_font_size, see g_alabel_her.c */
		_plotter->drawstate->font_ascent = 
		  HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_ASCENT);
		_plotter->drawstate->font_descent = 
		  HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_DESCENT);
		return;
	      }
	} 
    }

  /* Not a Hershey font; will need to be a font available on X server, or
     in our cache of previously obtained X fonts. */

  /* try to retrieve an X font with specified (name, size, rotation) */
  if (_retrieve_X_font_internal (R___(_plotter)
				 _plotter->drawstate->font_name, 
				 _plotter->drawstate->font_size, 
				 _plotter->drawstate->text_rotation))
    return;
  
  /* failure: so try to retrieve the X Plotter's default font, if it's a
     non-Hershey font, at specified size and at specified rotation angle */
  if (_plotter->default_font_type != F_HERSHEY)
    {
      const char *default_font_name;

      switch (_plotter->default_font_type)
	{
	case F_POSTSCRIPT:
	default:
	  default_font_name = DEFAULT_POSTSCRIPT_FONT;
	  break;
	case F_PCL:
	  default_font_name = DEFAULT_PCL_FONT;
	  break;
	case F_STICK:
	  default_font_name = DEFAULT_STICK_FONT;
	  break;
	}
      if (_retrieve_X_font_internal (R___(_plotter)
				     default_font_name,
				     _plotter->drawstate->font_size, 
				     _plotter->drawstate->text_rotation))
	/* Success, so squawk and then return.  But squawk only if we're on
	   the point of rendering or computing the width of a label (a hint
	   to that effect is passed in _plotter->drawstate->x_label). */
	{
	  if (_plotter->issue_font_warning && !_plotter->font_warning_issued 
	      && _plotter->drawstate->x_label)
	    {
	      char *buf;
	      
	      buf = (char *)_plot_xmalloc (strlen (_plotter->drawstate->font_name) + strlen (default_font_name) + 100);
	      sprintf (buf, "cannot retrieve font \"%s\", using default \"%s\"", 
		       _plotter->drawstate->font_name, default_font_name);
	      _plotter->warning (R___(_plotter) buf);
	      free (buf);
	      _plotter->font_warning_issued = true;
	    }
	  return;
	}
    }
  
  /* Either we couldn't retrieve the default X font, or maybe the default
     font for this Plotter was a Hershey font rather than an X font.  So
     retrieve the default Hershey font. */
  if (_plotter->have_hershey_fonts)
    {
      const char *user_specified_name;
      
      user_specified_name = _plotter->drawstate->font_name;
      _plotter->drawstate->font_name = DEFAULT_HERSHEY_FONT;
      _x_retrieve_font (S___(_plotter)); /* recursive call */
      _plotter->drawstate->font_name = user_specified_name;

      /* Squawk, but only if we're on the point of rendering or computing
       the width of a label (a hint to that effect is passed in
       _plotter->drawstate->x_label). */
      if (_plotter->issue_font_warning && !_plotter->font_warning_issued 
	  && _plotter->drawstate->x_label)
	{
	  char *buf;
	  
	  buf = (char *)_plot_xmalloc (strlen (_plotter->drawstate->font_name) + strlen (DEFAULT_HERSHEY_FONT) + 100);
	  sprintf (buf, "cannot retrieve font \"%s\", using default \"%s\"", 
		   _plotter->drawstate->font_name, 
		   DEFAULT_HERSHEY_FONT);
	  _plotter->warning (R___(_plotter) buf);
	  free (buf);
	  _plotter->font_warning_issued = true;
	}
    }
  else
    /* shouldn't happen, this library should always support Hershey fonts */
    _plotter->error (R___(_plotter)
		    "cannot provide requested font or a replacement, exiting");

  return;
}

/* _retrieve_X_font_internal() retrieves an X font specified by a triple,
   namely {name, size, rotation}.  Three possible interpretations are
   tried, in order.

   1. `name' is taken to be an alias for an XLFD base name, as listed in
   our table in g_fontdb.c.  (Aliases for the 35 standard font names appear
   there).  E.g., name="times-roman" or "Times-Roman".

   2. `name' is taken to be an XLFD base name (no aliasing), of the form
   foundry-family-weight-slant-width, with exactly four hyphens.  E.g.,
   name="bitstream-courier-medium-r-normal".  NOT YET IMPLEMENTED.

   3. `name' is taken to be an XLFD base name (no aliasing), of the form
   family-weight-slant-width, with exactly three hyphens.  E.g.,
   name="grotesk-bold-r-normal", or "times-medium-r-normal".

   4. `name' is taken to be a full X font name, in which case `size' and
   `rotation' are ignored.  E.g.,
   name="-schumacher-clean-medium-r-normal--6-60-75-75-c-50-iso8859-1" or
   name="vtbold".  This option is really to support old (pre-XLFD) fonts,
   such as "vtbold".

   If an X font is successfully retrieved (which will set the fields
   true_font_size, font_ascent, font_descent, and font_iso8859_1 of the
   drawing state, and the X-specific fields x_font_struct,
   x_font_pixmatrix, and x_native_positioning too), then this function sets
   font_type (F_POSTSCRIPT in case 1, F_OTHER in cases 2, 3 and 4) and
   returns true.  If a font is not successfully retrieved, this function
   returns false.

   typeface_index and font_index are also set in case 1.  In the other
   cases (F_OTHER) it's hardly worth it to set them, since switching to
   other fonts, except for a symbol font (`font #0') won't be supported.
   See g_cntrlify.c. */

bool
#ifdef _HAVE_PROTOS
_retrieve_X_font_internal(R___(Plotter *_plotter) const char *name, double size, double rotation)
#else
_retrieve_X_font_internal(R___(_plotter) name, size, rotation)
     S___(Plotter *_plotter;)
     const char *name;
     double size;
     double rotation;
#endif
{
  bool is_zero[4];		/* pixel matrix el.s are zero? (see above) */
  bool matched_builtin = false;	/* font name matches name of a builtin font? */
  bool success;			/* font retrieved from cache or server? */
  char *x_name_buf;		/* buffer for creating font name */
  const char *name_p;
  const char *x_name = NULL, *x_name_alt = NULL; /* from our font tables */
  double rot[4];		/* font rotation matrix */
  double user_size;		/* font size in user units */
  int i, hyphen_count;
  int typeface_index = 0, font_index = 0; /* from our tables of builtin fonts*/
  int font_type = F_POSTSCRIPT; /* from our tables of builtin fonts */

  if (size == 0.0)		/* don't try to retrieve zero-size fonts */
    return false;

  if (strlen (name) > MAX_USER_FONT_NAME_LENGTH) /* avoid buffer overflow */
    return false;

  user_size = size;		/* user units */
  rot[0] = cos (M_PI * rotation / 180.0);
  rot[1] = sin (M_PI * rotation / 180.0);
  rot[2] = - sin (M_PI * rotation / 180.0);
  rot[3] = cos (M_PI * rotation / 180.0);

  /* Search null-terminated table of recognized PS fonts, in g_fontdb.c,
     for a name matching the passed name.  We support either PS-style names
     (e.g. "Times-Roman") or shortened XLFD-style names
     (e.g. "times-medium-r-normal").  Alternative versions of latter are
     supported because some X servers use "zapfdingbats" instead of "itc
     zapf dingbats", etc. */
  i = -1;
  while (_ps_font_info[++i].ps_name) /* array ends in NULL */
    {
      if ((strcasecmp (_ps_font_info[i].ps_name, name) == 0)
	  /* check alternative ps font name if any */
	  || (_ps_font_info[i].ps_name_alt
	      && strcasecmp (_ps_font_info[i].ps_name_alt, name) == 0)
	  /* check 2nd alternative ps font name if any */
	  || (_ps_font_info[i].ps_name_alt2
	      && strcasecmp (_ps_font_info[i].ps_name_alt2, name) == 0)
	  /* check X font name */
	  || (strcasecmp (_ps_font_info[i].x_name, name) == 0)
	  /* check alternative X font name if any */
	  || (_ps_font_info[i].x_name_alt
	      && strcasecmp (_ps_font_info[i].x_name_alt, name) == 0))
	break;
    }
  
  if (_ps_font_info[i].ps_name)	/* matched name of a builtin PS font */
    {
      matched_builtin = true;
      x_name = _ps_font_info[i].x_name;
      x_name_alt = _ps_font_info[i].x_name_alt;
      font_type = F_POSTSCRIPT;
      typeface_index = _ps_font_info[i].typeface_index;
      font_index = _ps_font_info[i].font_index;
    }

#ifdef USE_LJ_FONTS_IN_X
  if (matched_builtin == false)	/* PS match failed, so try PCL fonts too */
    {
      i = -1;
      while (_pcl_font_info[++i].ps_name) /* array ends in NULL */
	{
	  if ((strcasecmp (_pcl_font_info[i].ps_name, name) == 0)
	      /* check alternative ps font name if any */
	      || (_pcl_font_info[i].ps_name_alt
		  && strcasecmp (_pcl_font_info[i].ps_name_alt, name) == 0)
	      /* check X font name */
	      || (strcasecmp (_pcl_font_info[i].x_name, name) == 0))
	    break;
	}
  
      if (_pcl_font_info[i].ps_name) /* matched name of a builtin PCL font */
	{
	  matched_builtin = true;
	  x_name = _pcl_font_info[i].x_name;
	  x_name_alt = NULL;
	  font_type = F_PCL;
	  typeface_index = _pcl_font_info[i].typeface_index;
	  font_index = _pcl_font_info[i].font_index;
	}
    }
#endif /* USE_LJ_FONTS_IN_X */

  /* prepare buffer for font name assemblage */
  x_name_buf = (char *)_plot_xmalloc ((MAX_FONT_NAME_LENGTH + 1) * sizeof (char));

  if (matched_builtin)
    {
      /* User passed the name of a builtin PS or PCL font.  Build full XLFD
	 name of font, computing pixel matrix values if we have something
	 other than an unrotated, uniformly scaled font.  */
      if ((rotation == (double)0.0) 
	  && _plotter->drawstate->transform.axes_preserved 
	  && _plotter->drawstate->transform.uniform
	  && _plotter->drawstate->transform.nonreflection
	  && _plotter->drawstate->transform.m[0] > 0.0)
	/* case 1: zero text rotation, uniformly scaled */
	{
	  int size_in_pixels;
	  
	  size_in_pixels = 
	    IROUND(_plotter->drawstate->transform.m[0] * user_size);
	  
	  /* if integer size in terms of pixels is zero, bail */
	  if (size_in_pixels == 0)
	    {
	      free (x_name_buf);
	      return false;
	    }
	  
	  /* no text rotation, pixel matrix will be diagonal */
	  is_zero[0] = is_zero[3] = false;
	  is_zero[1] = is_zero[2] = true;

	  /* punch size into template, try to retrieve font from server or
	     cache list */
	  sprintf (x_name_buf, _xlfd_template, x_name, size_in_pixels);
	  success = _select_X_font_carefully (R___(_plotter)
					      x_name_buf, is_zero, 
					      _plotter->drawstate->x_label);
	  if (success == false && x_name_alt)
	    /* try alternative X font name */
	    {
	      sprintf (x_name_buf, _xlfd_template, x_name_alt, size_in_pixels);
	      success = _select_X_font_carefully (R___(_plotter)
						  x_name_buf, is_zero, 
						  _plotter->drawstate->x_label);
	    }
	}
      else
	/* case 2: nonzero font rotation, or non-uniform scaling */
	{
	  double pm[4];		/* pixel matrix */
	  char a[4][256];	/* ascii representation */
	  char *p;
	  int k;
	  
	  /* pixel matrix: product of text rotation matrix, and
	     transformation from user to device coordinates; note flipped-y
	     convention affecting _plotter->drawstate->transform.m[1] and
	     _plotter->drawstate->transform.m[3].  A factor user_size is
	     also included; see below. */
	  pm[0] =  (rot[0] * _plotter->drawstate->transform.m[0] 
		    + rot[1] * _plotter->drawstate->transform.m[2]);
	  pm[1] =  - (rot[0] * _plotter->drawstate->transform.m[1] 
		      + rot[1] * _plotter->drawstate->transform.m[3]);
	  pm[2] =  (rot[2] * _plotter->drawstate->transform.m[0] 
		    + rot[3] * _plotter->drawstate->transform.m[2]);
	  pm[3] =  - (rot[2] * _plotter->drawstate->transform.m[1] 
		      + rot[3] * _plotter->drawstate->transform.m[3]);

	  /* don't attempt to retrieve zero-size fonts */
	  if (pm[0] == 0.0 && pm[1] == 0.0 && pm[2] == 0.0 && pm[3] == 0.0)
	    {
	      free (x_name_buf);
	      return false;
	    }
	  
	  /* ascii entries in pixel matrix string */
	  for (k = 0; k < 4; k++)
	    {
	      sprintf (a[k], "%f", user_size * pm[k]);
	      /* test whether pixel matrix elements are zero, to the
		 precision supported by %f (6 significant digits) */
	      if (strcmp (a[k], "0.000000") == 0
		  || strcmp (a[k], "-0.000000") == 0)
		is_zero[k] = true;
	      else 
		is_zero[k] = false;
	    }

	  /* convert minus signs to tildes */
	  for (k = 0; k < 4; k++)
	    for (p = a[k]; *p; p++)
	      if (*p == '-')
		*p = '~';
	  
	  /* punch size into template, try to retrieve font from server or
	     cache list */
	  sprintf (x_name_buf, _xlfd_template_with_scaling,
		   x_name, a[0], a[1], a[2], a[3]);
	  success = _select_X_font_carefully (R___(_plotter)
					      x_name_buf, is_zero, 
					      _plotter->drawstate->x_label);
	  if (success == false && x_name_alt)
	    /* try alternative X font name */
	    {
	      sprintf (x_name_buf, _xlfd_template_with_scaling,
		       x_name_alt, a[0], a[1], a[2], a[3]);
	      success = _select_X_font_carefully (R___(_plotter)
						  x_name_buf, is_zero, 
						  _plotter->drawstate->x_label);
	    }
	}

      if (success)
	/* we've retrieved an X font version of a built-in font; good */
	{
	  _plotter->drawstate->font_type = font_type;
	  _plotter->drawstate->typeface_index = typeface_index;
	  _plotter->drawstate->font_index = font_index;
	  free (x_name_buf);
	  return true;
	}
    }
  
  /* User-specified font name didn't match either of the names of any
     built-in PS [or PCL] font, so first consider possibility that it's an
     XLFD base name for some other X font (e.g. "bembo-medium-r-normal"),
     with exactly three hyphens. */
  name_p = name;
  hyphen_count = 0;
  while (*name_p)
    hyphen_count += (*name_p++ == '-' ? 1 : 0);

  if (hyphen_count == 3)
    /* treat as base of an XLFD name */
    {
      /* build full XLFD name, computing pixel matrix values if we have
	 something other than an unrotated, uniformly scaled font */
      if ((rotation == (double)0.0) 
	  && _plotter->drawstate->transform.axes_preserved 
	  && _plotter->drawstate->transform.uniform
	  && _plotter->drawstate->transform.nonreflection
	  && _plotter->drawstate->transform.m[0] > 0.0)
	/* case 1: zero text rotation, uniformly scaled */
	{
	  int size_in_pixels;

	  size_in_pixels = 
	    IROUND(_plotter->drawstate->transform.m[0] * user_size);

	  /* if integer size in terms of pixels is zero, bail */
	  if (size_in_pixels == 0)
	    {
	      free (x_name_buf);
	      return false;
	    }

	  /* no text rotation, pixel matrix will be diagonal */
	  is_zero[0] = is_zero[3] = false;
	  is_zero[1] = is_zero[2] = true;

	  /* punch size into template, try to retrieve font from server or
	     cache list */
	  sprintf (x_name_buf, _xlfd_template, name, 
		   size_in_pixels);
	  success = _select_X_font_carefully (R___(_plotter)
					      x_name_buf, is_zero,
					      _plotter->drawstate->x_label);
	}
      else
	/* case 2: nonzero font rotation, or non-uniform scaling */
	{
	  double pm[4];		/* pixel matrix */
	  char a[4][256];	/* ascii representation */
	  char *p;
	  int k;
      
	  /* pixel matrix: product of text rotation matrix, and
	     transformation from user to device coordinates; note flipped-y
	     convention affecting _plotter->drawstate->transform.m[1] and
	     _plotter->drawstate->transform.m[3].  A factor user_size is
	     also included; see below. */
	  pm[0] =  (rot[0] * _plotter->drawstate->transform.m[0] 
		    + rot[1] * _plotter->drawstate->transform.m[2]);
	  pm[1] =  - (rot[0] * _plotter->drawstate->transform.m[1] 
		      + rot[1] * _plotter->drawstate->transform.m[3]);
	  pm[2] =  (rot[2] * _plotter->drawstate->transform.m[0] 
		    + rot[3] * _plotter->drawstate->transform.m[2]);
	  pm[3] =  - (rot[2] * _plotter->drawstate->transform.m[1] 
		      + rot[3] * _plotter->drawstate->transform.m[3]);
      
	  /* don't attempt to retrieve zero-size fonts */
	  if (pm[0] == 0.0 && pm[1] == 0.0 && pm[2] == 0.0 && pm[3] == 0.0)
	    {
	      free (x_name_buf);
	      return false;
	    }
	  
	  /* ascii entries in pixel matrix string */
	  for (i = 0; i < 4; i++)
	    {
	      sprintf (a[i], "%f", user_size * pm[i]);
	      /* test whether pixel matrix elements are zero, to the
		 precision supported by %f (6 significant digits) */
	      if (strcmp (a[i], "0.000000") == 0
		  || strcmp (a[i], "-0.000000") == 0)
		is_zero[i] = true;
	      else 
		is_zero[i] = false;
	    }
      
	  /* convert minus signs to tildes */
	  for (k = 0; k < 4; k++)
	    for (p = a[k]; *p; p++)
	      if (*p == '-')
		*p = '~';
      
	  /* punch size into template, try to retrieve font from server or
	     cache list */
	  sprintf (x_name_buf, _xlfd_template_with_scaling,
		   name, a[0], a[1], a[2], a[3]);
	  success = _select_X_font_carefully (R___(_plotter)
					      x_name_buf, is_zero,
					      _plotter->drawstate->x_label);
	}
  
      if (success)
	/* we've retrieved a scalable X font that isn't one of our PS fonts */
	{
	  _plotter->drawstate->font_type = F_OTHER;
	  /* these two fields are irrelevant because we don't support
	     switching among user-defined fonts */
	  _plotter->drawstate->typeface_index = 0;
	  _plotter->drawstate->font_index = 1;
	  free (x_name_buf);
	  return true;
	}  
    }
  
  /* User-passed name didn't have exactly 3 hyphens, so try treating it as
     a full X fontname; ignore size.  This a kludge, needed to support
     pre-XLFD fonts, e.g. "vtsingle", and aliases for XLFD fonts,
     e.g. "fixed".  Most of the latter are really pre-XLFD names. */
  
  /* We don't do this unless the desired text rotation is zero, since
     pre-XLFD fonts all have rotation 0.  Also we insist that the
     user->device coordinate scaling should preserve coordinate axes, have
     the same scaling factors in both directions, and not involve a
     reflection. */

  if ((rotation == (double)0.0) 
      && _plotter->drawstate->transform.axes_preserved 
      && _plotter->drawstate->transform.uniform 
      && _plotter->drawstate->transform.nonreflection
      && _plotter->drawstate->transform.m[0] > 0.0)
    {
      /* don't assume that any elements of pixel matrix are zero (user
	 perhaps could have passed us a bizarre font name containing an
	 explicit pixel matrix?) */
      is_zero[0] = is_zero[3] = false;
      is_zero[1] = is_zero[2] = false;

      /* try to retrieve font from server or cache list */
      success = _select_X_font_carefully (R___(_plotter)
					  name, is_zero, 
					  _plotter->drawstate->x_label);
      if (success)
	{
	  _plotter->drawstate->font_type = F_OTHER;
	  _plotter->drawstate->x_native_positioning = true;
	  /* these two fields are irrelevant because we don't support
	     switching among user-defined fonts */
	  _plotter->drawstate->typeface_index = 0;
	  _plotter->drawstate->font_index = 1;
	  free (x_name_buf);
	  return true;
	}
    }
  
  /* couldn't retrieve a matching X font; signal failure */
  free (x_name_buf);
  return false;
}

/* _select_X_font_carefully() is a wrapper around _select_X_font() below.
   It attempts to retrieve a font (with charset subsetting) from the X
   server or the font cache.  The charset subset (if any) is specified by a
   string.  NULL means that we are only retrieving the font to look at its
   metrics, so we attempt to retrieve a singleton subset consisting of the
   space character.  If a valid string is supplied, we attempt to retrieve
   a subset that will allow the string to be rendered.  In both cases, in
   the event of failure we attempt a complete retrieval of the font (as
   would be needed on a pre-X11R6 server, for example). */

bool
#ifdef _HAVE_PROTOS
_select_X_font_carefully (R___(Plotter *_plotter) const char *name, bool is_zero[4], const unsigned char *s)
#else
_select_X_font_carefully (R___(_plotter) name, is_zero, s)
     S___(Plotter *_plotter;)
     const char *name;
     bool is_zero[4];
     const unsigned char *s;
#endif
{
  bool success;
  
  if (s == NULL)		/* need metrics only */
    s = (const unsigned char *)" ";

  /* Following #ifdef requested by Georgy Salnikov <sge@nmr.nioch.nsc.ru>,
     to fix a problem with XFree86-3.2.  If an XLFD font name ends in
     *-*[32], i.e. font contains only a single space, the X server reports
     an error retrieving the font, and the client exits.  The bug does not
     surface in XFree86-3.3, or in Solaris-2.4 or Irix-5.3.  Retrieving the
     entire font is what he recommends. */
#ifdef XFREE86_FONTNAME_BUG_FIX
  {
    bool replace_space = true;
    const unsigned char *t = s;
    
    while (*t)
      if (*t++ != ' ')
	{
	  replace_space = false;
	  break;
	}
    if (replace_space)
      s = NULL;
  }
#endif

  success = _select_X_font (R___(_plotter) name, is_zero, s);
  if (s != NULL && success == false)
    /* request entire font */
    success = _select_X_font (R___(_plotter) name, is_zero, NULL);
  
  return success;
}

/* _select_X_font() attempts to retrieve a specified X font by (1) checking
   the list of fonts previously retrieved with XLoadQueryFont(), and (2) if
   the font is not in the list, calling XLoadQueryFont.  X11R6-style
   charset subsetting is supported (subsetting may be specified by passing
   a non-NULL pointer to a string).  If retrieval succeeds, i.e. we obtain
   an XFontStruct, we call _set_X_font_dimensions() to fill in the
   true_font_size, x_font_pixmatrix, font_ascent, font_descent,
   x_native_positioning, and font_is_iso8859_1 fields of the drawing state.
   We don't set the font in the GC used for drawing though (for that, see
   x_alab_x.c). */
bool
#ifdef _HAVE_PROTOS
_select_X_font (R___(Plotter *_plotter) const char *name, bool is_zero[4], const unsigned char *s)
#else
_select_X_font (R___(_plotter) name, is_zero, s)
     S___(Plotter *_plotter;)
     const char *name;
     bool is_zero[4];
     const unsigned char *s;
#endif
{
  plFontRecord *fptr;
  bool subsetting = (s == NULL ? false : true);
  bool found = false;
  unsigned char bitvec[32];

#ifdef DEBUG
  fprintf (stderr, "request for font \"%s\", subsetting=%d\n", name, subsetting);
  fprintf (stderr, "\t subset string = \"%s\"\n", s);
#endif

  /* construct 256-bit vector specifying charset subset */
  if (subsetting)
    _string_to_bitvector (s, bitvec);

  /* attempt to find font in cache list */
  for (fptr = _plotter->x_fontlist; fptr; fptr = fptr->next)
    if (strcmp (name, fptr->name) == 0)
      {
	if ((subsetting && fptr->subset
	       && _is_a_subset (bitvec, fptr->subset_vector))
	    || (subsetting && (fptr->subset == false))
	    || (subsetting == false && fptr->subset == false))
	  {
	    found = true;
	    _plotter->drawstate->x_font_struct = fptr->x_font_struct;
	    break;
	  }
      }

  if (found == false)
    /* no record, try to retrieve font from X server */
    {
      char *tmpname, *tmpname_perm, *_charset_subset_list = NULL;
      int extra = 0;
      plFontRecord *record = 
	(plFontRecord *)_plot_xmalloc (sizeof (plFontRecord));

      if (subsetting)
	{
	  _charset_subset_list = 
	    (char *)_plot_xmalloc ((MAX_CHARSET_SUBSET_LIST_LENGTH + 1) * sizeof (char));
	  _print_bitvector (bitvec, _charset_subset_list);
	  extra = strlen (_charset_subset_list);
	}
      tmpname_perm = (char *)_plot_xmalloc (1 + strlen (name));
      strcpy (tmpname_perm, name);
      tmpname = (char *)_plot_xmalloc (1 + strlen (name) + extra);
      strcpy (tmpname, name);
      if (subsetting)
	{
	  /* append X11R6 list-of-ranges to name to be sent to server */
	  strcat (tmpname, _charset_subset_list);
	  free (_charset_subset_list);
	}

#ifdef DEBUG
      fprintf (stderr, "attempting to retrieve uncached font \"%s\"\n", tmpname);
#endif

      _plotter->drawstate->x_font_struct = 
	XLoadQueryFont (_plotter->x_dpy, tmpname);
      free (tmpname);
      
      /* Whether successfully retrieved or not, add a record of this font
	 to cache list.  (If retrieval failed, recorded x_font_struct will
	 be NULL.) */
      record->name = tmpname_perm; /* stored name doesn't include subset */
      record->x_font_struct = _plotter->drawstate->x_font_struct;
      record->subset = subsetting;
      if (subsetting)
	memcpy (record->subset_vector, bitvec, 32 * sizeof (unsigned char));
      record->next = _plotter->x_fontlist;
      _plotter->x_fontlist = record;

      if (_plotter->drawstate->x_font_struct)
	/* successfully retrieved a font from the server */
	{
#ifdef DEBUG
	  fprintf (stderr, "retrieved font \"%s\"\n", name);
#endif	  
	  /* fill in abovementioned six fields of the drawing state */
	  _set_X_font_dimensions (R___(_plotter) is_zero);
	  /* copy these six fields into the record in the cache */
	  record->true_font_size = _plotter->drawstate->true_font_size;
	  record->font_pixmatrix[0] = _plotter->drawstate->x_font_pixmatrix[0];
	  record->font_pixmatrix[1] = _plotter->drawstate->x_font_pixmatrix[1];
	  record->font_pixmatrix[2] = _plotter->drawstate->x_font_pixmatrix[2];
	  record->font_pixmatrix[3] = _plotter->drawstate->x_font_pixmatrix[3];
	  record->font_ascent = _plotter->drawstate->font_ascent;
	  record->font_descent = _plotter->drawstate->font_descent;
	  record->native_positioning = _plotter->drawstate->x_native_positioning;
	  record->font_is_iso8859_1 = _plotter->drawstate->font_is_iso8859_1;

	  return true;		/* X font selected */
	}
      else
	/* couldn't find font either in cache or on server */
	{
#ifdef DEBUG
	  fprintf (stderr, "failed to retrieve font \"%s\"\n", name);
#endif	  
	  return false;
	}
    }
  else
    /* record for font was found in cache */
    {
      if (_plotter->drawstate->x_font_struct) 
	/* font record was a genuine one */
	{
#ifdef DEBUG
	  fprintf (stderr, "found in cache: \"%s\"\n", name);
#endif
	  /* copy abovementioned six fields from cached font record */
	  _plotter->drawstate->true_font_size = fptr->true_font_size;
	  _plotter->drawstate->x_font_pixmatrix[0] = fptr->font_pixmatrix[0];  
	  _plotter->drawstate->x_font_pixmatrix[1] = fptr->font_pixmatrix[1];
	  _plotter->drawstate->x_font_pixmatrix[2] = fptr->font_pixmatrix[2];
	  _plotter->drawstate->x_font_pixmatrix[3] = fptr->font_pixmatrix[3];
	  _plotter->drawstate->font_ascent = fptr->font_ascent;
	  _plotter->drawstate->font_descent = fptr->font_descent;
	  _plotter->drawstate->x_native_positioning = fptr->native_positioning;
	  _plotter->drawstate->font_is_iso8859_1 = fptr->font_is_iso8859_1;

	  return true;		/* X font selected */
	}
      else
	{
#ifdef DEBUG
	  fprintf (stderr, "found in cache: NULL record for \"%s\"\n", name);
#endif
	  /* we don't have a font; previous retrieval attempt failed */
	  return false;		/* X font not selected */
	}	  
    }  
}

/* _set_X_font_dimensions() is called when an X font has been retrieved and
   an XFontStruct placed in the x_font_struct field of the
   _plotter->drawstate structure.  It attempts to compute and fill in
   several other fields.  This includes (1) the `true_font_size' field
   (essentially pixel size, see comments at head of file, transformed from
   the device frame to the user frame; it's what will be returned by
   e.g. the fontsize() operation), (2) the `pixmatrix' field, and (3) the
   `x_native_positioning' field.

   If the font has a server-returned XLFD name, i.e., a FONT property that
   is an official XLFD string, the pixel size and pixel matrix are
   extracted from it; if not, we try to extract (or compute) the PIXEL_SIZE
   property of the font.  Only if we fail at this do we punt, and fill in
   the `true_font_size' field from the font height (i.e. font ascent + font
   descent) provided by the XFontStruct.  The `x_native_positioning' field
   is set if the pixel matrix is proportional to the identity matrix.

   We also fill in (4) the `font_ascent' field, and (5) the `font_descent'
   field.  Either they are taken from properties, or they are computed from
   the `x_font_struct' field.  We fill in (6) the `font_is_iso8859_1' field
   too.

   If the font doesn't have an XLFD name, various kludges are used. */

void
#ifdef _HAVE_PROTOS
_set_X_font_dimensions(R___(Plotter *_plotter) bool is_zero[4])
#else
_set_X_font_dimensions(R___(_plotter) is_zero)
     S___(Plotter *_plotter;)
     bool is_zero[4];
#endif
{
  unsigned long retval;
  char *name, *pixel_field;
  double size;
  char *charset_major_field, *charset_minor_field;

#ifdef DEBUG
  {
    int i;

    for (i = 0; i < _plotter->drawstate->x_font_struct->n_properties; i++)
      fprintf (stderr, "\tproperty %s [atom %lu] is %ld\n", 
	       XGetAtomName(_plotter->x_dpy, _plotter->drawstate->x_font_struct->properties[i].name),
	       _plotter->drawstate->x_font_struct->properties[i].name,
	       _plotter->drawstate->x_font_struct->properties[i].card32);
  }    
#endif

  if (XGetFontProperty (_plotter->drawstate->x_font_struct, XA_FONT, &retval))
    /* this font has a FONT property, as any well behaved font should */
    {
      /* Extract relevant fields from this property (i.e. from X server's
	 idea of the font name).  This will work if it's an XLFD name. */
      name = XGetAtomName(_plotter->x_dpy, retval); 
      /* N.B. it's here that the bogus tiny entries ("1e-311") can show up */

#ifdef DEBUG
      fprintf (stderr, "FONT property is \"%s\"\n", name);
#endif
      pixel_field = _xlfd_field (name, FIELD_PIXELS);
      charset_major_field = _xlfd_field (name, FIELD_CHARACTER_SET_MAJOR);
      charset_minor_field = _xlfd_field (name, FIELD_CHARACTER_SET_MINOR);
      XFree (name);

      /* determine whether font encoding is ISO-Latin-1 */
      if ((charset_major_field != NULL) && (charset_minor_field != NULL)
	  && strcasecmp (charset_major_field, "iso8859") == 0
	  && charset_minor_field[0] == (char)'1')
	_plotter->drawstate->font_is_iso8859_1 = true;
      else
	_plotter->drawstate->font_is_iso8859_1 = false;

      if (charset_major_field)
	free (charset_major_field);
      if (charset_minor_field)
	free (charset_minor_field);

      if (pixel_field != NULL)	
	/* font presumably has an XLFD name, since it has a pixel field */
	{
	  /* extract x_font_pixmatrix, x_native_positioning from the pixel
             field; keep track of which elements of pixmatrix should be
             exactly zero */
	  _parse_pixmatrix (pixel_field, _plotter->drawstate->x_font_pixmatrix, 
			    &(_plotter->drawstate->x_native_positioning),
			    is_zero);
	  free (pixel_field);
	  
	  /* compute true_font_size from the pixmatrix */
	  {
	    double sx, sy;
	    double ux, uy;
	    
	    /* (sx, sy) is a sort of minimum inter-line displacement
               vector, in the device frame (i.e. in terms of pixel coors) */
	    sx = _plotter->drawstate->x_font_pixmatrix[2];
	    sy = - _plotter->drawstate->x_font_pixmatrix[3];	  

	    /* (ux, uy) is the same, in the user frame; it should be
	       perpendicular to the escapement vector, the angle of which
	       the user specifies with textangle() */
	    ux = XUV (sx, sy);
	    uy = YUV (sx, sy);

	    /* `true_font_size' is length of this vector, in user frame */
	    _plotter->drawstate->true_font_size = sqrt (ux*ux + uy*uy);
	  }
	  
	    /* Try to fill in the font_{descent,ascent} fields by
	       retrieving the RAW_DESCENT and RAW_ASCENT properties of the
	       font.  (``What descent and ascent would be for a 1000-pixel
	       untransformed font''.)  If they don't exist, this must be an
	       XLFD font without a matrix pixmatrix (hence no rotation).
	       So in that case we get the descent and ascent from the
	       XFontStruct, instead.

	       These two fields are used only for computing vertical
	       offsets when writing label strings (see x_alab_x.c). */
	  {
	    Atom raw_descent_atom, raw_ascent_atom;
	    bool descent_success, ascent_success;
	    unsigned long font_raw_descent, font_raw_ascent;
	    
	    raw_descent_atom = XInternAtom (_plotter->x_dpy, "RAW_DESCENT", false);
	    raw_ascent_atom = XInternAtom (_plotter->x_dpy, "RAW_ASCENT", false);
	    
	    descent_success = 
	      XGetFontProperty (_plotter->drawstate->x_font_struct, 
				raw_descent_atom, &font_raw_descent) ? true : false;
	    
	    ascent_success = 
	      XGetFontProperty (_plotter->drawstate->x_font_struct, 
				raw_ascent_atom, &font_raw_ascent) ? true : false;

#ifdef DEBUG
	    if (descent_success)
	      fprintf (stderr, "RAW_DESCENT property is \"%lu\"\n", 
		       font_raw_descent);
	    else
	      fprintf (stderr, "RAW_DESCENT property does not exist\n");
	    if (ascent_success)
	      fprintf (stderr, "RAW_ASCENT property is \"%lu\"\n", 
		       font_raw_ascent);
	    else
	      fprintf (stderr, "RAW_ASCENT property does not exist\n");
#endif

	    /* If no success, this must be a pre-X11R6 font of some kind
	       (so we can assume the transformation from user coordinates
	       to device coordinates is a uniform scaling).  So we get the
	       descent and ascent from the XFontStruct and
	       inverse-transform them to `1000-pixel font units', assuming
	       that the pixel matrix is a multiple of the identity. */

	    if (!descent_success)
	      {
		font_raw_descent 
		  = IROUND(1000.0 * _plotter->drawstate->x_font_struct->descent 
			   / _plotter->drawstate->x_font_pixmatrix[3]);
#ifdef DEBUG
		fprintf (stderr, "kludged RAW_DESCENT property is \"%lu\", from %d\n", 
			 font_raw_descent, _plotter->drawstate->x_font_struct->descent);
#endif		
	      }

	    if (!ascent_success)
	      {
		font_raw_ascent 
		  = IROUND(1000.0 * _plotter->drawstate->x_font_struct->ascent 
			   / _plotter->drawstate->x_font_pixmatrix[3]);
#ifdef DEBUG
		fprintf (stderr, "kludged RAW_ASCENT property is \"%lu\", from %d\n", 
			 font_raw_ascent, _plotter->drawstate->x_font_struct->ascent);
#endif		
	      }
	    
	    _plotter->drawstate->font_ascent 
	      = ((double)font_raw_ascent / 1000.0) * _plotter->drawstate->true_font_size;
	    _plotter->drawstate->font_descent
	      = ((double)font_raw_descent / 1000.0) * _plotter->drawstate->true_font_size;
	  }

	  /* we've set all fields, so we can return */
	  return;
	}
    }
  else
    {
#ifdef DEBUG
      fprintf (stderr, "FONT property does not exist\n");
#endif
    }
  
  /* if we reached here, font doesn't have an XLFD name (so no pixel size
     field, and hence no rotation), or there's no FONT property at all (a
     bad situation) */

  _plotter->drawstate->font_is_iso8859_1 = false;

  {
    Atom pixel_size_atom;
    
    pixel_size_atom = XInternAtom (_plotter->x_dpy, "PIXEL_SIZE", false);
    if (XGetFontProperty (_plotter->drawstate->x_font_struct, 
			  pixel_size_atom, &retval))
      /* there's a PIXEL_SIZE property, so use it to compute font size */
      {
#ifdef DEBUG
	fprintf (stderr, "PIXEL_SIZE property is \"%lu\"\n", retval);
#endif	
	size = (double)retval;
      }
    else
      /* no PIXEL_SIZE, so try to compute size from POINT_SIZE and
	 RESOLUTION_Y properties */
      {
	Atom resolution_y_atom;
	unsigned long point_size, resolution_y;
	
#ifdef DEBUG
	fprintf (stderr, "PIXEL_SIZE property does not exist\n");
#endif
	resolution_y_atom = XInternAtom (_plotter->x_dpy, "RESOLUTION_Y", false);
	if (XGetFontProperty (_plotter->drawstate->x_font_struct, 
			      XA_POINT_SIZE, &point_size)
	    && (XGetFontProperty (_plotter->drawstate->x_font_struct, 
				  resolution_y_atom, &resolution_y)))
	  {
#ifdef DEBUG
	    fprintf (stderr, "POINT_SIZE property is \"%lu\"\n", 
		     point_size);
	    fprintf (stderr, "RESOLUTION_Y property is \"%lu\"\n", 
		     resolution_y);
#endif	
	    size = (double)point_size * (double)resolution_y / 722.7;
	  }
	else 
	  /* we can't compute the font size legitimately, so punt: estimate
	     it from the XFontStruct (may not be reliable) */
	  {
#ifdef DEBUG
	    fprintf (stderr, "POINT_SIZE and/or RESOLUTION_Y properties do not exist\n");
#endif
	    size = (double)(_plotter->drawstate->x_font_struct->ascent 
			    + _plotter->drawstate->x_font_struct->descent);
	  }
      }

    /* now that the font size is known, handcraft a pixmatrix for this
       (presumed) pre-XLFD font */
    _plotter->drawstate->x_font_pixmatrix[0] = size;
    _plotter->drawstate->x_font_pixmatrix[1] = 0.0;
    _plotter->drawstate->x_font_pixmatrix[2] = 0.0;
    _plotter->drawstate->x_font_pixmatrix[3] = size;

    {
      double ux, uy;
      
      /* (ux, uy) is a sort of minimum inter-line displacement vector, in
	 the user frame */
      ux = XUV (0.0, size);
      uy = YUV (0.0, size);
      
      /* the true_font_size field is the length of this vector */
      _plotter->drawstate->true_font_size = sqrt(ux*ux + uy*uy);
    }
    
    /* for non-XLFD fonts, we use these alternative definitions (kludges)
       for the descent and ascent drawing state fields */
    _plotter->drawstate->font_descent 
      = _plotter->drawstate->true_font_size 
	* _plotter->drawstate->x_font_struct->descent 
	/ _plotter->drawstate->x_font_pixmatrix[3];
    
    _plotter->drawstate->font_ascent 
      = _plotter->drawstate->true_font_size 
	* _plotter->drawstate->x_font_struct->ascent 
	/ _plotter->drawstate->x_font_pixmatrix[3];
  }

  return;
}

/* Extract a field from an XLFD name string, by number, and return it, via
   a call to malloc.  If `name' doesn't appear to be an XLFD name, NULL is
   returned. */

static char *
#ifdef _HAVE_PROTOS
_xlfd_field(const char *name, int field)
#else
_xlfd_field(name, field)
     const char *name;
     int field;
#endif
{
  const char *p;
  const char *fields[NUM_XLFD_FIELDS];
  char *retstring;
  int len[NUM_XLFD_FIELDS];
  int i, n, m;
  /* split into fields at hyphens */
  for (p = name, i = 0, n = 0, m = 0; 
       *p && (i < NUM_XLFD_FIELDS); 
       p++, n++, m++)
    {
      if (*p == '-')
	{
	  if (i > 0)
	    len[i-1] = n;
	  n = 0;
	  fields[i++] = p;
	}
    }
  if (i < NUM_XLFD_FIELDS)
    return NULL;

  len[NUM_XLFD_FIELDS - 1] = strlen (name) - (m - 1); /* final field exhausts string */

  /* for len[] and fields[], each field includes initial hyphen */
  retstring = (char *)_plot_xmalloc (len[field] * sizeof(char));
  strncpy (retstring, fields[field] + 1, 
	   (unsigned int)(len[field] - 1)); /* skip initial - */
  retstring[len[field] - 1] = '\0';
  
  return retstring;
}     

/* _parse_pixmatrix() parses a string (the pixel string from an XLFD font
   name).  If the string is a scalar pixel size, the parsed pixel matrix
   (stored in d[]) is defined to be [size 0.0 0.0 size].  If the string is
   an actual pixel matrix, its four elements are copied into d[].

   This routine also sets the x_native_positioning field, depending on
   whether or not the pixel size was a scalar.  In the scalar case, native
   X11 positioning for characters within a string may be used; in the
   non-scalar case, it may not.

   It's in this routine that we use the advisory is_zero[] array, which is
   passed down from several levels above.  If any element of the pixel
   matrix is, or should be, quite close to zero, we set it to zero exactly.
   This works around the problem that silly, tiny floating point numbers
   (e.g., 1e-311) passed back from some X servers are so small they can't
   be manipulated arithmetically on DEC Alphas. */

static void
#ifdef _HAVE_PROTOS
_parse_pixmatrix(const char *pixel_string, double d[4], bool *x_native_positioning, bool is_zero[4]) 
#else
_parse_pixmatrix(pixel_string, d, x_native_positioning, is_zero) 
     const char *pixel_string; 
     double d[4]; 
     bool *x_native_positioning; 
     bool is_zero[4];
#endif
{ 
  int len, i;
  char *s[4];
  
  len = strlen (pixel_string);
  for (i = 0; i < 4; i++)
    s[i] = (char *)_plot_xcalloc (1, (len + 1) * sizeof(char));
  sscanf (pixel_string, "[ %s %s %s %s ]", s[0], s[1], s[2], s[3]);

  if (*(s[0]) && *(s[1]) && *(s[2]) && *(s[3]))
    /* if we got four nonempty strings, good... */
    {
      int j;
      
      for (j = 0; j < 4; j++)
	{
	  char *p;
	  
	  /* convert all hyphens to minuses */
	  for (p = s[j]; *p; p++)
	    if (*p == '~')
	      *p = '-';

	  /* convert strings to doubles */
	  if (is_zero[j])
	    /* set to zero _exactly_, no matter what the server says */
	    d[j] = 0.0;
	  else
	    sscanf (s[j], "%lf", &(d[j]));
	  *x_native_positioning = false;
	}
    }

  /* otherwise parse pixel field as a scalar integer size */
  else
    {
      int size;
      
      sscanf (pixel_string, "%d", &size);
      d[0] = (double)size;
      d[1] = 0.0;
      d[2] = 0.0;
      d[3] = (double)size;
      *x_native_positioning = true;
    }
  
  for (i = 0; i < 4; i++)
    free (s[i]);

  return;
}

/* This prepares a bit vector (length 256 bits, i.e. 32 bytes) indicating
   which characters in the range 1..255 are used in a string. */

static void
#ifdef _HAVE_PROTOS
_string_to_bitvector (const unsigned char *s, unsigned char v[32])
#else
_string_to_bitvector (s, v)
     const unsigned char *s;
     unsigned char v[32];
#endif
{
  unsigned char c;
  int k;

  for (k = 0; k < 32; k++)
    v[k] = 0;

  while ((c = *s) != (unsigned char)'\0')
    {
      unsigned int i, j;
      
      i = c / 8;
      j = c % 8;
#ifdef DEBUG2
      fprintf (stderr, "saw char %d (i.e. %c), stored as %d,%d\n", c, c, i, j);
#endif
      v[i] |= (1 << j);
      s++;
    }
}

/* This writes a bitvector as a string, in the form used in X11R6-style
   charset subsetting.  Each range of chars may require the writing of up
   to 8 bytes, e.g. " 160_180". The list of ranges is contained within
   brackets. */

static void
#ifdef _HAVE_PROTOS
_print_bitvector (unsigned char v[32], char *s)
#else
_print_bitvector (s, v)
     unsigned char v[32];
     char *s;
#endif
{
  int i, num_ranges_output = 0, num_chars_output = 0;
  int start_of_range = 0;
  bool used;
  bool in_range = false;
  
  *s++ = '[';
  for (i = 0; i <= 256; i++)
    {
      if (i == 256)
	used = false;
      else
	used = (v[i / 8] & (1 << (i % 8))) ? true : false;

#ifdef DEBUG2
      if (used)
	fprintf (stderr, "stored char %d (i.e. %c), from %d,%d\n", i, i, i/8, i%8);
#endif

      if (used && in_range == false)
	/* begin new range */
	{
	  start_of_range = i;
	  in_range = true;
	}
      else if (used == false && in_range)
	/* end of range, so output the range */
	{
	  int hundreds, tens, ones;
	  bool hundreds_output;

	  if (num_chars_output > MAX_CHARSET_SUBSET_LIST_LENGTH - 8)
	    break;		/* abort to avoid buffer overrun */

	  if (num_ranges_output > 0)
	    /* use space as separator */
	    {
	      *s++ = ' ';
	      num_chars_output++;
	    }
	  
#ifdef DEBUG2
	  fprintf (stderr, "outputting character range %d..%d, i.e. %c..%c\n",
		   start_of_range, i-1, start_of_range, i-1);
#endif
	  if (start_of_range < (i - 1))
	    /* have a genuine range, start..(i-1), not a singleton */
	    {
	      /* output start of range, followed by underscore */
	      hundreds = start_of_range / 100;
	      tens = (start_of_range - hundreds * 100) / 10;
	      ones = start_of_range % 10;
	      hundreds_output = false;
	      if (hundreds > 0)
		{
		  *s++ = (char)'0' + hundreds;
		  hundreds_output = true;
		  num_chars_output++;
		}
	      if (hundreds_output || tens > 0)
		{
		  *s++ = (char)'0' + tens;
		  num_chars_output++;
		}
	      *s++ = (char)'0' + ones;
	      num_chars_output++;
	      *s++ = (char)'_';
	      num_chars_output++;	      
	    }

	  /* output end of range, which is i-1 */
	  hundreds = (i-1) / 100;
	  tens = ((i-1) - hundreds * 100) / 10;
	  ones = (i-1) % 10;
	  hundreds_output = false;
	  if (hundreds > 0)
	    {
	      *s++ = (char)'0' + hundreds;
	      hundreds_output = true;
	      num_chars_output++;
	    }
	  if (hundreds_output || tens > 0)
	    {
	      *s++ = (char)'0' + tens;
	      num_chars_output++;
	    }
	  *s++ = (char)'0' + ones;
	  num_chars_output++;

	  /* no longer in range */
	  in_range = false;
	  num_ranges_output++;
	}
    }
  *s++ = ']';
  /* add final null */
  *s = '\0';
}

static bool
#ifdef _HAVE_PROTOS
_is_a_subset (unsigned char set1[32], unsigned char set2[32])
#else
_is_a_subset (set1, set2)
     unsigned char set1[32];
     unsigned char set2[32];
#endif
{
  int i;
  bool retval = true;
  
  for (i = 0; i < 32; i++)
    if (set1[i] & ~(set2[i]))
      {
	retval = false;
	break;
      }

  return retval;
}
