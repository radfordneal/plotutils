/* This file is the point-reader half of GNU graph.  Included here are
   routines that will read one or more points from an input file.  The
   input file may be in ascii format (a sequence of floating-point
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
   pendown=true means that a polyline is being drawn; pendown=false means
   that a polyline has just ended, and that the point (x,y), which begins a
   new polyline, should be moved to without drawing a line segment.  By
   convention, the final seven fields, and have_?_errorbar, are the same
   for each point in a polyline.  We use the term `dataset' to refer to the
   sequence of points in an input file that gives rise to a single
   polyline.

   A dataset may be read from an input file by calling read_dataset().
   The return value indicates whether the dataset ended with an explicit
   end-of-dataset separator, i.e., whether another dataset is expected to
   follow, or whether, instead, the dataset terminated because the end of
   the stream was reached.  If the input file is in ascii format, two
   \n's in succession serves as a separator between datasets.  If, instead,
   the input file is in binary format (a sequence of doubles), then a
   single DBL_MAX serves as a dataset separator.  If the input file is
   in gnuplot `table' format, then two \n's in succession serves as a
   separator.  But there are always two \n's before EOF; this is different
   from ascii format.

   [A single point, rather than an entire dataset, may be read from an
   input file by calling read_point().  See comments in the code in
   regard to its return value.]

   Before any points are read, the point-reader must first be initialized
   by a call to initialize_reader().  It sets various reader parameters;
   for example, the initial linemode and symbol type, and the format of the
   input file.  reset_reader() should be called after completing the
   reading of each dataset.  Besides breaking the polyline, it increments
   the linemode by unity (if the reader's `autobump' flag is set).  Also,
   after finishing with an input file, and before switching to another
   input file as a source of points, set_reader_parameters() is normally
   called.  It updates the parameters of the reader that may differ from
   stream to stream.

   Note that directives in the input file, specifying a change of
   linemode / symbol type, are supported.  Any such directive automatically
   terminates a dataset, and begins a new one.  This is in agreement with
   the convention that every point in a polyline have the same linemode,
   and the same plotting symbol.  During the reading of an ascii-format
   input file, a string of the form "#m=%d,S=%d" will be interpreted as a
   directive to change to a specified linemode / symbol type.  Here the two
   %d's are the new linemode and symbol type, respectively.  There is
   currently no way of changing to a specific linemode / symbol type during
   the reading of a input file that is in binary format. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* The reader structure defines the parameters and internal state of the
   point reader.  It is initialized by initialize_reader(). */

typedef struct
{
/* parameters that are constant for the duration of each plot, and which
   affect the computation of the returned point structures */
  bool transpose_axes;	/* x <-> y ? */
  int log_axis;			/* x,y axes are logarithmic? (portmanteau) */
/* parameters that are constant for the duration of each file  */
  data_type input_type;		/* file format (T_ASCII, T_DOUBLE, etc.) */
  bool auto_abscissa;		/* auto-generate x values?  */
  double delta_x;		/* increment for x value, if auto-generated */
  double initial_abscissa;	/* initial value for x, if auto-generated */
  bool auto_bump;		/* bump linemode when starting next polyline?*/
/* parameters that are constant for the duration of each dataset  */
  int symbol;			/* symbol type */
  double symbol_size;		/* symbol size (in `box coordinates') */
  char *symbol_font_name;	/* font used for marker types >= 32 */
  int linemode;			/* linemode */
  double line_width;		/* line width (as frac. of display size) */
  double fill_fraction;		/* in interval [0,1], <0 means no filling */
  bool use_color;		/* color/monochrome interp. of linemode */
/* state variables, updated during reader operation */
  double abscissa;		/* x value, if auto-generated */
  bool first_plot_point;	/* true for first point of dataset only */
  bool need_break;		/* draw next point with pen up ? */
}
Reader;				/* internal state of a point reader */

static Reader reader;		/* our point reader */

