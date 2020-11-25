/* This file is the driving routine for the GNU plot filters.  It includes
   code to read a stream of commands, in GNU-enhanced plot(5) format, and
   call libplot functions to draw the graphics.

   Copyright (C) 1989-1997 Free Software Foundation, Inc. */

#include "sys-defines.h"
#include "plot.h"
#include "getopt.h"

char *base_progname = "plot";
char *progname;			/* name of this program */

char *font_name = NULL;		/* inital fontname, can be spec'd by user */
char *pen_color = NULL;		/* initial pen color, can be spec'd by user */
double font_size = -1.0;	/* initial fractional plot size for text.
				   <0 means just use the default. */
double line_width = -1.0;	/* initial line width, <0 means default */

/* for listing of fonts, if this executable is linked with a version of
   libplot that contains the _ps_font_info[] array */
#ifdef LIST_FONTS
struct vector_font_info_struct 
{
  char *name;			/* font name */
  char *othername;		/* alternative font name */
  int chars[256];		/* array of vector glyphs */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  Boolean obliquing;		/* whether to apply obliquing */
  Boolean iso8859_1;		/* whether font encoding is iso8859-1 */
  Boolean visible;		/* whether font is visible, i.e. not internal */
};
extern struct vector_font_info_struct _vector_font_info[];

struct ps_font_info_struct 
{
  char *ps_name;		/* the postscript font name */
  char *x_name;			/* the X Windows font name */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int width[256];		/* the font width information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  int fig_id;			/* Fig's font id */
  Boolean used;			/* whether font has been used */
  Boolean iso8859_1;		/* whether font encoding is iso8859-1 */
};
extern struct ps_font_info_struct _ps_font_info[];
#endif

/* Long options we recognize */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] = 
{
  { "ascii-input",	ARG_NONE,	NULL, 'I' },
  { "ascii-output",	ARG_NONE,	NULL, 'O' },
  { "guess-byte-order",	ARG_NONE,	NULL, 'g' },
  { "high-byte-first",	ARG_NONE,	NULL, 'h' },
  { "low-byte-first",	ARG_NONE,	NULL, 'l' },
  { "font-name",	ARG_REQUIRED,	NULL, 'F' },
  { "font-size",	ARG_REQUIRED,	NULL, 'f' },
  { "line-width",	ARG_REQUIRED,	NULL, 'W' },
  /* Long options with no equivalent short option alias */
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help-fonts",	ARG_NONE,	NULL, 'f' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL,  0}
};
    

/* If true, input is in an ascii format.  All strings and coordinates are
   preceded by a space, and coordinates are in ascii rather than binary. */
Boolean ascii_input = FALSE;

/* Format for integers.  1 or -1 signify traditional two-byte format; 
   1 means high byte first, -1 means low byte first.  0 means use the system
   default, which may not even be a two-byte format. */
int input_high_byte_first = 0;

/* guess_byte_order is a flag which, if set, indicates that the function
   find_byte_order should be used to guess the byte order for the input
   file, if input_high_byte_first is non-zero. */
Boolean guess_byte_order = FALSE;

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
double fcoord P__((FILE *input));
int coord P__((FILE *input));
int swap_bytes P__((int x));
void display_usage P__((void)), display_version P__((void));
void display_fonts P__((void));
void find_byte_order P__((int *x0, int *y0, int *x1, int *y1, int *x2, int *y2));
void read_plot P__((FILE *in_stream, char *buffer, int *buffer_length));
void read_string P__((FILE *input, char *buffer, int *buffer_length));
Voidptr xmalloc P__ ((unsigned int length));
Voidptr xrealloc P__ ((Voidptr p, unsigned int length));
#undef P__

/* swap_bytes returns the bottom two bytes of its integer argument with
   their bytes reversed. */
int
swap_bytes (x)
     int x;
{
  unsigned char a, b;
  a = x & 0xff;
  b = (x >> 8) & 0xff;
  return ((a << 8)|b);
}

/* This is a set of known values for the maximum x (abscissa) values
   specified in the plot size command for known devices.  Using this set we
   can construct a heuristic proceedure for recognizing plot files in which
   the bytes of two byte integers are reversed.  We can recognize these
   files by looking for size commands containing these known sizes in byte
   reversed form. The last entry should be 0. */
