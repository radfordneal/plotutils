/* Besides definitions, this file contains external declarations for the
   private variables and low-level functions of libplotps.  We attempt to
   keep namespace clean by beginning the name of each variable appearing in
   an external declaration with an underscore. */

/* The plot will be scaled equally in both directions, so as to fit (just)
   within a specified rectangular region.  The scaled plot will be placed
   in the lower left corner of this region.  This is arranged in space.c.

   The following macros define the rectangular region we'll use.  We define
   it to be an 8.5" by 8.5" square, centered on an 8.5" by 11" page. */

/* SCALING is the scaling in the global current transformation matrix (CTM)
   for the entire plot.  For the PS device, 1 device unit will equal
   SCALING points.

   Don't change this value for SCALING.  In principle, reducing it would
   make the drawing of thinner lines possible (1 device unit being our
   minimum line width, for compatibility with idraw).  Unfortunately idraw
   does not seem to pay attention to the global CTM at the head of the
   output file.  So changing SCALING will introduce linewidth-related
   incompatibilities between what a PS device will show, and what idraw
   will show. */

#define SCALING 1.0

#define DEVICE_UNITS_TO_POINTS(devunits) ((devunits)*(SCALING))

/* 0.0 in <= X <= 8.5 in, pseudo-pixel range [0,612], physical range=8.5
   in (the entire width of a standard US page) */

#define DEVICE_X_LEFT (0.00 * 72. / (SCALING)) /* min output x coordinate */
#define DEVICE_X_RIGHT (8.50 * 72. / (SCALING)) /* max output x coordinate */

/* 1.25 in <= Y <= 9.75 in, pseudo-pixel range [90,702], physical
   range=8.5 in, centered in interval [0.0 in,11.0 in] */
#define DEVICE_Y_BOTTOM (1.25 * 72. / (SCALING)) /* min output y coordinate */
#define DEVICE_Y_TOP (9.75 * 72. / (SCALING)) /* max output y coordinate */

/* Because idraw insists that point coordinates be in terms of integers,
   for polylines and ellipses we apply a per-object CTM which is scaled
   down considerably, so that the point coordinates (which are
   correspondingly scaled up) can be rounded to the closest integer without
   much loss of precision.  [Would that we could do the same with
   linewidths...]  This is the scaling factor we use. */
#define GRANULARITY 100.0

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

/* Since PS isn't really a raster device, despite the idraw-related
   quantizations, the user->device coordinate transformation macros below
   are supplied in floating point versions only.  */

/* X Device: transform user x coordinate to device x coordinate */
#define XD(x,y) (_drawstate->transform.m[4] + (x) * _drawstate->transform.m[0] + (y) * _drawstate->transform.m[2])

/* Y Device: transform user y coordinate to device y coordinate */
#define YD(x,y) (_drawstate->transform.m[5] + (x) * _drawstate->transform.m[1] + (y) * _drawstate->transform.m[3])

/* X Device Vector: transform user vector to device vector's x component */
#define XDV(x,y) (_drawstate->transform.m[0] * (x) + _drawstate->transform.m[2] * (y))

/* Y Device Vector: transform user vector to device vector's y component */
#define YDV(x,y) (_drawstate->transform.m[1] * (x) + _drawstate->transform.m[3] * (y))

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

/* PS line styles (our numbering, indexes into following arrays) */

#define L_SOLID 0
#define L_DOTTED 1
#define L_DOTDASHED 2
#define L_SHORTDASHED 3
#define L_LONGDASHED 4

/* arrays, see linemod.c */

extern long _ps_line_type_bit_vector[]; /* line style for idraw, 16-bit brush */
extern char *_ps_line_type_setdash[]; /* dasharray arg for PS setdash command */

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
#define PS_RADICAL_WIDTH 0.549
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
   needed only because bounding boxes must appear at the head of the output
   file.  So all writes, until the very end, are to the resizable output
   buffer.  After the bounding box line is computed and written to the
   output stream, we copy all text in the output buffer to the output
   stream. */

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

/* Font types */
typedef enum 
{ 
  F_STANDARD, F_STROKE, F_DEVICE_SPECIFIC
} font_type;

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
  int ps_line_type;		/* one of L_* */
  double ps_fgcolor_red;	/* RGB for fgcolor, each in [0.0,1.0] */
  double ps_fgcolor_green;
  double ps_fgcolor_blue;
  double ps_fillcolor_red;	/* RGB for fillcolor, each in [0.0,1.0] */
  double ps_fillcolor_green;
  double ps_fillcolor_blue;
  Boolean ps_fgcolor_defined;	/* whether PS fgcolor has been computed */
  Boolean ps_fillcolor_defined;	/* whether PS fillcolor has been computed */
  int idraw_fgcolor;		/* index of idraw fgcolor in table */
  int idraw_bgcolor;		/* index of idraw bgcolor in table */
  int idraw_shading;		/* index of idraw shading in table */
/* pointer to previous state */
  struct lib_state *previous;
} State;

/* default drawing state, and pointer to state at head of linked list */

extern State _default_drawstate, *_drawstate;

/* initializations for default values of state variables when space() is
   called */

#define DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH (14.0/(8.5*72.0))
#define DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH (1.0/(8.5*100.0))

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

/* information on colors known to idraw, see colordb2.c */

#define IDRAW_NUM_STD_COLORS 12

extern Color _idraw_stdcolors[IDRAW_NUM_STD_COLORS];
extern char *_idraw_stdcolornames[IDRAW_NUM_STD_COLORS];

/* information on shadings known to idraw, see colordb2.c */

#define IDRAW_NUM_STD_SHADINGS 5

extern double _idraw_stdshadings[IDRAW_NUM_STD_SHADINGS];

/* USER_HAS_PROLOGUE is a flag. If it is non-zero then the openpl() routine
   should output the user specified prologue contained in the file
   specified in the string USERS_PROLOGUE, instead of the builtin prologue */

extern int user_has_prologue;

/* USERS_PROLOGUE is a string containing the file name for any user specified
   postscript prologue.  This file is a substitute for the prologue contained
   in the file header.c. */

extern char *users_prologue;

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
extern unsigned short * _controlify P__((const unsigned char *));
extern void _draw_circular_arc P__((Point p0, Point p1, Point pc));
extern void _draw_ellipse P__((double xc, double yc, double rx, double ry, double angle));
extern void _draw_elliptic_arc P__((Point p0, Point p1, Point pc));
extern void _evaluate_ps_fgcolor P__ ((void));
extern void _evaluate_ps_fillcolor P__ ((void));
extern void _fakearc P__ ((Point p0, Point p1, int arc_type, const double m[4]));
extern void _get_range P__ ((double *xmin, double *xmax, double *ymin, double *ymax));
extern void _initialize_buffer P__ ((Outbuffer *bufp));
extern void _matrix_product P__ ((const double m[6], const double n[6], double product[6]));
extern void _prepare_chord_table P__ ((double sagitta));
extern void _reset_buffer P__((Outbuffer *bufp));
extern void _reset_range P__ ((void));
extern void _set_range P__ ((double x, double y));
extern void _update_buffer P__((Outbuffer *bufp));

#undef P__
