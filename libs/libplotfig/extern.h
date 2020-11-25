/* Besides definitions, this file contains external declarations for the
   private variables and low-level functions of libplotfig.  We attempt to
   keep namespace clean by beginning the name of each variable appearing in
   an external declaration with an underscore. */

/* The plot will be scaled equally in both directions, so as to fit (just)
   within a specified rectangular region.  The scaled plot will be placed
   in the upper left corner of this region.  This is arranged in space.c.

   The following macros define our rectangular region, in device (xfig)
   coordinates.  We define it to be an 8.5"x8.5" square, in the upper left
   corner of the xfig display.  So the scaled plot will touch the (0,0)
   corner in xfig space. */

#define FIG_RESOLUTION 1200	/* Fig units/inch, in version 3.0+ */

/* Our rectangular region is 8.5" by 8.5", for which pixel range is
   [0,10200]x[0,10200]; y increases downward. */

#define DEVICE_X_LEFT IROUND(0.0*FIG_RESOLUTION)     
#define DEVICE_X_RIGHT (IROUND(8.5*FIG_RESOLUTION))  
#define DEVICE_Y_TOP IROUND(0.0*FIG_RESOLUTION)      
#define DEVICE_Y_BOTTOM (IROUND(8.5*FIG_RESOLUTION)) 

/* a struct specifying a user->device coordinate transformation (each
   drawing state has such a struct as an element) */
typedef struct 
{ 
  /* the two components of an affine transformation */
  double m[6];			/* 1. a linear transformation (4 elements) */
  				/* 2. a translation (2 elements) */
  /* related data kept here for convenience */
  Boolean uniform;		/* whether transf. scaling is uniform */
  Boolean axes_preserved;	/* whether transf. preserves axis directions */
  Boolean is_raster;		/* does device use ints or floats as coors? */
} Transform;

/* X Device: transform user x coordinate to device x coordinate */
#define XD(x,y) (_drawstate->transform.m[4] + (x) * _drawstate->transform.m[0] + (y) * _drawstate->transform.m[2])

/* Y Device: transform user y coordinate to device y coordinate */
#define YD(x,y) (_drawstate->transform.m[5] + (x) * _drawstate->transform.m[1] + (y) * _drawstate->transform.m[3])

/* X Device Vector: transform user vector to device vector's x component */
#define XDV(x,y) (_drawstate->transform.m[0] * (x) + _drawstate->transform.m[2] * (y))

/* Y Device Vector: transform user vector to device vector's y component */
#define YDV(x,y) (_drawstate->transform.m[1] * (x) + _drawstate->transform.m[3] * (y))

/* device units <-> printer's points; number of points per inch == 72 */
#define FIG_UNITS_TO_POINTS(size) ((size)*72.0/FIG_RESOLUTION)
#define POINTS_TO_FIG_UNITS(size) ((size)*FIG_RESOLUTION/72.0)

/* xfig specifies line widths in `Fig display units' rather than `Fig units'
   (there are 80 of the former per inch). */
#define FIG_UNITS_TO_FIG_DISPLAY_UNITS(width) ((width)*80.0/FIG_RESOLUTION)

/* For historical reasons, xfig scales the fonts down by a factor
   FONT_SCALING, i.e., (80.0)/(72.0).  So we have to premultiply font sizes
   by the same factor.  The current release of xfig unfortunately can't
   handle font sizes that aren't integers, so it rounds them.  Ah well. */
#define FIG_FONT_SCALING ((80.0)/(72.0))

/* Maximum number of times an arc is recursively subdivided, when it is
   being approximated by an inscribed polyline.  The polyline will contain
   no more than 2**MAX_ARC_SUBDIVISIONS line segments.  MAX_ARC_SUBDIVISIONS
   must be no longer than TABULATED_ARC_SUBDIVISIONS below (see fakearc.h)*/
#define MAX_ARC_SUBDIVISIONS 8

/* Types of arc (which may or may not be drawn via recursive subdivision).
   These index into the table in fakearc.h. */
