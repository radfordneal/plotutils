/* This file contains the _retrieve_font() function, an internal routine in
   libplotX.  It is called by fontname(), fontsize(), and textangle(),
   because changing any of the elements {name, size, rotation} of the
   _drawstate structure can dictate a change of font.

   The routine _retrieve_font_internal() is what actually retrieves the
   font from the X server.  We run through various possibilities: first we
   interpret `name' as one of our built-in aliases (e.g. "times-roman"),
   then we try to intepret it as an XLFD name
   (e.g. "times-medium-r-normal"), etc.  See comments far below, just
   before the routine.  Note that we also support retrieval of fonts with
   pre-XLFD names (e.g. "fixed").

   If a font is retrieved, _drawstate->font_type and
   _drawstate->x_font_size are set (and also _drawstate->font_struct and
   _drawstate->font_raw_{ascent,descent} if it is a genuine X font rather
   than a vector font).  If after going through various contortions we
   can't retrieve the desired font or a reasonable facsimile, we print an
   error message.

   `x_font_size' is a bit difficult to determine for an X font.  Ideally it
   would be a minimum interline spacing, because that is what must be
   returned (in user units) by fontname(), fontsize, and textangle().  But
   even for non-rotated, non-sheared fonts, there are several possibilities
   for this: (1) the so-called pixel size (which can be extracted only if
   the font has an XLFD name, or if the font has a PIXEL_SIZE or POINT_SIZE
   property), (2) the sum of the `logical' font_ascent and font_descent
   (from the font structure), (3) the sum of the maximum (over characters)
   ascent and the maximum descent, (4) the sum of the cap height and the
   maximum descender length (which may both be difficult to extract), etc.

   After some thought, I decided to use (1).  It is easy to compute the
   analogue for affinely transformed fonts (from the pixel matrix).
   However, for old non-XLFD (and hence non-rotated, non-sheared) fonts,
   there is the possibility that the PIXEL_SIZE property may not be
   available.  For such fonts, the POINT_SIZE and RESOLUTION_Y properties
   are used to compute it.  If they in turn are not available, we fall back
   on definition (2).

   The _drawstate->font_raw_{ascent,descent} fields are filled in, for X
   fonts, by our best guess as to what the font ascent and descent would
   be, for a version of the font with pixel_size=1000.  For affinely
   transformed fonts, e.g., we obtain this information from the RAW_ASCENT
   and RAW_DESCENT properties.  (We use the font_raw_{ascent,descent}
   fields in alabel_X(), for vertical justification). */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* An XLFD template, with holes into which we can punch the base name (a
   string with exactly three hyphens in it) and the integer size in terms
   of decipoints */
static char *_xlfd_template = "-*-%s-*-*-%d-*-*-*-*-*-*";

/* XLFD template for an affinely transformed font (with holes for the base
   name and for four dimensions, in terms of points) */
static char *_xlfd_template_with_scaling = "-*-%s-*-*-[%s %s %s %s]-*-*-*-*-*-*";

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
#define FIELD_CHARACTER_SET_MINOR 13

/* We keep track of which fonts we request from the X server, by
   constructing a linked list of these records.  A linked list is good
   enough if we don't have huge numbers of font changes. */
typedef struct lib_fontrecord
{
  char *name;			/* font name, e.g. XLFD name */
  XFontStruct *x_font_struct;	/* font structure */
  struct lib_fontrecord *next;	/* most previously retrieved font */
} Fontrecord;

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static char *_xlfd_field P__((const char *name, int field));
static void _parse_pixmatrix P__((const char *pixel_string, double d[4], 
			   Boolean *native_positioning));
static void _set_x_font_dimensions P__((XFontStruct *x_font_struct));
#undef P__

