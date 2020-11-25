/* This file contains the main routine, and a few support subroutines, for
   GNU graph.

   Copyright (C) 1989-1998 Free Software Foundation, Inc. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"
#include "getopt.h"

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] =
{
  /* The most important option */
  {"display-type",	ARG_REQUIRED,	NULL, 'T'},
  /* Other frequently used options */
  {"auto-abscissa",	ARG_OPTIONAL,	NULL, 'a'}, /* 0 or 1 or 2 */
  {"clip-mode",		ARG_REQUIRED,	NULL, 'K'},
  {"fill-fraction",	ARG_REQUIRED,	NULL, 'q'},
  {"font-name",		ARG_REQUIRED,	NULL, 'F'},
  {"font-size",		ARG_REQUIRED,	NULL, 'f'},
  {"grid-style",	ARG_REQUIRED,	NULL, 'g'},
  {"height-of-plot",	ARG_REQUIRED,	NULL, 'h'},
  {"input-format",	ARG_REQUIRED,	NULL, 'I'},
  {"line-mode",		ARG_REQUIRED,	NULL, 'm'},
  {"line-width",	ARG_REQUIRED,	NULL, 'W'},
  {"right-shift",	ARG_REQUIRED,	NULL, 'r'},
  {"save-screen",	ARG_NONE,	NULL, 's'},
  {"symbol",		ARG_OPTIONAL,	NULL, 'S'}, /* 0 or 1 or 2 */
  {"tick-size",		ARG_REQUIRED,	NULL, 'k'},
  {"toggle-auto-bump",	ARG_NONE,	NULL, 'B'},
  {"toggle-log-axis",	ARG_REQUIRED,	NULL, 'l'},
  {"toggle-no-ticks",	ARG_REQUIRED,	NULL, 'N'},
  {"toggle-round-to-next-tick",	ARG_REQUIRED,	NULL, 'R'},
  {"toggle-transpose-axes",	ARG_NONE,	NULL, 't'},
  {"toggle-use-color",	ARG_NONE,	NULL, 'C'},
  {"top-label",		ARG_REQUIRED,	NULL, 'L'},
  {"upward-shift",      ARG_REQUIRED,	NULL, 'u'},
  {"width-of-plot",	ARG_REQUIRED,	NULL, 'w'},
  {"x-label",		ARG_REQUIRED,	NULL, 'X'},
  {"x-limits",		ARG_OPTIONAL,	NULL, 'x'}, /* 0, 1, 2, or 3 */
  {"y-label",		ARG_REQUIRED,	NULL, 'Y'},
  {"y-limits",		ARG_OPTIONAL,	NULL, 'y'}, /* 0, 1, 2, or 3 */
  /* Long options with no equivalent short option alias */
  {"bg-color",		ARG_REQUIRED,	NULL, 'q' << 8},
  {"bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8},
  {"blankout",		ARG_REQUIRED,	NULL, 'b' << 8},  
  {"frame-line-width",	ARG_REQUIRED,	NULL, 'W' << 8},
  {"frame-color",	ARG_REQUIRED,	NULL, 'C' << 8},
  {"max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8},
  {"pen-colors",	ARG_REQUIRED,	NULL, 'p' << 8},
  {"reposition",	ARG_REQUIRED,	NULL, 'R' << 8}, /* 3 */
  {"symbol-font-name",	ARG_REQUIRED,	NULL, 'G' << 8},
  {"title-font-name",	ARG_REQUIRED,	NULL, 'Z' << 8},
  {"title-font-size",	ARG_REQUIRED,	NULL, 'F' << 8},
  {"toggle-rotate-y-label",	ARG_NONE,	NULL, 'N' << 8},
  {"toggle-switch-axis-end",	ARG_REQUIRED,	NULL, 's' << 8},
  {"page-size",		ARG_REQUIRED,	NULL, 'P' << 8},
  /* Options relevant only to raw graph (refers to plot(5) output) */
  {"portable-output",	ARG_NONE,	NULL, 'O'},
  /* Documentation options */
  {"help-fonts",	ARG_NONE,	NULL, 'f' << 8},
  {"list-fonts",	ARG_NONE,	NULL, 'l' << 8},
  {"version",		ARG_NONE,	NULL, 'V' << 8},
  {"help",		ARG_NONE,	NULL, 'h' << 8},
  {NULL, 0, 0, 0}
};

/* null-terminated list of options that we don't show to the user */
int hidden_options[] = { 0 };

const char *progname = "graph";	/* name of this program */

const char *usage_appendage = " [FILE]...\n\
With no FILE, or when FILE is -, read standard input.\n";

/* forward references */
static void close_file __P ((char *filename, FILE *stream));
static void open_file_for_reading __P ((char *filename, FILE **input));
static bool parse_pen_string __P ((const char *pen_s));

int
#ifdef _HAVE_PROTOS
main (int argc, char *argv[])
#else
main (argc, argv)
     int argc;
     char *argv[];
#endif
{
  /* Variables related to getopt parsing */

  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered in getopt parsing */
  bool using_getopt = true;	/* true until end of command-line options */
  bool continue_parse = true;	/* reset e.g. when --help or --version seen */
  bool show_version = false;	/* show version message? */
  bool show_usage = false;	/* show usage message? */
  bool show_fonts = false;	/* supply help on fonts? */
  bool do_list_fonts = false;	/* show a list of fonts? */
  bool filter = false;		/* will we act as a filter? */
  bool new_symbol = false;
  bool new_symbol_size = false;
  bool new_symbol_font_name = false;
  bool new_linemode = false;
  bool new_plot_line_width = false;
  bool new_fill_fraction = false;
  bool new_use_color = false;
  bool first_file_of_plot = true;
  bool first_plot = true;
  FILE *data_file = NULL;
  char *display_type = "meta";	/* default libplot output format */

  /* Variables related to the point reader */

  data_type input_type = T_ASCII; /* by default we read ascii data */
  bool auto_bump = true;	/* auto-bump linemode between polylines? */
  bool auto_abscissa = false;	/* generate abscissa values automatically? */
  double x_start = 0.;		/* start and increment, for auto-abscissa */
  double delta_x = 1.;
  /* polyline attributes */
  int linemode_index = 1;	/* linemode for polylines, 1=solid, etc. */
  double plot_line_width = -0.001; /* polyline width (as frac. of display width), negative means default provided by libplot) */
  int symbol_index = 0;		/* 0=none, 1=dot, 2=plus, 3=asterisk, etc. */
  double symbol_size = .03;	/* symbol size (as frac. of plotting box width) */
  double fill_fraction = -1.0;	/* negative means regions aren't filled */
  bool use_color = false;	/* color / monochrome */

  /* Variables related to both the point reader and the point plotter */

  bool transpose_axes = false;	/* true means -x applies to y axis, etc. */

  /* Variables related to the point plotter */

  grid_type grid_spec = AXES_AND_BOX; /* frame type for the plot */
  char *frame_color = "black";	/* color of frame (and plot, if no -C option)*/
  char *bg_color = NULL;	/* color of background, if non-NULL */
  bool save_screen = false;	/* save screen, i.e. no erase before plot? */
  bool no_rotate_y_label = false; /* used for pre-X11R6 servers */
  int clip_mode = 1;		/* clipping mode (cf. gnuplot) */
  /* following variables are portmanteau: x and y are included as bitfields*/
  int log_axis = 0;		/* log axes or linear axes? */
  int round_to_next_tick = 0;	/* round axis limits to nearest tick? */
  int switch_axis_end = 0;	/* axis at top/right instead of bottom/left? */
  int omit_ticks = 0;		/* omit ticks and tick labels from an axis? */

  /* plotter dimensions, expressed as fractions of the width of the libplot
     graphics display [by convention square]; <0.0 means use libplot default*/
  double frame_line_width = -0.001; /* width of lines in the plot frame */

  /* dimensions of plotting area, expressed as fractions of the width of
     the libplot graphics display [by convention square] */
  double margin_below = .2;	/* margin below the plot */
  double margin_left = .2;	/* margin left of the plot */
  double plot_height = .6;	/* height of the plot */
  double plot_width = .6;	/* width of the plot */

  /* dimensions, expressed as fractions of the width of the plotting area */
  double tick_size = .02;	/* size of tick marks (< 0.0 allowed) */
  double font_size = 0.0525;	/* fontsize */
  double title_font_size = 0.07; /* title fontsize */
  double blankout_fraction = 1.3; /* this fraction of size of plotting box
				   is erased before the plot is drawn */

  /* text-related */
  char *font_name = NULL;	/* font name, NULL -> device default */
  char *title_font_name = NULL;	/* title font name, NULL -> device default */
  char *symbol_font_name = "ZapfDingbats"; /* symbol font name, NULL -> default */
  char *x_label = NULL;		/* label for the x axis, NULL -> no label */
  char *y_label = NULL;		/* label for the y axis, NULL -> no label */
  char *top_label = NULL;	/* title above the plot, NULL -> no title */

  /* user-specified limits on the axes */
  double min_x = 0.0, min_y = 0.0, max_x = 0.0, max_y = 0.0;
  double spacing_x = 0.0, spacing_y = 0.0;

  /* flags indicating which axis limits the user has specified */
  bool spec_min_x = false, spec_min_y = false;
  bool spec_max_x = false, spec_max_y = false;
  bool spec_spacing_x = false, spec_spacing_y = false;

  /* misc. local variables used in getopt parsing, counterparts to the above */
  double local_x_start, local_delta_x;
  int local_grid_style;
  int local_symbol_index;
  int local_clip_mode;
  double local_symbol_size, local_font_size, local_title_font_size;
  double local_frame_line_width, local_plot_line_width;
  double local_min_x, local_min_y;
  double local_max_x, local_max_y;
  double local_spacing_x, local_spacing_y;
  double local_fill_fraction;
  
  /* `finalized' arguments to initialize_plotter() (computed at the time
     the first file of a plot is seen, and continuing in effect over the
     duration of the plot) */
  int final_log_axis = 0;
  int final_round_to_next_tick = 0;
  double final_min_x = 0.0, final_max_x = 0.0, final_spacing_x = 0.0;
  double final_min_y = 0.0, final_max_y = 0.0, final_spacing_y = 0.0;  
  bool final_spec_min_x = false, final_spec_min_y = false;
  bool final_spec_max_x = false, final_spec_max_y = false;
  bool final_spec_spacing_x = false, final_spec_spacing_y = false;
  bool final_transpose_axes = false;

  /* for storage of data points (if we're not acting as a filter) */
  Point *p;			/* points array */
  int points_length = 1024;	/* length of the points array */
  int no_of_points = 0;		/* number of points stored in it */

  /* support for multiplotting */
  double reposition_trans_x = 0.0, reposition_trans_y = 0.0;
  double reposition_scale = 1.0;
  double old_reposition_trans_x, old_reposition_trans_y;
  double old_reposition_scale;

  /* parse argv[] */
  while (continue_parse)
    {
      if (using_getopt)		/* true until end of options */
	{
	  option = getopt_long (argc, argv, 
				"-BCOVstF:f:g:h:k:K:I:l:L:m:N:q:R:r:T:u:w:W:X:Y:a::x::y::S::", 
			    long_options, &opt_index);
	  if (option == EOF)	/* end of options */
	    {
	      using_getopt = false;
	      continue;		/* back to top of while loop */
	    }
	  if (option == 1)	/* filename embedded among options */
	    if (strcmp (optarg, "-") == 0)
	      data_file = stdin; /* interpret "-" as stdin */
	    else
	      open_file_for_reading (optarg, &data_file);
	}
      else			/* processing filenames manually */
	{
	  if (optind >= argc)	/* all files processed */
	    {
	      if (first_plot && first_file_of_plot)
		/* no file appeared on command line, read stdin instead */
		{
		  data_file = stdin;
		  option = 1;	/* code for pseudo-option */
		}
	      else
		break;		/* all files done, break out of while loop */
	    }
	  else			/* have files yet to process */
	    {
	      if (strcmp (argv[optind], "-") == 0)
		data_file = stdin;
	      else
		open_file_for_reading (argv[optind], &data_file);
	      optarg = argv[optind]; /* keep track of name of opened file */
	      optind++;
	      option = 1;	/* code for pseudo-option */
	    }
	}

      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 's':		/* Don't erase display before plot, ARG NONE */
	  save_screen = true;
	  break;
	case 't':		/* Toggle transposition of axes, ARG NONE */
	  transpose_axes = (transpose_axes == true ? false : true);
	  break;
	case 'B':		/* Toggle linemode auto-bumping, ARG NONE */
	  auto_bump = (auto_bump == true ? false : true);
	  break;
	case 'C':		/* Toggle color/monochrome, ARG NONE */
	  new_use_color = true;
	  use_color = (use_color == true ? false : true);
	  break;
	case 'O':
	  parampl ("META_PORTABLE", "yes"); /* portable format, ARG NONE */
	  break;
	case 'V' << 8:		/* Version, ARG NONE		*/
	  show_version = true;
	  continue_parse = false;
	  break;
	case 'h' << 8:		/* Help, ARG NONE		*/
	  show_usage = true;
	  continue_parse = false;
	  break;
	case 'f' << 8:		/* Help on fonts, ARG NONE	*/
	  show_fonts = true;
	  continue_parse = false;
	  break;
	case 'l' << 8:		/* List fonts, ARG NONE		*/
	  do_list_fonts = true;
	  continue_parse = false;
	  break;
	case 'N' << 8:		/* Toggle rotation of y-label, ARG NONE */
	  no_rotate_y_label = (no_rotate_y_label == true ? false : true);
	  break;

	  /*----------- options with a single argument --------------*/

	case 'I':		/* Input format, ARG REQUIRED	*/
	  switch (*optarg)
	    {
	    case 'a':
	    case 'A':
	      /* ASCII format, records and fields within records are
		 separated by whitespace, and datasets are separated by a
		 pair of newlines.  Record length = 2. */
	      input_type = T_ASCII;
	      break;
	    case 'f':
	    case 'F':
	      /* Binary single precision, records and fields within records
		 are contiguous, and datasets are separated by a FLT_MAX.
		 Record length = 2. */
	      input_type = T_SINGLE;
	      break;
	    case 'd':
	    case 'D':
	      /* Binary double precision, records and fields within records
		 are contiguous, and datasets are separated by a DBL_MAX.
		 Record length = 2. */
	      input_type = T_DOUBLE;
	      break;
	    case 'i':
	    case 'I':
	      /* Binary integer, records and fields within records are
		 contiguous, and datasets are separated by an occurrence of
		 INT_MAX.  Record length = 2. */
	      input_type = T_INTEGER;
	      break;
	    case 'e':
	    case 'E':
	      /* Same as T_ASCII, but record length = 3. */
	      input_type = T_ASCII_ERRORBAR;
	      break;
	    case 'g':
	    case 'G':
	      /* Sui generis. */
	      input_type = T_GNUPLOT;	/* gnuplot `table' format */
	      break;
	    default:
	      fprintf (stderr,
		       "%s: error: unrecognized data option `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'f':		/* Font size, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_font_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: font size must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_font_size < 0.0)
	    fprintf (stderr, "%s: ignoring negative font size `%f'\n",
		     progname, local_font_size);
	  else if (local_font_size == 0.0)
	    fprintf (stderr, "%s: ignoring zero font size\n",
		     progname);
	  else
	    font_size = local_font_size;
	  break;
	case 'g':		/* Grid style, ARG REQUIRED	*/
	  if (sscanf (optarg, "%d", &local_grid_style) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: grid style must be a (small) integer, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	      break;
	    }
	  switch (local_grid_style)
	    /* the subset ordering is: 0 < 1 < 2 < 3; 4 is different */
	    {
	    case 0:
	      /* no frame at all; just the plot */
	      grid_spec = NO_AXES;
	      break;
	    case 1:
	      /* box, ticks, gridlines, labels */
	      grid_spec = AXES;
	      break;
	    case 2:
	      /* box, ticks, no gridlines, labels */
	      grid_spec = AXES_AND_BOX;
	      break;
	    case 3:
	      /* `half-box', partial ticks, no gridlines, labels */
	      grid_spec = AXES_AND_BOX_AND_GRID;
	      break;
	    case 4:
	      /* no box, no gridlines; specially positioned axes, labels */
	      grid_spec = AXES_AT_ORIGIN;
	      break;
	    default:
	      fprintf (stderr,
		       "%s: error: grid style number `%s' out of bounds\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'h':		/* Height of plot, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &plot_height) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: plot height must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'K':		/* Clip mode, ARG REQUIRED */
	  if ((sscanf (optarg, "%d", &local_clip_mode) <= 0)
	      || local_clip_mode < 0 || local_clip_mode > 2)
	    fprintf (stderr,
		     "%s: ignoring bad clip mode `%s' (must be 0, 1, or 2)\n",
		     progname, optarg);
	  else
	    clip_mode = local_clip_mode;
	  break;
	case 'l':		/* Toggle log/linear axis, ARG REQUIRED */
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      log_axis ^= X_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      log_axis ^= Y_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: ignoring unrecognized axis specification `%s'\n", 
		       progname, optarg);
	      break;
	    }
	  break;
	case 'N':		/* Toggle omission of labels, ARG REQUIRED */
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      omit_ticks ^= X_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      omit_ticks ^= Y_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: ignoring unrecognized axis specification `%s'\n", 
		       progname, optarg);
	      break;
	    }
	  break;
	case 'm':		/* Linemode, ARG REQUIRED	*/
	  new_linemode = true;
	  if (sscanf (optarg, "%d", &linemode_index) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: linemode must be a (small) integer, was `%s'.\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'q':		/* Fill fraction, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_fill_fraction) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: fill fraction must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_fill_fraction > 1.0)
	    fprintf (stderr, 
		       "%s: ignoring bad region fill fraction `%f' (must be <=1.0)\n",
		     progname, local_fill_fraction);
	  else
	    {
	      fill_fraction = local_fill_fraction;
	      new_fill_fraction = true;
	    }
	  break;
	case 'r':		/* Right shift, ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &margin_left) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: plot rightward displacement must be a number, was `%s'.\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'u':		/* Upward shift, ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &margin_below) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: plot upward displacement must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'w':		/* Width of plot, ARG REQUIRED 	*/
	  if (sscanf (optarg, "%lf", &plot_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: plot width must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'T':		/* Display type, ARG REQUIRED      */
	  display_type = xstrdup (optarg);
	  break;
	case 'F':		/* Font name, ARG REQUIRED      */
	  font_name = xstrdup (optarg);
	  break;
	case 'Z' << 8:		/* Title Font name, ARG REQUIRED      */
	  title_font_name = xstrdup (optarg);
	  break;
	case 'G' << 8:		/* Symbol Font name, ARG REQUIRED      */
	  symbol_font_name = xstrdup (optarg);
	  new_symbol_font_name = true;
	  break;
	case 'R':		/* Toggle rounding to next tick, ARG REQUIRED*/
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      round_to_next_tick ^= X_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      round_to_next_tick ^= Y_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: ignoring unrecognized axis specification `%s'\n", 
		       progname, optarg);
	      break;
	    }
	  break;
	case 'L':		/* Top title, ARG REQUIRED	*/
	  top_label = xstrdup (optarg);
	  break;
	case 'k':		/* Tick size, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &tick_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: tick size must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'W':		/* Line width, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_plot_line_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: plot line width must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_plot_line_width < 0.0)
	    fprintf (stderr, "%s: ignoring negative plot line width `%f'\n",
		     progname, local_plot_line_width);
	  else
	    {
	      plot_line_width = local_plot_line_width;
	      new_plot_line_width = true;
	    }
	  break;
	case 'X':		/* X axis title, ARG REQUIRED	*/
	  x_label = xstrdup (optarg);
	  break;
	case 'Y':		/* Y axis title, ARG REQUIRED	*/
	  y_label = xstrdup (optarg);
	  break;
	case 's' << 8:		/* Toggle switching of axis to other end, 
				   ARG REQUIRED */
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      switch_axis_end ^= X_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      switch_axis_end ^= Y_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: ignoring unrecognized axis specification `%s'\n", 
		       progname, optarg);
	      break;
	    }
	  break;
	case 'b' << 8:		/* Blankout fraction, ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &blankout_fraction) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: fractional blankout must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'B' << 8:		/* Bitmap size, ARG REQUIRED	*/
	  parampl ("BITMAPSIZE", optarg);
	  break;
	case 'F' << 8:		/* Title font size, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_title_font_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: title font size must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_title_font_size < 0.0)
	    fprintf (stderr, "%s: ignoring negative title font size `%f'\n",
		     progname, local_title_font_size);
	  if (local_title_font_size == 0.0)
	    fprintf (stderr, "%s: ignoring zero title font size\n",
		     progname);
	  else
	    title_font_size = local_title_font_size;
	  break;
	case 'W' << 8:		/* Frame line width, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_frame_line_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: frame line width must be a number, was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_frame_line_width < 0.0)
	    fprintf (stderr, "%s: ignoring negative frame line width `%f'\n",
		     progname, local_frame_line_width);
	  else
	    frame_line_width = local_frame_line_width;
	  break;
	case 'M' << 8:		/* Max line length, ARG REQUIRED	*/
	  parampl ("MAX_LINE_LENGTH", optarg);
	  break;
	case 'P' << 8:		/* Page size, ARG REQUIRED	*/
	  parampl ("PAGESIZE", optarg);
	  break;
	case 'Q' << 8:		/* Plot rotation angle, ARG REQUIRED	*/
	  parampl ("ROTATE", optarg);
	  break;
	case 'p' << 8:		/* Pen color string, ARG REQUIRED      */
	  if (parse_pen_string (optarg) == false)
	    {
	      fprintf (stderr, "%s: ignoring unparseable pen string `%s'\n",
		       progname, optarg);
	    }
	  break;
	case 'q' << 8:		/* Background color, ARG REQUIRED      */
	  bg_color = xstrdup (optarg);
	  break;
	case 'C' << 8:		/* Frame color, ARG REQUIRED      */
	  frame_color = xstrdup (optarg);
	  break;

	  
	  /*------ options with zero or more arguments ---------*/

	case 'a':		/* Auto-abscissa, ARG OPTIONAL [0,1,2] */
	  auto_abscissa = true;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_delta_x) <= 0)
	    break;
	  optind++;	/* tell getopt we recognized delta_x */
	  if (local_delta_x == 0.0)
	    /* "-a 0" turns off auto-abscissa for next file */
	    {
	      auto_abscissa = false;
	      break;
	    }
	  delta_x = local_delta_x;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_x_start) <= 0)
	    break;
	  x_start = local_x_start;
	  optind++;	/* tell getopt we recognized x_start */
	  break;
	case 'x':		/* X limits, ARG OPTIONAL [0,1,2,3] */
	  if ((optind >= argc)
	      || (sscanf (argv[optind], "%lf", &local_min_x) <= 0))
	    {
	      spec_min_x = spec_max_x = spec_spacing_x = false;
	      break;
	    }
	  min_x = local_min_x;
	  spec_min_x = true;
	  optind++;	/* tell getopt we recognized min_x */
	  if ((optind >= argc)
	      || (sscanf (argv [optind], "%lf", &local_max_x) <= 0))
	    {
	      spec_max_x = spec_spacing_x = false;
	      break;
	    }
	  max_x = local_max_x;
	  spec_max_x = true;
	  optind++;	/* tell getopt we recognized max_x */
	  if ((optind >= argc)
	      || (sscanf (argv [optind], "%lf", &local_spacing_x) <= 0))
	    {
	      spec_spacing_x = false;
	      break;
	    }
	  spacing_x = local_spacing_x;
	  spec_spacing_x = true;
	  optind++;	/* tell getopt we recognized spacing_x */
	  break;

	case 'y':		/* Y limits, ARG OPTIONAL [0,1,2,3] */
	  if ((optind >= argc) 
	      || (sscanf (argv [optind], "%lf", &local_min_y) <= 0))
	    {
	      spec_min_y = spec_max_y = spec_spacing_y = false;
	      break;
	    }
	  min_y = local_min_y;
	  spec_min_y = true;
	  optind++;	/* tell getopt we recognized min_y */
	  if ((optind >= argc)
	      || (sscanf (argv [optind], "%lf", &local_max_y) <= 0))
	    {
	      spec_max_y = spec_spacing_y = false;
	      break;
	    }
	  max_y = local_max_y;
	  spec_max_y = true;
	  optind++;	/* tell getopt we recognized max_y */
	  if ((optind >= argc)
	      || (sscanf (argv [optind], "%lf", &local_spacing_y) <= 0))
	    {
	      spec_spacing_y = false;	      
	      break;
	    }
	  spacing_y = local_spacing_y;
	  spec_spacing_y = true;
	  optind++;	/* tell getopt we recognized spacing_y */
	  break;
	case 'S':		/* Symbol, ARG OPTIONAL	[0,1,2]		*/
	  new_symbol = true;
	  symbol_index = M_DOT; /* symbol # 1 is switched to by -S alone */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%d", &local_symbol_index) <= 0)
	    break;
	  if (local_symbol_index < 0 || local_symbol_index > 255)
	    fprintf (stderr, "%s: ignoring symbol type `%d' (must be in range 0..255)\n",
		     progname, local_symbol_index);
	  else
	    symbol_index = local_symbol_index;
	  optind++;		/* tell getopt we recognized symbol_index */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_symbol_size) <= 0)
	    break;
	  if (local_symbol_size < 0.0)
	    fprintf (stderr, "%s: ignoring negative symbol size `%f'\n",
		     progname, local_symbol_size);
	  else if (local_symbol_size == 0.0)
	    fprintf (stderr, "%s: ignoring zero symbol size\n",
		     progname);
	  else
	    {
	      symbol_size = local_symbol_size;
	      new_symbol_size = true;
	    }
	  optind++;		/* tell getopt we recognized symbol_size */
	  break;

	  /* ---------- options with one or more arguments ---------- */
	  
	case 'R' << 8:		/* End plot and reposition, ARG REQUIRED [3]*/
	  old_reposition_trans_x = reposition_trans_x;
	  old_reposition_trans_y = reposition_trans_y;
	  old_reposition_scale = reposition_scale;
	  
	  if (sscanf (optarg, "%lf", &reposition_trans_x) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: x repositioning must be a number, was `%s'\n",
		       progname, optarg);
	      return 1;
	    }
	  if (optind >= argc)
	    {
	      fprintf (stderr,
		       "%s: error: missing arg[s] to --reposition option\n",
		       progname);
	      return 1;
	    }
	  if (sscanf (argv[optind], "%lf", &reposition_trans_y) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: y repositioning must be a number, was `%s'\n",
		       progname, argv[optind]);
	      return 1;
	    }
	  optind++;		/* tell getopt we recognized trans_y */
	  if (optind >= argc)
	    {
	      fprintf (stderr,
		       "%s: error: missing arg[s] to --reposition option\n",
		       progname);
	      return 1;
	    }
	  if (sscanf (argv[optind], "%lf", &reposition_scale) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: reposition scale factor must be a number, was `%s'\n",
		       progname, optarg);
	      return 1;
	    }
	  if (reposition_scale == 0.0)
	    {
	      fprintf (stderr,
		       "%s: error: reposition scale factor cannot be zero\n", progname);
	      return 1;
	    }
	  optind++;		/* tell getopt we recognized trans_x */

	  if (!first_file_of_plot)
	    /* there is a plot in progress, and it must be ended */
	    {
	      if (!filter)
		/* Since we haven't been acting as a real-time filter for
		   the duration of this plot, the plot isn't already drawn
		   on the display.  Instead, we have a points array and
		   need to plot it, after computing bounds. */
		{
		  /* fill in any of min_? and max_? that user didn't
		     specify (the prefix "final_" means these arguments
		     were finalized at the time the first file of the plot
		     was processed) */
		  array_bounds (p, no_of_points, final_transpose_axes,
				&final_min_x, &final_min_y, 
				&final_max_x, &final_max_y,
				final_spec_min_x, final_spec_min_y, 
				final_spec_max_x, final_spec_max_y);
		  
		  /* font selection, saves typing */
		  if ((title_font_name == NULL) && (font_name != NULL))
		    title_font_name = font_name;
	      
		  /* initialize plotter, using (in part) finalized arguments */
		  initialize_plotter(display_type,
				     save_screen, /* for open_plotter() only */
				     bg_color,
				     frame_line_width,
				     frame_color,
				     top_label,
				     title_font_name, title_font_size, /* for title */
				     tick_size, grid_spec,
				     final_min_x, final_max_x, final_spacing_x,
				     final_min_y, final_max_y, final_spacing_y,
				     final_spec_spacing_x,
				     final_spec_spacing_y,
				     plot_width, plot_height, margin_below, margin_left,
				     font_name, font_size, /* for abs. label */
				     x_label, 
				     font_name, font_size, /* for ord. label */
				     y_label,
				     no_rotate_y_label,
				     /* these args are portmanteaux */
				     final_log_axis, 
				     final_round_to_next_tick,
				     switch_axis_end, omit_ticks, 
				     /* more args */
				     clip_mode,
				     blankout_fraction,
				     final_transpose_axes);
	      
		  if (first_plot)
		    /* haven't opened plotter yet, do so now */
		    {
		      if (open_plotter() < 0)
			{
			  fprintf (stderr, 
				   "%s: error: couldn't open plot device\n", progname);
			  return 1;
			}
		    }
		  
		  /* push new libplot drawing state onto stack of states */
		  savestate();

		  /* concatenate the current transformation matrix with a
		     matrix formed from the repositioning parameters (this
		     will be in effect for duration of the plot) */
		  fconcat (old_reposition_scale, 0.0, 
			   0.0, old_reposition_scale,
			   old_reposition_trans_x * PLOT_SIZE, 
			   old_reposition_trans_y * PLOT_SIZE);
		  
		  /* draw the plot frame (grid, ticks, etc.); draw a
		     `canvas' (a background opaque white rectangle) only if
		     this isn't the first plot */
		  plot_frame((first_plot == true ? false : true));
	      
		  /* plot the laboriously read-in array */
		  plot_point_array (p, no_of_points);
	      
		  /* free points array */
		  free (p);
		  no_of_points = 0;
		  first_file_of_plot = false;
	      
		} /* end of not-filter case */
	  
	      /* pop the plot-specific libplot drawing state off the stack
                 of drawing states */
	      restorestate();

	      /* on to next plot */
	      first_plot = false;
	      first_file_of_plot = true;
	    }
	  
	  break;

	  /* ---------------- pseudo-options -------------- */

	  /* File specified on command line, returned in order (along with
	     the true options).  The first time we reach this point, we
	     perform special initializations and determine whether or not,
	     for the duration of this plot, we'll be acting as a filter.  A
	     plot may consist of many files.  A plot in progress is
	     terminated if a --reposition option (which moves us to the
	     next plot of a multiplot) is seen, or when the last
	     command-line option is processed. */
	case 1:
	  if (first_file_of_plot)
	    {
	      /* For plots with a logarithmic axis, compute logs of axis
		 limits, since coordinates along the axis, as obtained from
		 the reader, are stored in logarithmic form. */
	      if (log_axis & X_AXIS)
		{
		  if (spec_min_x)
		    {
		      if (min_x > 0.0)
			min_x = log10(min_x);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: nonpositive limit %g on logarithmic axis\n", 
				  progname, min_x);
			  return 1;
			}
		    }
		  if (spec_max_x)
		    {
		      if (max_x > 0.0)
			max_x = log10(max_x);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: nonpositive limit %g on logarithmic axis\n", 
				  progname, max_x);
			  return 1;
			}
		    }
		}

	      if (log_axis & Y_AXIS)
		{
		  if (spec_min_y)
		    {
		      if (min_y > 0.0)
			min_y = log10(min_y);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: nonpositive limit %g on logarithmic axis\n", 
				  progname, min_y);
			  return 1;
			}
		    }
		  if (spec_max_y)
		    {
		      if (max_y > 0.0)
			max_y = log10(max_y);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: nonpositive limit %g on logarithmic axis\n", 
				  progname, max_y);
			  return 1;
			}
		    }
		}

	      /* We now finalize the following parameters (arguments to
		 initialize_plotter()), even though we won't call
		 initialize_plotter() for a while yet, if it turns out we
		 need to act as a real-time filter. */

	      /* portmanteaux */
	      final_log_axis = log_axis;
	      final_round_to_next_tick = round_to_next_tick;

	      /* bool */
	      final_transpose_axes = transpose_axes;

	      /* x-axis specific */
	      final_min_x = min_x;
	      final_max_x = max_x;
	      final_spacing_x = spacing_x;
	      final_spec_min_x = spec_min_x;
	      final_spec_max_x = spec_max_x;
	      final_spec_spacing_x = spec_spacing_x;

	      /* y-axis specific */
	      final_min_y = min_y;
	      final_max_y = max_y;
	      final_spec_min_y = spec_min_y;
	      final_spec_max_y = spec_max_y;
	      final_spacing_y = spacing_y;
	      final_spec_spacing_y = spec_spacing_y;

	      /* If user didn't specify the limits or tick spacing for an
		 axis, by default we'll round the axis limits to the
		 nearest tick, after computing them.  (If the limits were
		 specified by the user, to request rounding the user must
		 specify the -R option as well.) */
	      if (!final_spec_min_x)
		final_round_to_next_tick |= X_AXIS;
	      if (!final_spec_min_y)
		final_round_to_next_tick |= Y_AXIS;
	      
	      /* The case when x_min, x_max, y_min, y_max are all specified
		 by the luser is special: we set the `filter' flag for the
		 duration of this plot, to indicate that we can function as
		 a real-time filter, calling read_and_plot_file() on each
		 file, rather than calling read_file() on each one
		 separately to create an array of points, and then calling
		 plot_point_array(). */
	      filter = ((final_spec_min_x && final_spec_max_x 
			 && final_spec_min_y && final_spec_max_y) 
			? true : false);

	    } /* end of first-file-of-plot initialization */

	  if (filter)		/* will call read_and_plot() on the file */
	    {
	      if (first_file_of_plot)
		{
		  /* font selection, saves typing */
		  if ((title_font_name == NULL) && (font_name != NULL))
		    title_font_name = font_name;
	      
		  /* following is in effect for the entire plot */
		  initialize_plotter(display_type,
				     save_screen, /* for open_plotter() only */
				     bg_color,
				     frame_line_width, 
				     frame_color,
				     top_label,
				     title_font_name, title_font_size, /* for title */
				     tick_size, grid_spec,
				     final_min_x, final_max_x, final_spacing_x,
				     final_min_y, final_max_y, final_spacing_y,
				     final_spec_spacing_x,
				     final_spec_spacing_y,
				     plot_width, plot_height, 
				     margin_below, margin_left,
				     font_name, font_size, /* for abscissa */
				     x_label, 
				     font_name, font_size, /* for ordinate */
				     y_label,
				     no_rotate_y_label,
				     /* these args are portmanteaux */
				     final_log_axis, 
				     final_round_to_next_tick,
				     switch_axis_end,
				     omit_ticks, 
				     /* more args */
				     clip_mode,
				     blankout_fraction,
				     final_transpose_axes);

		  if (first_plot)
		    /* need to open the plotter */
		    {
		      if (open_plotter() < 0)
			{
			  fprintf (stderr, 
				   "%s: error: couldn't open plot device\n", 
				   progname);
			  return 1;
			}
		    }
		  
		  /* push a plot-specific drawing state onto libplot's
                     stack of drawing states */
		  savestate();

		  /* concatenate the current transformation matrix with a
		     matrix formed from the repositioning parameters (this
		     will take effect for the duration of the plot) */
		  fconcat (reposition_scale, 0.0, 
			   0.0, reposition_scale,
			   reposition_trans_x * PLOT_SIZE, 
			   reposition_trans_y * PLOT_SIZE);
	      
		  /* draw the plot frame (grid, ticks, etc.); draw a
		     `canvas' (a background opaque white rectangle) only if
		     this isn't the first plot */
		  plot_frame (first_plot ? false : true);
		  
		  initialize_reader (input_type,
				     auto_abscissa, delta_x, x_start,
				     /* following three are plot-specific */
				     final_transpose_axes, 
				     final_log_axis, auto_bump,
				     /* following args are file-specific
					(they set dataset attributes) */
				     symbol_index, symbol_size,
				     symbol_font_name,
				     linemode_index, plot_line_width, 
				     fill_fraction, use_color);
		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
	      else	/* not first file of plot, but do some things anyway */
		{
		  /* set reader parameters that may change when we move
		     from file to file within a plot */
		  set_reader_parameters (input_type,
					 auto_abscissa, delta_x, x_start,
					 /* following args set dataset 
					    attributes */
					 symbol_index, symbol_size, 
					 symbol_font_name,
					 linemode_index, plot_line_width, 
					 fill_fraction, use_color,
					 /* following bools make up a mask*/
					 new_symbol, new_symbol_size,
					 new_symbol_font_name,
					 new_linemode, new_plot_line_width, 
					 new_fill_fraction, new_use_color);

		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
    
	      /* call read_and_plot_file() on the file; each dataset in the
		 file yields a polyline */
	      read_and_plot_file (data_file);

	    } /* end of filter case */
	  
	  else	/* not acting as filter */

	    /* Luser didn't specify enough information for us to act as a
	       filter, so we do things the hard way: we call read_file() on
	       each file to create a points array, and at the end of the
	       plot we'll call plot_point_array() on the array.  For now,
	       we don't even call initialize_plotter(). */
	    {
	      if (first_file_of_plot)	/* some additional initializations */
		{
		  p = (Point *)xmalloc (points_length * sizeof (Point));
		  
		  initialize_reader (input_type, 
				     auto_abscissa, delta_x, x_start,
				     /* following three are plot-specific */
				     final_transpose_axes, 
				     final_log_axis, auto_bump,
				     /* following args are file-specific
					(they set dataset attributes) */
				     symbol_index, symbol_size,
				     symbol_font_name,
				     linemode_index, plot_line_width, 
				     fill_fraction, use_color);
		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
	      else	/* not first file of plot, but do some things anyway */
		{
		  /* set reader parameters that may change when we move
		     from file to file within a plot */
		  set_reader_parameters (input_type, 
					 auto_abscissa, delta_x, x_start,
					 /* following args set dataset 
					    attributes */
					 symbol_index, symbol_size, 
					 symbol_font_name,
					 linemode_index, plot_line_width, 
					 fill_fraction, use_color,
					 /* following bools make up a mask*/
					 new_symbol, new_symbol_size,
					 new_symbol_font_name,
					 new_linemode, new_plot_line_width, 
					 new_fill_fraction, new_use_color);

		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
	      
	      /* add points to points array by calling read_file() on file */
	      read_file (data_file, &p, &points_length, &no_of_points);

	    } /* end of not-filter case */

	  /* close file */
	  if (data_file != stdin)
	    close_file (optarg, data_file);

	  first_file_of_plot = false;
	  break;		/* end of `case 1' in switch() */
	  
	  /*---------------- End of options ----------------*/

	default:		/* Default, unknown option */
	  errcnt++;
	  continue_parse = false;
	  break;
	}			/* end of switch() */

      if (errcnt > 0)
	continue_parse = false;
    }				/* end of while loop */
  
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

  /* At this point, we need to terminate the plot currently in progress, if
     it's nonempty (zero or more files could have been plotted).  */

  if (!first_file_of_plot)
    {
      /* If we're acting as a real-time filter, then the plot is already
	 drawn on the display and there's nothing for us to do.  But if
	 not, we need to draw the plot. */
      if (!filter)
	{

	  /* fill in any of min_? and max_? that user didn't specify (the
	     prefix "final_" means these arguments were finalized at the
	     time the first file of the plot was processed) */
	  array_bounds (p, no_of_points, final_transpose_axes,
			&final_min_x, &final_min_y, &final_max_x, &final_max_y,
			final_spec_min_x, final_spec_min_y, 
			final_spec_max_x, final_spec_max_y);
	  
	  /* font selection, saves typing */
	  if ((title_font_name == NULL) && (font_name != NULL))
	    title_font_name = font_name;
	      
	  initialize_plotter(display_type, 
			     save_screen, /* for open_plotter() only */
			     bg_color,
			     frame_line_width,
			     frame_color,
			     top_label,
			     title_font_name, title_font_size, /* for title */
			     tick_size, grid_spec,
			     final_min_x, final_max_x, final_spacing_x,
			     final_min_y, final_max_y, final_spacing_y,
			     final_spec_spacing_x,
			     final_spec_spacing_y,
			     plot_width, plot_height, margin_below, margin_left,
			     font_name, font_size, /* for abscissa label */
			     x_label, 
			     font_name, font_size, /* for ordinate label */
			     y_label,
			     no_rotate_y_label,
			     /* these args are portmanteaux */
			     final_log_axis,
			     final_round_to_next_tick,
			     switch_axis_end, omit_ticks, 
			     /* more args */
			     clip_mode,
			     blankout_fraction,
			     final_transpose_axes);
	  
	  if (first_plot)
	    /* still haven't opened plotter, do so now */
	    {
	      if (open_plotter() < 0)
		{
		  fprintf (stderr, 
			   "%s: error: couldn't open plot device\n", progname);
		  return 1;
		}
	    }
	  
	  /* push new libplot drawing state onto stack of states */
	  savestate();
	  
	  /* concatenate the current transformation matrix with a
	     matrix formed from the repositioning parameters */
	  fconcat (reposition_scale, 0.0, 
		   0.0, reposition_scale,
		   reposition_trans_x * PLOT_SIZE, 
		   reposition_trans_y * PLOT_SIZE);
	  
	  /* draw the plot frame (grid, ticks, etc.); draw a `canvas' (a
	     background opaque white rectangle) only if this isn't the
	     first plot */
	  plot_frame((first_plot == true ? false : true));
	  
	  /* plot the laboriously read-in array */
	  plot_point_array (p, no_of_points);
	  
	  /* free points array */
	  free (p);
	  no_of_points = 0;

	} /* end of not-filter case */

      /* pop plot-specific drawing state off the stack of drawing states */
      restorestate();

    } /* end of nonempty-plot case */
  
  if (close_plotter() < 0)
    {
      fprintf (stderr, "%s: error: could not close plot device\n", 
	       progname);
      return 1;
    }
  
  return 0;
}


static void
#ifdef _HAVE_PROTOS
open_file_for_reading (char *filename, FILE **input)
#else
open_file_for_reading (filename, input)
     char *filename;
     FILE **input;
#endif
{
  FILE *data_file;
		
  data_file = fopen (filename, "r");
  if (data_file == NULL)
    {
      fprintf (stderr, "%s: %s: %s\n", progname, filename, strerror(errno));
      exit (1);
    }
  else
    *input = data_file;
}  

static void
#ifdef _HAVE_PROTOS
close_file (char *filename, FILE *stream)
#else
close_file (filename, stream)
     char *filename;
     FILE *stream;
#endif
{
  if (fclose (stream) < 0)
    fprintf (stderr, 
	     "%s: could not close input file `%s'\n", 
	     progname, filename);
}

static bool
#ifdef _HAVE_PROTOS
parse_pen_string (const char *pen_s)
#else
parse_pen_string (pen_s)
     const char *pen_s;
#endif
{
  const char *charp;
  char name[MAX_COLOR_NAME_LEN];
  int i;

  charp = pen_s;
  while (*charp)
    {
      int pen_num;
      bool got_digit;
      const char *tmp;

      if (*charp == ':')	/* skip any ':' */
	{
	  charp++;
	  continue;		/* back to top of while loop */
	}
      pen_num = 0;
      got_digit = false;
      while (*charp >= '0' && *charp <= '9')
	{
	  pen_num = 10 * pen_num + (int)*charp - (int)'0';
	  got_digit = true;
	  charp++;
	}
      if (!got_digit || pen_num < 1 || pen_num > NO_OF_LINEMODES)
	return false;
      if (*charp != '=')
	return false;
      charp++;
      for (tmp = charp, i = 0; i < MAX_COLOR_NAME_LEN; tmp++, i++)
	{
	  if (*tmp == ':') /* end of color name string */
	    {
	      name[i] = '\0';
	      charp = tmp + 1;
	      break;
	    }
	  else if (*tmp == '\0') /* end of name string and env var also */
	    {
	      name[i] = '\0';
	      charp = tmp;
	      break;
	    }
	  else
	    name[i] = *tmp;
	}

      /* replace pen color name by user-specified color name */
      colorstyle[pen_num - 1] = xstrdup (name);
    }
  return true;
}
