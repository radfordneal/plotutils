#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#ifndef HAVE_STRCASECMP
int
#ifdef _HAVE_PROTOS
strcasecmp(const char *s1, const char *s2)
#else
strcasecmp(s1, s2)
     const char *s1, *s2;
#endif
{
  bool retval_set = false;
  int retval = 0;
  char *t1, *t2, *t1_base, *t2_base;
  
  t1 = t1_base = (char *)_plot_xmalloc (strlen (s1) + 1);
  t2 = t2_base = (char *)_plot_xmalloc (strlen (s2) + 1);
  strcpy (t1, s1);
  strcpy (t2, s2);  

  while (*t1 && *t2)
    {
      unsigned int c1 = tolower ((int)(unsigned char)*t1);
      unsigned int c2 = tolower ((int)(unsigned char)*t2);
      
      if (c1 > c2)
	{
	  retval = 1;
	  retval_set = true;
	  break;
	}
      else if (c1 < c2)
	{
	  retval = -1;
	  retval_set = true;
	  break;
	}
      else
	{
	  t1++; 
	  t2++;
	}
    }      
  
  if (!retval_set)
    {
      if (*t1)
	retval = 1;
      else if (*t2)
	retval = -1;
      else
	retval = 0;
    }
  
  free (t1_base);
  free (t2_base);
  
  return retval;
}
#endif /* not HAVE_STRCASECMP */
