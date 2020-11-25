/* This file is the main routine for tek2plot.  It includes code to read a
   stream of Tektronix commands, and draw graphics by calling libplot routines.

   Copyright (C) 1989-1997 Free Software Foundation, Inc. */

/* We emulate the non-interactive features of a Tektronix 4014 with
   extended graphics module (EGM), as described in the 4014 Service Manual
   (Tektronix Part #070-1648-00, dated 8/74; there is also a User Manual
   [Part #070-1647-00]).  We don't attempt to support interactive features,
   such as GIN mode, status enquiry, or bypass mode.  We also don't support
   write-through mode or beam defocusing.
   
   We also emulate a few features of Tektronix 4014 emulators (see comments
   in the code), in particular the ANSI ISO-6429 color extensions
   recognized by the MS-DOS Kermit v2.31 Tektronix emulator.  And we make a
   point of recognizing, and ignoring, the control sequences ESC [ ?38h
   (switch to Tek mode), ESC [ ?38l (switch to native mode), and ESC ^C
   (switch to native mode).  Such control sequences are accepted by the
   Kermit emulator, VT-style terminals doing Tek emulation, and/or X11
   xterm (when in Tek emulation mode).

   We could do a lot more to emulate the Kermit Tektronix emulator (by
   parsing the `draw rectangle' and `fill rectangle' commands in
   particular).  But this version is good enough, for example, to display
   the output of gnuplot's `kc_tek40xx' (Kermit color), `km_tek40xx'
   (Kermit monochrome), `tek40xx', and `vttek' Tektronix terminal
   drivers. */

#include "sys-defines.h"
#include "plot.h"
#include "getopt.h"

char *base_progname = "tek";
char *progname;			/* name of this program */

#define DEFAULT_FONT_NAME "courier" /* ideally, monospaced */

#define PLOT_SIZE 4096

#define MARGIN2_X  2048		/* left margin flips between 0 and 2048 */
#define PLOT_Y_SIZE 3120
#define PLOT_HOME_Y 3071

#define ONE_BIT (0x1)
#define TWO_BITS (0x3)
#define FOUR_BITS (0x0f)
#define FIVE_BITS (0x1f)
#define TEN_BITS (0x3ff)

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] = 
{
  { "ascii-output",	ARG_NONE,	NULL, 'O' },
  { "font-name",	ARG_REQUIRED,	NULL, 'F' },
  { "font-size",	ARG_REQUIRED,	NULL, 'f' },
  { "line-width",	ARG_REQUIRED,	NULL, 'W' },
  /* Long options with no equivalent short option alias */
  { "use-tek-fonts",	ARG_NONE,	NULL, 't' << 8 },
  { "high-byte-first", 	ARG_NONE,	NULL, 'H' << 8 },
  { "low-byte-first",	ARG_NONE,	NULL, 'L' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL, 0}
};

typedef struct
{
  int red;
  int green;
  int blue;
} Color;

/* ANSI (ISO-6429) color extensions.  Scheme is essentially:
	0 = normal, 1 = bright
        foreground color (30-37) = 30 + colors
                where colors are   1=red, 2=green, 4=blue
	background color is similar, with `40' replacing `30'. */

Color ansi_color[16] =
{
  {0x0000, 0x0000, 0x0000},		/* black, \033[0;30m */
  {0x8b8b, 0x0000, 0x0000},		/* red4 \033[0;31m */
  {0x0000, 0x8b8b, 0x0000},		/* green4 \033[0;32m */
  {0x8b8b, 0x8b8b, 0x0000},		/* yellow4 \033[0;33m */
  {0x0000, 0x0000, 0x8b8b},		/* blue4 \033[0;34m */
  {0x8b8b, 0x0000, 0x8b8b},		/* magenta4 \033[0;35m */
  {0x0000, 0x8b8b, 0x8b8b},		/* cyan4, \033[0;36m */
  {0x8b8b, 0x8b8b, 0x8b8b},		/* gray55 \033[0;37m */
  {0x4d4d, 0x4d4d, 0x4d4d},		/* gray30 \033[1;30m */
  {0xffff, 0x0000, 0x0000},		/* red \033[1;31m */
  {0x0000, 0xffff, 0x0000},		/* green \033[1;32m */
  {0xffff, 0xffff, 0x0000},		/* yellow \033[1;33m */
  {0x0000, 0x0000, 0xffff},		/* blue \033[1;34m */
  {0xffff, 0x0000, 0xffff},		/* magenta \033[1;35m */
  {0x0000, 0xffff, 0xffff},		/* cyan \033[1;36m */
  {0xffff, 0xffff, 0xffff}		/* white \033[1;37m */
};

