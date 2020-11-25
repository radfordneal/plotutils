/* This file is the driving routine for the GNU `plotfont' program.  It
   includes code to plot all characters in a specified font. */

/* Copyright (C) 1998 Free Software Foundation, Inc. */

#include "sys-defines.h"
#include "plot.h"
#include "getopt.h"

const char *progname = "plotfont"; /* name of this program */

const char *usage_appendage = " FONT...\n";

/* possible rotation angles for plot within graphics display */
enum rotation { ROT_0, ROT_90, ROT_180, ROT_270 };

enum radix { DECIMAL, OCTAL, HEXADECIMAL };

char *bg_color = NULL;		/* initial bg color, can be spec'd by user */
char *font_name = NULL;		/* initial font name, can be spec'd by user */
char *pen_color = NULL;		/* initial pen color, can be spec'd by user */
char *title_font_name = NULL;	/* title font name, NULL -> current font */
char *numbering_font_name = NULL; /* numbering font name, NULL -> default */

/* Long options we recognize */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] = 
{
  /* The most important option */
  { "display-type",	ARG_REQUIRED,	NULL, 'T'},
  /* Other frequently used options */
  { "box",		ARG_NONE,	NULL, 'b' << 8 },
  { "octal",		ARG_NONE,	NULL, 'o'},
  { "hexadecimal",	ARG_NONE,	NULL, 'x'},
  { "jis-page",		ARG_REQUIRED,	NULL, 'J'},
  { "jis-row",		ARG_REQUIRED,	NULL, 'j'},
  { "lower-half",	ARG_NONE,	NULL, '1'},
  { "upper-half",	ARG_NONE,	NULL, '2'},
  { "font-name",	ARG_REQUIRED,	NULL, 'F' }, /* hidden */
  /* Long options with no equivalent short option alias */
  { "numbering-font-name", ARG_REQUIRED, NULL, 'N' << 8 },
  { "title-font-name",	ARG_REQUIRED,	NULL, 'Z' << 8 },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "bg-color",		ARG_REQUIRED,	NULL, 'q' << 8 },
  { "bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8 },
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8 },
  { "rotation",		ARG_REQUIRED,	NULL, 'r' << 8},
  /* Options relevant only to raw plotfont (refers to metafile output) */
  { "portable-output",	ARG_NONE,	NULL, 'O' },
  /* Documentation options */
  { "help-fonts",	ARG_NONE,	NULL, 'f' << 8 },
  { "list-fonts",	ARG_NONE,	NULL, 'l' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL,  0}
};
    
/* null-terminated list of options that we don't show to the user */
int hidden_options[] = { (int)'J', (int)'F', 0 };

/* forward references */
bool do_font ____P((const char *name, bool upper_half, char *pen_color_name, char *numbering_font_name, char *title_font_name, bool bearings, enum radix base, enum rotation rotation_angle, int jis_page, bool do_jis_page));
void write_three_bytes ____P((int charnum, char *numbuf, int radix));
void write_two_bytes ____P((int charnum, char *numbuf, int radix));

/* from libcommon */
extern int display_fonts ____P((const char *display_type, const char *progname));
extern int list_fonts ____P((const char *display_type, const char *progname));
extern void display_usage ____P((const char *progname, const int *omit_vals, const char *appendage, bool fonts));
extern void display_version ____P((const char *progname)); 
extern Voidptr xcalloc ____P ((unsigned int nmemb, unsigned int size));
extern Voidptr xmalloc ____P ((unsigned int size));
extern Voidptr xrealloc ____P ((Voidptr p, unsigned int length));
extern char *xstrdup ____P ((const char *s));


int
#ifdef _HAVE_PROTOS
main (int argc, char *argv[])
#else
main (argc, argv)
     int argc;
     char *argv[];
