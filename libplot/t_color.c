/* For TekPlotter objects, setting the pen color or background color has no
   effect unless the plotter is using the Tektronix emulation of MS-DOS
   kermit.  If so, we compute a quantized color and output the appropriate
   ANSI escape sequence, if the color is different from the ANSI.SYS color
   the emulation is currently using.

   When the TekPlotter is created, the current ANSI.SYS pen and bg colors
   are set to -1 (see t_defplot.c).  That's a nonsensical value, equivalent
   to `unknown'. */

#include "sys-defines.h"
#include "extern.h"

#define ONEBYTE (0xff)

/* forward references */
static int _kermit_pseudocolor ____P((int red, int green, int blue));

void
#ifdef _HAVE_PROTOS
_t_set_pen_color(S___(Plotter *_plotter))
#else
_t_set_pen_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (_plotter->tek_display_type == D_KERMIT)
    {
      int new_kermit_fgcolor;

      new_kermit_fgcolor = 
	_kermit_pseudocolor (_plotter->drawstate->fgcolor.red, 
			     _plotter->drawstate->fgcolor.green, 
			     _plotter->drawstate->fgcolor.blue);
      if (new_kermit_fgcolor != _plotter->tek_kermit_fgcolor)
	{
	  _plotter->write_string (R___(_plotter) 
				  _kermit_fgcolor_escapes[new_kermit_fgcolor]);
	  _plotter->tek_kermit_fgcolor = new_kermit_fgcolor;
	}
    }
}  

void
#ifdef _HAVE_PROTOS
_t_set_bg_color(S___(Plotter *_plotter))
#else
_t_set_bg_color(S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (_plotter->tek_display_type == D_KERMIT)
    {
      int new_kermit_bgcolor;

      new_kermit_bgcolor = 
	_kermit_pseudocolor (_plotter->drawstate->bgcolor.red, 
			     _plotter->drawstate->bgcolor.green, 
			     _plotter->drawstate->bgcolor.blue);
      if (new_kermit_bgcolor != _plotter->tek_kermit_bgcolor)
	{
	  _plotter->write_string (R___(_plotter)
				  _kermit_bgcolor_escapes[new_kermit_bgcolor]);
	  _plotter->tek_kermit_bgcolor = new_kermit_bgcolor;
	}
    }
}  

/* _kermit_pseudocolor quantizes to one of kermit's native 16 colors.
   (They provide a [rather strange] partition of the color cube; see
   t_color2.c.) */

/* find closest known point within the RGB color cube, using Euclidean
   distance as our metric */
static int
#ifdef _HAVE_PROTOS
_kermit_pseudocolor (int red, int green, int blue)
#else
_kermit_pseudocolor (red, green, blue)
     int red, green, blue;
#endif
{
  unsigned long int difference = INT_MAX;
  int i;
  int best = 0;
  
  /* reduce to 24 bits */
  red = (red >> 8) & ONEBYTE;
  green = (green >> 8) & ONEBYTE;
  blue = (blue >> 8) & ONEBYTE;

  for (i = 0; i < KERMIT_NUM_STD_COLORS; i++)
    {
      unsigned long int newdifference;
      
      if (_kermit_stdcolors[i].red == 0xff
	  && _kermit_stdcolors[i].green == 0xff
	  && _kermit_stdcolors[i].blue == 0xff)
	/* white is a possible quantization only for white itself (our
           convention) */
	{
	  if (red == 0xff && green == 0xff && blue == 0xff)
	    {
	      difference = 0;
	      best = i;
	    }
	  continue;
	}

      newdifference = (((_kermit_stdcolors[i].red - red) 
			* (_kermit_stdcolors[i].red - red))
		       + ((_kermit_stdcolors[i].green - green) 
			  * (_kermit_stdcolors[i].green - green))
		       + ((_kermit_stdcolors[i].blue - blue) 
			  * (_kermit_stdcolors[i].blue - blue)));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }
  return best;
}

