/* This file is the driving routine for the GNU plot filters.  It includes
   code to read a stream of commands, in GNU metafile format, and call
   libplot functions to draw the graphics.

   Copyright (C) 1989-1998 Free Software Foundation, Inc. */

#include "sys-defines.h"
#include "plot.h"
#include "getopt.h"

/* input formats we recognize */
typedef enum 
{
  GNU_BINARY, GNU_PORTABLE, GNU_OLD_BINARY, GNU_OLD_PORTABLE, 
  PLOT5_HIGH, PLOT5_LOW
  /* We support two GNU metafile formats: binary and portable.  

     GNU_OLD_BINARY is the binary format used in pre-2.0 releases, with no
     initial comment line, short ints instead of ints, and no OPENPL or
     CLOSEPL.  GNU_OLD_PORTABLE also lacks the initial comment line and has
     no OPENPL or CLOSEPL.  But it should be parseable by the present
     filter if the obsolete, undocumented -I option is used.

     PLOT5_HIGH and PLOT5_LOW are the two distinct versions of Unix plot(5)
     format, which we also support.  They are requested by the -h and -l
     options respectively.  A user will probably not need to use those
     options explicitly, since if sizeof(short int)=2 then GNU_OLD_BINARY,
     which uses the system representation for short integers, subsumes
     plot(5) format. */
} plot_format;

const char *progname = "plot";	/* name of this program */

char *font_name = NULL;		/* initial font name, can be spec'd by user */
char *pen_color = NULL;		/* initial pen color, can be spec'd by user */
double font_size = -1.0;	/* initial fractional plot size for text.
				   <0 means just use the default. */
double line_width = -1.0;	/* initial line width, <0 means default */

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
  /* For backward compatibility (hidden option, will go away in 2.x) */
  { "portable-input",	ARG_REQUIRED,	NULL, 'I' },
  /* Long options with no equivalent short option alias */
  { "max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8 },
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8 },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "rotate",		ARG_REQUIRED,	NULL, 'Q' << 8 },
  /* Frequently used options dealing with input format */
  { "high-byte-first-input",	ARG_NONE,	NULL, 'h' },
  { "low-byte-first-input",	ARG_NONE,	NULL, 'l' },
  /* Options relevant only to plot2plot (refers to metafile output) */
  { "portable-output",	ARG_NONE,	NULL, 'O' },
  /* Documentation options */
  { "help-fonts",	ARG_NONE,	NULL, 'f' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL,  0}
};
    
/* null-terminated list of options that we don't show to the user */
int hidden_options[] = { (int)'I', 0 };

/* Default input format (see list of supported formats above).  For
   backward compatibility, don't change this.  We switch to the appropriate
   format by peeking at the first line of the input file. */
plot_format input_format = GNU_OLD_BINARY;

/* whether display device is open */
bool display_open = false;


/* forward references */
bool read_plot __P((FILE *in_stream));
char *read_string __P((FILE *input, bool *badstatus));
double read_float __P((FILE *input, bool *badstatus));
double read_int __P((FILE *input, bool *badstatus));
int read_true_int __P((FILE *input, bool *badstatus));
int swap_bytes __P((int x));
unsigned char read_byte __P((FILE *input, bool *badstatus));
/* from libcommon */
extern void display_fonts __P((const char *display_type, const char *progname));
extern void display_usage __P((const char *progname, const int *omit_vals, bool files, bool fonts));
extern void display_version __P((const char *progname)); 
extern Voidptr xcalloc __P ((unsigned int nmemb, unsigned int size));
extern Voidptr xmalloc __P ((unsigned int size));
extern Voidptr xrealloc __P ((Voidptr p, unsigned int length));
extern char *xstrdup __P ((const char *s));

/* swap_bytes returns the bottom two bytes of its integer argument with
   their bytes reversed. */
int
#ifdef _HAVE_PROTOS
swap_bytes (int x)
#else
swap_bytes (x)
     int x;
#endif
{
  unsigned char a, b;
  a = x & 0xff;
  b = (x >> 8) & 0xff;
  return ((a << 8)|b);
}

/* read a single byte from input stream */
unsigned char
#ifdef _HAVE_PROTOS
read_byte (FILE *input, bool *badstatus)
#else
read_byte (input, badstatus)
     FILE *input;
     bool *badstatus;
