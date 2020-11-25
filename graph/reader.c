/* This file is the point-reader half of GNU graph.  Included here are
   routines that will read one or more points from an input stream.  The
   input stream may be in ascii format (a sequence of floating-point
   numbers, separated by whitespace), or in binary format (a sequence of
   doubles).  Gnuplot table format is also supported.

   A `point' is a structure.  Each point structure contains the following
   fields:

      x and y coordinates of the point
      a `have_x_errorbar' flag (true or false)
      a `have_y_errorbar' flag (true or false)
      xmin and xmax (meaningful only if have_x_errorbar is set)
      ymin and ymax (meaningful only if have_y_errorbar is set)
      a `pendown' flag

      a symbol type (a small integer, interpreted as a marker type)
      a symbol size (a fraction of the size of the plotting area)
      a symbol font name (relevant only for symbol types >= 32, see plotter.c)
      a linemode (a small integer)
      a linewidth (a fraction of the size of the display device)
      a polyline fill-fraction (in the interval [0,1], <0 means no fill)
      a use_color flag (true or false)

   A sequence of points defines a polyline, or a sequence of polylines.
   pendown=TRUE means that a polyline is being drawn; pendown=FALSE means
   that a polyline has just ended, and that the point (x,y), which begins a
   new polyline, should be moved to without drawing a line segment.  By
   convention, the final seven fields, and have_?_errorbar, are the same
   for each point in a polyline.  We use the term `dataset' to refer to the
   sequence of points in an input stream that gives rise to a single
   polyline.

   A dataset may be read from an input stream by calling read_dataset().
   The return value indicates whether the dataset ended with an explicit
   end-of-dataset separator, i.e., whether another dataset is expected to
   follow, or whether, instead, the dataset terminated because the end of
   the stream was reached.  If the input stream is in ascii format, two
   \n's in succession serves as a separator between datasets.  If, instead,
   the input stream is in binary format (a sequence of doubles), then a
   single MAXDOUBLE serves as a dataset separator.  If the input stream is
   in gnuplot `table' format, then two \n's in succession serves as a
   separator.  But there are always two \n's before EOF; this is different
   from ascii format.

   [A single point, rather than an entire dataset, may be read from an
   input stream by calling read_point().  See comments in the code in
   regard to its return value.]

   Before any points are read, the point-reader must first be initialized
   by a call to initialize_reader().  It sets various reader parameters;
   for example, the initial linemode and symbol type, and the format of the
   input stream.  reset_reader() should be called after completing the
   reading of each dataset.  Besides breaking the polyline, it increments
   the linemode by unity (if the reader's `autobump' flag is set).  Also,
   after finishing with an input stream, and before switching to another
   input stream as a source of points, set_reader_parameters() is normally
   called.  It updates the parameters of the reader that may differ from
   stream to stream.

   Note that directives in the input stream, specifying a change of
   linemode / symbol type, are supported.  Any such directive automatically
   terminates a dataset, and begins a new one.  This is in agreement with
   the convention that every point in a polyline have the same linemode,
   and the same plotting symbol.  During the reading of an ascii-format
   input stream, a string of the form "#m=%d,S=%d" will be interpreted as a
   directive to change to a specified linemode / symbol type.  Here the two
   %d's are the new linemode and symbol type, respectively.  There is
   currently no way of changing to a specific linemode / symbol type during
   the reading of a input stream that is in binary format. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* The reader structure defines the parameters and internal state of the
   point reader.  It is initialized by initialize_reader(). */

