/* This file contains the outfile method, which is a GNU extension to
   libplot.  It selects an output stream for all subsequent plot commands.
   The outfile method may only be invoked outside an openpl()...closepl()
   pair. */

/* THIS METHOD IS NOW DEPRECATED.  IT WILL SOON GO AWAY. */

#include "sys-defines.h"
#include "extern.h"

/* outfile takes a single argument, a stream that has been opened for
   writing.  It may be called only outside an openpl()....closepl() pair.
   It switches all future output to the new, specified stream.  The old
   output stream, which is not closed, is returned. */

FILE *
#ifdef _HAVE_PROTOS
_API_outfile(R___(Plotter *_plotter) FILE *outfile)
#else
_API_outfile(R___(_plotter) outfile)
     S___(Plotter *_plotter;) 
     FILE *outfile;
#endif
{
  FILE *oldoutfile;
  
  if (_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "outfile: invalid operation");
      return (FILE *)NULL;
    }

  oldoutfile = _plotter->data->outfp;
  _plotter->data->outfp = outfile;
#ifdef LIBPLOTTER
  _plotter->data->outstream = NULL;
#endif

  _plotter->data->page_number = 0;	/* reset */

  return oldoutfile;
}
