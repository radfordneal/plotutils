/* This file is the driving routine for the GNU `plot' program.  It
   includes code to read a stream of commands, in GNU metafile format, and
   call libplot functions to draw the graphics.

   Copyright (C) 1989-1998 Free Software Foundation, Inc. */

#include "sys-defines.h"
#include "plot.h"
#include "getopt.h"

/* The six input formats we recognize */
typedef enum 
{
  /* There are two GNU metafile formats: binary and portable (ascii). */
  GNU_BINARY, GNU_PORTABLE, 

  /* PLOT5_HIGH and PLOT5_LOW are the two distinct versions of Unix plot(5)
     format (high/low byte first), which we also support.  They are
     requested by the -h and -l options respectively.  The user may not
     need to specify either of those options explicitly, since if
     sizeof(short int)=2 then plot(5) input format is subsumed by
     GNU_OLD_BINARY format (see below). */
  PLOT5_HIGH, PLOT5_LOW,

  /* GNU_OLD_BINARY [obsolete] is the binary format used in pre-2.0
     releases, with no initial magic string, short ints instead of ints,
     and no OPENPL or CLOSEPL directives.  By default, we assume that the
     input format is GNU_OLD_BINARY, and we switch to GNU_BINARY or
     GNU_PORTABLE if we see the appropriate magic header string.

     GNU_OLD_PORTABLE [obsolete] is the ascii format used in pre-2.0
     releases, with no initial magic string, and no OPENPL or CLOSEPL
     directives.  It subsumes the ascii version of plot(5) format, found on
     some Unix systems.  If the user wishes to parse GNU_OLD_PORTABLE
     format, he/she should use the -A option.  */
  GNU_OLD_BINARY, GNU_OLD_PORTABLE

} plot_format;

const char *progname = "plot";	/* name of this program */

const char *usage_appendage = " [FILE]...\n\
With no FILE, or when FILE is -, read standard input.\n";

bool single_page_is_requested = false; /* set if user uses -p option */
char *bg_color = NULL;		/* initial bg color, can be spec'd by user */
char *font_name = NULL;		/* initial font name, can be spec'd by user */
char *pen_color = NULL;		/* initial pen color, can be spec'd by user */
double font_size = -1.0;	/* initial fractional size, <0 means default */
double line_width = -1.0;	/* initial line width, <0 means default */
int requested_page = 0;		/* user sets this via -p option */

/* Default input file format (see list of supported formats above).  Don't
   change this (GNU_OLD_BINARY is an obsolete format, but it subsumes
   plot(5) format on many operating systems).  We'll switch to the
   appropriate modern format by peeking at the first line of the input file. */
plot_format user_specified_input_format = GNU_OLD_BINARY;
plot_format input_format = GNU_OLD_BINARY;

/* Variable used for working around a problem libplot currently has, of not
   recognizing absurdly large font size requests, which can crash X
   servers.  (You used to be able to do this by piping any EPS file to
   `plot -TX', since the `S' on the first line was interepreted as a font
   size request!)  We no longer process the `S' op code unless one of the
   `space' commands is seen first.  This adds a little safety, since a
   `space' command should appear very early in any well-behaved
   metafile. */
bool space_seen = false;