typedef struct
{
/* parameters that are constant for the duration of each plot, and which
   affect the computation of the returned point structures */
  Boolean transpose_axes;	/* x <-> y ? */
  int log_axis;			/* x,y axes are logarithmic? (portmanteau) */
/* parameters that are constant for the duration of each file  */
  data_type data_spec;		/* file format (T_ASCII, T_DOUBLE, etc.) */
  Boolean auto_abscissa;	/* auto-generate x values?  */
  double delta_x;		/* increment for x value, if auto-generated */
  double initial_abscissa;	/* initial value for x, if auto-generated */
  Boolean auto_bump;		/* bump linemode when starting next polyline?*/
/* parameters that are constant for the duration of each dataset  */
  int symbol;			/* symbol type */
  double symbol_size;		/* symbol size (in `box coordinates') */
  char *symbol_font_name;	/* font used for marker types >= 32 */
  int linemode;			/* linemode */
  double line_width;		/* line width (as frac. of display size) */
  double fill_fraction;		/* in interval [0,1], <0 means no filling */
  Boolean use_color;		/* color/monochrome interp. of linemode */
/* state variables, updated during reader operation */
  double abscissa;		/* x value, if auto-generated */
  Boolean first_plot_point;     /* true for first point of dataset only */
  Boolean need_break;		/* draw next point with pen up ? */
}
Reader;				/* internal state of a point reader */

static Reader reader;		/* our point reader */

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
static Boolean skip_whitespace P__ ((FILE *stream));
static int read_and_plot_dataset P__((FILE *input));
static int read_dataset P__((FILE *input, Point **p, int *length, int *no_of_points));
static int read_point P__ ((FILE *input, Point *point));
static int read_point_ascii P__ ((FILE *input, Point *point));
static int read_point_ascii_errorbar P__ ((FILE *input, Point *point));
static int read_point_double P__ ((FILE *input, Point *point));
static int read_point_gnuplot P__ ((FILE *input, Point *point));
#undef P__

/* initialize_reader() is called before reading data points from each file */

void 
initialize_reader (data_spec, auto_abscissa, delta_x, abscissa,
		   transpose_axes, log_axis, auto_bump,
		   symbol, symbol_size, symbol_font_name,
		   linemode, line_width, fill_fraction, use_color)
     data_type data_spec;	/* double, or ascii, etc. */
     Boolean auto_abscissa;
     double delta_x;
     double abscissa;
     Boolean transpose_axes;
     int log_axis;
     Boolean auto_bump;
     int symbol;
     double symbol_size;	/* for markers */
     char *symbol_font_name;	/* for markers >= 32 */
     int linemode;
     double line_width;		/* as fraction of display size */
     double fill_fraction;	/* in [0,1], <0 means no fill (transparent) */
     Boolean use_color;
{
  reader.first_plot_point = TRUE;
  reader.need_break = TRUE;	/* force break in polyline */

  reader.data_spec = data_spec;
  reader.auto_abscissa = auto_abscissa;
  reader.delta_x = delta_x;
  reader.initial_abscissa = abscissa;
  reader.abscissa = reader.initial_abscissa;
  reader.transpose_axes = transpose_axes;
  reader.log_axis = log_axis;
  reader.auto_bump = auto_bump;
  reader.symbol = symbol;
  reader.symbol_size = symbol_size;
  reader.symbol_font_name = symbol_font_name;
  reader.linemode = linemode;
  reader.line_width = line_width;
  reader.fill_fraction = fill_fraction;
  reader.use_color = use_color;

  return;
}

/* reset_reader() is called after reading each dataset from a file.  A new
   polyline will be begun, the linemode will be bumped if auto-bumping is
   in effect, and the abscissa will be reset if auto-abscissa is in
   effect. */

void
reset_reader()
{
  reader.need_break = TRUE;	/* force break in polyline */

  /* bump linemode if appropriate */
  if (reader.auto_bump)
    reader.linemode += ((reader.linemode > 0) ? 1 : -1);

  /* reset abscissa if auto-abscissa is in effect */
  if (reader.auto_abscissa)
    reader.abscissa = reader.initial_abscissa;

  return;
}

