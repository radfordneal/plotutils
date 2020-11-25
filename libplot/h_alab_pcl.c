#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "h_stick.h"

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))
#define MIN_TERMINATOR 0x21
#define MAX_TERMINATOR 0xfe

/* forward references */
static unsigned char _label_terminator __P((const unsigned char *s));

double
#ifdef _HAVE_PROTOS
_h_falabel_pcl (const unsigned char *s, int h_just)
#else
_h_falabel_pcl (s, h_just)
     const unsigned char *s;
     int h_just;  /* horizontal justification: JUST_LEFT, CENTER, or RIGHT */
#endif
{
  int master_font_index, typeface;
  double theta, costheta, sintheta;
  double width;
  unsigned char *t;
  unsigned char terminator;
  bool stick_font = false;
  
  if (*s == (unsigned char)'\0')
    return 0.0;

  if (h_just != JUST_LEFT)
    {
      _plotter->warning ("ignoring request to use non-default justification for a label");
      return 0.0;
    }

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* width of the string in user units */
  /* N.B. we don't re-encode Stick font (see below), since it's monospaced */
  width = _plotter->flabelwidth_pcl (s);

  /* compute index of font in master table of PCL fonts, in g_fontdb.h */
  master_font_index =
    (_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  typeface = _pcl_font_info[master_font_index].pcl_typeface;  
  stick_font = (typeface == HPGL2_STICK);

  if (stick_font)
    /* font is a HP-GL/2 Stick font, which uses HP's Roman-8 encoding, so
       re-encode ISO-Latin-1 as Roman-8 */
      {
	unsigned const char *sptr = s;
	unsigned char *tptr;
	
	t = (unsigned char *)_plot_xmalloc (strlen ((const char *)s) + 1);
	tptr = t;
	while (*sptr)
	  {
	    if (*sptr < 0x80)
	      *tptr++ = *sptr++;
	    else
	      *tptr++ = _iso_to_roman8[(*sptr++) - 0x80];
	  }
	*tptr = '\0';

	/* shift rightward to compensate for the fact that characters in
	   fixed-width Stick fonts are off-center, positioned in the left
	   2/3 of the cell */
	_plotter->drawstate->pos.x += 
	  costheta * _plotter->drawstate->font_size / 6.0;
	_plotter->drawstate->pos.y += 
	  sintheta * _plotter->drawstate->font_size / 6.0;
      }
  else
    t = (unsigned char *)s;

  /* sync font and pen position */
  _plotter->set_font();
  _plotter->set_position();

  /* draw label */
  terminator = _label_terminator (t);
  if (t == (unsigned char)0)
    _plotter->warning ("label contains too many distinct characters to be drawn");
  else
    {
      if (terminator != _plotter->label_terminator)
	{
	  /* output instruction, update */
	  sprintf (_plotter->outbuf.current, "DT%c;", (char)terminator);
	  _update_buffer (&_plotter->outbuf);
	  _plotter->label_terminator = terminator;
	}
      sprintf (_plotter->outbuf.current, "LB%s%c;", 
	       t, _plotter->label_terminator);
      _update_buffer (&_plotter->outbuf);
      /* where is the plotter pen now located?? we don't know, exactly */
      _plotter->position_is_unknown = true;
    }

  if (stick_font)
    {
      free (t);
      /* undo above horizontal shift */
      _plotter->drawstate->pos.x -= 
	costheta * _plotter->drawstate->font_size / 6.0;
      _plotter->drawstate->pos.y -= 
	sintheta * _plotter->drawstate->font_size / 6.0;
    }

  /* reposition after drawing label */
  _plotter->drawstate->pos.x += costheta * width;
  _plotter->drawstate->pos.y += sintheta * width;

  return width;
}

static unsigned char
#ifdef _HAVE_PROTOS
_label_terminator (const unsigned char *s)
#else
_label_terminator (s)
     const unsigned char *s;	/* label we'll be rendering */
#endif
{
  unsigned char i;

  for (i = MIN_TERMINATOR; i <= MAX_TERMINATOR; i++)
    {
      const unsigned char *sptr;
      bool suitable;

      sptr = s;
      suitable = true;
      while (*sptr)
	if (*sptr++ == i)
	  {
	    suitable = false;
	    break;		/* unsuitable */
	  }
      if (suitable)
	return i;
    }
  
  /* label contains every possible character, won't be able to render it */
  return (unsigned char)0;
}
