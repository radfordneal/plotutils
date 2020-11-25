#include "sys-defines.h"
#include "extern.h"

#define ONEBYTE 0xff
#define REGIS_NUM_STD_COLORS 8

/* standard ReGIS colors */
const plColor _regis_stdcolors[REGIS_NUM_STD_COLORS] = 
{
  {0xff, 0x00, 0x00},		/* Red */
  {0x00, 0xff, 0x00},		/* Green */
  {0x00, 0x00, 0xff},		/* Blue */
  {0x00, 0xff, 0xff},		/* Cyan */
  {0xff, 0x00, 0xff},		/* Magenta */
  {0xff, 0xff, 0x00},		/* Yellow */
  {0x00, 0x00, 0x00},		/* Black */
  {0xff, 0xff, 0xff}		/* White */
};

/* corresponding one-letter abbreviations (in same order as preceding) */
const char _regis_color_chars[REGIS_NUM_STD_COLORS] =
{ 'r', 'g', 'b', 'c', 'm', 'y', 'd', 'w' };

/* forward references */
static int _rgb_to_stdcolor ____P((plColor rgb));

void
#ifdef _HAVE_PROTOS
_r_set_pen_color(S___(Plotter *_plotter))
#else
_r_set_pen_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int new_color;

  new_color = _rgb_to_stdcolor (_plotter->drawstate->fgcolor);
  if (_plotter->regis_fgcolor_is_unknown
      || _plotter->regis_fgcolor != new_color)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "W(I(%c))\n", 
	       _regis_color_chars[new_color]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_fgcolor = new_color;
      _plotter->regis_fgcolor_is_unknown = false;
    }
}

void
#ifdef _HAVE_PROTOS
_r_set_fill_color(S___(Plotter *_plotter))
#else
_r_set_fill_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int new_color;

  /* sanity check */
  if (_plotter->drawstate->fill_type == 0)
    return;

  new_color = _rgb_to_stdcolor (_plotter->drawstate->fillcolor);
  if (_plotter->regis_fgcolor_is_unknown
      || _plotter->regis_fgcolor != new_color)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "W(I(%c))\n", 
	       _regis_color_chars[new_color]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_fgcolor = new_color;
      _plotter->regis_fgcolor_is_unknown = false;
    }
}

void
#ifdef _HAVE_PROTOS
_r_set_bg_color(S___(Plotter *_plotter))
#else
_r_set_bg_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  int new_color;

  new_color = _rgb_to_stdcolor (_plotter->drawstate->bgcolor);
  if (_plotter->regis_bgcolor_is_unknown
      || _plotter->regis_bgcolor != new_color)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "S(I(%c))\n", 
	       _regis_color_chars[new_color]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_bgcolor = new_color;
      _plotter->regis_bgcolor_is_unknown = false;

      /* note: must do an erase, for the just-set background color to show
	 up on the ReGIS display */
    }
}

/* compute best approximation, in color table, to a specified 48-bit color */
static int
#ifdef _HAVE_PROTOS
_rgb_to_stdcolor (plColor rgb)
#else
_rgb_to_stdcolor (rgb)
     plColor rgb;
#endif
{
  int red, green, blue;
  unsigned long int difference = INT_MAX;
  int i, best = 0;		/* keep compiler happy */
  
  /* convert from 48-bit color to 24-bit */
  red = rgb.red;
  green = rgb.green;
  blue = rgb.blue;
  red = (red >> 8) & ONEBYTE;
  green = (green >> 8) & ONEBYTE;
  blue = (blue >> 8) & ONEBYTE;

  for (i = 0; i < REGIS_NUM_STD_COLORS; i++)
    {
      unsigned long int newdifference;
      
      newdifference = (((_regis_stdcolors[i].red - red) 
			* (_regis_stdcolors[i].red - red))
		       + ((_regis_stdcolors[i].green - green) 
			  * (_regis_stdcolors[i].green - green))
		       + ((_regis_stdcolors[i].blue - blue) 
			  * (_regis_stdcolors[i].blue - blue)));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }
  return best;
}