/* set_reader_parameters() is called just before reading datapoints from
   the second file, the third file,...  It sets the file format, resets the
   abscissa (if auto-abscissa is in effect), and updates the linemode,
   symbol type, etc., if requested.  (We use the last feature to permit
   command-line specification of linemode/symbol type on a per-file basis.)  */

void 
set_reader_parameters (data_spec, auto_abscissa, delta_x, abscissa,
		       symbol, symbol_size, symbol_font_name,
		       linemode, line_width, fill_fraction, use_color,
		       new_symbol, new_symbol_size, new_symbol_font_name,
		       new_linemode, new_line_width, new_fill_fraction, 
		       new_use_color)
     data_type data_spec;
     Boolean auto_abscissa;
     double delta_x;
     double abscissa;
     int symbol;
     double symbol_size;
     char *symbol_font_name;
     int linemode;
     double line_width;
     double fill_fraction;
     Boolean use_color;
     /* following booleans make up a mask */
     Boolean new_symbol, new_symbol_size, new_symbol_font_name;
     Boolean new_linemode, new_line_width, new_fill_fraction, new_use_color;
{
  reader.need_break = TRUE;	/* force break in polyline */
  reader.data_spec = data_spec;
  reader.auto_abscissa = auto_abscissa;
  reader.delta_x = delta_x;
  reader.initial_abscissa = abscissa;
  reader.abscissa = reader.initial_abscissa;
  /* test bits in mask to determine which polyline attributes need updating */
  if (new_symbol)
    reader.symbol = symbol;
  if (new_symbol_size)
    reader.symbol_size = symbol_size;
  if (new_symbol_font_name)
    reader.symbol_font_name = symbol_font_name;
  if (new_linemode)
    reader.linemode = linemode;
  if (new_line_width)
    reader.line_width = line_width;
  if (new_fill_fraction)
    reader.fill_fraction = fill_fraction;
  if (new_use_color)
    reader.use_color = use_color;

  return;
}

/* read_point() attempts to read a data point from a file (auto-abscissa is
   supported).  Return value is 0 if a data point was read, and 1 if no
   data point could be read due to EOF or garbage in file.  A return value
   of 2 indicates that no point was read, but an explicit end-of-dataset
   indicator was seen in the input stream.  For an ascii stream this is two
   newlines in succession; for a double stream this is a MAXDOUBLE.  A
   return value of 3 is special: it indicates that no point was read, but a
   `set linemode / symbol type' directive was seen in the input stream.  By
   convention we break any polyline under construction when such a
   directive is seen.  A return value of 4 signifies a `soft EOF' (we
   already returned an end-of-dataset, and this EOF shouldn't be
   interpreted as terminating an additional [null] dataset).

   This routine calls read_point_ascii(), read_point_ascii_errorbar(),
   read_point_double(), or read_point_gnuplot() to do the actual reading.
*/