void
_retrieve_font ()
{
  int i;
  
  /* is font a vector font? */
  i = -1;
  while (_vector_font_info[++i].name)
    {
      if (_vector_font_info[i].visible)	/* i.e. font not internal-only */
	if (strcasecmp (_vector_font_info[i].name, _drawstate->font_name) == 0
	    || strcasecmp (_vector_font_info[i].othername, _drawstate->font_name) == 0)
	  {
	    _drawstate->font_type = F_STROKE;
	    _drawstate->typeface_index = _vector_font_info[i].typeface_index;
	    _drawstate->font_index = _vector_font_info[i].font_index;
	    _drawstate->font_is_iso8859 = _vector_font_info[i].iso8859_1;
	    
	    /* no quantization or other funny business */
	    _drawstate->x_font_size = _drawstate->font_size;
	    _drawstate->x_font_struct = (XFontStruct *)NULL; /* don't try to free */
	    return;
	  }
    }

  /* try to retrieve a font with specified (name, size, rotation) */
  if (_retrieve_font_internal(_drawstate->font_name, 
			      _drawstate->font_size, 
			      _drawstate->text_rotation))
    return;

  /* try to retrieve a font with default font name and specified font size,
     at the specified rotation angle */
  if (_retrieve_font_internal(_default_drawstate.font_name, 
			      _drawstate->font_size, 
			      _drawstate->text_rotation))
    return;
  
  /* try to retrieve a font with specified name and size, at zero rotation
     angle */
  if (_retrieve_font_internal(_drawstate->font_name, 
			      _drawstate->font_size, 
			      0.0))
    {
      fprintf (stderr, "libplot: couldn't retrieve a font \"%s\", or default font \"%s\", with size=%g, at rotation angle %g degrees; substituting \"%s\" at 0 degrees\n",
	       _drawstate->font_name, _default_drawstate.font_name,
	       _drawstate->font_size, _drawstate->text_rotation,
	       _drawstate->font_name);
      
      return;
    }
      
  /* try to retrieve a font with default font name and specified font size, at zero
     rotation angle */
  
  if (_retrieve_font_internal(_default_drawstate.font_name, 
			      _drawstate->font_size,
			      0.0))
      {
	fprintf (stderr, "libplot: couldn't retrieve  font \"%s\", or default font \"%s\", with size=%g, at rotation angle %g degrees; substituting \"%s\" at 0 degrees\n",
		 _drawstate->font_name, _default_drawstate.font_name,
		 _drawstate->font_size, _drawstate->text_rotation,
		 _default_drawstate.font_name);
	
	return;
      }

  /* complete failure */
  fprintf (stderr, "libplot: couldn't retrieve font \"%s\", or default font \"%s\", with size=%g, at rotation angle %g degrees or 0 degrees\n",
	   _drawstate->font_name, _default_drawstate.font_name,
	   _drawstate->font_size, _drawstate->text_rotation);
  
  return;
}

/* _retrieve_font_internal() retrieves an X font specified by a triple,
   namely {name, size, rotation}.  Three possible interpretations are
   tried, in order.

   1. `name' is taken to be an alias for an XLFD base name, as listed in
   our table in fontdb.c.  (Aliases for the 35 standard font names appear
   there).  E.g., name="times-roman".

   2. `name' is taken to be an XLFD base name (no aliasing), of the form
   family-weight-slant-width, with exactly three hyphens.  E.g.,
   name="grotesk-bold-r-normal", or "times-medium-r-normal".

   3. `name' is taken to be an XLFD base name (no aliasing), of the form
   foundry-family-weight-slant-width, with exactly four hyphens.  E.g.,
   name="bitstream-courier-medium-r-normal".  NOT YET IMPLEMENTED.

   4. `name' is taken to be a full X font name, in which case `size' and
   `rotation' are ignored.  E.g.,
   name="-schumacher-clean-medium-r-normal--6-60-75-75-c-50-iso8859-1" or
   name="vtbold".  This option is mostly to support old (pre-XLFD) fonts,
   such as "vtbold".

   If a font is successfully retrieved (which will set
   _drawstate->font_struct, _drawstate->x_font_size,
   _drawstate->font_pixmatrix, and _drawstate->font_raw_{ascent,descent},
   and _drawstate->native_positioning), this function sets
   _drawstate->font_type (F_STANDARD in case 1, F_DEVICE_SPECIFIC in cases
   2 and 3 and 4) and returns TRUE.  If a font is not successfully
   retrieved, this function returns FALSE.

   _drawstate->typeface_index and _drawstate->font_index are also set in
   case 1.  */

#define MAX_FONT_NAME_LENGTH 256