Boolean save_screen = FALSE;	/* save the screen == do not erase */
Boolean use_tek_fonts = FALSE;	/* not installed everywhere */
int hspacing = 56, vspacing = 88; /* use large characters initially */
int x = 0, y = 0;		/* position */
int xlomargin = 0;		/* toggles between 0 and 2048 (MARGIN2_X) */
char *vector_mode = "solid";	/* use solid vectors initially */
Boolean ignore_vector = TRUE;	/* ignore initial dark vector */
Boolean disconnected = FALSE;	/* true in [special] point plot mode */
Boolean special = FALSE;	/* true in special point plot mode */
Boolean ignore_byte = FALSE;	/* don't ignore initial [intensity] byte
  				   (used in special point plot mode only) */

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
Boolean parse_control_sequence P__ ((FILE *in_stream, int byte_read));
int read_byte P__((FILE *stream));
void display_version P__((void)), display_usage P__((void));
void read_plot P__((FILE *in_stream));
Voidptr xmalloc P__ ((unsigned int length));
#undef P__


int
main (argc, argv)
     int argc;
     char *argv[];
{
  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered */
  Boolean show_version = FALSE;	/* remember to show version message */
  Boolean show_usage = FALSE;	/* remember whether to output usage message. */
  char *font_name = NULL;	/* font name, can be specified by user */
  double font_size = -1.0;	/* fractional text size,
				   <0 means just use the default. */
  double line_width = -1.0;	/* line width, <0 means default */
  FILE *output_stream = stdout;	/* stream for plot library output */
  int named_input = 0;		/* count named plot files on command line. */
  char *buffer;
  int  buffer_length;

  progname = xmalloc (strlen (base_progname) 
		      + IMAX(strlen (_libplot_suffix), 4) + 2);
  strcpy (progname, base_progname);
  strcat (progname, "2");
  if (*_libplot_suffix)
    strcat (progname, _libplot_suffix);
  else
    strcat (progname, "plot");

  buffer_length = 1024;
  buffer = (char *)xmalloc (buffer_length);

  while ((option = getopt_long(argc, argv, "OF:f:W:", long_options, &opt_index)) != EOF) 
    {
      if (option == 0)
	option = long_options[opt_index].val;
      
      switch (option)
	{
	case 's':		/* Don't erase screen before plotting */
	  save_screen = TRUE;
	  break;
	case 'F':		/* set the initial font */
	  font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (font_name, optarg);
	  break;
	case 'f':		/* set the initial fontsize */
	  if (sscanf (optarg, "%lf", &font_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: fractional font size must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'W':		/* set the initial line width */
	  if (sscanf (optarg, "%lf", &line_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: line width must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'O':		/* Ascii output */
	  _libplot_output_is_ascii = 1;	/* set libplot variable */
	  break;

	  /*---------------- Long options below here ----------------*/
	case 'h' << 8:		/* Help */
	  show_usage = TRUE;
	  break;
	case 't' << 8:
	  use_tek_fonts = TRUE;
	  break;
	case 'H' << 8:		/* output byte order */
	  _libplot_output_high_byte_first = 1;
	  break;
	case 'L' << 8:		/* output byte order */
	  _libplot_output_high_byte_first = -1;
	  break;
	case 'V' << 8:		/* Version */
	  show_version = TRUE;
	  break;

	default:
	  errcnt++;
	  break;
	}
    }
  
  if (errcnt > 0)
    {
      fprintf (stderr, "Try `%s --help' for more information.\n", progname);
      return 1;
    }
  if (show_version)
    {
      display_version ();
      return 0;
    }
  if (show_usage)
    {
      display_usage ();
      return 0;
    }

  outfile (output_stream);	/* set output stream for plot library */
  if (openpl () < 0)
    {
      fprintf (stderr, "%s: error: could not open plot device\n", progname);
      return 1;
    }
  space (0, 0, PLOT_SIZE - 1, PLOT_SIZE - 1);
  if (!save_screen)
    erase();

  /* Set initial font, font size, and line width. */

  if (use_tek_fonts)
    fontname ("tekfont0");
  else
    {
      if (font_name)
	fontname (font_name);
      else
	fontname (DEFAULT_FONT_NAME);
      if (font_size >= 0.0)
	ffontsize (font_size * PLOT_SIZE);
    }
  if (line_width >= 0.0)
    flinewidth (line_width * PLOT_SIZE);
  
  if (optind < argc)
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
		  fprintf (stderr, "%s: warning: ignoring nonexistent or inaccessible file `%s'\n",
			   progname, argv[optind]);
		  continue;
		}
	    }
	  named_input++;
	  read_plot (data_file);
	  
	  if (data_file != stdin) /* Don't close stdin */
	    if (fclose (data_file) < 0)
	      {
		fprintf (stderr, "%s: error: could not close output file\n",
			 progname);
		return 1;
	      }
	}
    } /* endfor */
  
  if (!named_input)
    /* Read stdin if no files were named on the command line. */
    read_plot (stdin);
  
  if (closepl () < 0)
    {
      fprintf (stderr, "%s: error: could not close plot device\n",
	       progname);
      return 1;
    }
  
  if (output_stream != stdout) /* Don't close stdout */
    if (fclose (output_stream) < 0)
      {
	fprintf (stderr, "%s: error: could not close output file\n",
		 progname);
	return 1;
      }
  
  return 0;
}