static int
read_point (input, point)
     FILE *input;
     Point *point;
{
  Boolean success;

  /* following fields are constant throughout each polyline */
  point->symbol = reader.symbol;
  point->symbol_size = reader.symbol_size;
  point->symbol_font_name = reader.symbol_font_name;
  point->linemode = reader.linemode;
  point->line_width = reader.line_width;
  point->fill_fraction = reader.fill_fraction;
  point->use_color = reader.use_color;
  point->have_x_errorbar = FALSE; /* not supported yet */
  point->have_y_errorbar = (reader.data_spec == T_ASCII_ERRORBAR);
  
 head:

  switch (reader.data_spec)
    {
    case T_ASCII:
    default:
      success = read_point_ascii (input, point);
      break;
    case T_DOUBLE:
      success = read_point_double (input, point);
      break;
    case T_ASCII_ERRORBAR:
      success = read_point_ascii_errorbar (input, point);
      break;
    case T_GNUPLOT:		/* gnuplot `table' format */
      success = read_point_gnuplot (input, point);
      break;
    }

  if (success == 0)		/* got a point */
    {
      Boolean bad_point = FALSE;

      /* If we have log axes, the values we work with ALL refer to the log10
	 values of the data.  A nonpositive value generates a warning, and a
	 break in the polyline. */
      if (reader.log_axis & X_AXIS)
	{
	  if (point->x > 0.0)
	    point->x = log10 (point->x);
	  else
	    bad_point = TRUE;
	  if (point->have_x_errorbar)
	    {
	      if (point->xmin > 0.0)
		point->xmin = log10 (point->xmin);
	      else
		bad_point = TRUE;
	      if (point->xmax > 0.0)
		point->xmax = log10 (point->xmax);
	      else
		bad_point = TRUE;
	    }
	  
	  if (bad_point)
	    {
	      fprintf (stderr, "%s: warning: log plot, dropping point (%g,%g)\n",
		       progname, point->x, point->y);
	      reader.need_break = TRUE;
	      goto head;		/* on to next point */
	    }
	}
      if (reader.log_axis & Y_AXIS)
	{
	  if (point->y > 0.0)
	    point->y = log10 (point->y);
	  else
	    bad_point = TRUE;

	  if (point->have_y_errorbar)
	    {
	      if (point->ymin > 0.0)
		point->ymin = log10 (point->ymin);
	      else
		bad_point = TRUE;
	      if (point->ymax > 0.0)
		point->ymax = log10 (point->ymax);
	      else
		bad_point = TRUE;
	    }
	  
	  if (bad_point)
	    {
	      fprintf (stderr, "%s: warning: log plot, dropping point (%g,%g)\n",
		       progname, point->x, point->y);
	      reader.need_break = TRUE;
	      goto head;		/* on to next point */
	    }
	}
      
      if (reader.transpose_axes)
	{
	  double tmp;
	  Boolean tmp_bool;
	  
	  tmp = point->x;
	  point->x = point->y;
	  point->y = tmp;
	  tmp = point->xmin;
	  point->xmin = point->ymin;
	  point->ymin = tmp;
	  tmp = point->xmax;
	  point->xmax = point->ymax;
	  point->ymax = tmp;
	  tmp_bool = point->have_x_errorbar;
	  point->have_x_errorbar = point->have_y_errorbar;
	  point->have_y_errorbar = tmp_bool;
	}
      
      /* we have a point, but we may need to break the polyline before it */
      if (reader.need_break || reader.first_plot_point)
	point->pendown = FALSE;
      else
	point->pendown = TRUE;
      
      /* reset break-polyline flag */
      reader.need_break = FALSE;
      
      /* reader is no longer virgin */
      reader.first_plot_point = FALSE;
    }

  return success;		/* could be 0, 1, 2, 3, or 4 */
}

static int
read_point_ascii (input, point)
     FILE *input;
     Point *point;
{
  int items_read, lookahead;
  Boolean two_newlines;

 head:

  /* skip whitespace, up to but not including 2nd newline if any */
  two_newlines = skip_whitespace (input);
  if (two_newlines)
    return 2;			/* end of dataset */

  lookahead = getc (input);
  if (lookahead == EOF)
    return 1;			/* EOF */
  ungetc (lookahead, input);

  /* process linemode / symbol type directive */
  if (lookahead == (int)'#')
    {
      int new_symbol, new_linemode;
      int items_read;
      
      items_read = fscanf (input, 
			   "# m = %d, S = %d", &new_linemode, &new_symbol);
      if (items_read == 2)	/* insist on matching both */
	{
	  reader.linemode = new_linemode;
	  reader.symbol = new_symbol;
	  return 3;		/* `explicit directive' return value */
	}
      else			/* unknown comment line, ignore it */
	{
	  char c;
	  
	  do 
	    {
	      items_read = fread (&c, sizeof (c), 1, input);
	      if (items_read <= 0)
		return 1;	/* EOF */
	    }
	  while (c != '\n');
	  ungetc ((int)'\n', input); /* push back \n at the end of # line */
	  goto head;
	}
    }

  /* read coordinate(s) */
  if (reader.auto_abscissa)
    {
      point->x = reader.abscissa;
      reader.abscissa += reader.delta_x;
    }
  else
    {
      items_read = fscanf (input, "%lf", &(point->x));
      if (items_read != 1)
	return 1;		/* presumably EOF */
    }

  items_read = fscanf (input, "%lf", &(point->y));
  if (items_read == 1)
    return 0;			/* got a pair of floats */
  else 
    {
      if (!reader.auto_abscissa)
	fprintf (stderr, "%s: warning: truncated ascii-format input file\n", progname);
      return 1;			/* couldn't get y coor, effectively EOF */
    }
}

