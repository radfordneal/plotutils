/* This file contains the low-level _meta_emit_integer, _meta_emit_float,
   _meta_emit_byte, and _meta_emit_string routines, which are used by
   MetaPlotters.  They take into account the desired format (binary plot
   format or ascii [human-readable] plot format.

   In releases prior to plotutils-2.0, we assumed that in binary plot
   format a short integer sufficed to represent any integer.  This was the
   convention used in traditional plot(5) format, and can be traced to the
   PDP-11.  Unfortunately it confined us to the range -0x10000..0x7fff on
   modern two's complement machines.  Actually (see the parsing in plot.c)
   we always treated the arguments to pencolor(), fillcolor(), and
   filltype() specially.  An argument of any of those functions was treated
   as an unsigned integer, so it could be in the range 0..0xffff.

   In plotutils-2.0, we switched to representing integers as integers, in
   binary plot format.  The parsing of metafiles in plot.c now takes this
   into account.  There are options for backward compatibility.

   Our binary representation for floating-point numbers is simply the
   system representation for single-precision floating point.  plot(5)
   format did not support floating point arguments, so there is no need to
   maintain backward compatibility here. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_meta_emit_integer (int x)
#else
_meta_emit_integer (x)
     int x;
#endif
{
  if (_plotter->outfp)
    {
      if (_plotter->meta_portable_output)
	fprintf (_plotter->outfp, " %d", x);
      else
	fwrite ((Voidptr) &x, sizeof(int), 1, _plotter->outfp);
    }
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    {
      if (_plotter->meta_portable_output)
	(*(_plotter->outstream)) << ' ' << x;
      else
	_plotter->outstream->write((char *)&x, sizeof(int));
    }
#endif
}

void
#ifdef _HAVE_PROTOS
_meta_emit_float (double x)
#else
_meta_emit_float (x)
     double x;
#endif
{
  if (_plotter->outfp)
    {
      if (_plotter->meta_portable_output)
	fprintf (_plotter->outfp, " %g", x);
      else
	{
	  float f;
	  
	  f = FROUND(x);
	  fwrite ((Voidptr) &f, sizeof(float), 1, _plotter->outfp);
	}
    }
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    {
      if (_plotter->meta_portable_output)
	(*(_plotter->outstream)) << ' ' << x;
      else
	{
	  float f;
	  
	  f = FROUND(x);
	  _plotter->outstream->write((char *)&f, sizeof(float));
	}
    }
#endif
}

/* emit one unsigned character, passed as an int */
void
#ifdef _HAVE_PROTOS
_meta_emit_byte (int c)
#else
_meta_emit_byte (c)
     int c;
#endif
{
  if (_plotter->outfp)
    putc (c, _plotter->outfp);
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    _plotter->outstream->put ((unsigned char)c);
#endif
}

void
#ifdef _HAVE_PROTOS
_meta_emit_string (const char *s)
#else
_meta_emit_string (s)
     const char *s;
#endif
{
  char *nl;
  
  /* null pointer handled specially */
  if (s == NULL)
    s = "(null)";
  
  if ((nl = strchr (s, '\n')))
    *nl = '\0';		/* don't grok multiline arg strings */
      
  if (_plotter->outfp)
    {
      fputs (s, _plotter->outfp);
      putc ('\n', _plotter->outfp); /* append newline (plot(3) convention) */
    }
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    (*(_plotter->outstream)) << s << '\n';
#endif
}

/* this is invoked at the end of each directive, except the ones for which
   the final argument is a string, which will be emitted with its own final
   newline (an old plot(3) convention; see above) */
void
#ifdef _HAVE_PROTOS
_meta_emit_terminator (void)
#else
_meta_emit_terminator ()
#endif
{
  if (_plotter->outfp)
    {
      if (_plotter->meta_portable_output)
	putc ('\n', _plotter->outfp);
    }
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    (*(_plotter->outstream)) << '\n';
#endif
}
