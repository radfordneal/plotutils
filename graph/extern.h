#define PLOT_SIZE 4096.0	/* we use floating point libplot
				   coordinates in the range [0,PLOT_SIZE] */

/* Definition of the Point structure.  The point-reader (in reader.c)
   returns a stream of these structures, and the point-plotter (in
   plotter.c) interprets them as polyline vertices, and plots the resulting
   polyline(s).  Each polyline comprises a run of points, each (except the
   first) connected to the previous point.  The final seven fields should
   be the same for each point in a polyline. */

typedef struct
{
  double x, y;	    /* location of the point in user coordinates */
  bool have_x_errorbar, have_y_errorbar;
  double xmin, xmax; /* meaningful only if have_x_errorbar field is set */
  double ymin, ymax; /* meaningful only if have_y_errorbar field is set */
  bool pendown;  /* connect to previous point? */
  /* following fields are polyline attributes: constant over a polyline */
  int symbol;	    /* either a number indicating which standard marker
		     symbol is to be plotted at the point (<0 means none)
		     or an character to be plotted, depending on the value:
		     0-31: a marker number, or 32-up: a character. */
  double symbol_size;	/* symbol size, as frac. of size of plotting area */
  char *symbol_font_name; /* font from which symbols >= 32 are taken */
  int linemode;		/* linemode of polyline (<0 means no polyline) */
  double line_width;	/* line width as fraction of size of the display */
  double fill_fraction;	/* in interval [0,1], <0 means polyline isn't filled */
  bool use_color;	/* color/monochrome interpretation of linemode */
} Point;

/* type of data in input stream */
typedef enum
{
  T_ASCII, T_SINGLE, T_DOUBLE, T_INTEGER, T_GNUPLOT, T_ASCII_ERRORBAR
} data_type;

/* style of plot frame; the 1st four of these are increasingly fancy, but
   the last (AXES_AT_ORIGIN) is an altogether different style */
typedef enum
{
  NO_AXES, AXES, AXES_AND_BOX, AXES_AND_BOX_AND_GRID, AXES_AT_ORIGIN
} grid_type;

#define NORMAL_AXES(grid_spec) \
((grid_spec == AXES) || (grid_spec == AXES_AND_BOX) \
 || (grid_spec == AXES_AND_BOX_AND_GRID))

/* bit fields in portmanteau variables */
enum { X_AXIS = 0x1, Y_AXIS = 0x2 };

typedef struct
{
  int red;
  int green;
  int blue;
} Color;

#define NO_OF_LINEMODES 5	/* see linemode.c */
#define MAX_COLOR_NAME_LEN 32	/* long enough for all known colors */

/* types of line */
extern char *linemodes[NO_OF_LINEMODES];
extern char *colorstyle[NO_OF_LINEMODES];

extern const char	*progname; /* Program name */

/*------------prototypes for libcommon functions----------------------------*/

extern bool
display_fonts __P((const char *display_type, const char *progname));

extern bool
list_fonts __P((const char *display_type, const char *progname));

extern void 
display_usage __P((const char *progname, const int *omit_vals, const char *appendage, bool fonts));

extern void 
display_version __P((const char *progname));

extern Voidptr 
xmalloc __P ((unsigned int length));

extern Voidptr 
xrealloc __P ((Voidptr p, unsigned int length));

extern char * 
xstrdup __P((const char *s));

/*--------------------------------prototypes--------------------------------*/

extern void 
array_bounds __P((const Point *p, int length, bool transpose_axes, 
		  double *min_x, double *min_y,
		  double *max_x, double *max_y, bool spec_min_x, bool
		  spec_min_y, bool spec_max_x, bool spec_max_y));

extern void
read_file __P((FILE *input, Point **p, int *length, int *no_of_points));

extern void 
read_and_plot_file __P((FILE *input));

extern void
set_reader_parameters __P((data_type input_type, bool auto_abscissa, double
			   delta_x, double abscissa, 
			   int symbol, double symbol_size, 
			   char *symbol_font_name, 
			   int linemode, double line_width, 
			   double fill_fraction, bool use_color,
			   bool new_symbol, bool new_symbol_size, 
			   bool new_symbol_font_name,
			   bool new_linemode, bool new_line_width,
			   bool new_fill_fraction, bool new_use_color));

extern void
initialize_reader __P((data_type input_type, bool auto_abscissa, double
		       delta_x, double abscissa, bool transpose_axes, 
		       int log_axis, bool auto_bump, 
		       int symbol, double symbol_size, 
		       char *symbol_font_name, int linemode, 
		       double line_width, double fill_fraction, bool use_color));

extern void
reset_reader __P((void));

extern void
set_plotter_parameters __P ((double plot_line_width, char *point_label_font_name));

extern void
initialize_plotter __P((char *display_type,
			bool save_screen, 
			char *bg_color,
			double frame_line_width,
			char *frame_color,
			char *title, char *title_font_name, 
			double title_font_size,
			double tick_size, grid_type grid_spec, 
			double x_min, double x_max, double x_spacing, 
			double y_min, double y_max, double y_spacing, 
			bool spec_x_spacing, 
			bool spec_y_spacing, 
			double width, double height, double up, double right, 
			char *x_font_name, double x_font_size, char *x_label,
			char *y_font_name, double y_font_size, 
			char *y_label, bool no_rotate_y_label,
			int log_axis,
			int round_to_next_tick,
			int switch_axis_end, int omit_labels, 
			int clip_mode,
			double blankout_fraction,
			bool transpose_axes));

extern void
plot_frame __P((bool draw_canvas));

extern void
plot_point_array __P((const Point *p, int length));

extern void
plot_point __P((const Point *point));

extern int
open_plotter __P((void));

extern int
close_plotter __P((void));