static int
read_point_ascii_errorbar (input, point)
     FILE *input;
     Point *point;
{
  int items_read, lookahead;
  Boolean two_newlines;
  double error_size;

 head:

  /* skip whitespace, up to but not including 2nd newline if any */
  two_newlines = skip_whitespace (input);
  if (two_newlines)
    return 2;			/* end of dataset */

  lookahead = getc (input);
  if (lookahead == EOF)
    return 1;			/* EOF */
  ungetc (lookahead, input);

  /* process linemode / symbol type directive */
  if (lookahead == (int)'#')
    {
      int new_symbol, new_linemode;
      int items_read;
      
      items_read = fscanf (input, 
			   "# m = %d, S = %d", &new_linemode, &new_symbol);
      if (items_read == 2)	/* insist on matching both */
	{
	  reader.linemode = new_linemode;
	  reader.symbol = new_symbol;
	  return 3;		/* `explicit directive' return value */
	}
      else			/* unknown comment line, ignore it */
	{
	  char c;
	  
	  do 
	    {
	      items_read = fread (&c, sizeof (c), 1, input);
	      if (items_read <= 0)
		return 1;	/* EOF */
	    }
	  while (c != '\n');
	  ungetc ((int)'\n', input); /* push back \n at the end of # line */
	  goto head;
	}
    }

  /* read coordinate(s) */
  if (reader.auto_abscissa)
    {
      point->x = reader.abscissa;
      reader.abscissa += reader.delta_x;
    }
  else
    {
      items_read = fscanf (input, "%lf", &(point->x));
      if (items_read != 1)
	return 1;		/* presumably EOF */
    }

  items_read = fscanf (input, "%lf", &(point->y));
  if (items_read != 1)
    {
      if (!reader.auto_abscissa)
	fprintf (stderr, "%s: warning: truncated errorbar-format input file\n", progname);
      return 1;			/* couldn't get y coor, effectively EOF */
    }

  items_read = fscanf (input, "%lf", &error_size);
  if (items_read != 1)
    {
      fprintf (stderr, "%s: warning: truncated errorbar-format input file\n", progname);
      return 1;			/* couldn't get y coor, effectively EOF */
    }

  point->ymin = point->y - error_size;
  point->ymax = point->y + error_size;

  /* don't support reading of x errorbars yet */
  point->xmin = 0.0;
  point->xmax = 0.0;

  return 0;
}

static int
read_point_double (input, point)
     FILE *input;
     Point *point;
{
  int items_read;
  
  /* read coordinate(s) */
  if (reader.auto_abscissa)
    {
      point->x = reader.abscissa;
      reader.abscissa += reader.delta_x;
    }
  else
    {
      items_read = fread ((Voidptr) &(point->x), sizeof (point->x), 1, input);
      if (items_read <= 0)
	return 1;		/* presumably EOF */
    }

  if (point->x == MAXDOUBLE)
    return 2;			/* end of dataset */

  items_read = fread ((Voidptr) &(point->y), sizeof (point->y), 1, input);
  if (items_read == 1)
    return 0;			/* got a pair of floats */
  else 
    {
      if (!reader.auto_abscissa)
	fprintf (stderr, "%s: warning: truncated double-format input file\n", progname);
      return 1;			/* effectively EOF */
    }
}

