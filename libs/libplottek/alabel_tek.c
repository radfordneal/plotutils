/* This file contains the device-specific versions of alabel(), called
   _alabel_standard() and _alabel_device().  This version is for libplottek.

   The width of the string in user units is returned. */

/* For libplottek we do nothing, since we don't attempt to rasterize and
   plot the 35 standard Postscript fonts, and we don't attempt to use the
   Tektronix [emulator] fonts; these routines should never even be
   called. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

double
_alabel_standard (x_justify, y_justify, s)
     int x_justify, y_justify;
     const unsigned short *s;
{
  return 0.0;
}

double
_alabel_device (x_justify, y_justify, s)
     int x_justify, y_justify;
     const unsigned short *s;
{
  return 0.0;
}

double
_labelwidth_standard (s)
     const unsigned short *s;
{
  return 0.0;
}

double
_labelwidth_device (s)
     const unsigned short *s;
{
  return 0.0;
}
