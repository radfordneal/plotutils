/* This file contains the fontname, fontsize, and textangle methods, which
   are a GNU extension to libplot.  They set drawing attributes: the name
   of the font used for text subsequent drawn on the graphics device, the
   size, and the text angle. 

   The fontname, fontsize, and textangle methods return the fontsize in
   user units, as an aid to vertical positioning by the user.  (The
   fontsize is normally taken to be a minimum vertical spacing between
   adjacent lines of text.)

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence, e.g., on the arguments given to
   the space() method.

   Note that the size of the font may change when the rotation angle is
   changed, since some fonts may not be available at all rotation angles,
   so that a default font must be switched to.  Also, not all font sizes
   may be available (there may need to be some size quantization).  So the
   return value should always be checked. */

#include "sys-defines.h"
#include "extern.h"

double
#ifdef _HAVE_PROTOS
_API_ffontname (R___(Plotter *_plotter) const char *s)
#else
_API_ffontname (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  char *font_name;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "ffontname: invalid operation");
      return -1;
    }

  /* Null pointer resets to default.  (N.B. we don't look at the font_name
     field in _default_drawstate, because it's a dummy.) */
  if ((s == NULL) || (*s == '\0') || !strcmp(s, "(null)"))
    switch (_plotter->data->default_font_type)
      {
      case F_HERSHEY:
      default:
	s = DEFAULT_HERSHEY_FONT;
	break;
      case F_POSTSCRIPT:
	s = DEFAULT_POSTSCRIPT_FONT;
	break;
      case F_PCL:
	s = DEFAULT_PCL_FONT;
	break;
      case F_STICK:
	s = DEFAULT_STICK_FONT;
	break;
      }

  /* save new font name */
  free ((char *)_plotter->drawstate->font_name);
  font_name = (char *)_plot_xmalloc (strlen (s) + 1);
  strcpy (font_name, s);
  _plotter->drawstate->font_name = font_name;

  /* retrieve font and metrics; compute `true' font size (may differ) */
  _set_font (S___(_plotter));

  /* return value is size in user units */
  return _plotter->drawstate->true_font_size;
}

double
#ifdef _HAVE_PROTOS
_API_ffontsize (R___(Plotter *_plotter) double size)
#else
_API_ffontsize (R___(_plotter) size)
     S___(Plotter *_plotter;) 
     double size;
#endif
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "ffontsize: invalid operation");
      return -1;
    }

  if (size < 0.0)		/* reset to default */
    {
      size = _plotter->drawstate->default_font_size;
      _plotter->drawstate->font_size_is_default = true;
    }
  else
    _plotter->drawstate->font_size_is_default = false;

  /* set the new nominal size in the drawing state */
  _plotter->drawstate->font_size = size;

  /* retrieve font and metrics; compute `true' font size (may differ) */
  _set_font (S___(_plotter));
  
  /* flag fontsize as having been invoked (so that fsetmatrix will no
     longer automatically adjust the font size to a reasonable value) */
  _plotter->data->fontsize_invoked = true;

  /* return quantized user-specified font size */
  return _plotter->drawstate->true_font_size;
}

double
#ifdef _HAVE_PROTOS
_API_ftextangle (R___(Plotter *_plotter) double angle)
#else
_API_ftextangle (R___(_plotter) angle)
     S___(Plotter *_plotter;)
     double angle;
#endif
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "ftextangle: invalid operation");
      return -1;
    }

  /* save new rotation angle */
  _plotter->drawstate->text_rotation = angle;
  
  /* retrieve font and metrics; compute `true' font size (may differ) */
  _set_font (S___(_plotter));
  
  /* return quantized user-specified font size */
  return _plotter->drawstate->true_font_size;
}

/* Below are four rather silly Plotter methods that are an undocumented
   part of the libplot/libplotter API.  Each returns a pointer to the head
   of a font database in g_fontdb.c, so that an application program that is
   too nosy for its own good can pry out font information.

   These should be replaced by a properly crafted API for querying font
   names, font metrics, etc. */

voidptr_t
#ifdef _HAVE_PROTOS
pl_get_hershey_font_info (S___(Plotter *_plotter))
#else
pl_get_hershey_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_hershey_font_info;
}

voidptr_t
#ifdef _HAVE_PROTOS
pl_get_ps_font_info (S___(Plotter *_plotter))
#else
pl_get_ps_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_ps_font_info;
}

voidptr_t
#ifdef _HAVE_PROTOS
pl_get_pcl_font_info (S___(Plotter *_plotter))
#else
pl_get_pcl_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_pcl_font_info;
}

voidptr_t
#ifdef _HAVE_PROTOS
pl_get_stick_font_info (S___(Plotter *_plotter))
#else
pl_get_stick_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_stick_font_info;
}