static int
read_point_gnuplot (input, point)
     FILE *input;
     Point *point;
{
  int lookahead, items_read;
  char directive, c;
  Boolean two_newlines;
  
 head:
  
  /* skip whitespace, up to but not including 2nd newline */
  two_newlines = skip_whitespace (input);
  if (two_newlines)
    /* end-of-dataset indicator */
    return 2;

  lookahead = getc (input);
  if (lookahead == EOF)
    return 4;			/* `soft' EOF (won't bump linemode) */
  ungetc (lookahead, input);
  switch (lookahead)
    {
    case 'C':			/* `Curve' line, discard it */
      do 
	{
	  items_read = fread (&c, sizeof (c), 1, input);
	  if (items_read <= 0)
	    return 1;		/* effectively EOF */
	}
      while (c != '\n');
      ungetc ((int)'\n', input); /* push back \n at the end of C line */
      goto head;
      break;

    case 'i':			/* read coordinates */
    case 'o':
      items_read = fscanf (input, 
			   "%c x=%lf y=%lf", 
			   &directive, &point->x, &point->y);
      if (items_read == 3)	/* must match all */
	return 0;		/* got a pair of floats */
      else
	{
	  fprintf (stderr, 
		   "%s: warning: truncated gnuplot-format input file\n", 
		   progname);
	  return 1;		/* effectively EOF */
	}

    case 'u':	
      /* `undefined', next point begins new polyline (same line mode) */
      do 
	{
	  items_read = fread (&c, sizeof (c), 1, input);
	  if (items_read <= 0)
	    {
	      fprintf (stderr, 
		       "%s: warning: truncated gnuplot-format input file\n", 
		       progname);
	    return 1;		/* effectively EOF */
	    }
	}
      while (c != '\n');

      /* break the polyline here in a soft way (don't start a new dataset) */
      reader.need_break = TRUE;	
      goto head;
    default:
      fprintf (stderr, 
	       "%s: warning: unknown gnuplot directive `%c' in input file \n", 
	       progname, (char)lookahead);
      return 1;			/* effectively EOF */
    }
}


/* read_dataset() reads an entire dataset (a sequence of points) from an
   input stream, and stores it.  The length of the block in which the
   points are stored, and the number of points, are passed back.  

   Return value = 1 means the dataset terminated with an EOF, and return
   value = 2 means the dataset terminated with an explicit end-of-dataset
   marker.  An end-of-dataset marker is two newlines in succession for an
   ascii stream, and a MAXDOUBLE for a stream of doubles.  Return value 3
   is special: it signals that a `set linemode / symbol type' directive was
   seen in the input stream.  By convention, we interpret such a directive
   as ending a dataset.  Return value 4 signifies a `soft EOF' (we already
   returned an end-of-dataset, and this EOF shouldn't be interpreted as
   terminating an additional [null] dataset).  The distinction between a
   hard EOF and a soft EOF is important if we are automatically bumping the
   line mode between datasets. */

static int
read_dataset (input, p_addr, length, no_of_points)
     FILE *input;
     Point **p_addr;
     int *length;  /* buffer length in bytes, should begin > 0 */
     int *no_of_points;
{
  Point *p = *p_addr;
  int success;

  while (TRUE)
    {
      /*
       * Grow the buffer if needed
       */
      if (*no_of_points >= *length)
	{
	  *length *= 2;
	  p = (Point *)xrealloc (p, *length * sizeof (Point));
	}

      success = read_point (input, &(p[*no_of_points]));
      if (success != 0)
	break;			/* success != 0 means we didn't get a point */

      (*no_of_points)++;
    }

  *p_addr = p;			/* update beginning of array if needed */

  return success;		/* return value = 1, 2, 3, or 4 */
}

/* read_file() reads all possible datasets from an input stream, and stores
   them.  The length of the block in which the data points are stored, and
   the number of points, are passed back.  */