/* forward references */
static bool skip_whitespace __P ((FILE *stream));
static int read_and_plot_dataset __P((FILE *input));
static int read_dataset __P((FILE *input, Point **p, int *length, int *no_of_points));
static int read_point __P ((FILE *input, Point *point));
static int read_point_ascii __P ((FILE *input, Point *point));
static int read_point_ascii_errorbar __P ((FILE *input, Point *point));
static int read_point_binary __P ((FILE *input, Point *point, data_type input_type));
static int read_point_gnuplot __P ((FILE *input, Point *point));

/* initialize_reader() is called before reading data points from each file */

void 
#ifdef _HAVE_PROTOS
initialize_reader (data_type input_type, bool auto_abscissa, 
		   double delta_x, double abscissa, bool transpose_axes,
		   int log_axis, bool auto_bump, int symbol, 
		   double symbol_size, char *symbol_font_name, int linemode,
		   double line_width, double fill_fraction, 
		   bool use_color)
#else
initialize_reader (input_type, auto_abscissa, delta_x, abscissa,
		   transpose_axes, log_axis, auto_bump,
		   symbol, symbol_size, symbol_font_name,
		   linemode, line_width, fill_fraction, use_color)
     data_type input_type;	/* double, or ascii, etc. */
     bool auto_abscissa;
     double delta_x;
     double abscissa;
     bool transpose_axes;
     int log_axis;
     bool auto_bump;
     int symbol;
     double symbol_size;	/* for markers */
     char *symbol_font_name;	/* for markers >= 32 */
     int linemode;
     double line_width;		/* as fraction of display size */
     double fill_fraction;	/* in [0,1], <0 means unfilled (transparent) */
     bool use_color;
#endif