#define NUM_ARC_TYPES 4

#define QUARTER_ARC 0
#define HALF_ARC 1
#define THREE_QUARTER_ARC 2
#define USER_DEFINED_ARC 3

#define TABULATED_ARC_SUBDIVISIONS 15	/* length of each table entry */

/* Default length an unfilled polyline is allowed to grow to, before it is
   flushed out (we put no [small] maximum on the length of filled
   polylines, since they need to be preserved as discrete objects if the
   filling is to be performed properly).  This may be changed by the user,
   via the external variable _libplot_max_unfilled_polyline_length. */

#define MAX_UNFILLED_POLYLINE_LENGTH 150

/* Hard limit on length of all polylines (not to be changed by user). */
extern long int _hard_polyline_length_limit;

/* whether or not to suppress the just-mentioned flushout (normally FALSE) */

extern Boolean _suppress_polyline_flushout;

/* Fig line styles.  DOTTED and DASHED line styles are specified by
   (respectively) the length of the gap between successive dots, and the
   length of each dash (equal to the length of the gap between successive
   dashes).  */

#define L_DEFAULT (-1)
#define L_SOLID 0
#define L_DASHED 1
#define L_DOTTED 2

/* Fig join styles */
#define JOIN_MITER 0
#define JOIN_ROUND 1
#define JOIN_BEVEL 2

/* Fig cap styles */
#define CAP_BUTT 0
#define CAP_ROUND 1
#define CAP_PROJECT 2

/* our information about the 35 standard fonts in fontdb.c, and the
   typefaces they belong to */

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

/* our information about the vector fonts in fontdb.c, and the typefaces
   they belong to */

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

/* accented character information (used in constructing Hershey ISO-Latin-1
   accented characters, see table in fontdb.c) */

struct accented_char_info_struct
{
  unsigned char composite, character, accent;
};

extern struct accented_char_info_struct _vector_accented_char_info[];

/* types of accent, for a composite character */

#define ACC0 (16384 + 0)	/* superimpose on character */
#define ACC1 (16384 + 1)	/* elevate by 7 Hershey units */
#define ACC2 (16384 + 2)	/* same, also shift right by 2 units */

/* a flag in the glyph number indicating a small Kana */

#define KS 8192

/* arrays of Hershey vector glyphs */

extern char *_occidental_vector_glyphs[];
extern char *_oriental_vector_glyphs[];

/* typeface information */

#define FONTS_PER_TYPEFACE 10

struct typeface_info_struct
{
  int numfonts;
  int fonts[FONTS_PER_TYPEFACE];
};

extern struct typeface_info_struct _ps_typeface_info[];
extern struct typeface_info_struct _vector_typeface_info[];

/* control codes (used internally when rendering a label) */

/* the order of these must agree with the order in control.h */
#define C_BEGIN_SUPERSCRIPT 0
#define C_END_SUPERSCRIPT 1
#define C_BEGIN_SUBSCRIPT 2
#define C_END_SUBSCRIPT 3
#define C_PUSH_LOCATION 4
#define C_POP_LOCATION 5
#define C_RIGHT_ONE_EM 6
#define C_RIGHT_HALF_EM 7
#define C_RIGHT_QUARTER_EM 8
#define C_RIGHT_SIXTH_EM 9
#define C_RIGHT_EIGHTH_EM 10
#define C_LEFT_ONE_EM 11
#define C_LEFT_HALF_EM 12
#define C_LEFT_QUARTER_EM 13
#define C_LEFT_SIXTH_EM 14
#define C_LEFT_EIGHTH_EM 15

#define C_RIGHT_RADICAL_SHIFT 254	/* for \rn in PS fonts */
#define C_LEFT_RADICAL_SHIFT 255
#define PS_RADICAL_WIDTH 0.515
#define RADICALEX 96

/* flags in each unsigned short in a `controlified' text string (mutually
   exclusive) */
#define CONTROL_CODE 0x8000
#define RAW_HERSHEY_GLYPH 0x4000
#define RAW_ORIENTAL_HERSHEY_GLYPH 0x2000