void
read_file (input, p_addr, length, no_of_points)
     FILE *input;
     Point **p_addr;
     int *length;  /* buffer length in bytes, should begin > 0 */
     int *no_of_points;
{
  int success;
  Boolean saved_auto_bump;

  do
    {
      success = read_dataset (input, p_addr, length, no_of_points);

      /* After each dataset, reset reader: force break in polyline, bump
	 linemode (if auto-bump is in effect), and reset abscissa (if
	 auto-abscissa is in effect).  If dataset ended with an explicit
	 set linemode / symbol style directive, don't bump the linemode. */
      if (success == 3)
	{
	  saved_auto_bump = reader.auto_bump;
	  reader.auto_bump = FALSE;
	  reset_reader();
	  reader.auto_bump = saved_auto_bump;
	}
      else if (success != 4)	/* don't reset on `soft EOF', already did so */
	reset_reader();
    }
  while (success != 1 && success != 4);	/* keep going until EOF seen */
}


/* read_and_plot_dataset() reads an entire dataset (a sequence of points)
   from an input stream; it calls plot_point() on each point as it is read.
   So plotting is accomplished in real time.  

   Return value = 1 means the dataset terminated with an EOF, and return
   value = 2 means the dataset terminated with an explicit end-of-dataset
   marker.  An end-of-dataset marker is two newlines in succession for an
   ascii stream, and a MAXDOUBLE for a stream of doubles.  Return value 3
   is special: it signals that a `set linemode / symbol type' directive was
   seen in the input stream.  By convention, we interpret such a directive
   as ending a dataset.  Return value 4 signifies a `soft EOF' (we already
   returned an end-of-dataset, and this EOF shouldn't be interpreted as
   terminating an additional [null] dataset). */

static int
read_and_plot_dataset (input)
     FILE *input;
{
  int success;

  while (TRUE)
    {
      Point point;

      success = read_point (input, &point);
      if (success != 0)
	break;			/* success != 0 means we didn't get a point */

      plot_point (&point);
    }
  
  return success;		/* return value = 1, 2, 3, or 4 */
}

/* read_and_plot_file() reads a sequence of datasets from a stream, and
   plots them as they are read. */

void
read_and_plot_file (input)
     FILE *input;
{
  int success;
  Boolean saved_auto_bump;

  do
    {
      success = read_and_plot_dataset (input);

      /* After each dataset, reset reader: force break in polyline, bump
	 linemode (if auto-bump is in effect), and reset abscissa (if
	 auto-abscissa is in effect).  If dataset ended with an explicit
	 set linemode / symbol style directive, don't bump the linemode. */
      if (success == 3)
	{
	  saved_auto_bump = reader.auto_bump;
	  reader.auto_bump = FALSE;
	  reset_reader();
	  reader.auto_bump = saved_auto_bump;
	}
      else if (success != 4)	/* don't reset on `soft EOF', already did so */
	reset_reader();

      /* after each dataset, flush the constructed polyline to the display
         device */
      if (success != 4)
	{
	  endpoly();
	  flushpl();
	}
    }
  while (success != 1 && success != 4);	/* keep going until EOF seen */
}


/* skip_whitespace() skips whitespace in an ascii-format input stream,
   up to but not including a second newline.  Return value indicates
   whether or not two newlines were in fact seen.  (For ascii-format
   input streams, two newlines signals an end-of-dataset.) */

static Boolean
skip_whitespace (stream)
     FILE *stream;
{
  int lookahead;
  int nlcount = 0;
  
  do 
    {
      lookahead = getc (stream);
      if (lookahead == (int)'\n')
	  nlcount++;
    }
  while (lookahead != EOF 
	 && isspace((unsigned char)lookahead)
	 && nlcount < 2);

  if (lookahead == EOF)
    return FALSE;
  
  ungetc (lookahead, stream);
  return (nlcount == 2);
}