#endif
{
  int newint;

  if (*badstatus == true)
    return 0;

  newint = getc (input);
  if (newint == EOF)
    {
      *badstatus = true;
      return 0;
    }
  else
    return (unsigned char)newint;
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

  if (*badstatus == true)
    return 0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      returnval = fscanf (input, " %d", &x);
      if (returnval == 1)
	return x;
      else
	{
	  *badstatus = true;
	  return 0;
	}
      break;
    case GNU_BINARY:		/* system format for integers */
    default:
      returnval = fread (&zi, sizeof(int), 1, input);
      if (returnval == 1)
	x = (int)zi;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_OLD_BINARY:	/* system format for short integers */
      returnval = fread (&zs, sizeof(short), 1, input);
      if (returnval == 1)
	x = (int)zs;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case PLOT5_HIGH:		/* two-byte format, high byte first */
      x = (((int)read_byte (input, badstatus)) << 8);
      x |= ((int)read_byte (input, badstatus));
      break;
    case PLOT5_LOW:		/* two-byte format, low byte first */
      x = ((int)read_byte (input, badstatus));
      x |= (((int) read_byte (input, badstatus)) << 8);
      break;
    }

  return x;
}
  
/* a relaxed version of the preceding routine: if the portable
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

  if (*badstatus == true)
    return 0.0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      {
	double r;

	returnval = fscanf (input, " %lf", &r);
	if (returnval == 1)
	  return r;
	else
	  {
	    *badstatus = true;
	    return 0.0;
	  }
	break;
      }
    case GNU_BINARY:		/* system format for integers */
    default:
      returnval = fread (&zi, sizeof(int), 1, input);
      if (returnval == 1)
	x = (int)zi;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_OLD_BINARY:	/* system format for short integers */
      returnval = fread (&zs, sizeof(short), 1, input);
      if (returnval == 1)
	x = (int)zs;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case PLOT5_HIGH:		/* two-byte format, high byte first */
      x = (((int)read_byte (input, badstatus)) << 8);
      x |= ((int)read_byte (input, badstatus));
      break;
    case PLOT5_LOW:		/* two-byte format, low byte first */
      x = ((int)read_byte (input, badstatus));
      x |= (((int) read_byte (input, badstatus)) << 8);
      break;
    }

  return (double)x;
}
  
/* read a floating point quantity from input stream (can be in ascii format
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
      returnval = fread (&f, sizeof(float), 1, input);
      break;
    case PLOT5_HIGH:
    case PLOT5_LOW:
      /* plot(5) didn't support floats */
      fprintf (stderr, 
	       "%s: error: bogus floating-point argument in plot(5) format\n", 
	       progname);
      exit (1);
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
   string, with \0 replacing \n, is allocated on the heap and can be
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
      c = (char)read_byte (input, badstatus);
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

/* read_plot() reads a file in GNU-enhanced plot(5) format from a stream,
   and calls a plot function according to each instruction found in the
   file.  Return value indicates whether stream was parsed successfully. */

bool
#ifdef _HAVE_PROTOS
read_plot (FILE *in_stream)
#else
read_plot (in_stream)
     FILE *in_stream;
