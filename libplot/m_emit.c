/* This file contains the low-level _meta_emit_integer, _meta_emit_float,
   _meta_emit_byte, and _meta_emit_string routines, which are used by
   MetaPlotters.  They take into account the desired format (binary plot
   format or ascii [human-readable] plot format.

   In releases prior to plotutils-2.0, we assumed that in binary plot
   format, a short machine integer sufficed to represent any integer.  This
   was the convention used in traditional plot(5) format, and can be traced
   to the PDP-11.  Unfortunately it confined us to the range
   -0x10000..0x7fff on modern two's complement machines.  Actually, the
   parsing in `plot' always treated the arguments to pencolor(),
   fillcolor(), and filltype() specially.  An argument of any of those
   functions was treated as an unsigned integer, so it could be in the
   range 0..0xffff.

   In plotutils-2.0, we switched in binary plot format to representing
   integers as machine integers.  The parsing of metafiles by `plot' now
   takes this into account.  `plot' has command-line options for backward
   compatibility.

   Our binary representation for floating-point numbers is simply the
   machine representation for single-precision floating point.  plot(5)
   format did not support floating point arguments, so there are no
   concerns over backward compatibility. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_meta_emit_integer (R___(Plotter *_plotter) int x)
#else
_meta_emit_integer (R___(_plotter) x)
     S___(Plotter *_plotter;) 
     int x;
#endif
{
  if (_plotter->outfp)
    {
      if (_plotter->meta_portable_output)
	fprintf (_plotter->outfp, " %d", x);
      else
	fwrite ((voidptr_t) &x, sizeof(int), 1, _plotter->outfp);
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
_meta_emit_float (R___(Plotter *_plotter) double x)
#else
_meta_emit_float (R___(_plotter) x)
     S___(Plotter *_plotter;) 
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
	  fwrite ((voidptr_t) &f, sizeof(float), 1, _plotter->outfp);
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
_meta_emit_byte (R___(Plotter *_plotter) int c)
#else
_meta_emit_byte (R___(_plotter) c)
     S___(Plotter *_plotter;) 
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
_meta_emit_string (R___(Plotter *_plotter) const char *s)
#else
_meta_emit_string (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  bool has_newline;
  char *t = NULL;		/* keep compiler happy */
  char *nl;
  const char *u;
  
  /* null pointer handled specially */
  if (s == NULL)
    s = "(null)";
  
  if (strchr (s, '\n'))
    /* don't grok multiline arg strings */
    {
      has_newline = true;
      t = (char *)_plot_xmalloc (strlen (s) + 1);      
      strcpy (t, s);
      nl = strchr (t, '\n');
      *nl = '\0';
      u = t;
    }
  else
    {
      has_newline = false;
      u = s;
    }
      
  if (_plotter->outfp)
    {
      fputs (u, _plotter->outfp);
      putc ('\n', _plotter->outfp); /* append newline (plot(3) convention) */
    }
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    (*(_plotter->outstream)) << u << '\n';
#endif

  if (has_newline)
    free (t);
}

/* this is invoked at the end of each directive, except the ones for which
   the final argument is a string, which will be emitted with its own final
   newline (an old plot(3) convention; see above) */
void
#ifdef _HAVE_PROTOS
_meta_emit_terminator (S___(Plotter *_plotter))
#else
_meta_emit_terminator (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  if (_plotter->meta_portable_output)
    {
      if (_plotter->outfp)
	putc ('\n', _plotter->outfp);
#ifdef LIBPLOTTER
      else if (_plotter->outstream)
	(*(_plotter->outstream)) << '\n';
#endif
    }
}
