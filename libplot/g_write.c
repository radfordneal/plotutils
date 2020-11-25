/* These are the lowest-level output routines in libplot/libplotter.  Most
   Plotters that write to output streams use these.  MetaPlotters use the
   special routines in m_emit.c. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_g_write_byte (R___(Plotter *_plotter) unsigned char c)
#else
_g_write_byte (R___(_plotter) c)
     S___(Plotter *_plotter;)
     unsigned char c;
#endif
{
  if (_plotter->outfp)
    putc ((int)c, _plotter->outfp);
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    _plotter->outstream->put (c);
#endif
}

void
#ifdef _HAVE_PROTOS
_g_write_bytes (R___(Plotter *_plotter) int n, const unsigned char *c)
#else
_g_write_bytes (R___(_plotter) n, c)
     S___(Plotter *_plotter;) 
     int n;
     const unsigned char *c;
#endif
{
  int i;

  if (_plotter->outfp)
    {
      for (i = 0; i < n; i++)
	putc ((int)(c[i]), _plotter->outfp);
    }
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    _plotter->outstream->write(c, n);
#endif
}

void
#ifdef _HAVE_PROTOS
_g_write_string (R___(Plotter *_plotter) const char *s)
#else
_g_write_string (R___(_plotter) s)
     S___(Plotter *_plotter;) 
     const char *s;
#endif
{
  if (_plotter->outfp)
    fputs (s, _plotter->outfp);
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    (*(_plotter->outstream)) << s;
#endif
}
