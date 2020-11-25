/* This file contains the linemod method, which is a standard part of
   libplot.  It sets a drawing attribute: the line mode used in subsequent
   drawing operations. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linemod (const char *s)
#else
_m_linemod (s)
     const char *s;
#endif
{
  char *nl;
  
  if (!_plotter->open)
    {
      _plotter->error ("linemod: invalid operation");
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
	       (int)O_LINEMOD, s);
    }
  
  /* invoke generic method */
  return _g_linemod (s);
}