int 
read_byte (stream)
     FILE *stream;
{
  int i;

  i = (int) getc (stream);
  i &= 0x7f;			/* high bit ignored */
  return i;
}

/* read_plot() reads a Tektronix input stream and calls a plot function
   according to each plot instruction found in the file.  We start out in
   graph mode, for plotting of vectors.  In fact this routine is
   essentially a vector-reading and plotting loop.  It calls another
   routine, parse_control_sequence(), whenever it reads a byte from the
   input stream that lacks the bit pattern identifying it as part of a
   vector. */

void
read_plot (in_stream)
     FILE *in_stream;
{
  int byte_read;
  int index = 0;
  
  /* variables for the point-reading DFA, initialized */
  int status_one = 0, status_three = 0;	/* 0=none, 1=seen one, 2=finished */
  Boolean got_lo_x = FALSE, got_lo_y = FALSE, got_hi_x = FALSE, got_hi_y = FALSE;
  int lo_x = 0, lo_y = 0, hi_x = 0, hi_y = 0;
  int saved_lo_y = 0, saved_hi_x = 0, saved_hi_y = 0;
  int egm = 0;
  int temp_three = 0;

  if (feof (in_stream))
    return;
  do				/* main vector-reading do..while loop */
    {
      byte_read = read_byte (in_stream);
      index++;

      if (special && ignore_byte)		
	/* discard the initial intensity byte, if we're in special point
           plot mode */
	{  
	  ignore_byte = FALSE;
	  continue;		/* i.e. go to bottom of do..while */
	}

      /* Ignore high bit (bit 8); bit pattern of next two bits (bits 7/6)
	 determines what sort of coordinate byte we have.  1 = Hi_X or
	 Hi_Y, 2 = Lo_X, 3 = Lo_Y or EGM.  Bytes appear in order

	 	[Hi_Y] [EGM] [Lo_Y] [Hi_X] Lo_X.  

		  1      3     3       1    2

	 All save last are optional, except that if EGM or Hi_X is
	 transmitted, also need need a Lo_Y.  We remember old values of
	 Hi_Y, Lo_Y, Hi_X, although not EGM or Lo_X, in our DFA. */

      if ((byte_read>>5) & TWO_BITS) /* have a positioning byte */
	{
	  int type = (byte_read>>5) & TWO_BITS; /* type of positioning byte */
	  byte_read &= FIVE_BITS; /* mask off 5 relevant bits */

	  switch (type)
	    {
	    case 1:		/* Hi_Y or Hi_X */
	      switch (status_one)
		{
		case 0:
		  if (status_three)
		    {
		      hi_x = byte_read; /* 2nd = Hi_X */
		      got_hi_x = TRUE;
		      if (status_three == 1) 
			{
			  lo_y = temp_three; /* Lo_Y */
			  got_lo_y = TRUE;
			}

		      status_one = 2; /* no more 1's */
		      status_three = 2; /* no more 3's */
		    }
		  else
		    {
		      hi_y = byte_read; /* 1st = Hi_Y */
		      got_hi_y = TRUE;
		      status_one = 1;
		    }
		  break;
		case 1:
		  if (status_three == 0)
		    {
		      fprintf (stderr, 
			       "%s: error: saw Hi_Y, Hi_X bytes with no Lo_Y in between\n",
			       progname);
		      exit (1);
		    }
		  if (status_three == 1) 
		    {
		      
		      lo_y = temp_three; /* Lo_Y */
		      got_lo_y = TRUE;
		    }
		  hi_x = byte_read; /* 2nd = Hi_X */
		  got_hi_x = TRUE;
		  status_one = 2; /* no more 1's */
		  status_three = 2; /* no more 3's */
		  break;
		case 2:
		  fprintf (stderr, 
			   "%s: error: saw too many Hi_Y/Hi_X bytes\n",
			   progname);
		  exit (1);
		}
	      break;
	    case 3:		/* EGM or Lo_Y */
	      switch (status_three)
		{
		case 0:
		  if (status_one == 2)
		    {
		      fprintf (stderr, 
			       "%s: error: saw EGM/Lo_Y byte after two Hi_X/Hi_Y bytes\n",
			       progname);
		      exit (1);
		    }
		  else
		    {
		      temp_three = byte_read;
		      status_three = 1;
		    }
		  break;
		case 1:
		  if (status_one == 2)
		    {
		      fprintf (stderr, 
			       "%s: error: saw EGM/Lo_Y byte after two Hi_X/Hi_Y bytes\n",
			       progname);
		      exit (1);
		    }
		  egm = temp_three; /* 1st = EGM */
		  lo_y = byte_read; /* 2nd = Lo_Y */
		  got_lo_y = TRUE;
		  status_three = 2;
		  break;
		case 2:
		  fprintf (stderr, 
			   "%s: error: saw too many EGM/Lo_Y bytes\n",
			   progname);
		  exit (1);
		  break;
		}
	      break;
	    case 2:		/* Lo_X, final byte */
	      {
		int low_res_x, low_res_y;
		
		if (status_three == 1)
		  {
		    lo_y = temp_three; /* Lo_Y */
		    got_lo_y = TRUE;
		  }
		lo_x = byte_read; /* Lo_X */
		
		lo_y = got_lo_y ? lo_y : saved_lo_y;	      
		hi_x = got_hi_x ? hi_x : saved_hi_x;
		hi_y = got_hi_y ? hi_y : saved_hi_y;
		
		saved_lo_y = lo_y;
		saved_hi_x = hi_x;
		saved_hi_y = hi_y;	      
		
		/* On a genuine Tektronix 4014, the MSB of the 5-bit EGM
		   byte sets the margin to equal Margin 2 */
		if ((egm >> 4) & ONE_BIT)
		  xlomargin = MARGIN2_X;
		
		/* low_res is what we'd use on a pre-EGM Tektronix */
		low_res_x = (hi_x << 5) | lo_x;
		low_res_y = (hi_y << 5) | lo_y;
		x = (low_res_x << 2) | (egm & TWO_BITS);
		y = (low_res_y << 2) | ((egm >> 2) & TWO_BITS);
		
		if (ignore_vector)
		  {
		    move (x, y);
		    ignore_vector = FALSE; /* dark no more */
		  }
		else
		  cont (x, y);
		
		/* reset point-reading DFA after each vector plotted */
		got_lo_x = got_lo_y = got_hi_x = got_hi_y = FALSE;
		status_one = status_three = egm = 0;
		
		/* reset 'ignore intensity byte' after each vector plotted
                   (this is relevant only if we're in special point plot
                   mode) */
		if (special)
		  ignore_byte = TRUE;
		
		break;
	      }
	    } /* end of switch statement for positionings */
	} /* end of positioning code */
      else			/* control command, not a positioning */
	{
	  /* reset point-reading DFA */
	  got_lo_x = got_lo_y = got_hi_x = got_hi_y = FALSE;
	  status_one = status_three = egm = 0; 

	  parse_control_sequence (in_stream, byte_read);
	}
    } /* end of main loop */
  while (!feof (in_stream));

  return;			/* end of read_plot() */
}