Boolean
_retrieve_font_internal(name, size, rotation)
     const char *name;
     double size;
     double rotation;
{
  int i;
  char *x_name;
  double user_size;		/* font size in user units */
  double rot[4];		/* font rotation matrix */

  /* fprintf (stderr, "RFI(%s,%f,%f)\n", name, size, rotation); */

  x_name = (char *)_plot_xcalloc (MAX_FONT_NAME_LENGTH, 1);

  user_size = size;		/* user units */
  rot[0] = cos (M_PI * rotation / 180.0);
  rot[1] = sin (M_PI * rotation / 180.0);
  rot[2] = - sin (M_PI * rotation / 180.0);
  rot[3] = cos (M_PI * rotation / 180.0);

  /* search null-terminated table of standard fonts for a name matching the
     passed name */
  i = -1;
  while (_ps_font_info[++i].ps_name)
    {
      if (strcasecmp (_ps_font_info[i].ps_name, name) == 0
	  || strcasecmp (_ps_font_info[i].x_name, name) == 0)
	break;
    }
  
  /* if a match is found, use it as a base (append size to X base font
     name) */
  if (_ps_font_info[i].ps_name)
    {
      /* build full name of X font, computing pixel matrix values in
	 case we have something other than an unrotated, uniformly
	 scaled font */
      if ((rotation == (double)0.0) 
	  && _drawstate->transform.axes_preserved 
	  && _drawstate->transform.uniform)
	{
	  /* case 1: zero text rotation, uniformly scaled */
	  
	  /*
	  fprintf (stderr, "at %d decipoints\n",
		   IROUND(10.0 * _drawstate->transform.m[0] * user_size));
	  */
	  
	  /* if integer size in terms of decipoints is zero, bail */
	  if (IROUND(10.0 * _drawstate->transform.m[0] * user_size) == 0)
	    return FALSE;
	  /* otherwise punch size in decipoints into the template */
	  sprintf (x_name, _xlfd_template,
		   _ps_font_info[i].x_name, 
		   IROUND(10.0 * _drawstate->transform.m[0] * user_size));
	}
      else
	/* case 2: nonzero font rotation, or non-uniform scaling */
	{
	  double pm[4];	/* pixel matrix */
	  char a[4][256];	/* ascii representation */
	  char *p;
	  int k;
	  
	  /* pixel matrix: product of text rotation matrix, and
	     transformation from user to device coordinates; note flipped-y
	     convention affecting _drawstate->transform.m[1] and
	     _drawstate->transform.m[3].  A factor user_size is also
	     included; see below. */
	  pm[0] =  (rot[0] * _drawstate->transform.m[0] 
		    + rot[1] * _drawstate->transform.m[2]);
	  pm[1] =  - (rot[0] * _drawstate->transform.m[1] 
		      + rot[1] * _drawstate->transform.m[3]);
	  pm[2] =  (rot[2] * _drawstate->transform.m[0] 
		    + rot[3] * _drawstate->transform.m[2]);
	  pm[3] =  - (rot[2] * _drawstate->transform.m[1] 
		      + rot[3] * _drawstate->transform.m[3]);
	  
	  /* ascii entries in pixel matrix string */
	  
	  for (k = 0; k < 4; k++)
	    {
	      sprintf (a[k], "%f", user_size * pm[k]);
	    }
	  
	  /* convert minus signs to tildes */
	  for (k = 0; k < 4; k++)
	    for (p = a[k]; *p; p++)
	      if (*p == '-')
		*p = '~';
	  
	  sprintf (x_name, _xlfd_template_with_scaling,
		   _ps_font_info[i].x_name, a[0], a[1], a[2], a[3]);
	}

      /* try to call XLoadQueryFont on the constructed name */
      if (_x_set_font (x_name))
	{
	  _drawstate->font_type = F_STANDARD;
	  _drawstate->typeface_index = _ps_font_info[i].typeface_index;
	  _drawstate->font_index = _ps_font_info[i].font_index;
	  
	  return TRUE;
	}
    }
  
  /* There wasn't a match, so assume user-specified font name is an XLFD
     base name (e.g. times-medium-r-normal); build full name from it.  */
  
  /* build full name of X font, computing pixel matrix values in case
     we have something other than an unrotated, uniformly scaled font */
  if ((rotation == (double)0.0) 
      && _drawstate->transform.axes_preserved 
      && _drawstate->transform.uniform)
    {
      /* case 1: zero text rotation, uniformly scaled */

      /* if integer size in terms of decipoints is zero, bail */
      if (IROUND(10.0 * _drawstate->transform.m[0] * user_size) == 0)
	return FALSE;
      /* otherwise punch size in decipoints into the template */
      sprintf (x_name, _xlfd_template,
	       name, 
	       IROUND(10.0 * _drawstate->transform.m[0] * user_size));
    }
  else
    /* case 2: nonzero font rotation, or non-uniform scaling */
    {
      double pm[4];	/* pixel matrix */
      char a[4][256];	/* ascii representation */
      char *p;
      int k;
      
      /* pixel matrix: product of text rotation matrix, and transformation
	 from user to device coordinates; note flipped-y convention
	 affecting _drawstate->transform.m[1] and
	 _drawstate->transform.m[3].  A factor user_size is also included;
	 see below. */
      pm[0] =  (rot[0] * _drawstate->transform.m[0] 
		+ rot[1] * _drawstate->transform.m[2]);
      pm[1] =  - (rot[0] * _drawstate->transform.m[1] 
		  + rot[1] * _drawstate->transform.m[3]);
      pm[2] =  (rot[2] * _drawstate->transform.m[0] 
		+ rot[3] * _drawstate->transform.m[2]);
      pm[3] =  - (rot[2] * _drawstate->transform.m[1] 
		  + rot[3] * _drawstate->transform.m[3]);
      
      /* ascii entries in pixel matrix string */
      for (i = 0; i < 4; i++)
	{
	  sprintf (a[i], "%f", user_size * pm[i]);
	}
      
      /* convert minus signs to tildes */
      for (k = 0; k < 4; k++)
	for (p = a[k]; *p; p++)
	  if (*p == '-')
	    *p = '~';
      
      sprintf (x_name, _xlfd_template_with_scaling,
	       name, a[0], a[1], a[2], a[3]);
  
    }
  
  /* try to call XLoadQueryFont on the constructed name */
  if (_x_set_font (x_name))
    {
      _drawstate->font_type = F_DEVICE_SPECIFIC;
      return TRUE;
    }  
  
  /* Didn't work, so try using passed name as a full X fontname; ignore
     size.  This is a kludge, needed mostly to support pre-XLFD fonts. */
  
  if ((rotation == (double)0.0) 
      && _drawstate->transform.axes_preserved 
      && _drawstate->transform.uniform)
    /* We don't do this unless the desired text rotation is zero; after
       all, pre-XLFD fonts all have rotation 0.  Also we insist that the
       user->device coordinate scaling in effect be uniform, so that the
       result won't look too unnatural (maybe should weaken this?). */
    {
      if (_x_set_font (name))
	{
	  _drawstate->font_type = F_DEVICE_SPECIFIC;
	  _drawstate->native_positioning = TRUE;
	  return TRUE;
	}
    }
  
  /* signal failure */
  return FALSE;
}