/* Long options we recognize */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] = 
{
  /* The most important option */
  { "display-type",	ARG_REQUIRED,	NULL, 'T'},
  /* Other frequently used options */
  { "font-name",	ARG_REQUIRED,	NULL, 'F' },
  { "font-size",	ARG_REQUIRED,	NULL, 'f' },
  { "line-width",	ARG_REQUIRED,	NULL, 'W' },
  /* Long options with no equivalent short option alias */
  { "bg-color",		ARG_REQUIRED,	NULL, 'q' << 8 },
  { "bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8 },
  { "max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8 },
  { "page-number",	ARG_REQUIRED,	NULL, 'p' },
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8 },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  /* Options relevant only to raw plot (refers to metafile output) */
  { "portable-output",	ARG_NONE,	NULL, 'O' },
  /* Old input formats, for backward compatibility */
  { "high-byte-first-input",	ARG_NONE,	NULL, 'h' },
  { "low-byte-first-input",	ARG_NONE,	NULL, 'l' },
  { "ascii-input",		ARG_NONE,	NULL, 'A' },
  /* obsolete hidden option [same as 'A'] */
  { "ascii-input",		ARG_NONE,	NULL, 'I' },
  /* Documentation options */
  { "help-fonts",	ARG_NONE,	NULL, 'f' << 8 },
  { "list-fonts",	ARG_NONE,	NULL, 'l' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL,  0}
};
    
/* null-terminated list of options that we don't show to the user */
int hidden_options[] = { (int)'I', 0 };


/* forward references */
bool read_plot __P((FILE *in_stream));
char *read_string __P((FILE *input, bool *badstatus));
double read_float __P((FILE *input, bool *badstatus));
double read_int __P((FILE *input, bool *badstatus));
int read_true_int __P((FILE *input, bool *badstatus));
unsigned char read_byte_as_unsigned_char __P((FILE *input, bool *badstatus));
unsigned int read_byte_as_unsigned_int __P((FILE *input, bool *badstatus));
/* from libcommon */
extern bool display_fonts __P((const char *display_type, const char *progname));
extern bool list_fonts __P((const char *display_type, const char *progname));
extern void display_usage __P((const char *progname, const int *omit_vals, const char *appendage, bool fonts));
extern void display_version __P((const char *progname)); 
extern Voidptr xcalloc __P ((unsigned int nmemb, unsigned int size));
extern Voidptr xmalloc __P ((unsigned int size));
extern Voidptr xrealloc __P ((Voidptr p, unsigned int length));
extern char *xstrdup __P ((const char *s));


int
#ifdef _HAVE_PROTOS
main (int argc, char *argv[])
#else
main (argc, argv)
     int argc;
     char *argv[];
#endif
{
  bool do_list_fonts = false;	/* show a list of fonts? */
  bool show_fonts = false;	/* supply help on fonts? */
  bool show_usage = false;	/* show usage message? */
  bool show_version = false;	/* show version message? */
  char *display_type = "meta";	/* default libplot output format */
  int errcnt = 0;		/* errors encountered */
  int handle;			/* libplot handle for Plotter object */
  int local_page_number;	/* temporary storage */
  int opt_index;		/* long option index */
  int option;			/* option character */
  int retval;			/* return value */

  while ((option = getopt_long (argc, argv, "hlAIOp:F:f:W:T:", long_options, &opt_index)) != EOF)
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
	  parampl ("META_PORTABLE", "yes");
	  break;
	case 'F':		/* set the initial font */
	  font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (font_name, optarg);
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
	  parampl ("BITMAPSIZE", optarg);
	  break;
	case 'P' << 8:		/* Page size */
	  parampl ("PAGESIZE", optarg);
	  break;
	case 'f':		/* set the initial fontsize */
	  {
	    double local_font_size;

	    if (sscanf (optarg, "%lf", &local_font_size) <= 0)
	      {
		fprintf (stderr,
			 "%s: error: initial font size must be a number, was `%s'\n",
			 progname, optarg);
		errcnt++;
		break;
	      }
	    if (local_font_size < 0.0)
	      fprintf (stderr, "%s: warning: ignoring negative initial font size `%f'\n",
		       progname, local_font_size);
	    else if (local_font_size == 0.0)
	      fprintf (stderr, "%s: ignoring zero initial font size\n",
		       progname);
	    else
	      font_size = local_font_size;
	    break;
	  }
	case 'p':		/* page number */
	  if (sscanf (optarg, "%d", &local_page_number) <= 0
	      || local_page_number < 1)
	    {
	      fprintf (stderr,
		       "%s: error: page number must be a positive integer, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    {
	      requested_page = local_page_number;
	      single_page_is_requested = true;
	    }
	  break;
	case 'W':		/* set the initial line width */
	  {
	    double local_line_width;

	    if (sscanf (optarg, "%lf", &local_line_width) <= 0)
	      {
		fprintf (stderr,
			 "%s: error: initial line width must be a number, was `%s'\n",
			 progname, optarg);
		errcnt++;
		break;
	      }
	    if (local_line_width < 0.0)
	      fprintf (stderr, "%s: ignoring negative initial line width `%f'\n",
		       progname, local_line_width);
	    else
	      line_width = local_line_width;
	    break;
	  }
	case 'h':	/* High-byte-first plot(5) metafile(s) */
	  user_specified_input_format = PLOT5_HIGH;
	  break;
	case 'l':	/* Low-byte-first plot(5) metafile(s) */
	  user_specified_input_format = PLOT5_LOW;
	  break;
	case 'A':	/* Old ascii metafile(s) */
	case 'I':
	  user_specified_input_format = GNU_OLD_PORTABLE;
	  break;
	case 'Q' << 8:		/* Plot rotation angle, ARG REQUIRED	*/
	  parampl ("ROTATE", optarg);
	  break;
	case 'M' << 8:		/* Max line length */
	  parampl ("MAX_LINE_LENGTH", optarg);
	  break;

	case 'V' << 8:		/* Version */
	  show_version = true;
	  break;
	case 'f' << 8:		/* Fonts */
	  show_fonts = true;
	  break;
	case 'h' << 8:		/* Help */
	  show_usage = true;
	  break;
	case 'l' << 8:		/* Fonts */
	  do_list_fonts = true;
	  break;

	default:
	  errcnt++;
	  break;
	}
    }

  if (errcnt > 0)
    {
      fprintf (stderr, "Try `%s --help' for more information\n", progname);
      return 1;
    }
  if (show_version)
    {
      display_version (progname);
      return 0;
    }
  if (do_list_fonts)
    {
      bool success;

      success = list_fonts (display_type, progname);
      if (success)
	return 0;
      else
	return 1;
    }
  if (show_fonts)
    {
      bool success;

      success = display_fonts (display_type, progname);
      if (success)
	return 0;
      else
	return 1;
    }
  if (show_usage)
    {
      display_usage (progname, hidden_options, usage_appendage, true);
      return 0;
    }

  if (bg_color)
    /* select user-specified background color */
    parampl ("BG_COLOR", bg_color);

  if ((handle = newpl (display_type, NULL, stdout, stderr)) < 0)
    {
      fprintf (stderr, "%s: error: could not open plot device\n", progname);
      return 1;
    }
  else
    selectpl (handle);

  retval = 0;
  if (optind < argc)
    /* input files (or stdin) named explicitly on the command line */
    {
      for (; optind < argc; optind++)
	{
	  FILE *data_file;
	  
	  if (strcmp (argv[optind], "-") == 0)
	    data_file = stdin;
	  else
	    {
	      data_file = fopen (argv[optind], "r");
	      if (data_file == NULL)
		{
		  fprintf (stderr, "%s: %s: %s\n", progname, argv[optind], strerror(errno));
		  fprintf (stderr, "%s: ignoring this file\n", progname);
		  errno = 0;	/* not quite fatal */
		  retval = 1;
		}
	      continue;	/* back to top of for loop */
	    }
	  if (read_plot (data_file) == false)
	    {
		  fprintf (stderr, "%s: could not parse input file `%s'\n",
			   progname, argv[optind]);
		  retval = 1;
		  break;	/* break out of for loop */
	    }

	  if (data_file != stdin) /* Don't close stdin */
	    if (fclose (data_file) < 0)
	      {
		fprintf (stderr, 
			 "%s: could not close input file `%s'\n",
			 progname, argv[optind]);
		retval = 1;
		continue;	/* back to top of for loop */
	      }
	}
    } /* endfor */
  else
    /* no files/streams spec'd on the command line, just read stdin */
    {
      if (read_plot (stdin) == false)
	{
	  fprintf (stderr, "%s: could not parse input\n", progname);
	  retval = 1;
	}
    }

  selectpl (0);
  if (deletepl (handle) < 0)
    {
      fprintf (stderr, "%s: could not close plot device\n", progname);
      retval = 1;
    }

  return retval;
}