int known_size[32] = 
{
  504,				/* plot3d output 504x504 */
  768,				/* 3 * 256 */
  2048,				/* Versatec plotter 2048x2048 */
  2100,				/* plot3d output */
  3120,				/* Tektronix 4010 terminal 3120x3120 */
  4096,				/* GSI 300 terminal 4096x4096 */
  /* same as preceding, but offset by unity */
  503,				/* plot3d output 504x504 */
  767,				/* 3 * 256 - 1 */
  2047,				/* Versatec plotter 2048x2048 */
  2099,				/* plot3d output */
  3119,				/* Tektronix 4010 terminal 3120x3120 */
  4095,				/* GSI 300 terminal 4096x4096 */
  0				/* the last entry should be 0 */
};

/* find_byte_order takes six integer arguments and matches the third one
   against a set of known values (sizes, see above). If there is a match it
   merely returns.  If there is no match, it check each of the values again
   with the bottom two bytes reversed. If such a match is found, the bottom
   two bytes of each argument are reversed, the input_high_byte_first flag
   (assumed to be +1 or -1) is flipped, to indicated how two-byte integers
   should be read, and the function returns. */
void
find_byte_order (x0, y0, x1, y1, x2, y2)
     int *x0, *y0, *x1, *y1, *x2, *y2;
{
  int i;
  for (i=0; known_size[i]!=0; i++)
    {
      if (*x1 == known_size[i])
	return;
    }
  /* now check to see if reversing the bytes allows a match... */
  for (i = 0 ; known_size[i] != 0; i++)
    {
      if (*x1 == swap_bytes (known_size[i]))
	{
	  *x0 = swap_bytes (*x0);
	  *y0 = swap_bytes (*y0);
	  *x1 = swap_bytes (*x1);
	  *y1 = swap_bytes (*y1);
	  *x2 = swap_bytes (*x2);
	  *y2 = swap_bytes (*y2);
	  input_high_byte_first = - input_high_byte_first;
	  return;
	}
    }

  /* finally, if none of this works, just assume that x1 > 128. */
  if ((*x1 < 128) && (swap_bytes (*x1) > 128))
    {
      *x0 = swap_bytes (*x0);
      *y0 = swap_bytes (*y0);
      *x1 = swap_bytes (*x1);
      *y1 = swap_bytes (*y1);
      *x2 = swap_bytes (*x2);
      *y2 = swap_bytes (*y2);
      input_high_byte_first = - input_high_byte_first;
      return;
    }
}

/* Read a byte */
#define read_byte(stream) (getc (stream))

/* Read a coordinate - an integer, perhaps in crufty old 2-byte format. */
int 
coord (input)
     FILE *input;
{
  int x;

  if (ascii_input)
    {
      fscanf (input, " %d", &x);
      return x;
    }

  if ((input_high_byte_first != 1) && (input_high_byte_first != -1))
    {
      short z;			/* keep this for a while, for compatibility */
      fread (&z, sizeof(short), 1, input);
      x = z;
    }
  else
    {
      if ( input_high_byte_first == 1)
	{
	  x = ((char) read_byte (input)) << 8; /* get sign from high byte */
	  x |= read_byte (input) & 0xFF; /* not from low byte */
	}
      else			/* -1 */
	{
	  x = read_byte (input) & 0xFF; /* ignore sign in low byte */
	  x |= ((char) read_byte (input)) << 8; /* get sign from high byte */
	}
    }
  return x;
}
  
double
fcoord (input)
     FILE *input;
{
  float f;
  
  if (ascii_input)
    fscanf (input, " %f", &f);
  else
    fread (&f, sizeof(float), 1, input);

  return (double)f;
}

/* Read a string, change termination to null.  Note: string (buffer) reads
   a newline terminated string. */
void
read_string (input, buffer, buffer_length)
     FILE *input;
     char *buffer;
     int *buffer_length;
{
  int length = 0;
  char termination = '\n';
  char c = '\0';

  while (!feof (input))
    {
      if (length > *buffer_length)
	{
	  *buffer_length *= 2;
	  buffer = (char *)xrealloc (buffer, (unsigned int)(*buffer_length));
	}
      c = read_byte (input);
      if (c == termination)
	break;
      buffer [length++] = c;
    }

  buffer [length] = '\0';	/*  null-terminate label */
  return;
}

