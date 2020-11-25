extern const char *progname;	/* program name */

/* Definition of the Point structure.  The point-reader (in reader.c)
   returns a list of these from a specified input stream, and the
   multigrapher (in plotter.c) interprets them as polyline vertices, and
   plots the resulting polyline(s).  Each polyline comprises a run of
   points, each (except the first) connected to the previous point,
   provided that `pendown' is true.  The final seven fields should be the
   same for each point in a polyline. */

typedef struct
{
  double x, y;	    /* location of the point in user coordinates */
  bool have_x_errorbar, have_y_errorbar;
  double xmin, xmax; /* meaningful only if have_x_errorbar field is set */
  double ymin, ymax; /* meaningful only if have_y_errorbar field is set */
  bool pendown;  /* connect to previous point? (if false, polyline ends) */
  /* following fields are polyline attributes: constant over a polyline */
  int symbol;	    /* either a number indicating which standard marker
		     symbol is to be plotted at the point (<0 means none)
		     or an character to be plotted, depending on the value:
		     0-31: a marker number, or 32-up: a character. */
  double symbol_size;	/* symbol size, as frac. of size of plotting area */
  const char *symbol_font_name; /* font from which symbols >= 32 are taken */
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

/* style of graph frame; the 1st four of these are increasingly fancy, but
   the last (AXES_AT_ORIGIN) is an altogether different style */
typedef enum
{
  NO_AXES = 0, AXES = 1, AXES_AND_BOX = 2, AXES_AND_BOX_AND_GRID = 3, AXES_AT_ORIGIN = 4
} grid_type;

#define NORMAL_AXES(grid_spec) \
((grid_spec == AXES) || (grid_spec == AXES_AND_BOX) \
 || (grid_spec == AXES_AND_BOX_AND_GRID))

/* bit fields in portmanteau variables */
enum { X_AXIS = 0x1, Y_AXIS = 0x2 };

#define NO_OF_LINEMODES 5	/* see linemode.c */
#define MAX_COLOR_NAME_LEN 32	/* long enough for all of libplot's colors */

/* types of line */
extern const char *linemodes[NO_OF_LINEMODES];
extern const char *colorstyle[NO_OF_LINEMODES];

/*------------prototypes for libcommon functions----------------------------*/

extern char * xstrdup ____P((const char *s));
extern int display_fonts ____P((const char *display_type, const char *progname));
extern int list_fonts ____P((const char *display_type, const char *progname));
extern void display_usage ____P((const char *progname, const int *omit_vals, const char *appendage, bool fonts));
extern void display_version ____P((const char *progname));
extern voidptr_t xmalloc ____P ((size_t length));
extern voidptr_t xrealloc ____P ((voidptr_t p, size_t length));

/*----------------- prototypes for functions in plotter.h -------------------*/

typedef struct MultigrapherStruct Multigrapher;

extern Multigrapher * new_multigrapher ____P((const char *display_type, const char *bg_color, const char *bitmap_size, const char *max_line_length, const char *meta_portable, const char *page_size, const char *rotation_angle, bool save_screen));

extern int delete_multigrapher ____P((Multigrapher *multigrapher));

extern void begin_graph ____P((Multigrapher *multigrapher, double scale, double trans_x, double trans_y));

extern void end_graph ____P((Multigrapher *multigrapher));

extern void set_graph_parameters ____P((Multigrapher *multigrapher, double frame_line_width, const char *frame_color, const char *title, const char *title_font_name, double title_font_size, double tick_size, grid_type grid_spec, double x_min, double x_max, double x_spacing, double y_min, double y_max, double y_spacing, bool spec_x_spacing, bool spec_y_spacing, double width, double height, double up, double right, const char *x_font_name, double x_font_size, const char *x_label, const char *y_font_name, double y_font_size, const char *y_label, bool no_rotate_y_label, int log_axis, int round_to_next_tick, int switch_axis_end, int omit_labels, int clip_mode, double blankout_fraction, bool transpose_axes));

extern void draw_frame_of_graph ____P((Multigrapher *multigrapher, bool draw_canvas));

extern void plot_point ____P((Multigrapher *multigrapher, const Point *point));

extern void plot_point_array ____P((Multigrapher *multigrapher, const Point *p, int length));

extern void end_polyline_and_flush ____P((Multigrapher *multigrapher));

/*----------------- prototypes for functions in reader.h -------------------*/

typedef struct ReaderStruct Reader;

extern Reader * new_reader ____P((FILE *input, data_type input_type, bool auto_abscissa, double delta_x, double abscissa, bool transpose_axes, int log_axis, bool auto_bump, int symbol, double symbol_size, const char *symbol_font_name, int linemode, double line_width, double fill_fraction, bool use_color));

extern void delete_reader ____P((Reader *reader));

extern void read_file ____P((Reader *reader, Point **p, int *length, int *no_of_points));

extern void read_and_plot_file ____P((Reader *reader, Multigrapher *multigrapher));

extern void alter_reader_parameters ____P((Reader *reader, FILE *input, data_type input_type, bool auto_abscissa, double delta_x, double abscissa, int symbol, double symbol_size, const char *symbol_font_name, int linemode, double line_width, double fill_fraction, bool use_color, bool new_symbol, bool new_symbol_size, bool new_symbol_font_name, bool new_linemode, bool new_line_width, bool new_fill_fraction, bool new_use_color));

/*----------------- prototypes for functions in misc.h -------------------*/

extern void array_bounds ____P((const Point *p, int length, bool transpose_axes, int clip_mode, double *min_x, double *min_y, double *max_x, double *max_y, bool spec_min_x, bool spec_min_y, bool spec_max_x, bool spec_max_y));

/*------------------------------------------------------------------------*/