{
  reader.first_plot_point = true;
  reader.need_break = true;	/* force break in polyline */

  reader.input_type = input_type;
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
#ifdef _HAVE_PROTOS
reset_reader(void)
#else
reset_reader()
#endif
{
  reader.need_break = true;	/* force break in polyline */

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
#ifdef _HAVE_PROTOS
set_reader_parameters (data_type input_type, bool auto_abscissa, 
		       double delta_x, double abscissa, int symbol,
		       double symbol_size, char *symbol_font_name, 
		       int linemode, double line_width, double fill_fraction, 
		       bool use_color, bool new_symbol, 
		       bool new_symbol_size, bool new_symbol_font_name,
		       bool new_linemode, bool new_line_width,
		       bool new_fill_fraction, bool new_use_color)
#else
set_reader_parameters (input_type, auto_abscissa, delta_x, abscissa,
		       symbol, symbol_size, symbol_font_name,
		       linemode, line_width, fill_fraction, use_color,
		       new_symbol, new_symbol_size, new_symbol_font_name,
		       new_linemode, new_line_width, new_fill_fraction, 
		       new_use_color)
     data_type input_type;
     bool auto_abscissa;
     double delta_x;
     double abscissa;
     int symbol;
     double symbol_size;
     char *symbol_font_name;
     int linemode;
     double line_width;
     double fill_fraction;
     bool use_color;
     /* following bools make up a mask */
     bool new_symbol, new_symbol_size, new_symbol_font_name;
     bool new_linemode, new_line_width, new_fill_fraction, new_use_color;
#endif
{
  reader.need_break = true;	/* force break in polyline */
  reader.input_type = input_type;
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
   indicator was seen in the input file.  For an ascii stream this is two
   newlines in succession; for a double stream this is a DBL_MAX.  A
   return value of 3 is special: it indicates that no point was read, but a
   `set linemode / symbol type' directive was seen in the input file.  By
   convention we break any polyline under construction when such a
   directive is seen.  A return value of 4 signifies a `soft EOF' (we
   already returned an end-of-dataset, and this EOF shouldn't be
   interpreted as terminating an additional [null] dataset).

   This routine calls read_point_ascii(), read_point_ascii_errorbar(),
   read_point_binary(), or read_point_gnuplot() to do the actual reading.
*/

static int
#ifdef _HAVE_PROTOS
read_point (FILE *input, Point *point)
#else
read_point (input, point)
     FILE *input;
     Point *point;
#endif
{
  int success;			/* return code from read_point_* */

  /* following fields are constant throughout each polyline */
  point->symbol = reader.symbol;
  point->symbol_size = reader.symbol_size;
  point->symbol_font_name = reader.symbol_font_name;
  point->linemode = reader.linemode;
  point->line_width = reader.line_width;
  point->fill_fraction = reader.fill_fraction;
  point->use_color = reader.use_color;
  point->have_x_errorbar = false; /* not supported yet */
  point->have_y_errorbar = (reader.input_type == T_ASCII_ERRORBAR ? true : false);
  
 head:

  switch (reader.input_type)
    {
    case T_ASCII:
    default:
      success = read_point_ascii (input, point);
      break;
    case T_SINGLE:
    case T_DOUBLE:
    case T_INTEGER:
      success = read_point_binary (input, point, reader.input_type);
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
      bool bad_point = false;

      /* If we have log axes, the values we work with ALL refer to the log10
	 values of the data.  A nonpositive value generates a warning, and a
	 break in the polyline. */
      if (reader.log_axis & X_AXIS)
	{
	  if (point->x > 0.0)
	    point->x = log10 (point->x);
	  else
	    bad_point = true;
	  if (point->have_x_errorbar)
	    {
	      if (point->xmin > 0.0)
		point->xmin = log10 (point->xmin);
	      else
		bad_point = true;
	      if (point->xmax > 0.0)
		point->xmax = log10 (point->xmax);
	      else
		bad_point = true;
	    }
	  
	  if (bad_point)
	    {
	      fprintf (stderr, "%s: log plot, dropping inappropriate point (%g,%g)\n",
		       progname, point->x, point->y);
	      reader.need_break = true;
	      goto head;		/* on to next point */
	    }
	}
      if (reader.log_axis & Y_AXIS)
	{
	  if (point->y > 0.0)
	    point->y = log10 (point->y);
	  else
	    bad_point = true;

	  if (point->have_y_errorbar)
	    {
	      if (point->ymin > 0.0)
		point->ymin = log10 (point->ymin);
	      else
		bad_point = true;
	      if (point->ymax > 0.0)
		point->ymax = log10 (point->ymax);
	      else
		bad_point = true;
	    }
	  
	  if (bad_point)
	    {
	      fprintf (stderr, "%s: log plot, dropping inappropriate point (%g,%g)\n",
		       progname, point->x, point->y);
	      reader.need_break = true;
	      goto head;		/* on to next point */
	    }
	}
      
      if (reader.transpose_axes)
	{
	  double tmp;
	  bool tmp_bool;
	  
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
	point->pendown = false;
      else
	point->pendown = true;
      
      /* reset break-polyline flag */
      reader.need_break = false;
      
      /* reader is no longer virgin */
      reader.first_plot_point = false;
    }

  return success;		/* could be 0, 1, 2, 3, or 4 */
}

static int
#ifdef _HAVE_PROTOS
read_point_ascii (FILE *input, Point *point)
#else
read_point_ascii (input, point)
     FILE *input;
     Point *point;
#endif
{
  int items_read, lookahead;
  bool two_newlines;

 head:

  /* skip whitespace, up to but not including 2nd newline if any */
  two_newlines = skip_whitespace (input);
  if (two_newlines)
    return 2;			/* end of dataset */
  if (feof (input))
    return 1;

  /* process linemode / symbol type directive */
  lookahead = getc (input);
  ungetc (lookahead, input);
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
	fprintf (stderr, "%s: input file terminated prematurely\n", progname);
      return 1;			/* couldn't get y coor, effectively EOF */
    }
}

static int
#ifdef _HAVE_PROTOS
read_point_ascii_errorbar (FILE *input, Point *point)
#else
read_point_ascii_errorbar (input, point)
     FILE *input;
     Point *point;
#endif
{
  int items_read, lookahead;
  bool two_newlines;
  double error_size;

 head:

  /* skip whitespace, up to but not including 2nd newline if any */
  two_newlines = skip_whitespace (input);
  if (two_newlines)
    return 2;			/* end of dataset */
  if (feof (input))
    return 1;

  /* process linemode / symbol type directive */
  lookahead = getc (input);
  ungetc (lookahead, input);
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
	fprintf (stderr, "%s: errorbar-format input file terminated prematurely\n", progname);
      return 1;			/* couldn't get y coor, effectively EOF */
    }

  items_read = fscanf (input, "%lf", &error_size);
  if (items_read != 1)
    {
      fprintf (stderr, "%s: errorbar-format input file terminated prematurely\n", progname);
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
#ifdef _HAVE_PROTOS
read_point_binary (FILE *input, Point *point, data_type input_type)
#else
read_point_binary (input, point, input_type)
     FILE *input;
     Point *point;
     data_type input_type;
#endif
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
      switch (input_type)
	{
	case T_DOUBLE:
	default:
	  items_read = 
	    fread ((Voidptr) &(point->x), sizeof (double), 1, input);
	  break;
	case T_SINGLE:
	  {
	    float fx;
	    
	    items_read = 
	      fread ((Voidptr) &fx, sizeof (fx), 1, input);
	    point->x = fx;
	  }
	  break;
	case T_INTEGER:
	  {
	    int ix;
	    
	    items_read = 
	      fread ((Voidptr) &ix, sizeof (ix), 1, input);
	    point->x = ix;
	  }
	  break;
	}
      if (items_read <= 0)
	return 1;		/* presumably EOF */
    }

  if ((input_type == T_DOUBLE && point->x == DBL_MAX)
      || (input_type == T_SINGLE && point->x == (double)FLT_MAX)
      || (input_type == T_INTEGER && point->x == (double)INT_MAX))
    return 2;			/* end of dataset */

  switch (input_type)
    {
    case T_DOUBLE:
    default:
      items_read = 
	fread ((Voidptr) &(point->y), sizeof (double), 1, input);
      break;
    case T_SINGLE:
      {
	float fy;
	
	items_read = 
	  fread ((Voidptr) &fy, sizeof (fy), 1, input);
	point->y = fy;
      }
      break;
    case T_INTEGER:
      {
	int iy;
	
	items_read = 
	  fread ((Voidptr) &iy, sizeof (iy), 1, input);
	point->y = iy;
      }
      break;
    }

  if (items_read != 1)		/* didn't get a pair of floats */
    {
      if (!reader.auto_abscissa)
	fprintf (stderr, "%s: binary input file terminated prematurely\n", progname);
      return 1;			/* effectively EOF */
    }
  if (point->x != point->x || point->y != point->y)
    {
      fprintf (stderr, "%s: encountered a NaN (not-a-number) in binary input file\n",
	       progname);
      return 1;			/* effectively EOF */
    }

  return 0;			/* got a pair of floats */

}

static int
#ifdef _HAVE_PROTOS
read_point_gnuplot (FILE *input, Point *point)
#else
read_point_gnuplot (input, point)
     FILE *input;
     Point *point;
#endif
{
  int lookahead, items_read;
  char directive, c;
  bool two_newlines;
  
 head:
  
  /* skip whitespace, up to but not including 2nd newline */
  two_newlines = skip_whitespace (input);
  if (two_newlines)
    return 2;			/* end of dataset */
  if (feof (input))
    return 4;			/* `soft' EOF (won't bump linemode) */

  lookahead = getc (input);
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
		   "%s: gnuplot-format input file terminated prematurely\n", 
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
		       "%s: gnuplot-format input file terminated prematurely\n", 
		       progname);
	    return 1;		/* effectively EOF */
	    }
	}
      while (c != '\n');

      /* break the polyline here in a soft way (don't start a new dataset) */
      reader.need_break = true;	
      goto head;
    default:
      fprintf (stderr, 
	       "%s: unknown gnuplot directive `%c' encountered in input file\n", 
	       progname, (char)lookahead);
      return 1;			/* effectively EOF */
    }
}