/* _x_set_font() attempts to select a specified X font by (1) checking the
   list of fonts previously retrieved with XLoadQueryFont(), and (2)
   calling XLoadQueryFont anew, if necessary.  If we can find the font, we
   call _set_font_dimensions() to fill in the x_font_size, font_pixmatrix,
   font_raw_ascent, font_raw_descent, and native_positioning fields of the
   drawing state. */
Boolean
_x_set_font (name)
     const char *name;
{
  static Fontrecord *fontlist_head = NULL;
  Fontrecord *fptr;
  Boolean missing = TRUE;

  for (fptr = fontlist_head; fptr; fptr = fptr->next)
    if (strcmp (name, fptr->name) == 0)
      {
	missing = FALSE;
	_drawstate->x_font_struct = fptr->x_font_struct;
	break;
      }

  if (missing)
    {
      _drawstate->x_font_struct = XLoadQueryFont (_xdata.dpy, name);
      if (_drawstate->x_font_struct)
	/* success, so add to head of our list */
	{
	  char *tmpname = (char *)_plot_xmalloc (1 + strlen (name));
	  Fontrecord *record = (Fontrecord *)_plot_xmalloc (sizeof (Fontrecord));

	  strcpy (tmpname, name);
	  record->name = tmpname;
	  record->x_font_struct = _drawstate->x_font_struct;
	  record->next = fontlist_head;
	  fontlist_head = record;
	}
    }

  if (_drawstate->x_font_struct) /* if we have a font */
    {
      XSetFont (_xdata.dpy, _drawstate->gc, _drawstate->x_font_struct->fid);
      /* fill in abovementioned fields */
      _set_x_font_dimensions (_drawstate->x_font_struct);
      return TRUE;		/* X font selected */
    }
  else
    return FALSE;			/* X font not found */
}  

