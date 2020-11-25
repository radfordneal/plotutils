/* This file contains the capmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the cap mode used when drawing
   subsequent lines and polylines. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_capmod (const char *s)
#else
_m_capmod (s)
     const char *s;
#endif
{
  char *nl;

  if (!_plotter->open)
    {
      _plotter->error ("capmod: invalid operation");
      return -1;
    }

  if (_plotter->outstream)
    {
      /* null pointer handled specially */
      if (s == NULL)
	s = "(null)";
      
      if ((nl = strchr (s, '\n')))
	*nl = '\0';		/* don't grok multiline arg strings */
      
      fprintf (_plotter->outstream, "%c%s\n", 
	       (int)O_CAPMOD, s);
    }
  
  /* invoke generic method */
  return _g_capmod (s);
}
