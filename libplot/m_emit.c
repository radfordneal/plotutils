/* This file contains the low-level _emit_integer routine, which takes an
   integer argument and writes it, either in binary plot format or in ascii
   plot format, to a specified output stream.  It is called by all
   MetaPlotter methods.

   In releases prior to 2.0, we assumed that in binary plot format a short
   integer sufficed to represent any integer.  This was the convention used
   in traditional plot(5) format, and can be traced to the PDP-11.
   Unfortunately it pretty well confined us to the range -0x10000..0x7fff
   on most modern two's complement machines.  Actually (see the parsing in
   plot.c) we always treated the arguments to pencolor(), fillcolor(), and
   filltype() specially.  An argument of any of those functions was treated as
   an unsigned integer, and could be handled so long as it was within the
   range 0..0xffff.

   In release 2.0, we switched to representing integers as integers, in
   binary plot format.  The parsing of metafiles in plot.c now takes this
   into account; there are options for backward compatibility.

   This file also contains the _emit_float routine, which is called by the
   methods that take floating point arguments.  Our binary representation
   for floating-point numbers is simply the system representation for
   single-precision floating point.  plot(5) format did not support
   floating point arguments, so there is no need to maintain backward
   compatibility here. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_emit_integer (int x)
#else
_emit_integer (x)
     int x;
#endif
{
  fwrite (&x, sizeof(x), 1, _plotter->outstream);
}

void
#ifdef _HAVE_PROTOS
_emit_float (double x)
#else
_emit_float (x)
     double x;
#endif
{
  float f;
  
  f = FROUND(x);
  fwrite (&f, sizeof(f), 1, _plotter->outstream);
}