/* parse_control_sequence() parses a control sequence from the input
   stream.  Control sequences include single control characters (e.g. CR),
   and escape sequences, which by definition begin with ESC.  They also
   include the mode-switching commands (e.g. \037, which switches to ALPHA
   mode, and \034, which switches to VECTOR mode).  According to our
   definition, a control sequence beginning with \037 terminates only when
   ALPHA mode is exited.  I.e. any sequence of printable ascii characters
   preceded by \037 is itself a control sequence (which may itself contain
   certain embedded control sequences, e.g. CR or the clear-screen command,
   ESC ^L).

   parse_control_sequence() is called from within the vector-processing
   loop, to process a byte sequence beginning with any byte that isn't a
   vector coordinate.  It is also called recursively, since when processing
   a string of ascii characters in ALPHA mode, as mentioned above, we may
   have to process a control sequence, e.g. CR or a clear-screen command.
   To facilitate recursive invocation, this routine returns a Boolean.
   TRUE means that the emulator, if it is in ALPHA mode, will have to exit
   ALPHA mode.

   This should be rewritten to use lex. */

Boolean
parse_control_sequence (in_stream, byte_read)
     FILE *in_stream;
     int byte_read;
{
  switch (byte_read)
    {
    case '\000':		/* NULL - ignored */
    case '\001':		/* ctrl A - ignored */
    case '\002':		/* ctrl B - ignored */
    case '\003':		/* ctrl C - ignored */
    case '\004':		/* ctrl D - ignored */
      /* ctrl E has unknown semantics */
    case '\006':		/* ctrl F - ignored */
      return FALSE;
    case '\007':		/* ctrl G - BELL */
      ignore_vector = FALSE;
      return FALSE;
    case '\010':		/* ctrl H - backspace */
      x -=  hspacing;
      if (x < 0)
	x = 0;
      ignore_vector = FALSE;
      return FALSE;
    case '\011':		/* ctrl I - tab (wrap if nec.) */
      x += hspacing;
      if (x >= PLOT_SIZE)
	{
	  x = xlomargin;	/* if wrapping, CR */
	  y -= vspacing; /* also LF */
	  if (y < 0)
	    y = PLOT_HOME_Y;	/* should interchange margins? */
	}
      ignore_vector = FALSE;
      return FALSE;
    case '\013':		/* ctrl K - vertical tab */
      y += vspacing;
      if (y >= PLOT_Y_SIZE)
	y = 0;
      ignore_vector = FALSE;
      return FALSE;
      /* ctrl L has unknown semantics */
    case '\012':		/* ctrl J - linefeed, enter ALPHA mode */
    case '\015':		/* ctrl M - CR, enter ALPHA mode */
      x = xlomargin;
      y -= vspacing;	
      if (y < 0)		/* wrap */
	{
	  y = PLOT_HOME_Y;
	  if (xlomargin)	/* toggle MARGIN1 <-> MARGIN2 */
	    xlomargin = 0;
	  else
	    xlomargin = MARGIN2_X;
	}
      ungetc ((int)'\037', in_stream); /* will enter ALPHA mode */
      return FALSE;
      /* ctrl N has unknown semantics */
      /* ctrl O has unknown semantics */
    case '\020':		/* ctrl P - ignore */
    case '\021':		/* ctrl Q - ignore */
    case '\022':		/* ctrl R - ignore */
    case '\023':		/* ctrl S - ignore */
    case '\024':		/* ctrl T - ignore */
    case '\025':		/* ctrl U - ignore */
    case '\026':		/* ctrl V - ignore */
    case '\031':		/* ctrl Y -  ignore */
      /* ctrl Z has unknown semantics */
      return FALSE;
    case '\030':		/* ctrl X - set bypass condition, which is
				   cleared by any number of things.
				   Kermit emulator uses this for 
				   `end Tek emulation', so we flush */
      flushpl();
      return FALSE;
    case '\033':		/* ESC, begins escape sequence */
      if (feof (in_stream))
	{
	  fprintf (stderr, 
		   "%s: warning: EOF while reading escape sequence\n",
		   progname);
	  return FALSE;
	}
      
      byte_read = read_byte (in_stream);
      switch (byte_read)
	{
	case '\003':	/* ctrl C, xterm switch-to-VT-window,
			   ignored */
	  return FALSE;
	case '\005':	/* ctrl E, request position, status */
	  fprintf (stderr, 
		   "%s: warning: ESC ^E received, but don't support sending status byte\n",
		   progname);
	  return FALSE;
	case '\014':	/* ESC ctrl L */
	  erase ();	/* erase screen */
	  x = 0;	
	  y = PLOT_HOME_Y; /* home cursor */
	  xlomargin = 0;	/* active margin is now Margin 1 */
	  ungetc ((int)'\037', in_stream); /* will enter ALPHA mode */
	  return FALSE;
	case '\015':	/* ESC ctrl M (ignored here, in fact subsequent
			   CR's are supposed to be ignored too) */
	case '\016':	/* ESC ctrl N - SO (switch to alt char set) */
	case '\017':	/* ESC ctrl O - SI (switch to ascii char set) */
	case '\027':	/* ESC ctrl W - print screen, ignore here */
	case '\030':	/* ESC ctrl X - enter bypass mode [xterm
			   emulator]; also some Tek emulators regard
			   this as `switch back from emulation mode';
			   we ignore this */
	  return FALSE;
	  
	case '\032':	/* ESC ctrl Z, enter GIN mode */
	  fprintf (stderr,
		   "%s: warning: ESC ^Z received, but do not support GIN mode\n",
		   progname);
	  return FALSE;		/* if in ALPHA mode, return to main loop */
	  
	  /* xterm emulator allows 'enter VECTOR mode' or 'enter
	     INCREMENTAL PLOT mode' byte to be preceded by ESC */
	case '\034':
	case '\036':
	  ungetc (byte_read, in_stream);
	  return TRUE;		/* if in ALPHA mode, return to main loop */
	  
	case '\035':	/* enter SPECIAL POINT PLOT mode */
	  linemod ("disconnected");
	  disconnected = TRUE;
	  special = TRUE;
	  ignore_byte = TRUE; /* ignore initial intensity byte */
	  return TRUE;		/* must exit alpha mode, if in it */
	  
	case '8':	/* large characters, 74/line, 35 lines/page */
	case '3':		/* `3' used by some emulators */
	  /* space = 14 tekpoints, LF = 22 */
	  if (use_tek_fonts)
	    fontname ("tekfont0");
	  else
	    fontsize (88); /* `large' is default size */
	  hspacing = 56;
	  vspacing = 88;
	  return FALSE;
	case '9':	/* #2 characters, 81/line, 38 lines/page */
	case '2':		/* `2' used by some emulators */
	  /* space = 12.75 tekpoints, LF = 20.5 */
	  if (use_tek_fonts)
	    fontname ("tekfont1");
	  else
	    fontsize (82);
	  hspacing = 51;
	  vspacing = 82;
	  return FALSE;
	case ':':	/* #3 characters, 121/line, 58 lines/page */
	case '1':		/* `1' used by some emulators */
	  /* space = 8.5 tekpoints, LF = 13.25 */
	  if (use_tek_fonts)
	    fontname ("tekfont2");
	  else
	    fontsize (53);
	  hspacing = 34;
	  vspacing = 53;
	  return FALSE;
	case ';':	/* small characters, 133/line, 64 lines/page */
	case '0':		/* `0' used by some emulators */
	  /* space = 7.75 tekpoints, LF = 12 */
	  if (use_tek_fonts)
	    fontname ("tekfont3");
	  else
	    fontsize (48);
	  hspacing = 31;
	  vspacing = 48;
	  return FALSE;
	  
	case '@':
	  fill (1);	/* filling of polylines [emulator] */
	  return FALSE;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	  fill (0x7FFF); /* half-filling of polylines [emulator] */
	  return FALSE;
	  
	case 'H': /* defocus 2 smaller char sizes, as well as the
		     two large ones (we ignore this) */
	  return FALSE;
	  
	  /* ESC \ through ESC ~ are interpreted as affecting display
	     operating modes, mostly for the EGM.  Should ignore any
	     not recognized?  The below may be only a partial table
	     (also it's not clear to me if any of these are valid in
	     alpha mode as well as in vector modes). */
	  
	case '\\':	/* normal vectors or alpha, normal z-axis */
	case 'g':	/* normal z-axis */
	case 'o':	/* defocused z-axis */
	case 'w':	/* write-thru mode */
	  return FALSE;
	  
	case '`':  /* solid vectors, normal z-axis */   
	case 'h':  /* solid vectors, defocused z-axis */
	  	   /* VT-based Tek emulators use this as a distinct type */
	case 'p':  /* solid vectors, write-thru mode */ 
	  linemod ("solid");
	  vector_mode = "solid";
	  return FALSE;
	case 'a':  /* dotted vectors, normal z-axis */   
	case 'i':  /* dotted vectors, defocused z-axis */
	  	   /* VT-based Tek emulators use this as a distinct type */
	case 'q':  /* dotted vectors, write-thru mode */ 
	  linemod ("dotted");
	  vector_mode = "dotted";
	  return FALSE;
	case 'b':  /* dotdashed vectors, normal z-axis */   
	case 'j':  /* dotdashed vectors, defocused z-axis */
	  	   /* VT-based Tek emulators use this as a distinct type */
	case 'r':  /* dotdashed vectors, write-thru mode */ 
	  linemod ("dotdashed");
	  vector_mode = "dotdashed";
	  return FALSE;
	case 'c':  /* shortdashed vectors, normal z-axis */   
	case 'k':  /* shortdashed vectors, defocused z-axis */
	  	   /* VT-based Tek emulators use this as a distinct type */
	case 's':  /* shortdashed vectors, write-thru mode */ 
	  linemod ("shortdashed");
	  vector_mode = "shortdashed";
	  return FALSE;
	case 'd':  /* longdashed vectors, normal z-axis */   
	case 'l':  /* longdashed vectors, defocused z-axis */
	  	   /* VT-based Tek emulators use this as a distinct type */
	case 't':  /* longdashed vectors, write-thru mode */ 
	  linemod ("longdashed");
	  vector_mode = "longdashed";
	  return FALSE;
	  
	  /* The Kermit Tek emulator accepts six line types, not just five,
	     aargh.  We map the sixth one to our first. */
	case 'e':		/* kermit `sixth type' */
	  linemod ("solid");
	  return FALSE;

	  /* If we see ESC [ we check for the the sequences ESC [ ? 3 8 h
	     (switch to Tek mode) and ESC [ ? 3 8 l (switch to native
	     mode), which some emulators accept but which we ignore.  We
	     also check for ANSI color escape sequences, of the sort
	     accepted by the Kermit Tek emulator: ESC [ 0-or-1 3 ; 3 color m
	     (see table at head of file). */
	case '[':       
	  byte_read = read_byte (in_stream);
	  switch (byte_read)
	    {
	      int intensity, color_index;

	    case '?':		/* check for VT340 mode-switch sequence */
	      byte_read = read_byte (in_stream);
	      if (byte_read != '3')
		goto bad_ansi;
	      byte_read = read_byte (in_stream);
	      if (byte_read != '8')
		goto bad_ansi;
	      byte_read = read_byte (in_stream);
	      if (byte_read != 'h' && byte_read != 'l')
		goto bad_ansi;
	      break;
	    case '0':
	      intensity = 0;
	      goto parse_color;
	    case '1':
	      intensity = 1;
	      goto parse_color;
	    default:
	      goto bad_ansi;
	      
	    parse_color:
	      byte_read = read_byte (in_stream);	      
	      if (byte_read != ';')
		goto bad_ansi;
	      
	      byte_read = read_byte (in_stream);	      
	      if (byte_read == '4')
		/* request to set background color, ignore */
		{
		  byte_read = read_byte (in_stream); /* discard digit */
		  byte_read = read_byte (in_stream); /* discard 'm' */
		  return FALSE;
		}
	      if (byte_read != '3')
		goto bad_ansi;
	      /* request to set foreground color */
	      color_index = read_byte (in_stream);
	      if (color_index > '7' || color_index < '0')
		goto bad_ansi;
	      color_index -= '0';
	      byte_read = read_byte (in_stream);
	      if (byte_read != 'm')
		goto bad_ansi;
	      color (ansi_color[8 * intensity + color_index].red,
		     ansi_color[8 * intensity + color_index].green,
		     ansi_color[8 * intensity + color_index].blue);
	      break;
	    bad_ansi:
	      fprintf (stderr, 
		       "%s: error: unknown ANSI escape sequence read\n", 
		       progname);
	      exit (1);
	    }
	  return FALSE;

	  /* Following escape sequences (beginning ESC /) include ones
	     similar to those recognized by Retrographics cards (which
	     provide Tek emulation for DEC VT102, VT131, and Televideo
	     950).  Format: ESC / x0 ; x1 ; ... ; char */
	case '/':
	  {
	    int i=0, x[10];
	    
	    sscanf("%d", (char *) &x[i]);
	    byte_read = read_byte (in_stream);
	    while (byte_read == ';')
	      {
		i++;
		sscanf("%d", (char *) &x[i]);
		byte_read = read_byte (in_stream);
	      }
	    switch (byte_read)
	      {
	      case 'e':	/* set text rotation angle */
		x[0] = (x[0]%8) * 45 - 90;
		if (x[0] < 0)
		  x[0] += 360;
		textangle (x[0]);
		break;
	      case 'd':	
		/* Retro graphics had sequences ESC / 0 d etc. */
		/* Not supported yet. */
		switch (x[0])
		  {
		  case 0: /* set pixels along vector */
		  case 1: /* clear pixels along vector */
		  case 2: /* complement pixels along vector (XOR) */
		    break;
		  }
		break;
	      case 'A': /* arc, center (x[0],x[1]), radius x[2],
			   startangle x[3], range x[4] */
		{
		  if (i < 3)
		    x[3] = 0;
		  if (i < 4)
		    x[4] = 360;
		  arc (x[0], x[1],
		       (int) (x[0] + x[2] * cos (x[3]*M_PI/180.)),
		       (int) (x[1] + x[2] * sin (x[3]*M_PI/180.)),
		       (int) (x[0] + x[2] * cos ((x[3]+x[4])*M_PI/180.)),
		       (int) (x[1] + x[2] * sin ((x[3]+x[4])*M_PI/180.)));
		default:
		  fprintf (stderr, 
			   "%s: warning: unrecognized escape sequence ESC / ... 0x%x ignored\n", 
			   progname, byte_read);
		}
		break;
	      }
	  }
	  return FALSE;		/* i.e. after doing ESC / ... */
	  
	default:
	  fprintf (stderr, 
		   "%s: warning: unrecognized escape sequence ESC 0x%x ignored\n", 
		   progname, byte_read);
	  return FALSE;
	} /* end of switch() for processing escape sequences */
      
    case '\034':		/* enter POINT PLOT mode */
      if (!special)	/* can't transit from special plot mode */
	{
	  linemod ("disconnected");
	  disconnected = TRUE;
	  ignore_vector = FALSE; /* no `dark vector' */
	}
      return TRUE;		/* if in ALPHA mode, return to main loop */
      
    case '\035':		/* enter VECTOR mode */
      if (!disconnected)	/* can't transit from special point plot
				   mode or point plot mode */
	{
	  linemod (vector_mode);
	  special = FALSE;
	  ignore_vector = TRUE; /* first vector is dark */
	}
      return TRUE;		/* if in ALPHA mode, return to main loop */
      
    case '\036':		/* enter INCREMENTAL PLOT mode */
      {
	Boolean pendown = TRUE;	/* initially */
	
	/* handle this mode via a local loop; only way out is to switch
	   to ALPHA mode, via '\037' */
	if (feof (in_stream))
	  return TRUE;		/* if in ALPHA mode, return to main loop */
	
	do
	  {
	    int x_incr = 0, y_incr = 0;
	    
	    byte_read = read_byte (in_stream);
	    switch (byte_read)
	      /* should really mask off individual bits;
		 among bits 8 7 6 5 4 3 2 1, bits 8/7 are ignored,
		 6/5 are penup/pendown, 4/3 are -Y/+Y, and 2/1
		 are -X/+X */
	      {
	      case ' ':	/* pen up */
		pendown = FALSE;
		break;
	      case 'A':	/* east */
		x_incr++;
		break;
	      case 'B':	/* west */
		x_incr--;
		break;
	      case 'D':	/* north */
		y_incr++;
		break;
	      case 'E':	/* northeast */
		x_incr++;
		y_incr++;
		break;
	      case 'F':	/* northwest */
		x_incr--;
		y_incr++;
		break;
	      case 'H':	/* south */
		y_incr--;
		break;
	      case 'I':	/* southeast */
		x_incr++;
		y_incr--;
		break;
	      case 'J':	/* southwest */
		x_incr--;
		y_incr--;
		break;
	      case 'P':	/* pen down */
		pendown = TRUE;
		break;
	      default:
		if (byte_read != '\037')
		  fprintf (stderr, 
			   "%s: warning: ignoring unknown byte 0x%x in incremental plot mode\n", 
			   progname, byte_read);
		break;
	      }
	    
	    if (byte_read != '\037')
	      {
		x += x_incr;
		y += y_incr;
		if (x < 0)
		  x = 0;
		if (x >= PLOT_SIZE)
		  x = PLOT_SIZE - 1;
		if (y < 0)
		  y = 0;
		if (y >= PLOT_Y_SIZE)
		  y = PLOT_SIZE - 1;

		if (pendown)
		  cont (x, y);
		else
		  move (x, y);
	      }
	  }
	while(!feof (in_stream) && (byte_read != '\037'));
	
	if (byte_read == '\037')
	  ungetc (byte_read, in_stream); /* push back */
	return TRUE;		/* if in ALPHA mode, return to main loop */
      }
      
    case '\037':		/* enter ALPHA mode, plot string */
      {
	char labelbuf[3];

	labelbuf[0] = labelbuf[1] = labelbuf[2] = '\0';

	while (!feof (in_stream))
	  {
	    byte_read = read_byte (in_stream);
	    if (byte_read < 32 || byte_read > 126)
	      {
		Boolean retval;

		retval = parse_control_sequence (in_stream, byte_read);
		if (retval)	/* leave ALPHA mode, return to main loop */
		  {
		    disconnected = FALSE;
		    special = FALSE;
		    return TRUE;
		  }
		else
		  continue;	/* return to top of while loop */
	      }

	    if (x + hspacing >= PLOT_SIZE)
	      {
		x = 0;
		y -= vspacing;
		if (y < 0)
		  y = PLOT_HOME_Y;
	      }
	    move (x, y);
	    labelbuf[0] = (char)byte_read;
	    if (byte_read == '\\') /* escape backslashes */
	      {
		labelbuf[1] = (char)'\\';
		alabel ('l', 'x', labelbuf);
		labelbuf[1] = (char)'\0';
	      }
	    else
	      {
		alabel ('l', 'x', labelbuf);
	      }
		  
	    x += hspacing;
	    if (x >= PLOT_SIZE)
	      {
		x = xlomargin;
		y -= vspacing;	
		if (y < 0)		/* wrap */
		  {
		    y = PLOT_HOME_Y;
		    if (xlomargin)	/* toggle MARGIN1 <-> MARGIN2 */
		      xlomargin = 0;
		    else
		      xlomargin = MARGIN2_X;
		  }
	      }
	  }
	      
	return TRUE;		/* leave ALPHA mode, return to main loop */
	

      } /* end of ALPHA mode processing within parse_control_sequence() */
      
      /* characters \040 through \176 are printable ASCII chars
	 and should not be seen in main loop */
      
    case '\177':		/* DEL, ignore */
      return FALSE;
      
    default:
      fprintf (stderr, 
	       "%s: warning: unrecognized char 0x%x in vector mode/point mode/special point mode ignored\n",
	       progname, byte_read);
      return FALSE;
    } /* end of switch for processing control sequences */

  /* control should never get to this point, because we either return FALSE
     or return TRUE after processing each type of control sequence */
}