/* _set_x_font_dimensions(), once an X font has been retrieved, attempts to
   compute and fill several fields of the _drawstate structure.  This
   includes (1) the `x_font_size' field (essentially pixel size, see
   comments at head of file, transformed from the device frame to the user
   frame; it's what gets returned by e.g. fontsize()), (2) the `pixmatrix'
   field, and (3) the `native_positioning' field.

   If the font has an XLFD name, the pixel size and pixel matrix are
   extracted from it; if not, we try to extract (or compute) the PIXEL_SIZE
   property of the font.  Only if we fail at this do we punt, and take the
   font height (i.e. font ascent + font descent) from the XFontStruct, to
   fill the `x_font_size' field.  The native_positioning field is set
   if the pixel matrix is proportional to the identity matrix.

   This routine also fills in (4) the `raw_ascent' field, and (5) the
   `raw_descent' field.  Either they are taken from properties, or they are
   computed from the XFontStruct.  Whether the font has an ISO8859
   character set is also determined. */

static void
_set_x_font_dimensions(font)
     XFontStruct *font;
{
  unsigned long retval;
  char *name, *pixel_field;
  double size;
  char *charset_major_field, *charset_minor_field;

  if (XGetFontProperty (font, XA_FONT, &retval))
    {
      name = XGetAtomName(_xdata.dpy, retval); 
      pixel_field = _xlfd_field (name, FIELD_PIXELS);
      charset_major_field = _xlfd_field (name, FIELD_CHARACTER_SET_MAJOR);
      charset_minor_field = _xlfd_field (name, FIELD_CHARACTER_SET_MINOR);

      XFree (name);

      /* do this first: determine whether font charset is ISO8859 */
      if ((charset_major_field != NULL) && (charset_minor_field != NULL))
	{
	  if (strcasecmp (charset_major_field, "iso8859") == 0)
	    _drawstate->font_is_iso8859 = TRUE;
	  else
	    _drawstate->font_is_iso8859 = FALSE;
	}
      if (charset_major_field)
	free (charset_major_field);
      if (charset_minor_field)
	free (charset_minor_field);

      if (pixel_field != NULL)	
	/* font has an XLFD name, since it has a pixel field */
	{
	  /* extract font_pixmatrix, native_positioning from the pixel
             field */
	  _parse_pixmatrix (pixel_field, _drawstate->font_pixmatrix, 
			    &(_drawstate->native_positioning));
	  free (pixel_field);
	  
	  /* compute x_font_size from the pixmatrix */
	  {
	    double sx, sy;
	    double ux, uy;
	    
	    /* (sx, sy) is a sort of minimum inter-line displacement
               vector, in the device frame (i.e. in terms of pixel coors) */
	    sx = _drawstate->font_pixmatrix[2];
	    sy = - _drawstate->font_pixmatrix[3];	  

	    /* (ux, uy) is the same, in the user frame; it should be
	       perpendicular to the escapement vector, the angle of which
	       the user specifies with textangle() */
	    ux = XUV (sx, sy);
	    uy = YUV (sx, sy);

	    /* `x_font_size' is length of this vector, in user frame */
	    _drawstate->x_font_size = sqrt (ux*ux + uy*uy);
	  }
	  
	    /* Try to fill in the font_raw_{descent,ascent} fields by
	       retrieving the RAW_DESCENT and RAW_ASCENT properties of the
	       font.  (``What descent and ascent would be for a 1000-pixel
	       untransformed font''.)  If they don't exist, this must be an
	       XLFD font without a matrix pixmatrix (hence no rotation).
	       So in that case we get the descent and ascent from the
	       XFontStruct, instead.

	       These two fields are used only for computing vertical
	       offsets when writing label strings (see alabel_X.c). */
	  {
	    Atom raw_descent_atom, raw_ascent_atom;
	    Boolean descent_success, ascent_success;
	    
	    raw_descent_atom = XInternAtom (_xdata.dpy, "RAW_DESCENT", FALSE);
	    raw_ascent_atom = XInternAtom (_xdata.dpy, "RAW_ASCENT", FALSE);
	    
	    descent_success = 
	      XGetFontProperty (_drawstate->x_font_struct, raw_descent_atom, 
				&(_drawstate->font_raw_descent));
	    
	    ascent_success = 
	      XGetFontProperty (_drawstate->x_font_struct, raw_ascent_atom, 
				&(_drawstate->font_raw_ascent));

	    /* If no success, we assume this is a pre-XLFD font (and that
	       the transformation from user coordinates to device
	       coordinates is a uniform scaling, since otherwise we
	       shouldn't be using a pre-XLFD font).  So we get the descent
	       and ascent from the XFontStruct and inverse-transform them
	       to `1000-pixel font units', assuming that the pixel matrix
	       is a multiple of the identity.  Under the above assumptions,
	       this should be true. */

	    if (!descent_success)
	      _drawstate->font_raw_descent 
		= IROUND(1000.0 * _drawstate->x_font_struct->descent 
			 / _drawstate->font_pixmatrix[3]);

	    if (!ascent_success)
	      _drawstate->font_raw_ascent 
		= IROUND(1000.0 * _drawstate->x_font_struct->ascent 
			 / _drawstate->font_pixmatrix[3]);
	  }
	  return;
	}
    }
  
  /* if we reached this point, font doesn't have an XLFD name (so no pixel
     size field, and hence no rotation), or perhaps there's no FONT
     property at all (a bad situation) */

  _drawstate->font_is_iso8859 = FALSE;

  {
    Atom pixel_size_atom;
    
    /* if there's a PIXEL_SIZE property, use it... */
    pixel_size_atom = XInternAtom (_xdata.dpy, "PIXEL_SIZE", FALSE);
    if (XGetFontProperty (font, pixel_size_atom, &retval))
      size = (double)retval;
    else
      /* otherwise try to compute pixel size from POINT_SIZE property */
      {
	Atom resolution_y_atom;
	unsigned long point_size, resolution_y;
	
	resolution_y_atom = XInternAtom (_xdata.dpy, "RESOLUTION_Y", FALSE);
	if (XGetFontProperty (font, XA_POINT_SIZE, &point_size)
	    && (XGetFontProperty (font, resolution_y_atom, &resolution_y)))
	  size = (double)point_size * (double)resolution_y / 722.7;
	else /* if we can't even do that, punt */
	  size = (double)(font->ascent + font->descent);
      }

    /* now that the pixel size is known, handcraft a pixmatrix for 
       this (presumed) pre-XLFD font */
    _drawstate->font_pixmatrix[0] = size;
    _drawstate->font_pixmatrix[1] = 0.0;
    _drawstate->font_pixmatrix[2] = 0.0;
    _drawstate->font_pixmatrix[3] = size;

    {
      double ux, uy;
      
      /* (ux, uy) is a sort of minimum inter-line displacement
	 vector, in the user frame */
      ux = XUV (0.0, size);
      uy = YUV (0.0, size);
      
      /* the x_font_size field is the length of this vector */
      _drawstate->x_font_size = sqrt(ux*ux + uy*uy);
    }
    
    /* for non-XLFD fonts, we use these alternative definitions (kludges)
       for the RAW_DESCENT and RAW_ASCENT properties */
    _drawstate->font_raw_descent 
      = IROUND(1000.0 * _drawstate->x_font_struct->descent 
	       / _drawstate->font_pixmatrix[3]);
    _drawstate->font_raw_ascent 
      = IROUND(1000.0 * _drawstate->x_font_struct->ascent 
	       / _drawstate->font_pixmatrix[3]);
  }
  return;
}