/* read_plot reads a file in plot(5) format from a stream, and calls a plot
   function according to each instruction found in the file. */
void
read_plot (in_stream, buffer, buffer_length)
     FILE *in_stream;
     char *buffer;
     int *buffer_length;
{
  char x_adjust, y_adjust;
  int x0, y0, x1, y1, x2, y2;
  double d0, e0, d1, e1, d2, e2;
  int instruction;
  char *pattern;		/* for dot() */
  int i;
  
  instruction = read_byte (in_stream);
  while (!feof (in_stream))
    {
      switch (instruction)
	{
	  /*  Note: we must get all but the last argument before calling to
	      ensure reading them in the proper order. */
	  
	case ALABEL:
	  x_adjust = read_byte (in_stream);
	  y_adjust = read_byte (in_stream); 
	  read_string (in_stream, buffer, buffer_length);
	  alabel (x_adjust, y_adjust, buffer);
	  break;
	case ARC:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  x2 = coord (in_stream);
	  y2 = coord (in_stream); 
	  arc (x0, y0, x1, y1, x2, y2);
	  break;
	case ARCREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  x2 = coord (in_stream);
	  y2 = coord (in_stream); 
	  arcrel (x0, y0, x1, y1, x2, y2);
	  break;
	case BOX:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  box (x0, y0, x1, y1);
	  break;
	case BOXREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  boxrel (x0, y0, x1, y1);
	  break;
	case CAPMOD:
	  read_string (in_stream, buffer, buffer_length);
	  capmod (buffer);
	  break;
	case CIRCLE:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  circle (x0, y0, x1);
	  break;
	case CIRCLEREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  circlerel (x0, y0, x1);
	  break;
	case COLOR:		/* redundant op code, to be removed */
	  x0 = coord (in_stream)&0xFFFF;
	  y0 = coord (in_stream)&0xFFFF;
	  x1 = coord (in_stream)&0xFFFF;
	  color (x0, y0, x1);
	  break;
	case COMMENT:
	  read_string (in_stream, buffer, buffer_length);
	  break;
	case CONT:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  cont (x0, y0);
	  break;
	case CONTREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  contrel (x0, y0);
	  break;
	case DOT:
	  x0 = coord (in_stream)&0xFFFF;
	  y0 = coord (in_stream)&0xFFFF;
	  x1 = coord (in_stream)&0xFFFF;
	  y1 = coord (in_stream)&0xFFFF;
	  if (ascii_input)
	    read_byte (in_stream); /* discard SP */
	  pattern = (char *)xmalloc(y1 * sizeof(char));
	  for (i = 0; i < y1; i++)
	    *pattern++ = read_byte (in_stream);
	  dot (x0, y0, x1, y1, pattern);
	  free (pattern);
	  if (ascii_input)
	    read_byte (in_stream); /* discard CR */
	  break;
	case ELLARC:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream);
	  x2 = coord (in_stream);
	  y2 = coord (in_stream);
	  ellarc (x0, y0, x1, y1, x2, y2);	  
	  break;
	case ELLARCREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream);
	  x2 = coord (in_stream);
	  y2 = coord (in_stream);
	  ellarcrel (x0, y0, x1, y1, x2, y2);	  
	  break;
	case ELLIPSE:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream);
	  x2 = coord (in_stream);
	  ellipse (x0, y0, x1, y1, x2);
	  break;
	case ELLIPSEREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream);
	  x2 = coord (in_stream);
	  ellipserel (x0, y0, x1, y1, x2);
	  break;
	case ENDPOLY:
	  endpoly ();
	  break;
	case ERASE:
	  erase ();
	  break;
	case FILL:
	  fill (coord (in_stream)&0xFFFF); 
	  break;
	case FILLCOLOR:
	  x0 = coord (in_stream)&0xFFFF;
	  y0 = coord (in_stream)&0xFFFF;
	  x1 = coord (in_stream)&0xFFFF;
	  fillcolor (x0, y0, x1);
	  break;
	case FONTNAME:
	  read_string (in_stream, buffer, buffer_length);
	  fontname (buffer);
	  break;
	case FONTSIZE:
	  fontsize (coord (in_stream));
	  break;
	case JOINMOD:
	  read_string (in_stream, buffer, buffer_length);
	  joinmod (buffer);
	  break;
	case LABEL:
	  read_string (in_stream, buffer, buffer_length);
	  label (buffer);
	  break;
	case LINE:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  line (x0, y0, x1, y1);
	  break;
	case LINEREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  linerel (x0, y0, x1, y1);
	  break;
	case LINEMOD:
	  read_string (in_stream, buffer, buffer_length);
	  linemod (buffer);
	  break;
	case LINEWIDTH:
	  x0 = coord (in_stream);
	  linewidth (x0);
	  break;
	case MARKER:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream);
	  marker (x0, y0, x1, y1);
	  break;
	case MARKERREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream);
	  markerrel (x0, y0, x1, y1);
	  break;
	case MOVE:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  move (x0, y0);
	  break;
	case MOVEREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  moverel (x0, y0);
	  break;
	case PENCOLOR:
	  x0 = coord (in_stream)&0xFFFF;
	  y0 = coord (in_stream)&0xFFFF;
	  x1 = coord (in_stream)&0xFFFF;
	  pencolor (x0, y0, x1);
	  break;
	case POINT:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  point (x0, y0);
	  break;
	case POINTREL:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  pointrel (x0, y0);
	  break;
	case RESTORESTATE:
	  restorestate ();
	  break;
	case SAVESTATE:
	  savestate ();
	  break;
	case SPACE:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  x2 = y2 = 0;
	  if (guess_byte_order && input_high_byte_first)
	    find_byte_order (&x0, &y0, &x1, &y1, &x2, &y2);
	  space (x0, y0, x1, y1);
	  /* insert these after the call to space(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * abs (x1 - x0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * abs (x1 - x0));
	  break;
	case SPACE2:
	  x0 = coord (in_stream);
	  y0 = coord (in_stream);
	  x1 = coord (in_stream);
	  y1 = coord (in_stream); 
	  x2 = coord (in_stream);
	  y2 = coord (in_stream); 
	  if (guess_byte_order && input_high_byte_first)
	    find_byte_order (&x0, &y0, &x1, &y1, &x2, &y2);
	  space2 (x0, y0, x1, y1, x2, y2);
	  /* insert these after the call to space(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * abs (x1 - x0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * abs (x1 - x0));
	  break;
	case TEXTANGLE:
	  x0 = coord (in_stream);
	  textangle (x0);
	  break;

        /* floating point routines */
	case FARC:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  d2 = fcoord (in_stream);
	  e2 = fcoord (in_stream); 
	  farc (d0, e0, d1, e1, d2, e2);
	  break;
	case FARCREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  d2 = fcoord (in_stream);
	  e2 = fcoord (in_stream); 
	  farcrel (d0, e0, d1, e1, d2, e2);
	  break;
	case FBOX:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  fbox (d0, e0, d1, e1);
	  break;
	case FBOXREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  fboxrel (d0, e0, d1, e1);
	  break;
	case FCIRCLE:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  fcircle (d0, e0, d1);
	  break;
	case FCIRCLEREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  fcirclerel (d0, e0, d1);
	  break;
	case FCONT:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  fcont (d0, e0);
	  break;
	case FCONTREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  fcontrel (d0, e0);
	  break;
	case FELLARC:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream);
	  d2 = fcoord (in_stream);
	  e2 = fcoord (in_stream);
	  fellarc (d0, e0, d1, e1, d2, e2);
	  break;
	case FELLARCREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream);
	  d2 = fcoord (in_stream);
	  e2 = fcoord (in_stream);
	  fellarcrel (d0, e0, d1, e1, d2, e2);
	  break;
	case FELLIPSE:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream);
	  d2 = fcoord (in_stream);
	  fellipse (d0, e0, d1, e1, d2);
	  break;
	case FELLIPSEREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream);
	  d2 = fcoord (in_stream);
	  fellipserel (d0, e0, d1, e1, d2);
	  break;
	case FFONTSIZE:
	  ffontsize (fcoord (in_stream));
	  break;
	case FLINE:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  fline (d0, e0, d1, e1);
	  break;
	case FLINEREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  flinerel (d0, e0, d1, e1);
	  break;
	case FLINEWIDTH:
	  d0 = fcoord (in_stream);
	  flinewidth (d0);
	  break;
	case FMARKER:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  x0 = coord (in_stream);
	  e1 = fcoord (in_stream);
	  fmarker (d0, e0, x0, e1);
	  break;
	case FMARKERREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  x0 = coord (in_stream);
	  e1 = fcoord (in_stream);
	  fmarkerrel (d0, e0, x0, e1);
	  break;
	case FMOVE:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  fmove (d0, e0);
	  break;
	case FMOVEREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  fmoverel (d0, e0);
	  break;
	case FPOINT:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  fpoint (d0, e0);
	  break;
	case FPOINTREL:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  fpointrel (d0, e0);
	  break;
	case FSPACE:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  fspace (d0, e0, d1, e1);
	  /* insert these after the call to fspace(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * fabs (d1 - d0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * fabs (d1 - d0));
	  break;
	case FSPACE2:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  d2 = fcoord (in_stream);
	  e2 = fcoord (in_stream); 
	  fspace2 (d0, e0, d1, e1, d2, e2);
	  /* insert these after the call to fspace(), if user insists on
	     including them (should estimate sizes better) */
	  if (pen_color)
	    pencolorname (pen_color);
	  if (font_name)
	    fontname (font_name);
	  if (font_size >= 0.0)
	    ffontsize (font_size * fabs (d1 - d0));
	  if (line_width >= 0.0)
	    flinewidth (line_width * fabs (d1 - d0));
	  break;
	case FTEXTANGLE:
	  d0 = fcoord (in_stream);
	  ftextangle (d0);
	  break;

        /* floating point routines with no integer counterparts */
	case FCONCAT:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  d1 = fcoord (in_stream);
	  e1 = fcoord (in_stream); 
	  d2 = fcoord (in_stream);
	  e2 = fcoord (in_stream); 
	  fconcat (d0, e0, d1, e1, d2, e2);
	  break;
	case FROTATE:
	  d0 = fcoord (in_stream);
	  frotate (d0);
	  break;
	case FSCALE:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  fscale (d0, e0);
	  break;
	case FTRANSLATE:
	  d0 = fcoord (in_stream);
	  e0 = fcoord (in_stream);
	  ftranslate (d0, e0);
	  break;

        case '\n':
	  if (ascii_input)
	    break;
	default:
	  {
	    fprintf (stderr, "%s: unrecognized command (0x%x hexadecimal) seen in input, aborting\n", 
		     progname, instruction);
	    return;
	  }
	}
      instruction = read_byte (in_stream);
    }
  return;
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
  fprintf (stderr, 
	   "For information on available fonts, type `%s --help-fonts'.\n",
	   progname);
}