void
display_version ()
{
  fprintf (stderr, "%s (GNU plotutils) %s\n", progname, VERSION);
  fprintf (stderr, "Copyright (C) 1997 Free Software Foundation, Inc.\n");
  fprintf (stderr, 
	   "The GNU plotutils come with NO WARRANTY, to the extent permitted by law.\n");
  fprintf (stderr, "You may redistribute copies of the GNU plotutils\n");
  fprintf (stderr, "under the terms of the GNU General Public License.\n");
}

void
display_usage ()
{
  int i;
  int col = 0;
  
  fprintf (stderr, "Usage: %s", progname);
  col += (strlen (progname) + 7);
  for (i = 0; long_options[i].name; i++)
    {
      int option_len = strlen (long_options[i].name);

      if (col >= 80 - (option_len + 14))
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
      else
	{
	  fprintf (stderr, "]");
	  col++;
	}
    }
  fprintf (stderr, "\n");
}

Voidptr 
xmalloc (length)
     unsigned int length;
{
  Voidptr p;
  p = (Voidptr) malloc (length);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "%s: ", progname);
      perror ("malloc failed");
      exit (1);
    }
  return p;
}

/* special color linetypes for MS-DOS Kermit v2.31 tektronix emulator */
/*      0 = normal, 1 = bright
        foreground color (30-37) = 30 + colors
                where colors are   1=red, 2=green, 4=blue */
/*
static char *kermit_color[15]= {"\033[0;37m","\033[1;30m",
                "\033[0;32m","\033[0;36m","\033[0;31m","\033[0;35m",
                "\033[1;34m","\033[1;33m","\033[1;31m","\033[1;37m",
                "\033[1;35m","\033[1;32m","\033[1;36m","\033[0;34m",
                "\033[0;33m"};
*/