/* read_dataset() reads an entire dataset (a sequence of points) from an
   input file, and stores it.  The length of the block in which the
   points are stored, and the number of points, are passed back.  

   Return value = 1 means the dataset terminated with an EOF, and return
   value = 2 means the dataset terminated with an explicit end-of-dataset
   marker.  An end-of-dataset marker is two newlines in succession for an
   ascii stream, and a DBL_MAX for a stream of doubles.  Return value 3
   is special: it signals that a `set linemode / symbol type' directive was
   seen in the input file.  By convention, we interpret such a directive
   as ending a dataset.  Return value 4 signifies a `soft EOF' (we already
   returned an end-of-dataset, and this EOF shouldn't be interpreted as
   terminating an additional [null] dataset).  The distinction between a
   hard EOF and a soft EOF is important if we are automatically bumping the
   line mode between datasets. */

static int
#ifdef _HAVE_PROTOS
read_dataset (FILE *input, Point **p_addr, int *length, int *no_of_points)
#else
read_dataset (input, p_addr, length, no_of_points)
     FILE *input;
     Point **p_addr;
     int *length;  /* buffer length in bytes, should begin > 0 */
     int *no_of_points;
#endif
{
  Point *p = *p_addr;
  int success;

  while (true)
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

/* read_file() reads all possible datasets from an input file, and stores
   them.  The length of the block in which the data points are stored, and
   the number of points, are passed back.  */

void
#ifdef _HAVE_PROTOS
read_file (FILE *input, Point **p_addr, int *length, int *no_of_points)
#else
read_file (input, p_addr, length, no_of_points)
     FILE *input;
     Point **p_addr;
     int *length;  /* buffer length in bytes, should begin > 0 */
     int *no_of_points;
#endif
{
  int success;
  bool saved_auto_bump;

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
	  reader.auto_bump = false;
	  reset_reader();
	  reader.auto_bump = saved_auto_bump;
	}
      else if (success != 4)	/* don't reset on `soft EOF', already did so */
	reset_reader();
    }
  while (success != 1 && success != 4);	/* keep going until EOF seen */
}