#endif
{
  bool bearings = false;	/* show sidebearings on characters? */
  bool do_jis_page = false;	/* show page of HersheyEUC in JIS encoding? */
  bool do_list_fonts = false;	/* show a list of fonts? */
  bool show_fonts = false;	/* supply help on fonts? */
  bool show_usage = false;	/* show usage message? */
  bool show_version = false;	/* show version message? */
  bool upper_half = false;	/* upper half of font, not lower? */
  char *display_type = "meta";	/* default libplot output format */
  char *option_font_name = NULL; /* allows user to use -F */
  enum radix base = DECIMAL;
  enum rotation rotation_angle = ROT_0;
  int errcnt = 0;		/* errors encountered */
  int handle;			/* libplot handle for Plotter object */
  int jis_page = 33;		/* page of HersheyEUC in JIS encoding */
  int opt_index;		/* long option index */
  int option;			/* option character */

  while ((option = getopt_long (argc, argv, "12oxOj:J:F:T:", long_options, &opt_index)) != EOF)
    {
      if (option == 0)
	option = long_options[opt_index].val;
      
      switch (option) 
	{
	case 'T':		/* Display type, ARG REQUIRED      */
	  display_type = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (display_type, optarg);
	  break;
	case 'O':		/* Ascii output */
	  pl_parampl ("META_PORTABLE", "yes");
	  break;
	case '1':		/* Lower half */
	  upper_half = false;
	  break;
	case '2':		/* Upper half */
	  upper_half = true;
	  break;
	case 'o':		/* Octal */
	  base = OCTAL;
	  break;
	case 'x':		/* Hexadecimal */
	  base = HEXADECIMAL;
	  break;
	case 'F':		/* set the initial font */
	  option_font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (option_font_name, optarg);
	  break;
	case 'N' << 8:		/* Numbering Font name, ARG REQUIRED */
	  numbering_font_name = xstrdup (optarg);
	  break;
	case 'Z' << 8:		/* Title Font name, ARG REQUIRED */
	  title_font_name = xstrdup (optarg);
	  break;
	case 'C' << 8:		/* set the initial pen color */
	  pen_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (pen_color, optarg);
	  break;
	case 'q' << 8:		/* set the initial background color */
	  bg_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (bg_color, optarg);
	  break;
	case 'B' << 8:		/* Bitmap size */
	  pl_parampl ("BITMAPSIZE", optarg);
	  break;
	case 'P' << 8:		/* Page size */
	  pl_parampl ("PAGESIZE", optarg);
	  break;
	case 'r' << 8:		/* Rotation angle */
	  if (strcmp (optarg, "90") == 0)
	    rotation_angle = ROT_90;
	  else if (strcmp (optarg, "180") == 0)
	    rotation_angle = ROT_180;
	  else if (strcmp (optarg, "270") == 0)
	    rotation_angle = ROT_270;
	  else rotation_angle = ROT_0;
	  break;
	case 'b' << 8:		/* Bearings requested */
	  bearings = true;
	  break;
	case 'V' << 8:		/* Version */
	  show_version = true;
	  break;
	case 'f' << 8:		/* Fonts */
	  show_fonts = true;
	  break;
	case 'l' << 8:		/* Fonts */
	  do_list_fonts = true;
	  break;
	case 'J':		/* JIS page */
	  if (sscanf (optarg, "%d", &jis_page) <= 0
	      || (jis_page < 33) || (jis_page > 126))
	    {
	      fprintf (stderr,
		       "%s: bad JIS page, must be in the range 33..126\n",
		       progname);
	      errcnt++;
	    }
	  else
	    do_jis_page = true;
	  break;
	case 'j':		/* JIS row */
	  if (sscanf (optarg, "%d", &jis_page) <= 0
	      || (jis_page < 1) || (jis_page > 94))
	    {
	      fprintf (stderr,
		       "%s: bad JIS row, must be in the range 1..94\n",
		       progname);
	      errcnt++;
	    }
	  else
	    {
	      jis_page += 32;
	      do_jis_page = true;
	    }
	  break;
	case 'h' << 8:		/* Help */
	  show_usage = true;
	  break;

	default:
	  errcnt++;
	  break;
	}
    }

  if (errcnt > 0)
    {
      fprintf (stderr, "Try `%s --help' for more information\n", progname);
      return EXIT_FAILURE;
    }
  if (show_version)
    {
      display_version (progname);
      return EXIT_SUCCESS;
    }
  if (do_list_fonts)
    {
      int success;

      success = list_fonts (display_type, progname);
      if (success)
	return EXIT_SUCCESS;
      else
	return EXIT_FAILURE;
    }
  if (show_fonts)
    {
      int success;

      success = display_fonts (display_type, progname);
      if (success)
	return EXIT_SUCCESS;
      else
	return EXIT_FAILURE;
    }
  if (show_usage)
    {
      display_usage (progname, hidden_options, usage_appendage, true);
      return EXIT_SUCCESS;
    }

  if (option_font_name == NULL && optind >= argc)
    {
      fprintf (stderr, "%s: no font(s) specified \n", progname);
      return EXIT_FAILURE;
    }

  if (do_jis_page)
    {
      if ((!((option_font_name == NULL && optind == argc - 1)
	    || (option_font_name && optind >= argc)))
	  || (option_font_name && strcasecmp (option_font_name, "HersheyEUC") != 0)
	  || (!option_font_name && strcasecmp (argv[optind], "HersheyEUC") != 0))
	{
	  fprintf (stderr, "%s: JIS page can only be specified for HersheyEUC \n font", progname);
	  return EXIT_FAILURE;
	}	  
    }

  if (bg_color)
    /* select user-specified background color */
    pl_parampl ("BG_COLOR", bg_color);

  if ((handle = pl_newpl (display_type, NULL, stdout, stderr)) < 0)
    {
      fprintf (stderr, "%s: error: could not open plot device\n", progname);
      return EXIT_FAILURE;
    }
  else
    pl_selectpl (handle);

  if (option_font_name)
    /* user specifed a font with -F */
    {
      if (do_font (option_font_name, upper_half, pen_color, numbering_font_name, title_font_name, bearings, base, rotation_angle, jis_page, do_jis_page) == false)
	return EXIT_FAILURE;
    }
  
  if (optind < argc)
    /* 1 or more fonts named explicitly on command line */
    {
      for (; optind < argc; optind++)
	{
	  char *font_name;

	  font_name = argv[optind];
	  if (do_font (font_name, upper_half, pen_color, numbering_font_name, title_font_name, bearings, base, rotation_angle, jis_page, do_jis_page) == false)
	    return EXIT_FAILURE;
	}
    }

  pl_selectpl (0);
  if (pl_deletepl (handle) < 0)
    {
      fprintf (stderr, "%s: could not close plot device\n", progname);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

#define NUM_ROWS 12

#define SIZE 1.0
#define MAX_TITLE_LENGTH 0.9

#define HSPACING 0.1	/* must have 8*HSPACING < SIZE */
#define VSPACING 0.06	/* must have 12*VSPACING < SIZE-LINE_VOFFSET */

#define LINE_HOFFSET (0.5 * ((SIZE) - 8 * (HSPACING)))
#define LINE_VOFFSET 0.15	/* top line down from top of display by this */
#define CHAR_HOFFSET ((LINE_HOFFSET) + 0.5 * (HSPACING))
#define CHAR_VOFFSET ((LINE_VOFFSET) + 0.5 * (VSPACING))

#define TOP (SIZE - (LINE_VOFFSET))
#define BOTTOM ((SIZE - (LINE_VOFFSET)) - 12 * (VSPACING))
#define LEFT (LINE_HOFFSET)
#define RIGHT (LINE_HOFFSET + 8 * HSPACING)

#define FONT_SIZE 0.04
#define TITLE_FONT_SIZE 0.045
#define NUMBERING_FONT_SIZE 0.015

/* shifts of numbers in grid cells leftward and upward */
#define N_X_SHIFT 0.015
#define N_Y_SHIFT 0.05

bool
#ifdef _HAVE_PROTOS
do_font (const char *name, bool upper_half, char *pen_color_name, char *numbering_font_name, char *title_font_name, bool bearings, enum radix base, enum rotation rotation_angle, int jis_page, bool do_jis_page)
#else
do_font (name, upper_half, pen_color_name, numbering_font_name, title_font_name, bearings, base, rotation_angle, jis_page, do_jis_page)
     const unsigned char *name;
     bool upper_half;
     char *pen_color_name;
     char *numbering_font_name;
     char *title_font_name;
     bool bearings;
     enum radix base;
     enum rotation rotation_angle;
     int jis_page;
     bool do_jis_page;
#endif
{
  char buf[16];
  char numbuf[16];
  char suffixbuf[16];
  char *titlebuf;
  char *suffix;
  double title_width;
  int i, j, bottom_octet, top_octet;

  if (do_jis_page)
    {
      switch (base)
	{
	case DECIMAL:
	default:
	  sprintf (suffixbuf, " (row %d)", jis_page - 32);
	  break;
	case OCTAL:
	  sprintf (suffixbuf, " (row 0%o)", jis_page - 32);
	  break;
	case HEXADECIMAL:
	  sprintf (suffixbuf, " (row 0x%X)", jis_page - 32);
	  break;
	}
      suffix = suffixbuf;
    }
  else
    suffix = upper_half ? " (upper half)" : " (lower half)";
  titlebuf = (char *)xmalloc (strlen (name) + strlen (suffix) + 1);
  strcpy (titlebuf, name);
  strcat (titlebuf, suffix);

  if (pl_openpl () < 0)
    {
      fprintf (stderr, "%s: error: could not open plot device\n", progname);
      return false;
    }

  switch ((int)(rotation_angle))
    {
    case (int)ROT_0:
    default:
      pl_fspace (0.0, 0.0, (double)SIZE, (double)SIZE);
      break;
    case (int)ROT_90:
      pl_fspace2 (0.0, (double)SIZE,
		  0.0, 0.0,
		  (double)SIZE, (double)SIZE);
      break;
    case (int)ROT_180:
      pl_fspace2 ((double)SIZE, (double)SIZE,
		  0.0, (double)SIZE,
		  (double)SIZE, 0.0);
      break;
    case (int)ROT_270:
      pl_fspace2 ((double)SIZE, 0.0,
		  (double)SIZE, (double)SIZE,
		  0.0, 0.0);
      break;
    }
  pl_erase();
  if (pen_color_name)
    pl_pencolorname (pen_color_name);
  
  pl_fmove (0.5 * SIZE, 0.5 * (SIZE + TOP));
  if (title_font_name)
    pl_fontname (title_font_name);
  else
    pl_fontname (name);
  pl_ffontsize ((double)(TITLE_FONT_SIZE));

  title_width = pl_flabelwidth (titlebuf);
  if (title_width > MAX_TITLE_LENGTH)
    /* squeeze title to fit */
    pl_ffontsize ((double)(TITLE_FONT_SIZE) * (MAX_TITLE_LENGTH / title_width));

  /* print title */
  pl_alabel ('c', 'c', titlebuf);

  if (do_jis_page)
    bottom_octet = 4;
  else			/* ordinary map */
    {
      if (upper_half)
	bottom_octet = 20;
      else
	bottom_octet = 4;
    }
  top_octet = bottom_octet + NUM_ROWS - 1;

  /* draw grid */

  pl_linemod ("solid");
  pl_fbox (LEFT, BOTTOM, RIGHT, TOP);
  for (i = 1; i <= 7; i++)
    /* boustrophedon */
    {
      if (i % 2)
	pl_fline (LINE_HOFFSET + i * HSPACING, BOTTOM, 
		  LINE_HOFFSET + i * HSPACING, TOP);
      else
	pl_fline (LINE_HOFFSET + i * HSPACING, TOP,
		  LINE_HOFFSET + i * HSPACING, BOTTOM);
    }      
  for (j = 1; j <= 11; j++)
    /* boustrophedon */
    {
      if (j % 2)
	pl_fline (RIGHT, TOP - j * VSPACING,
		  LEFT, TOP - j * VSPACING);
      else
	pl_fline (LEFT, TOP - j * VSPACING,
		  RIGHT, TOP - j * VSPACING);
    }

  /* number grid cells */

  if (numbering_font_name)
    pl_fontname (numbering_font_name);
  else				/* select default font */
    pl_fontname ("");
  pl_ffontsize ((double)(NUMBERING_FONT_SIZE));
  if (bearings)
    pl_linemod ("dotted");
  for (i = bottom_octet; i <= top_octet; i++)
    for (j = 0; j < 8; j++)
      {
	int row, column, charnum;

	row = i - bottom_octet;	/* row, 0..11 */
	column = j;		/* column, 0..7 */

	charnum = (unsigned char)(8 * i + j);
	if (charnum == 127)	/* 0xff not a legitimate glyph */
	  continue;
	if (do_jis_page && charnum == 32)
	  continue;		/* 0x20 not legitimate for JIS */

	switch (base)
	  {
	  case HEXADECIMAL:
	    write_two_bytes (charnum - (do_jis_page ? 32 : 0), numbuf, 16);
	    break;
	  case DECIMAL:
	  default:
	    write_three_bytes (charnum - (do_jis_page ? 32 : 0), numbuf, 10);
	    break;
	  case OCTAL:
	    write_three_bytes (charnum - (do_jis_page ? 32 : 0), numbuf, 8);
	    break;
	  }
	
	pl_fmove ((double)(LINE_HOFFSET + HSPACING * (column + 1 - N_X_SHIFT)),
		  (double)(SIZE - (LINE_VOFFSET + VSPACING * (row + 1 - N_Y_SHIFT))));
	pl_alabel ('r', 'x', numbuf);
      }

  /* fill grid cells with characters */

  pl_fontname (name);
  pl_ffontsize ((double)(FONT_SIZE));
  for (i = bottom_octet; i <= top_octet; i++)
    for (j = 0; j < 8; j++)
      {
	int row, column, charnum;

	row = i - bottom_octet;	/* row, 0..11 */
	column = j;		/* column, 0..7 */
	
	charnum = (unsigned char)(8 * i + j);
	if (charnum == 127)	/* 0xff not a legitimate glyph */
	  continue;
	if (do_jis_page && charnum == 32)
	  continue;		/* 0x20 not legitimate for JIS */

	if (!do_jis_page)
	  {
	    buf[0] = charnum;
	    buf[1] = '\0';
	  }
	else			/* do JIS page */
	  {
	    /* two bytes, set high bits on both page and character */
	    buf[0] = jis_page + 0x80;
	    buf[1] = charnum + 0x80;
	    buf[2] = '\0';
	  }

	pl_fmove ((double)(LINE_HOFFSET + HSPACING * (column + 0.5)),
		  (double)(SIZE - (LINE_VOFFSET + VSPACING * (row + 0.5))));
	/* place glyph on page */
	pl_alabel ('c', 'c', (char *)buf);
	if (bearings)
	  {
	    double halfwidth;

	    /* compute glyph width */
	    halfwidth = 0.5 * pl_flabelwidth ((char *)buf);
	    if (halfwidth == 0.0)
	      /* empty glyph, draw only one vertical dotted line */
	      pl_fline ((double)(CHAR_HOFFSET + HSPACING * column),
			(double)(SIZE - (CHAR_VOFFSET + VSPACING * (row - 0.5))),
			(double)(CHAR_HOFFSET + HSPACING * column),
			(double)(SIZE - (CHAR_VOFFSET + VSPACING * (row + 0.5))));
	    else
	      /* draw vertical dotted lines to either side of glyph */
	      {
		pl_fline ((double)(CHAR_HOFFSET + HSPACING * column - halfwidth),
			  (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row - 0.5))),
			  (double)(CHAR_HOFFSET + HSPACING * column - halfwidth),
			  (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row + 0.5))));
		pl_fline ((double)(CHAR_HOFFSET + HSPACING * column + halfwidth),
			  (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row - 0.5))),
			  (double)(CHAR_HOFFSET + HSPACING * column + halfwidth),
			  (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row + 0.5))));
	      }
	  }
      }
  
  if (pl_closepl () < 0)
    {
      fprintf (stderr, "%s: error: could not close plot device\n", progname);
      return false;
    }
  return true;
}

