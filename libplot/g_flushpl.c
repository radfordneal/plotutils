/* This file contains the flushpl method, which is a GNU extension to
   libplot.  It flushes (i.e. pushes onward) all plot commands sent to the
   graphics device. */

/* This file also contains the internal _flush_plotter_outstreams() method,
   which should be invoked by any Plotter that forks an auxiliary process.
   (E.g., an XPlotter forks off an auxiliary process when closepl() is
   invoked, to manage its `spun-off' window.) */

#include "sys-defines.h"
#include "extern.h"

#ifdef MSDOS
#include <unistd.h>		/* for fsync() */
#endif

/* Mutex for locking _plotters[] and _plotters_len.  Defined in
   g_defplot.c. */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
extern pthread_mutex_t _plotters_mutex;
#endif
#endif

int
#ifdef _HAVE_PROTOS
_g_flushpl (S___(Plotter *_plotter))
#else
_g_flushpl (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "flushpl: invalid operation");
      return -1;
    }

  if (_plotter->outfp)
    {
      if (fflush(_plotter->outfp) < 0
#ifdef MSDOS
	  /* data can be caught in DOS buffers, so do an fsync() too */
	  || fsync (_plotter->outfp) < 0
#endif
	  )
	{
	  _plotter->error (R___(_plotter) 
			   "output stream jammed");
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
	      _plotter->error (R___(_plotter) "output stream jammed");
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
   closepl() operation is invoked should call this. 

   This code is messy, because there are four cases to cover, and in the
   final three, the global variables _plotters[] and _plotters_len need to
   be locked and unlocked. */
void
#ifdef _HAVE_PROTOS
_flush_plotter_outstreams (S___(Plotter *_plotter))
#else
_flush_plotter_outstreams (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
#ifndef LIBPLOTTER 

#ifdef HAVE_NULL_FLUSH
  fflush ((FILE *)NULL);
#else  /* not HAVE_NULL_FLUSH */
  int i;
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->outfp)
	  fflush (_plotters[i]->outfp);
	if (_plotters[i]->errfp)
	  fflush (_plotters[i]->errfp);
      }
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif
#endif /* not HAVE_NULL_FLUSH */

#else  /* LIBPLOTTER */
  int i;
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif
#ifdef HAVE_NULL_FLUSH
  fflush ((FILE *)NULL);
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->outstream)
	  _plotters[i]->outstream->flush ();
	if (_plotters[i]->errstream)
	  _plotters[i]->errstream->flush ();
      }
#else  /* not HAVE_NULL_FLUSH */
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->outfp)
	  fflush (_plotters[i]->outfp);
	if (_plotters[i]->errfp)
	  fflush (_plotters[i]->errfp);
	if (_plotters[i]->outstream)
	  _plotters[i]->outstream->flush ();
	if (_plotters[i]->errstream)
	  _plotters[i]->errstream->flush ();
      }
#endif /* not HAVE_NULL_FLUSH */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif

#endif /* LIBPLOTTER */
}
