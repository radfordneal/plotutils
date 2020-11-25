/* This file contains the flushpl method, which is a GNU extension to
   libplot.  It flushes (i.e. pushes onward) all plot commands sent to the
   graphics device. */

/* This file also contains the internal _flush_plotter_outstreams() method,
   which should be invoked by any Plotter that forks an auxiliary process.
   (E.g., an XPlotter forks off an auxiliary process when closepl() is
   invoked, to manage its `spun-off' window.) */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_g_flushpl (void)
#else
_g_flushpl ()
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("flushpl: invalid operation");
      return -1;
    }

  if (_plotter->outfp)
    {
      if (fflush(_plotter->outfp) < 0)
	{
	  _plotter->error ("output stream jammed");
	  return -1;
	}
      else
	return 0;
    }
#ifdef LIBPLOTTER
  if (_plotter->outstream)
	{
	  _plotter->outstream->flush ();
	  if (!(*(_plotter->outstream)))
	    {
	      _plotter->error ("output stream jammed");
	      return -1;
	    }
	  else
	    return 0;
	}
#endif

  /* no output stream of either sort */
  return 0;
}

/* Flush output streams for all Plotters.  Plotters that fork when the
   closepl() operation is invoked should call this. */

void
#ifdef _HAVE_PROTOS
_flush_plotter_outstreams (void)
#else
_flush_plotter_outstreams ()
#endif
{
#ifndef HAVE_NULL_FLUSH
  int i;
  
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->outfp)
	  fflush (_plotters[i]->outfp);
	if (_plotters[i]->errfp)
	  fflush (_plotters[i]->errfp);
#ifdef LIBPLOTTER
	if (_plotters[i]->outstream)
	  _plotters[i]->outstream->flush ();
	if (_plotters[i]->errstream)
	  _plotters[i]->errstream->flush ();
#endif
      }
  /* maybe should fflush stdout too? */

#else  /* HAVE_NULL_FLUSH */
  /* can do more: fflush _all_ output (FILE *)'s before forking */
  fflush ((FILE *)NULL);
#ifdef LIBPLOTTER
  for (int i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->outstream)
	  _plotters[i]->outstream->flush ();
	if (_plotters[i]->errstream)
	  _plotters[i]->errstream->flush ();
      }
#endif
#endif /* HAVE_NULL_FLUSH */
}
