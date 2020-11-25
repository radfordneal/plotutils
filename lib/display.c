/* This file contains the display_usage routine, which is used in
   user-level programs in the plotutils package.  It prints program options
   and long options in a reasonably nice format.
   
   This file also contains the display_version routine. */

#include "sys-defines.h"
#include "getopt.h"

/* global array of long options, in program we're to be linked with */
extern struct option long_options[];

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

/* forward references */
bool elementp __P((int item, const int *list));
void display_usage __P((const char *progname, const int *omit_vals, bool files, bool fonts));
void display_version __P((const char *progname));

bool
#ifdef _HAVE_PROTOS
elementp (int item, const int *list)
#else
elementp (item, list)
     int item;
     const int *list;		/* null-terminated list of integers */
#endif
{
  int list_item;

  while ((list_item = *list++) != 0)
    {
      if (item == list_item)
	return true;
    }
  return false;
}

void
#ifdef _HAVE_PROTOS
display_usage (const char *progname, const int *omit_vals, bool files, bool fonts)
#else
display_usage (progname, omit_vals, files, fonts)
     const char *progname;
     const int *omit_vals;
     bool files, fonts;
#endif
{
  int i;
  int col = 0;
  
  fprintf (stderr, "Usage: %s", progname);
  col += (strlen (progname) + 7);
  for (i = 0; long_options[i].name; i++)
    {
      int option_len;
      
      if (elementp (long_options[i].val, omit_vals))
	continue;

      option_len = strlen (long_options[i].name);
      if (col >= 80 - (option_len + 16))
	{
	  fprintf (stderr, "\n\t");
	  col = 8;
	}
      fprintf (stderr, " [--%s", long_options[i].name);
      col += (option_len + 4);
      if ((unsigned int)(long_options[i].val) < 256)
	{
	  fprintf (stderr, " | -%c", long_options[i].val);
	  col += 5;
	}
      if (long_options[i].has_arg == ARG_REQUIRED)
	{
	  fprintf (stderr, " arg]");
	  col += 5;
	}
      else if (long_options[i].has_arg == ARG_OPTIONAL)
	{
	  fprintf (stderr, " [arg(s)]]");
	  col += 10;
	}
      else
	{
	  fprintf (stderr, "]");
	  col++;
	}
    }

  if (files)
    {
      fprintf (stderr, " [FILE]...\n");
      fprintf (stderr, 
	       "\nWith no FILE, or when FILE is -, read standard input.\n");
    }
  else
    fprintf (stderr, "\n");

  if (fonts)
    fprintf (stderr, "\n\
To list available fonts, type `%s -T \"device\" --help-fonts',\n\
where \"device\" is the display device: X, ps, fig, hpgl, or tek.\n",
	     progname);
}

void
#ifdef _HAVE_PROTOS
display_version (const char *progname)
#else
display_version (progname)
     const char *progname;
#endif
{
  fprintf (stderr, 
	   "%s (GNU %s) %s\n", progname, PACKAGE, VERSION);
  fprintf (stderr, 
	   "Copyright (C) 1989-1998 Free Software Foundation, Inc.\n");
  fprintf (stderr, 
	   "The GNU %s package comes with NO WARRANTY, to the extent permitted\n", PACKAGE);
  fprintf (stderr, 
	   "by law. You may redistribute copies of the GNU %s package\n", PACKAGE);
  fprintf (stderr, 
	   "under the terms of the GNU General Public License.\n");
}