/* read_plot() reads a file in GNU metafile format or plot(5) format from a
   stream, and calls a plot function according to each instruction found in
   the file.  Return value indicates whether stream was parsed
   successfully. */
bool
#ifdef _HAVE_PROTOS
read_plot (FILE *in_stream)
#else
read_plot (in_stream)
     FILE *in_stream;
#endif
{
  bool argerr = false;	/* error occurred while reading argument? */
  bool display_open = false;	/* display device open? */
  bool first_command = true;	/* first command of file? */
  bool in_page = false;		/* within an openpl..closepl? */
  bool parameters_initted = false; /* user-specified parameters initted? */
  bool unrec = false;	/* unrecognized command seen? */
  char *s;
  double x0, y0, x1, y1, x2, y2;
  int i0, i1, i2;
  int instruction;
  static int current_page = 1;	/* page count is continued from file to file */
  
  /* User may specify one of the formats PLOT5_HIGH, PLOT5_LOW, and
     GNU_OLD_PORTABLE on the command line.  If user doesn't specify a
     format, this is by default set to GNU_OLD_BINARY [obsolete], and we'll
     figure out whether the file is in a modern format, and if so, 
     which one. */
  input_format = user_specified_input_format;

  /* peek at first instruction in file */
  instruction = getc (in_stream);

  /* Switch away from GNU_OLD_BINARY to GNU_BINARY if a header line,
     interpreted here as a comment, is seen at top of file.  See also
     parsing of the COMMENT instruction below (we further switch to
     GNU_PORTABLE if the header line indicates we should). */
  if (input_format == GNU_OLD_BINARY && instruction == COMMENT)
    input_format = GNU_BINARY;

  while (instruction != EOF)
    {
      /* If a pre-modern format, OPENPL directive is not supported.  So
	 open display device if it hasn't already been opened, and
	 we're on the right page. */
      if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
	if ((!single_page_is_requested || current_page == requested_page)
	    && instruction != COMMENT && display_open == false)
	  {
	    if (openpl() < 0)
	      {
		fprintf (stderr, "%s: error: could not open plot device\n", 
			 progname);
		exit (1);
	      }
	    else
	      display_open = true;
	  }
  
      switch (instruction)
	{
	case ALABEL:
	  {
	    char x_adjust, y_adjust;

	    x_adjust = (char)read_byte_as_unsigned_char (in_stream, &argerr);
	    y_adjust = (char)read_byte_as_unsigned_char (in_stream, &argerr); 
	    s = read_string (in_stream, &argerr);
	    if (!argerr)
	      {
		if (!single_page_is_requested || current_page == requested_page)
		  alabel (x_adjust, y_adjust, s);
		free (s);
	      }
	  }
	  break;
	case ARC:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      farc (x0, y0, x1, y1, x2, y2);
	  break;
	case ARCREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      farcrel (x0, y0, x1, y1, x2, y2);
	  break;
	case BGCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      bgcolor (i0, i1, i2);
	  break;
	case BOX:
	  	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fbox (x0, y0, x1, y1);
	  break;
	case BOXREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fboxrel (x0, y0, x1, y1);
	  break;
	case CAPMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		capmod (s);
	      free (s);
	    }
	  break;
	case CIRCLE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcircle (x0, y0, x1);
	  break;
	case CIRCLEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcirclerel (x0, y0, x1);
	  break;
	case COLOR:		/* redundant op code, to be removed */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      color (i0, i1, i2);
	  break;
	case CLOSEPL:
	  if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
	    /* shouldn't be seeing a CLOSEPL */
	    {
	      if (display_open && closepl () < 0)
		{
		  fprintf (stderr, "%s: error: could not close plot device\n",
			   progname);
		  exit (1);
		}
	      current_page++;
	      return false;	/* signal a parse error */
	    }
	  else
	    /* GNU_BINARY or GNU_PORTABLE format, so this may be legitimate */
	    {
	      if (in_page == false)
		/* shouldn't be seeing a CLOSEPL */
		{
		  current_page++;
		  return false;	/* signal a parse error */
		}
	      else
		/* the CLOSEPL is legitimate */
		{
		  if (!single_page_is_requested 
		      || current_page == requested_page)
		    {
		      if (closepl() < 0)
			{
			  fprintf (stderr, 
				   "%s: error: could not close plot device\n", 
				   progname);
			  exit (1);
			}
		      display_open = false;
		    }
		  in_page = false;
		  current_page++; /* `page' is an OPENPL..CLOSEPL */
		}
	    }
	  break;
	case COMMENT:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      /* if a header line, switch to appropriate modern format */
	      if (first_command
		  && input_format != PLOT5_HIGH
		  && input_format != PLOT5_LOW
		  && (strlen (s) >= 6)
		  /* check magic number */
		  && strncmp (s, "PLOT ", 5) == 0)
		switch (s[5])
		  {
		  case '1':
		    input_format = GNU_BINARY;
		    break;		
		  case '2':
		    input_format = GNU_PORTABLE;
		    break;		
		  default:
		    fprintf (stderr, 
			     "%s: input file is of an unrecognized metafile type\n",
			     progname);
		    break;
		  }
	      free (s);
	    }
	  break;
	case CONT:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcont (x0, y0);
	  break;
	case CONTREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcontrel (x0, y0);
	  break;
	case ELLARC:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellarc (x0, y0, x1, y1, x2, y2);	  
	  break;
	case ELLARCREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellarcrel (x0, y0, x1, y1, x2, y2);	  
	  break;
	case ELLIPSE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellipse (x0, y0, x1, y1, x2);
	  break;
	case ELLIPSEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellipserel (x0, y0, x1, y1, x2);
	  break;
	case ENDPATH:
	  if (!single_page_is_requested || current_page == requested_page)
	    endpath ();
	  break;
	case ERASE:
	  if (!single_page_is_requested || current_page == requested_page)
	    erase ();
	  break;
	case FILLTYPE:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      filltype (i0);
	  break;
	case FILLCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fillcolor (i0, i1, i2);
	  break;
	case FONTNAME:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		fontname (s);
	      free (s);
	    }
	  break;
	case FONTSIZE:
	  x0 = read_int (in_stream, &argerr);
	  if (space_seen)	/* workaround, see comment above */
	    {
	      if (!argerr)
		if (!single_page_is_requested || current_page == requested_page)
		  ffontsize (x0);
	    }
	  break;
	case JOINMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		joinmod (s);
	      free (s);
	    }
	  break;
	case LABEL:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		label (s);
	      free (s);
	    }
	  break;
	case LINE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fline (x0, y0, x1, y1);
	  break;
	case LINEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      flinerel (x0, y0, x1, y1);
	  break;
	case LINEMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		linemod (s);
	      free (s);
	    }
	  break;
	case LINEWIDTH:
	  x0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      flinewidth (x0);
	  break;
	case MARKER:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmarker (x0, y0, i0, y1);
	  break;
	case MARKERREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmarkerrel (x0, y0, i0, y1);
	  break;
	case MOVE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmove (x0, y0);
	  break;
	case MOVEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmoverel (x0, y0);
	  break;
	case OPENPL:
	  if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
	    /* shouldn't be seeing an OPENPL */
	    {
	      if (display_open && closepl () < 0)
		{
		  fprintf (stderr, "%s: error: could not close plot device\n",
			   progname);
		  exit (1);
		}
	      current_page++;
	      return false;	/* signal a parse error */
	    }
	  else
	    /* GNU_BINARY or GNU_PORTABLE format, so may be legitimate */
	    {
	      if (in_page)
		/* shouldn't be seeing another OPENPL */
		{
		  if (display_open && closepl () < 0)
		    {
		      fprintf (stderr, 
			       "%s: error: could not close plot device\n",
			       progname);
		      exit (1);
		    }
		  current_page++;
		  return false;	/* signal a parse error */
		}

	      /* this OPENPL is legitimate */
	      if (!single_page_is_requested || current_page == requested_page)
		{
		  if (openpl() < 0)
		    {
		      fprintf (stderr, "%s: error: could not open plot device\n", 
			       progname);
		      exit (1);
		    }
		  else
		    display_open = true;
		}
	      /* we're now in an openpl..closepl pair */
	      in_page = true;
	    }
	  break;
	case PENCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pencolor (i0, i1, i2);
	  break;
	case POINT:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fpoint (x0, y0);
	  break;
	case POINTREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fpointrel (x0, y0);
	  break;
	case RESTORESTATE:
	  if (!single_page_is_requested || current_page == requested_page)
	    restorestate ();
	  break;
	case SAVESTATE:
	  if (!single_page_is_requested || current_page == requested_page)
	    savestate ();
	  break;
	case SPACE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
	    fspace (x0, y0, x1, y1);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to space(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (pen_color)
		pencolorname (pen_color);
	      if (font_name)
		fontname (font_name);
	      if (font_size >= 0.0)
		ffontsize (font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		flinewidth (line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  space_seen = true;
	  break;
	case SPACE2:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
	    fspace2 (x0, y0, x1, y1, x2, y2);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to space(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (bg_color)
		{
		  bgcolorname (bg_color);
		  erase ();
		}
	      if (pen_color)
		pencolorname (pen_color);
	      if (font_name)
		fontname (font_name);
	      if (font_size >= 0.0)
		ffontsize (font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		flinewidth (line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  space_seen = true;
	  break;
	case TEXTANGLE:
	  x0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      ftextangle (x0);
	  break;

        /* floating point counterparts to the above */
	case FARC:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      farc (x0, y0, x1, y1, x2, y2);
	  break;
	case FARCREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      farcrel (x0, y0, x1, y1, x2, y2);
	  break;
	case FBOX:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fbox (x0, y0, x1, y1);
	  break;
	case FBOXREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fboxrel (x0, y0, x1, y1);
	  break;
	case FCIRCLE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcircle (x0, y0, x1);
	  break;
	case FCIRCLEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcirclerel (x0, y0, x1);
	  break;
	case FCONT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcont (x0, y0);
	  break;
	case FCONTREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fcontrel (x0, y0);
	  break;
	case FELLARC:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellarc (x0, y0, x1, y1, x2, y2);
	  break;
	case FELLARCREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellarcrel (x0, y0, x1, y1, x2, y2);
	  break;
	case FELLIPSE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellipse (x0, y0, x1, y1, x2);
	  break;
	case FELLIPSEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fellipserel (x0, y0, x1, y1, x2);
	  break;
	case FFONTSIZE:
	  x0 = read_float (in_stream, &argerr);
	  if (space_seen)	/* workaround, see comment above */
	    {
	      if (!argerr)
		if (!single_page_is_requested || current_page == requested_page)
		  ffontsize (x0);
	    }
	  break;
	case FLINE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fline (x0, y0, x1, y1);
	  break;
	case FLINEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      flinerel (x0, y0, x1, y1);
	  break;
	case FLINEWIDTH:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      flinewidth (x0);
	  break;
	case FMARKER:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmarker (x0, y0, i0, y1);
	  break;
	case FMARKERREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmarkerrel (x0, y0, i0, y1);
	  break;
	case FMOVE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmove (x0, y0);
	  break;
	case FMOVEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fmoverel (x0, y0);
	  break;
	case FPOINT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fpoint (x0, y0);
	  break;
	case FPOINTREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!single_page_is_requested || current_page == requested_page)
	    fpointrel (x0, y0);
	  break;
	case FSPACE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
	    fspace (x0, y0, x1, y1);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to fspace(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (bg_color)
		{
		  bgcolorname (bg_color);
		  erase ();
		}
	      if (pen_color)
		pencolorname (pen_color);
	      if (font_name)
		fontname (font_name);
	      if (font_size >= 0.0)
		ffontsize (font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		flinewidth (line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  space_seen = true;
	  break;
	case FSPACE2:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
		fspace2 (x0, y0, x1, y1, x2, y2);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to fspace(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (bg_color)
		{
		  bgcolorname (bg_color);
		  erase ();
		}
	      if (pen_color)
		pencolorname (pen_color);
	      if (font_name)
		fontname (font_name);
	      if (font_size >= 0.0)
		ffontsize (font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		flinewidth (line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  space_seen = true;
	  break;
	case FTEXTANGLE:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      ftextangle (x0);
	  break;

        /* floating point routines with no integer counterparts */
	case FCONCAT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fconcat (x0, y0, x1, y1, x2, y2);
	  break;
	case FROTATE:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      frotate (x0);
	  break;
	case FSCALE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      fscale (x0, y0);
	  break;
	case FTRANSLATE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      ftranslate (x0, y0);
	  break;
        case '\n':
	  if (input_format == GNU_PORTABLE
	      || input_format == GNU_OLD_PORTABLE)
	    /* harmless, ignore */
	    break;
	  /* FALLTHRU if a non-ascii format */
	default:
	  unrec = true;
	  break;
	} /* end of switch() */
      
      first_command = false;

      if (unrec)
	{
	  fprintf (stderr, "%s: unrecognized command `0x%x' encountered in input\n",
		   progname, instruction);
	  break;		/* break out of while loop */
	}
      if (argerr)
	{
	  int eof = feof (in_stream);
	  
	  if (eof)
	    fprintf (stderr, "%s: input terminated prematurely\n",
		     progname);
	  else
	    fprintf (stderr, "%s: unable to parse argument of command `0x%x' in input\n",
		     progname, instruction);
	  break;		/* break out of while loop */
	}
      
      instruction = getc (in_stream); /* get next instruction */
    } /* end of while loop, EOF reached */

  if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
    /* if a premodern format, this file contains only one page */
    {
      /* close display device at EOF, if it was ever opened */
      if (display_open && closepl () < 0)
	{
	  fprintf (stderr, "%s: error: could not close plot device\n",
		   progname);
	  exit (1);
	}
      current_page++;		/* bump page count at EOF */
    }
  else
    /* file is in a modern format, should have closed display device (if it
       was ever opened) */
    {
      if (in_page)
	/* shouldn't be the case; parse error */
	{
	  if (display_open && closepl () < 0)
	    {
	      fprintf (stderr, "%s: error: could not close plot device\n",
		       progname);
	      exit (1);
	    }
	  current_page++;
	  return false;		/* signal parse error */
	}
    }

  return ((argerr || unrec) ? false : true); /* file parsed successfully? */
}


/* read a single byte from input stream, return as unsigned char (0..255) */
unsigned char
#ifdef _HAVE_PROTOS
read_byte_as_unsigned_char (FILE *input, bool *badstatus)
#else
read_byte_as_unsigned_char (input, badstatus)
     FILE *input;
     bool *badstatus;
#endif
{
  int newint;

  if (*badstatus == true)
    return 0;

  newint = getc (input);
  /* have an unsigned char cast to an int, in range 0..255 */
  if (newint == EOF)
    {
      *badstatus = true;
      return 0;
    }
  else
    return (unsigned char)newint;
}

/* read a single byte from input stream, return as unsigned int (0..255) */
unsigned int
#ifdef _HAVE_PROTOS
read_byte_as_unsigned_int (FILE *input, bool *badstatus)
#else
read_byte_as_unsigned_int (input, badstatus)
     FILE *input;
     bool *badstatus;
#endif
{
  int newint;

  if (*badstatus == true)
    return 0;

  newint = getc (input);
  /* have an unsigned char cast to an int, in range 0..255 */
  if (newint == EOF)
    {
      *badstatus = true;
      return 0;
    }
  else
    return (unsigned int)newint;
}

/* read an integer from input stream (can be in ascii format, system binary
   format for integers or short integers, or perhaps in crufty old 2-byte
   format) */
int
#ifdef _HAVE_PROTOS
read_true_int (FILE *input, bool *badstatus)
#else
read_true_int (input, badstatus)
     FILE *input;
     bool *badstatus;
#endif
{
  int x, zi, returnval;
  short zs;
  unsigned int u;

  if (*badstatus == true)
    return 0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      returnval = fscanf (input, " %d", &x);
      if (returnval != 1)
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_BINARY:		/* system format for integers */
    default:
      returnval = fread (&zi, sizeof(zi), 1, input);
      if (returnval == 1)
	x = zi;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_OLD_BINARY:	/* system format for short integers */
      returnval = fread (&zs, sizeof(zs), 1, input);
      if (returnval == 1)
	x = (int)zs;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case PLOT5_HIGH:		/* two-byte format, high byte first */
      u = ((read_byte_as_unsigned_int (input, badstatus)) << 8);
      u |= read_byte_as_unsigned_int (input, badstatus);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    case PLOT5_LOW:		/* two-byte format, low byte first */
      u = read_byte_as_unsigned_int (input, badstatus);
      u |= (read_byte_as_unsigned_int (input, badstatus) << 8);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    }

  return x;
}
  
/* a relaxed version of the preceding routine: if a portable
   (human-readable) format is used, a floating point number may substitute
   for the integer */
double
#ifdef _HAVE_PROTOS
read_int (FILE *input, bool *badstatus)
#else
read_int (input, badstatus)
     FILE *input;
     bool *badstatus;
#endif
{
  int x, zi, returnval;
  short zs;
  unsigned int u;

  if (*badstatus == true)
    return 0.0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      {
	double r;

	returnval = fscanf (input, " %lf", &r);
	if (returnval != 1)
	  {
	    *badstatus = true;
	    r = 0.0;
	  }
	return r;
      }
    case GNU_BINARY:		/* system format for integers */
    default:
      returnval = fread (&zi, sizeof(zi), 1, input);
      if (returnval == 1)
	x = (int)zi;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_OLD_BINARY:	/* system format for short integers */
      returnval = fread (&zs, sizeof(zs), 1, input);
      if (returnval == 1)
	x = (int)zs;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case PLOT5_HIGH:		/* two-byte format, high byte first */
      u = ((read_byte_as_unsigned_int (input, badstatus)) << 8);
      u |= read_byte_as_unsigned_int (input, badstatus);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    case PLOT5_LOW:		/* two-byte format, low byte first */
      u = read_byte_as_unsigned_int (input, badstatus);
      u |= (read_byte_as_unsigned_int (input, badstatus) << 8);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    }

  return (double)x;
}
  
/* read a floating point quantity from input stream (may be in ascii format
   or system single-precision format) */
double
#ifdef _HAVE_PROTOS
read_float (FILE *input, bool *badstatus)
#else
read_float (input, badstatus)
     FILE *input;
     bool *badstatus;
#endif
{
  float f;
  int returnval;
  
  if (*badstatus == true)
    return 0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      /* human-readable format */
      returnval = fscanf (input, " %f", &f);
      break;
    case GNU_BINARY:
    case GNU_OLD_BINARY:
    default:
      /* system single-precision format */
      returnval = fread (&f, sizeof(f), 1, input);
      break;
    case PLOT5_HIGH:
    case PLOT5_LOW:
      /* plot(5) didn't support floats */
      returnval = 0;
      break;
    }

  if (returnval != 1 || f != f)
    /* failure, or NaN */
    {
      *badstatus = true;
      return 0.0;
    }
  else
    return (double)f;
}

/* Read a newline-terminated string from input stream.  As returned, the
   string, with \0 replacing \n, is allocated on the heap and may be
   freed. */
char *
#ifdef _HAVE_PROTOS
read_string (FILE *input, bool *badstatus)
#else
read_string (input, badstatus)
     FILE *input;
     bool *badstatus;
#endif
{
  int length = 0, buffer_length = 16; /* initial length */
  char *buffer;
  char c;

  if (*badstatus == true)
    return 0;

  buffer = (char *)xmalloc ((unsigned int)buffer_length);
  while (true)
    {
      if (length >= buffer_length)
	{
	  buffer_length *= 2;
	  buffer = (char *)xrealloc (buffer, (unsigned int)(buffer_length));
	}
      c = (char)read_byte_as_unsigned_char (input, badstatus);
      if ((*badstatus == true) || (c == '\n'))
	break;
      buffer [length++] = c;
    }

  if (*badstatus)
    {
      free (buffer);
      return NULL;
    }
  else
    {
      buffer [length] = '\0';	/*  null-terminate string */
      return buffer;
    }
}