/* Write an integer that is < (radix)**4 as three ascii bytes with respect
   to the specified radix.  Initial zeroes are converted to spaces. */
void 
#ifdef _HAVE_PROTOS
write_three_bytes (int charnum, char *numbuf, int radix)
#else
write_three_bytes (charnum, numbuf, radix)
     int charnum;
     char *numbuf;
     int radix;
#endif
{
  int i;

  numbuf[0] = charnum / (radix * radix);
  numbuf[1] = (charnum - (radix * radix) * (charnum / (radix * radix))) / radix;
  numbuf[2] = charnum % radix;
  numbuf[3] = '\0';

  for (i = 0 ; i <= 2 ; i++)
    {
      if (numbuf[i] >= 10)
	numbuf[i] += ('A' - 10);
      else 
	numbuf[i] += '0';
    }
  if (numbuf[0] == '0')
    {
      numbuf[0] = ' ';
      if (numbuf[1] == '0')
	numbuf[1] = ' ';
    }
}

/* Write an integer that is < (radix)**3 as two ascii bytes with respect to
   the specified radix.  Initial zeroes are converted are not changed. */
void 
#ifdef _HAVE_PROTOS
write_two_bytes (int charnum, char *numbuf, int radix)
#else
write_two_bytes (charnum, numbuf, radix)
     int charnum;
     char *numbuf;
     int radix;
#endif
{
  int i;

  numbuf[0] = charnum / radix;
  numbuf[1] = charnum % radix;
  numbuf[2] = '\0';

  for (i = 0 ; i <= 1 ; i++)
    {
      if (numbuf[i] >= 10)
	numbuf[i] += ('A' - 10);
      else 
	numbuf[i] += '0';
    }
}
