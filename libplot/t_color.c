/* For TekPlotter objects, setting the pen color has no effect unless the
   plotter is using the Tektronix emulation of MS-DOS kermit.  If so, we
   compute a quantized color and output the appropriate ANSI escape 
   sequence. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define ONEBYTE (0xff)

/* forward references */
static int _kermit_pseudocolor __P((int red, int green, int blue));

void
#ifdef _HAVE_PROTOS
_t_set_pen_color(void)
#else
_t_set_pen_color()
#endif
{
  if (_plotter->display_type == D_KERMIT)
    {
      int new_kermit_fgcolor;

      new_kermit_fgcolor = 
	_kermit_pseudocolor (_plotter->drawstate->fgcolor.red, 
			     _plotter->drawstate->fgcolor.green, 
			     _plotter->drawstate->fgcolor.blue);
      /* we can't be sure the background is black or white, so don't draw
	 with a black or a white pen: map them to gray30 and gray55 */
      if (new_kermit_fgcolor == ANSI_SYS_BLACK)
	new_kermit_fgcolor = ANSI_SYS_GRAY30;
      if (new_kermit_fgcolor == ANSI_SYS_WHITE)
	new_kermit_fgcolor = ANSI_SYS_GRAY55;
      if (new_kermit_fgcolor != _plotter->kermit_fgcolor)
	{
	  if (_plotter->outstream)
	    fprintf (_plotter->outstream,
		     _kermit_fgcolor_escapes[new_kermit_fgcolor]);
	  _plotter->kermit_fgcolor = new_kermit_fgcolor;
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

