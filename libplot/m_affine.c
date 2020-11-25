/* This file contains the translate, rotate and scale methods, which are
   GNU extensions to libplot.  They affect the affine transformation from
   user coordinates to device coordinates, as in Postscript. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_ftranslate (double x, double y)
#else
_m_ftranslate (x, y)
     double x,y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("ftranslate: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n",
		 FTRANSLATE, x, y);
      else
	{
	  putc (FTRANSLATE, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}  
    }

  /* invoke generic method */
  return _g_ftranslate (x, y);
}

int
#ifdef _HAVE_PROTOS
_m_frotate (double theta)
#else
_m_frotate (theta)
     double theta;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("frotate: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g\n",
		 FROTATE, theta);
      else
	{
	  putc (FROTATE, _plotter->outstream);
	  _emit_float (theta);
	}  
    }
      
  /* invoke generic method */
  return _g_frotate (theta);
}

int
#ifdef _HAVE_PROTOS
_m_fscale (double x, double y)
#else
_m_fscale (x, y)
     double x, y;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error ("fscale: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      if (_plotter->portable_output)
	fprintf (_plotter->outstream, "%c %g %g\n",
		 FSCALE, x, y);
      else
	{
	  putc (FSCALE, _plotter->outstream);
	  _emit_float (x);
	  _emit_float (y);
	}  
    }
      
  /* invoke generic method */
  return _g_fscale (x, y);
}