/* masks for extracting, from an unsigned short in a controlified text string,
   (1) the font, if none of the above flags is set, or (2) the raw glyph
   number, if either of the latter two flags is set */
#define ONE_BYTE 0xff
#define FONT_SHIFT 8
#define FONT_SPEC (ONE_BYTE << FONT_SHIFT)
#define GLYPH_SPEC 0x1fff

/* whether or not we're inside an openpl()...closepl() pair */

extern Boolean _grdevice_open;

/* An output buffer that may easily be resized; see outbuf.c.  This is
   needed only because xfig insists that user-defined colors [``color
   pseudo-objects''] be defined first in the .fig file.  So we need to
   output them first, which implies that we need to cache all other
   [genuine] objects.  Hence the need for a resizable buffer. */

typedef struct
{
  char *base;
  int len;
  char *current;
  int contents;
}
Outbuffer;

extern Outbuffer _outbuf;

/* Data structures for points and vectors */

typedef struct pointstruct
{
  double x,y;
} Point;

typedef Point Vector;

/* libplot supports 48-bit color, but xfig only supports 24-bit color.
   This structure will be used for storing both sorts. */
typedef struct
{
  int red;
  int green;
  int blue;
} Color;

/* structure used in colorname.c for storing names of colors we recognize */

typedef struct
{
  char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Colornameinfo;

/* number of Fig colors defined by user, and stored in usercolors[] */

extern int _num_usercolors;

extern long int _usercolors[];

/* these constants for Fig colors are hardcoded in xfig */

#define FIG_STD_COLOR_MIN 0	/* see colordb2.c for colors 0,...,31 */
#define C_BLACK 0		/* i.e. #0 in table in colordb2.c */
#define C_WHITE 7		/* i.e. #7 in table */
#define FIG_NUM_STD_COLORS 32
extern Color _fig_stdcolors[FIG_NUM_STD_COLORS];

#define FIG_USER_COLOR_MIN 32
#define FIG_MAX_NUM_USER_COLORS 512

/* by setting this undocumented variable, user may request quantization of
   colors (no user-defined colors, only native xfig ones). */
extern int _libplotfig_use_pseudocolor;

/* Font types */
typedef enum 
{ 
  F_STANDARD, F_STROKE, F_DEVICE_SPECIFIC
} font_type;

/* Drawing priority for each of the object types known to xfig.  At any
   specified value of xfig's `depth' attribute, objects with a lower
   priority value are drawn first.  This will make them vulnerable to being
   obscured by higher-priority objects, which are drawn later.  See
   DEPTH.doc. */
#define ARC_PRIORITY 1
#define COMPOUND_PRIORITY 2
#define ELLIPSE_PRIORITY 3
#define POLYLINE_PRIORITY 4
#define SPLINE_PRIORITY 5
#define TEXT_PRIORITY 6

/* xfig's depth attribute ranges from 0 to FIG_MAXDEPTH.  See DEPTH.doc. */
#define FIG_MAXDEPTH 999

/* depth of the first object we'll draw (we make it a bit less than
   FIG_MAXDEPTH, since the user may wish to edit the drawing with xfig to
   include deeper, i.e. obscured objects) */
#define FIG_INITIAL_DEPTH 989

/* Drawing state.  Includes drawing attributes, and the state of
   any uncompleted polyline object. */
typedef struct lib_state
{
/* affine transformation from user coordinates to device coordinates */
  Transform transform;		/* see definition of structure above */
/* graphics cursor position */
  Point pos;			/* graphics cursor position */
/* the state of any uncompleted polyline object */
  Point *datapoints;		/* array of accumulated points */
  int PointsInLine;		/* number of points accumulated */
  int datapoints_len;		/* length of point storage buffer (bytes) */
/* modal drawing attributes */
  char *line_mode;		/* line mode */
  char *join_mode;		/* join mode */
  char *cap_mode;		/* cap mode */
  double line_width;		/* width of lines in user coordinates */
  int fill_level;		/* fill level */
  char *font_name;		/* font name */
  double font_size;		/* font size in user coordinates */
  font_type font_type;		/* F_STANDARD / F_STROKE / F_DEVICE_SPECIFIC */
  int typeface_index;		/* typeface index (in fontdb.h table) */
  int font_index;		/* font index, within typeface */
  Boolean font_is_iso8859;	/* whether font has an iso8859 character set */
  double text_rotation;		/* degrees counterclockwise, for labels */
  Color fgcolor;		/* foreground color */
  Color fillcolor;		/* fill color */
  Boolean points_are_connected;	/* if not set, polyline displayed as points */
/* device-specific attributes */
  double fig_quantized_font_size; /* quantized version of fig_size */
  int fig_font_point_size;	/* font size in fig's idea of points */
  int fig_line_style;		/* fig's line style */
  int fig_join_style;		/* fig's join style */
  int fig_cap_style;		/* fig's cap style */
  double fig_dash_length;	/* fig's dash length */
  int fig_fill_level;		/* fig's fill level */
  Boolean fig_fgcolor_defined;	/* whether fig's fg color is evaluated */
  Boolean fig_fillcolor_defined;/* whether fig's fill color is evaluated */  
  int fig_fgcolor;		/* fig's foreground color */
  int fig_fillcolor;		/* fig's fill color */
  int fig_drawing_depth;	/* fig's current value for `depth' attribute */
  int fig_last_priority;	/* fig's drawing priority for last-drawn object */
/* pointer to previous state */
  struct lib_state *previous;
} State;

/* default drawing state, and pointer to state at head of linked list */

extern State _default_drawstate, *_drawstate;

/* initializations for default values of state variables when space() is
   called */

#define DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH (14.0/(8.5*72.0))
#define DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH (1.0/(8.5*100.0))

/* the output stream for all plot commands */

extern FILE *_outstream;

/* This elides the argument prototypes if the compiler does not support
   them. The name protarg is chosen in hopes that it will not collide with
   any others. */

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif

extern Boolean _clean_iso_string P__ ((unsigned char *s));
extern Vector *_vscale P__ ((Vector *v, double newlen));
extern Voidptr _plot_xcalloc P__ ((unsigned int nmemb, unsigned int size));
extern Voidptr _plot_xmalloc P__ ((unsigned int size));
extern Voidptr _plot_xrealloc P__ ((Voidptr p, unsigned int size));
extern double _alabel_device P__((int x_justify, int y_justify, const unsigned short *s));
extern double _alabel_standard P__((int x_justify, int y_justify, const unsigned short *s));
extern double _alabel_stroke P__((int x_justify, int y_justify, const unsigned short *s));
extern double _labelwidth_device P__((const unsigned short *s));
extern double _labelwidth_standard P__((const unsigned short *s));
extern double _labelwidth_stroke P__((const unsigned short *s));
extern double _matrix_norm P__((const double m[6]));
extern double _xatan2 P__((double y, double x));
extern int _codestring_len P__((const unsigned short *codestring));
extern int _device_line_width P__((void));
extern int _fig_color P__ ((int red, int green, int blue));
extern unsigned short * _controlify P__((const unsigned char *));
extern void _draw_circular_arc P__((Point p0, Point p1, Point pc));
extern void _draw_ellipse P__((double xc, double yc, double rx, double ry, double angle));
extern void _draw_elliptic_arc P__((Point p0, Point p1, Point pc));
extern void _evaluate_fig_fgcolor P__ ((void));
extern void _evaluate_fig_fillcolor P__ ((void));
extern void _fakearc P__ ((Point p0, Point p1, int arc_type, const double m[4]));
extern void _initialize_buffer P__ ((Outbuffer *bufp));
extern void _matrix_product P__ ((const double m[6], const double n[6], double product[6]));
extern void _prepare_chord_table P__ ((double sagitta));
extern void _reset_buffer P__((Outbuffer *bufp));
extern void _set_font_sizes P__((void));
extern void _update_buffer P__((Outbuffer *bufp));

#undef P__
