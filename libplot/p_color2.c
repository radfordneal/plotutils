#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* idraw's 12 standard colors, taken from idraw source.  These are the only
   pen colors that idraw understands, so we must quantize.  Quantization
   of fill colors is more complicated; see below.

   The list of 12 standard colors include 6 of the 8 vertices of the RGB
   cube; Magenta and Cyan are missing.  It also includes Orange and Indigo
   (on the boundary of the cube; they more or less substitute for Magenta
   and Cyan).  Also included are four points in the interior of the cube:
   Light Gray, Dark Gray, Violet, and Brown. */

const Color _idraw_stdcolors[IDRAW_NUM_STD_COLORS] =
{
  {0x0000, 0x0000, 0x0000},	/* Black */
  {0xa500, 0x2a00, 0x2a00},	/* Brown */
  {0xffff, 0x0000, 0x0000},	/* Red */  
  {0xffff, 0xa5a5, 0x0000},	/* Orange */
  {0xffff, 0xffff, 0x0000},	/* Yellow */
  {0x0000, 0xffff, 0x0000},	/* Green */
  {0x0000, 0x0000, 0xffff},	/* Blue */
  {0xbf00, 0x0000, 0xff00},	/* Indigo */
  {0x4f00, 0x2f00, 0x4f00},	/* Violet */
  {0xffff, 0xffff, 0xffff},	/* White */
  {0xc350, 0xc350, 0xc350},	/* LtGray */	  
  {0x80e8, 0x80e8, 0x80e8},	/* DkGray */	  
};

const char *_idraw_stdcolornames[IDRAW_NUM_STD_COLORS] = 
{
  "Black", "Brown", "Red", "Orange", "Yellow", "Green",
  "Blue", "Indigo", "Violet", "White", "LtGray", "DkGray",
};

/* Idraw allows a fill color to be an interpolation of a pen color
   (``foreground color'') and a background color; both must be in the above
   list.  The following is a list of the interpolations (``shadings'') that
   idraw recognizes.  0.0 means use foreground color, 1.0 means use
   background color.  (Idraw shadings are actually a special case of idraw
   patterns, which include bitmap fillings as well.) */

const double _idraw_stdshadings[IDRAW_NUM_STD_SHADINGS] =
{
  0.0, 0.25, 0.5, 0.75, 1.0,
};