/* read_and_plot_dataset() reads an entire dataset (a sequence of points)
   from an input file; it calls plot_point() on each point as it is read.
   So plotting is accomplished in real time.  

   Return value = 1 means the dataset terminated with an EOF, and return
   value = 2 means the dataset terminated with an explicit end-of-dataset
   marker.  An end-of-dataset marker is two newlines in succession for an
   ascii stream, and a DBL_MAX for a stream of doubles.  Return value 3 is
   special: it signals that a `set linemode / symbol type' directive was
   seen in the input file.  By convention, we interpret such a directive as
   ending a dataset.  Return value 4 signifies a `soft EOF' (we already
   returned an end-of-dataset, and this EOF shouldn't be interpreted as
   terminating an additional [null] dataset). */

static int
#ifdef _HAVE_PROTOS
read_and_plot_dataset (FILE *input)
#else
read_and_plot_dataset (input)
     FILE *input;
#endif
{
  int success;

  while (true)
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
#ifdef _HAVE_PROTOS
read_and_plot_file (FILE *input)
#else
read_and_plot_file (input)
     FILE *input;
#endif
{
  int success;
  bool saved_auto_bump;

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
	  reader.auto_bump = false;
	  reset_reader();
	  reader.auto_bump = saved_auto_bump;
	}
      else if (success != 4)	/* don't reset on `soft EOF', already did so */
	reset_reader();

      /* after each dataset, flush the constructed polyline to the display
         device */
      if (success != 4)
	{
	  endpath();
	  flushpl();
	}
    }
  while (success != 1 && success != 4);	/* keep going until EOF seen */
}


/* skip_whitespace() skips whitespace in an ascii-format input file,
   up to but not including a second newline.  Return value indicates
   whether or not two newlines were in fact seen.  (For ascii-format
   input files, two newlines signals an end-of-dataset.) */

static bool
#ifdef _HAVE_PROTOS
skip_whitespace (FILE *stream)
#else
skip_whitespace (stream)
     FILE *stream;
#endif
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
    return false;
  
  ungetc (lookahead, stream);
  return (nlcount == 2 ? true : false);
}