#endif
{
  double x0, y0, x1, y1, x2, y2;
  int i0, i1, i2;
  char *s;
  int instruction;
  bool unrec = false;	/* unrecognized command seen? */
  bool argerr = false;	/* error occurred while reading argument? */
  bool first_command = true;
  
  /* peek at first instruction */
  instruction = getc (in_stream);

  /* Switch to appropriate input format (a bit of a kludge).
     See also parsing of the COMMENT instruction below. */
  if (input_format == GNU_OLD_BINARY && instruction == COMMENT)
    input_format = GNU_BINARY;

  while (instruction != EOF)
    {
      /* If a pre-modern format, open display device if it hasn't already
	 been opened */
      if (input_format != GNU_BINARY && input_format != GNU_PORTABLE
	  && instruction != COMMENT && display_open == false)
	{
	  int openval;
	  
	  openval = openpl();
	  if (openval < 0)
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

	    x_adjust = (char)read_byte (in_stream, &argerr);
	    y_adjust = (char)read_byte (in_stream, &argerr); 
	    s = read_string (in_stream, &argerr);
	    if (!argerr)
	      {
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
	    farcrel (x0, y0, x1, y1, x2, y2);
	  break;
	case BOX:
	  	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    fbox (x0, y0, x1, y1);
	  break;
	case BOXREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    fboxrel (x0, y0, x1, y1);
	  break;
	case CAPMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      capmod (s);
	      free (s);
	    }
	  break;
	case CIRCLE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fcircle (x0, y0, x1);
	  break;
	case CIRCLEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fcirclerel (x0, y0, x1);
	  break;
	case COLOR:		/* redundant op code, to be removed */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    color (i0, i1, i2);
	  break;
	case CLOSEPL:
	  {
	    int closeval;
	    
	    closeval = closepl();
	    if (closeval < 0)
	      {
		fprintf (stderr, "%s: error: could not close plot device\n", 
			 progname);
		exit (1);
	      }
	    else
	      display_open = false;
	  }
	  break;
	case COMMENT:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
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
	    fcont (x0, y0);
	  break;
	case CONTREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
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
	    fellarcrel (x0, y0, x1, y1, x2, y2);	  
	  break;
	case ELLIPSE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fellipse (x0, y0, x1, y1, x2);
	  break;
	case ELLIPSEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fellipserel (x0, y0, x1, y1, x2);
	  break;
	case ENDPATH:
	  endpath ();
	  break;
	case ERASE:
	  erase ();
	  break;
	case FILLTYPE:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    filltype (i0);
	  break;
	case FILLCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    fillcolor (i0, i1, i2);
	  break;
	case FONTNAME:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      fontname (s);
	      free (s);
	    }
	  break;
	case FONTSIZE:
	  x0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    ffontsize (x0);
	  break;
	case JOINMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      joinmod (s);
	      free (s);
	    }
	  break;
	case LABEL:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
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
	    fline (x0, y0, x1, y1);
	  break;
	case LINEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    flinerel (x0, y0, x1, y1);
	  break;
	case LINEMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      linemod (s);
	      free (s);
	    }
	  break;
	case LINEWIDTH:
	  x0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    flinewidth (x0);
	  break;
	case MARKER:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fmarker (x0, y0, i0, y1);
	  break;
	case MARKERREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fmarkerrel (x0, y0, i0, y1);
	  break;
	case MOVE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fmove (x0, y0);
	  break;
	case MOVEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fmoverel (x0, y0);
	  break;
	case OPENPL:
	  {
	    int openval;
	    
	    openval = openpl();
	    if (openval < 0)
	      {
		fprintf (stderr, "%s: error: could not open plot device\n", 
			 progname);
		exit (1);
	      }
	    else
	      display_open = true;
	  }
	  break;
	case PENCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    pencolor (i0, i1, i2);
	  break;
	case POINT:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fpoint (x0, y0);
	  break;
	case POINTREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    fpointrel (x0, y0);
	  break;
	case RESTORESTATE:
	  restorestate ();
	  break;
	case SAVESTATE:
	  savestate ();
	  break;
	case SPACE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (argerr)
	    break;
	  fspace (x0, y0, x1, y1);
	  /* insert these after the call to space(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * fabs (x1 - x0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * fabs (x1 - x0));
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
	  fspace2 (x0, y0, x1, y1, x2, y2);
	  /* insert these after the call to space(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * fabs (x1 - x0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * fabs (x1 - x0));
	  break;
	case TEXTANGLE:
	  x0 = read_int (in_stream, &argerr);
	  if (!argerr)
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
	    farcrel (x0, y0, x1, y1, x2, y2);
	  break;
	case FBOX:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    fbox (x0, y0, x1, y1);
	  break;
	case FBOXREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    fboxrel (x0, y0, x1, y1);
	  break;
	case FCIRCLE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fcircle (x0, y0, x1);
	  break;
	case FCIRCLEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fcirclerel (x0, y0, x1);
	  break;
	case FCONT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fcont (x0, y0);
	  break;
	case FCONTREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
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
	    fellarcrel (x0, y0, x1, y1, x2, y2);
	  break;
	case FELLIPSE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fellipse (x0, y0, x1, y1, x2);
	  break;
	case FELLIPSEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fellipserel (x0, y0, x1, y1, x2);
	  break;
	case FFONTSIZE:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    ffontsize (x0);
	  break;
	case FLINE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    fline (x0, y0, x1, y1);
	  break;
	case FLINEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    flinerel (x0, y0, x1, y1);
	  break;
	case FLINEWIDTH:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    flinewidth (x0);
	  break;
	case FMARKER:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fmarker (x0, y0, i0, y1);
	  break;
	case FMARKERREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fmarkerrel (x0, y0, i0, y1);
	  break;
	case FMOVE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fmove (x0, y0);
	  break;
	case FMOVEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fmoverel (x0, y0);
	  break;
	case FPOINT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fpoint (x0, y0);
	  break;
	case FPOINTREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  fpointrel (x0, y0);
	  break;
	case FSPACE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    fspace (x0, y0, x1, y1);
	  /* insert these after the call to fspace(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * fabs (x1 - x0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * fabs (x1 - x0));
	  break;
	case FSPACE2:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    fspace2 (x0, y0, x1, y1, x2, y2);
	  /* insert these after the call to fspace(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * fabs (x1 - x0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * fabs (x1 - x0));
	  break;
	case FTEXTANGLE:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
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
	    fconcat (x0, y0, x1, y1, x2, y2);
	  break;
	case FROTATE:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    frotate (x0);
	  break;
	case FSCALE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    fscale (x0, y0);
	  break;
	case FTRANSLATE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    ftranslate (x0, y0);
	  break;
        case '\n':
	  if (input_format == GNU_PORTABLE) /* harmless, ignore */
	    break;
	  /* FALLTHRU if non-ascii */
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
    } /* end of while loop */

  return !(argerr || unrec);	/* was plot parsed successfully? */
}


