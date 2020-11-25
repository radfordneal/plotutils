/* This file contains the joinmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when drawing
   subsequent polylines. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_joinmod (const char *s)
#else
_m_joinmod (s)
     const char *s;
#endif
{
  char *nl;
  
  if (!_plotter->open)
    {
      _plotter->error ("joinmod: invalid operation");
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
	       (int)O_JOINMOD, s);
    }
  
  /* invoke generic method */
  return _g_joinmod (s);
}