/* Extract a field from an XLFD name string, by number, and return it, via
   a call to malloc.  If `name' doesn't appear to be an XLFD name, NULL is
   returned. */

static char *
_xlfd_field(name, field)
     const char *name;
     int field;
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
  retstring = (char *)_plot_xmalloc ((unsigned int)(len[field]));
  strncpy (retstring, fields[field] + 1, 
	   (unsigned int)(len[field] - 1)); /* skip initial - */
  retstring[len[field] - 1] = '\0';
  
  return retstring;
}     

/* _parse_pixmatrix() parses a string (the pixel string from an XLFD font
   name).  If the string is a scalar pixel size, the parsed pixel matrix
   (stored in d[]) is defined to be [size 0.0 0.0 size].  If the string is
   an actual pixel matrix, its four elements are copied into d[].

   This routine also sets the native_positioning field, depending on
   whether or not the pixel size was a scalar.  In the scalar case, native
   X11 positioning for characters within a string may be used; in the
   non-scalar case, it may not. */

static void
_parse_pixmatrix(pixel_string, d, native_positioning) 
     const char *pixel_string; 
     double d[4]; 
     Boolean *native_positioning; 
{ 
  int len, i;
  char *s[4];
  
  len = strlen (pixel_string);
  for (i = 0; i < 4; i++)
    s[i] = (char *)_plot_xcalloc (1, (unsigned int)(len + 1));
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
	  sscanf (s[j], "%lf", &(d[j]));
	  *native_positioning = FALSE;
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
      *native_positioning = TRUE;
    }
  
  for (i = 0; i < 4; i++)
    free (s[i]);

  return;
}
