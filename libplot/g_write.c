/* These are the lowest-level output routines in libplot/libplotter.
   Plotters that write to output streams use these. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_write_byte (const plPlotterData *data, unsigned char c)
#else
_write_byte (data, c)
     const plPlotterData *data;
     unsigned char c;
#endif
{
  if (data->outfp)
    putc ((int)c, data->outfp);
#ifdef LIBPLOTTER
  else if (data->outstream)
    data->outstream->put (c);
#endif
}

void
#ifdef _HAVE_PROTOS
_write_bytes (const plPlotterData *data, int n, const unsigned char *c)
#else
_write_bytes (data, n, c)
     const plPlotterData *data;
     int n;
     const unsigned char *c;
#endif
{
  int i;

  if (data->outfp)
    {
      for (i = 0; i < n; i++)
	putc ((int)(c[i]), data->outfp);
    }
#ifdef LIBPLOTTER
  else if (data->outstream)
    data->outstream->write(c, n);
#endif
}

void
#ifdef _HAVE_PROTOS
_write_string (const plPlotterData *data, const char *s)
#else
_write_string (data, s)
     const plPlotterData *data;
     const char *s;
#endif
{
  if (data->outfp)
    fputs (s, data->outfp);
#ifdef LIBPLOTTER
  else if (data->outstream)
    (*(data->outstream)) << s;
#endif
}