void
display_fonts ()
{
#ifdef LIST_FONTS
  int i;

  if (_libplot_have_vector_fonts)
    {
      fprintf (stderr, "Names of supported vector fonts (case-insensitive):\n");
      for (i=0; _vector_font_info[i].name; i++)
	if (_vector_font_info[i].visible)
	  fprintf (stderr, "\t%s\n", _vector_font_info[i].name);
    }

  if (_libplot_have_ps_fonts)
    {
      fprintf (stderr, "Names of supported Postscript fonts (case-insensitive):\n");
      for (i=0; _ps_font_info[i].ps_name; i++)
	fprintf (stderr, "\t%s\n", _ps_font_info[i].ps_name);
    }
#else
  fprintf (stderr, 
	   "%s: No font information available in this version\n", progname);
#endif
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


int
main (argc, argv)
     int argc;
     char *argv[];
{
  int option;			/* Option character */
  int opt_index;		/* Long option index */
  int errcnt = 0;		/* errors encountered */
  Boolean show_version = FALSE;	/* show version message? */
  Boolean show_usage = FALSE;	/* show usage message? */
  Boolean show_fonts = FALSE;	/* show a list of fonts? */
  FILE *out_stream = stdout;	/* stream for plot library output */
  int named_input = 0;		/* count named plot files on command line. */
  char *buffer;
  int buffer_length;
  double local_font_size, local_line_width;

  progname = xmalloc (strlen (base_progname) 
		      + IMAX(strlen (_libplot_suffix), 4) + 2);
  strcpy (progname, base_progname);
  strcat (progname, "2");
  if (*_libplot_suffix)
    strcat (progname, _libplot_suffix);
  else
    strcat (progname, "plot");

  buffer_length = 1024;
  buffer = (char *)xmalloc ((unsigned int)buffer_length);

  while ((option = getopt_long(argc, argv, "ghlIOF:f:W:", long_options, &opt_index)) != EOF)
    {
      if (option == 0)
	option = long_options[opt_index].val;
      
      switch (option) 
	{
	case 'I':		/* Ascii input	*/
	  ascii_input = TRUE;
	  break;
	case 'O':		/* Ascii output */
	  _libplot_output_is_ascii = 1; /* internal libplot variable */
	  break;
	case 'F':		/* set the initial font */
	  font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (font_name, optarg);
	  break;
	case 'C' << 8:		/* set the initial pen color */
	  pen_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (pen_color, optarg);
	  break;
	case 'f':		/* set the initial fontsize */
	  if (sscanf (optarg, "%lf", &local_font_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: initial font size must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_font_size < 0.0)
	    fprintf (stderr, "%s: warning: ignoring negative initial font size `%f'\n",
		     progname, local_font_size);
	  else if (local_font_size == 0.0)
	    fprintf (stderr, "%s: warning: ignoring zero initial font size\n",
		     progname);
	  else
	    font_size = local_font_size;
	  break;
	case 'W':		/* set the initial line width */
	  if (sscanf (optarg, "%lf", &local_line_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: initial line width must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_line_width < 0.0)
	    fprintf (stderr, "%s: warning: ignoring negative initial line width `%f'\n",
		     progname, local_line_width);
	  else
	    line_width = local_line_width;
	  break;
	case 'g':		/* try to guess the byte order */
	  guess_byte_order = TRUE;
	  break;
	case 'h':		/* read high byte first */
	  input_high_byte_first = 1;
	  break;
	case 'l':		/* read low byte first */
	  input_high_byte_first = -1;
	  break;
	case 'V' << 8:		/* Version */
	  show_version = TRUE;
	  break;
	case 'f' << 8:		/* Fonts */
	  show_fonts = TRUE;
	  break;
	case 'h' << 8:		/* Help */
	  show_usage = TRUE;
	  break;
	case 'M' << 8:		/* Max line length */
	  if (sscanf (optarg, "%d", 
		      &_libplot_max_unfilled_polyline_length) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: max line length must be an integer, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }

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
  if (show_fonts)
    {
      display_fonts ();
      return 0;
    }
  if (show_usage)
    {
      display_usage ();
      return 0;
    }

  outfile (out_stream);	/* set output stream for plot library */
  if (openpl () < 0)
    {
      fprintf (stderr, "%s: error: could not open plot device\n", progname);
      return 1;
    }

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
	  read_plot (data_file, buffer, &buffer_length);

	  if (data_file != stdin) /* Don't close stdin */
	    if (fclose (data_file) < 0)
	      {
		fprintf (stderr, "%s: error: couldn't close output file\n",
			 progname);
		return 1;
	      }
	}
    } /* endfor */

  if (!named_input)
    /* Read stdin if no files were named on the command line. */
    read_plot (stdin, buffer, &buffer_length);

  if (closepl () < 0)
    {
      fprintf (stderr, "%s: error: couldn't close plot device\n",
	       progname);
      return 1;
    }

  /* close output stream if not stdout */
  if (out_stream != stdout)
    if (fclose (out_stream) < 0)
      {
	fprintf (stderr, "%s: error: couldn't close output file\n",
		 progname);
	return 1;
      }

  return 0;
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

Voidptr 
xrealloc (p, length)
     Voidptr p;
     unsigned int length;
{
  p = (Voidptr) realloc (p, length);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "%s: ", progname);
      perror ("realloc failed");
      exit (1);
    }
  return p;
}

