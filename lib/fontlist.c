/* This file contains the display_fonts routine, which is used in
   user-level executables that are linked with libplot.  Its output is
   device-specific. */

#include "sys-defines.h"
#include "plot.h"

/* forward references */
void display_fonts __P((const char *display_type, const char *progname));

/* for use in printing font names in two columns; assumption is that all
   font names have lengths in range 0..MAX_FONTNAME_LEN, inclusive */
#define MAX_FONTNAME_LEN 36
static char spaces[MAX_FONTNAME_LEN+1] = "                                   ";

struct vector_font_info_struct 
{
  char *name;			/* font name */
  char *othername;		/* alternative font name */
  short chars[256];		/* array of vector glyphs */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
  bool visible;		/* whether font is visible, i.e. not internal */
};
extern struct vector_font_info_struct _vector_font_info[];

struct ps_font_info_struct 
{
  char *ps_name;		/* the postscript font name */
  char *x_name;			/* the X Windows font name */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  short width[256];		/* the font width information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  int fig_id;			/* Fig's font id */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern struct ps_font_info_struct _ps_font_info[];

struct pcl_font_info_struct 
{
  char *ps_name;		/* the postscript font name */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=proportional */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1 */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int cap_height;		/* the font's cap height */
  double hp_width_factor;	/* HP's width adjustment factor (why?) */
  short width[256];		/* the font width information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
};

extern struct pcl_font_info_struct _pcl_font_info[];

void
#ifdef _HAVE_PROTOS
display_fonts (const char *display_type, const char *progname)
#else
display_fonts (display_type, progname)
     const char *display_type, *progname;
#endif
{
  int numfonts, numpairs, i, j, k, handle;
  bool odd;

  if (strcmp (display_type, "meta") == 0)
    {
      fprintf (stderr, "\
To list available fonts, type `%s -T \"device\" --help-fonts',\n\
where \"device\" is the display device: X, ps, fig, hpgl, or tek.\n",
	       progname);
      return;
    }

  if ((handle = newpl ((char *)display_type, NULL, stdout, stderr)) < 0)
    {
      fprintf (stderr, 
	       "%s: no font information on display device \"%s\" is available\n",
	       progname, display_type);
      return;
    }
  else
    selectpl (handle);

  if (havecap ("HERSHEY_FONTS"))
    {
      int visible_num;

      numfonts = 0;
      for (i=0; _vector_font_info[i].name; i++)
	if (_vector_font_info[i].visible)
	numfonts++;
      odd = (numfonts % 2 == 1);
      numpairs = numfonts / 2;

      /* compute j and k: j=0, k=numpairs + (odd ? 1 : 0) in terms of visibles */
      j = 0;
      k = 0;
      visible_num = -1;
      for (i=0; _vector_font_info[i].name; i++)
	if (_vector_font_info[i].visible)
	  {
	    visible_num++;	/* visible_num is index into array of visibles */
	    if (visible_num == 0)
	      j = i;
	    else if (visible_num == numpairs + (odd ? 1 : 0))
	      k = i;
	  }

      fprintf (stderr, 
	       "Names of supported Hershey fonts (case-insensitive):\n");
      for (i=0; i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (_vector_font_info[j].name);
	  fprintf (stderr, "\t%s", _vector_font_info[j].name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stderr);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stderr, "%s\n", _vector_font_info[k].name);
	  /* bump both j and k */
	  do
	    j++;
	  while (_vector_font_info[j].visible == false);
	  if (i < numpairs - 1)
	    {
	      do
		k++;
	      while (_vector_font_info[k].visible == false);
	    }
	}
      if (odd)
	fprintf (stderr, "\t%s\n", _vector_font_info[j].name);
    }

  if (havecap ("PS_FONTS"))
    {
      numfonts = 0;
      for (i=0; _ps_font_info[i].ps_name; i++)
	numfonts++;
      odd = (numfonts % 2 == 1);
      numpairs = numfonts / 2;

      fprintf (stderr, 
	       "Names of supported Postscript fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (_ps_font_info[j].ps_name);
	  fprintf (stderr, "\t%s", _ps_font_info[j++].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stderr);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stderr, "%s\n", _ps_font_info[k++].ps_name);
	}
      if (odd)
	fprintf (stderr, "\t%s\n", _ps_font_info[j].ps_name);
    }

  if (havecap ("PCL_FONTS"))
    {
      numfonts = 0;
      for (i=0; _pcl_font_info[i].ps_name; i++)
	numfonts++;
      odd = (numfonts % 2 == 1);
      numpairs = numfonts / 2;

      fprintf (stderr, 
	       "Names of supported PCL fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (_pcl_font_info[j].ps_name);
	  fprintf (stderr, "\t%s", _pcl_font_info[j++].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stderr);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stderr, "%s\n", _pcl_font_info[k++].ps_name);
	}
      if (odd)
	fprintf (stderr, "\t%s\n", _pcl_font_info[j].ps_name);
    }
}

