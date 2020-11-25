/* This file contains the display_fonts routine, which is used in
   user-level executables that are linked with libplot.  Its output is
   device-specific. */

#include "sys-defines.h"
#include "plot.h"
#include "plotcompat.h"

/* forward references */
int display_fonts ____P((const char *display_type, const char *progname));
int list_fonts ____P((const char *display_type, const char *progname));

/* for use in printing font names in two columns; assumption is that all
   font names have lengths in range 0..MAX_FONTNAME_LEN, inclusive */
#define MAX_FONTNAME_LEN 36
static char spaces[MAX_FONTNAME_LEN+1] = "                                   ";

/* The definitions of these structures are taken from ../libplot/extern.h.
   IF THOSE STRUCTURES CHANGE, THESE SHOULD TOO.

   Font information is stored in ../libplot/g_fontdb.c. */

struct hershey_font_info_struct 
{
  char *name;			/* font name */
  char *othername;		/* an alias (for backward compatibility) */
  char *orig_name;		/* Allen Hershey's original name for it */
  short chars[256];		/* array of vector glyphs */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
  bool visible;		/* whether font is visible, i.e. not internal */
};
extern struct hershey_font_info_struct _hershey_font_info[];

struct ps_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
  const char *ps_name_alt;	/* alternative PS font name, if non-NULL */
  const char *x_name;		/* the X Windows font name */
  const char *x_name_alt;	/* alternative X Windows font name */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=variable */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  int fig_id;			/* Fig's font id */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern struct ps_font_info_struct _ps_font_info[];

struct pcl_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
  const char *substitute_ps_name; /* alt. name when in a PS file, if non-NULL */
  const char *x_name;		/* the X Windows font name */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=variable */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern struct pcl_font_info_struct _pcl_font_info[];

struct stick_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
				/* no x_name field */  
  bool basic;			/* basic stick font (supp. on all devices)? */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=variable */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1 */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int raster_width_lower;	/* width of abstract raster (lower half) */
  int raster_height_lower;	/* height of abstract raster (lower half) */
  int raster_width_upper;	/* width of abstract raster (upper half) */
  int raster_height_upper;	/* height of abstract raster (upper half) */
  int hp_charset_lower;		/* old HP character set number (lower half) */
  int hp_charset_upper;		/* old HP character set number (upper half) */
  int kerning_table_lower;	/* number of a kerning table (lower half) */
  int kerning_table_upper;	/* number of a kerning table (upper half) */
  char width[256];		/* per-character width information */
  int offset;			/* left edge (applies to all chars) */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* encoding is iso8859-1? (after reencoding) */
};

extern const struct stick_font_info_struct _stick_font_info[];

/* List of Plotter types we support getting font information from,
   NULL-terminated.  This list also appears in the program text below. */
#ifndef X_DISPLAY_MISSING
static char *_known_devices[] =
{ "X", "pnm", "gif", "ai", "ps", "fig", "pcl", "hpgl", "tek", "meta", NULL };
#else
static char *_known_devices[] =
{ "pnm", "gif", "ai", "ps", "fig", "pcl", "hpgl", "tek", "meta", NULL };
#endif

int
#ifdef _HAVE_PROTOS
display_fonts (const char *display_type, const char *progname)
#else
display_fonts (display_type, progname)
     const char *display_type, *progname;