int
#ifdef _HAVE_PROTOS
main (int argc, char *argv[])
#else
main (argc, argv)
     int argc;
     char *argv[];
#endif
{
  int option;			/* option character */
  int opt_index;		/* long option index */
  int errcnt = 0;		/* errors encountered */
  bool show_version = false;	/* show version message? */
  bool show_usage = false;	/* show usage message? */
  bool show_fonts = false;	/* show a list of fonts? */
  int retval;			/* return value */
  char *display_type = "meta";	/* default libplot output format */
  int handle;			/* libplot handle for Plotter object */

  while ((option = getopt_long(argc, argv, "hlIOF:f:W:T:", long_options, &opt_index)) != EOF)
    {
      if (option == 0)
	option = long_options[opt_index].val;
      
      switch (option) 
	{
	case 'T':		/* Display type, ARG REQUIRED      */
	  display_type = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (display_type, optarg);
	  break;
	case 'I':	/* Old portable Gnu metafile input [option obsolete]*/
	  input_format = GNU_OLD_PORTABLE;
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
	case 'h':		/* read high byte first */
	  input_format = PLOT5_HIGH;
	  break;
	case 'l':		/* read low byte first */
	  input_format = PLOT5_LOW;
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
  if (show_fonts)
    {
      display_fonts (display_type, progname);
      return 0;
    }
  if (show_usage)
    {
      display_usage (progname, hidden_options, true, true);
      return 0;
    }

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
		  fprintf (stderr, "%s: ignoring nonexistent or inaccessible file `%s'\n",
			   progname, argv[optind]);
		  retval = 1;
		  continue;	/* back to top of for loop */
		}
	    }
	  if (read_plot (data_file) == false)
	    {
		  fprintf (stderr, "%s: error: could not parse input file `%s'\n",
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
	  fprintf (stderr, "%s: error: could not parse input\n", progname);
	  retval = 1;
	}
    }

  /* close display device if it wasn't already closed */
  if (display_open)
    {
      if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
	/* premodern format, wouldn't be expected to include CLOSEPL */
	{
	  if (closepl () < 0)
	    {
	      
	      fprintf (stderr, "%s: error: could not close plot device\n",
		       
		       progname);
	      retval = 1;
	    }
	}
      else
	/* modern format, should have include a CLOSEPL */
	{
	  fprintf (stderr, 
		   "%s: error: input file(s) did not close plot device\n",
		   progname);
	  retval = 1;
	}
    }

  return retval;
}