#endif
{
  int numfonts, numpairs, i, j, k, handle;
  bool found = false, odd;
  char **device_ptr = _known_devices;

  while (*device_ptr)
    if (strcmp (display_type, *device_ptr++) == 0)
      {
	found = true;
	break;
      }

  if (found == false || strcmp (display_type, "meta") == 0)
    {
#ifndef X_DISPLAY_MISSING
      fprintf (stderr, "\
To list available fonts, type `%s -T \"format\" --help-fonts',\n\
where \"format\" is the output format:\n\
X, pnm, gif, ai, ps, fig, pcl, hpgl, or tek.\n",
	       progname);
#else  /* X_DISPLAY_MISSING */
      fprintf (stderr, "\
To list available fonts, type `%s -T \"format\" --help-fonts',\n\
where \"format\" is the output format:\n\
pnm, gif, ai, ps, fig, pcl, hpgl, or tek.\n",
	       progname);
#endif /* X_DISPLAY_MISSING */
      return 0;
    }

  if ((handle = newpl (display_type, NULL, stdout, stderr)) < 0)
    {
      fprintf (stderr, 
	       "%s: no font information on display device \"%s\" is available\n",
	       progname, display_type);
      return 0;
    }
  else
    selectpl (handle);

  if (havecap ("HERSHEY_FONTS"))
    {
      int visible_num;

      numfonts = 0;
      for (i=0; _hershey_font_info[i].name; i++)
	if (_hershey_font_info[i].visible)
	numfonts++;
      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      /* compute j and k: j=0, k=numpairs + (odd ? 1 : 0) in terms of visibles */
      j = 0;
      k = 0;
      visible_num = -1;
      for (i=0; _hershey_font_info[i].name; i++)
	if (_hershey_font_info[i].visible)
	  {
	    visible_num++;	/* visible_num is index into array of visibles */
	    if (visible_num == 0)
	      j = i;
	    else if (visible_num == numpairs + (odd ? 1 : 0))
	      k = i;
	  }

      fprintf (stdout, 
	       "Names of supported Hershey vector fonts (case-insensitive):\n");
      for (i=0; i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (_hershey_font_info[j].name);
	  fprintf (stdout, "\t%s", _hershey_font_info[j].name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", _hershey_font_info[k].name);
	  /* bump both j and k */
	  do
	    j++;
	  while (_hershey_font_info[j].visible == false);
	  if (i < numpairs - 1)
	    {
	      do
		k++;
	      while (_hershey_font_info[k].visible == false);
	    }
	}
      if (odd)
	fprintf (stdout, "\t%s\n", _hershey_font_info[j].name);
    }

  if (havecap ("STICK_FONTS"))
    {
      int extra_fonts, *goodfonts;

      numfonts = 0;
      for (i=0; _stick_font_info[i].ps_name; i++)
	numfonts++;

      /* if this Plotter doesn't support extras, skip them */
      extra_fonts = havecap ("EXTRA_STICK_FONTS");
      goodfonts = (int *)malloc (numfonts * sizeof(int));
      for (i=0, j=0; _stick_font_info[i].ps_name; i++)
	{
	  if (!extra_fonts && _stick_font_info[i].basic == false)
	    continue;
	  goodfonts[j++] = i;
	}
      numfonts = j;

      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      fprintf (stdout, 
	       "Names of supported HP vector fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (_stick_font_info[goodfonts[j]].ps_name);
	  fprintf (stdout, "\t%s", _stick_font_info[goodfonts[j++]].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", _stick_font_info[goodfonts[k++]].ps_name);
	}
      if (odd)
	fprintf (stdout, "\t%s\n", _stick_font_info[goodfonts[j]].ps_name);

      free (goodfonts);
    }

  if (havecap ("PCL_FONTS"))
    {
      numfonts = 0;
      for (i=0; _pcl_font_info[i].ps_name; i++)
	numfonts++;
      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      fprintf (stdout, 
	       "Names of supported PCL fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (_pcl_font_info[j].ps_name);
	  fprintf (stdout, "\t%s", _pcl_font_info[j++].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", _pcl_font_info[k++].ps_name);
	}
      if (odd)
	fprintf (stdout, "\t%s\n", _pcl_font_info[j].ps_name);
    }

  if (havecap ("PS_FONTS"))
    {
      numfonts = 0;
      for (i=0; _ps_font_info[i].ps_name; i++)
	numfonts++;
      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      fprintf (stdout, 
	       "Names of supported Postscript fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (_ps_font_info[j].ps_name);
	  fprintf (stdout, "\t%s", _ps_font_info[j++].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", _ps_font_info[k++].ps_name);
	}
      if (odd)
	fprintf (stdout, "\t%s\n", _ps_font_info[j].ps_name);
    }

  if (strcmp (display_type, "X") == 0)
    {
      fprintf (stdout, "Scalable X Window System fonts, such as utopia-medium-r-normal and\n");
      fprintf (stdout, "charter-medium-r-normal, may also be used.\n");
    }

  return 1;
}

int
#ifdef _HAVE_PROTOS
list_fonts (const char *display_type, const char *progname)
#else
list_fonts (display_type, progname)
     const char *display_type, *progname;
#endif
{
  bool found = false;
  int i, handle;
  char **device_ptr = _known_devices;

  while (*device_ptr)
    if (strcmp (display_type, *device_ptr++) == 0)
      {
	found = true;
	break;
      }

  if (found == false || 
      (handle = newpl (display_type, NULL, stdout, stderr)) < 0)
    {
      fprintf (stderr, 
	       "%s: no font information on display device \"%s\" is available\n",
	       progname, display_type);
      return 0;
    }
  else
    selectpl (handle);

  if (havecap ("HERSHEY_FONTS"))
    for (i=0; _hershey_font_info[i].name; i++)
      if (_hershey_font_info[i].visible)
	fprintf (stdout, "%s\n", _hershey_font_info[i].name);

  if (havecap ("PCL_FONTS"))
    for (i=0; _pcl_font_info[i].ps_name; i++)
      fprintf (stdout, "%s\n", _pcl_font_info[i].ps_name);

  if (havecap ("STICK_FONTS"))
    for (i=0; _stick_font_info[i].ps_name; i++)
      fprintf (stdout, "%s\n", _stick_font_info[i].ps_name);

  if (havecap ("PS_FONTS"))
    for (i=0; _ps_font_info[i].ps_name; i++)
      fprintf (stdout, "%s\n", _ps_font_info[i].ps_name);

  return 1;
}
