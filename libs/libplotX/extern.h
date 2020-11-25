/* Besides definitions, this file contains external declarations for the
   private variables and low-level functions of libplotX.  We attempt to
   keep namespace clean by beginning the name of each variable appearing in
   an external declaration with an underscore. */

/* we undefine Boolean, and re-define it, because the X header files 
   define it too (aargh) */

#undef Boolean

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#if USE_MOTIF
#include <Xm/Label.h>
#else
#include <X11/Xaw/Label.h>
#endif

#define Boolean int

/* We map the user-specified rectangle (or parallelogram) in user space to
   the [0,_xdata.width-1]x[0,_xdata.height-1] rectangle in integer X space.
   With these definitions, points in user space that are just slightly
   outside the user-specified region will be clipped.  Note: default X
   window dimensions _xdata.width and _xdata.height are specified, via
   fallback X resources, at the head of openpl.c.  To preserve aspect
   ratio, they should be equal. */

#define ROUNDING_FUZZ 0.0000001

#define DEVICE_X_LEFT (-0.5 + ROUNDING_FUZZ)
#define DEVICE_X_RIGHT (((int)(_xdata.width) - 0.5) - ROUNDING_FUZZ)
#define DEVICE_Y_BOTTOM (((int)(_xdata.height) - 0.5) - ROUNDING_FUZZ)
#define DEVICE_Y_TOP (-0.5 + ROUNDING_FUZZ)

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

/* X User Vector: transform device vector to user vector's x component */
#define XUV(x,y) ((_drawstate->transform.m[3] * (x) - _drawstate->transform.m[2] * (y)) / (_drawstate->transform.m[0] * _drawstate->transform.m[3] - _drawstate->transform.m[1] * _drawstate->transform.m[2]))

/* Y User Vector: transform device vector to user vector's y component */
#define YUV(x,y) ((- _drawstate->transform.m[1] * (x) + _drawstate->transform.m[0] * (y)) / (_drawstate->transform.m[0] * _drawstate->transform.m[3] - _drawstate->transform.m[1] * _drawstate->transform.m[2]))

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

/* Drawing state.  Includes drawing attributes, and the state of
   any uncompleted polyline object. */

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

#define L_SOLID 0
#define L_DOTTED 1
#define L_DOTDASHED 2
#define L_SHORTDASHED 3
#define L_LONGDASHED 4

typedef struct
{
  Display *dpy;			/* display */
  int screen;			/* screen number */
  Screen *Screen;		/* screen structure */
  int backing_store;		/* screen does backing store? */
  Colormap cmap;		/* colormap */
  XtAppContext app_con;		/* application context */
  Widget toplevel;		/* toplevel widget */
  Widget canvas;		/* Label widget */
  Window window;		/* Label widget's window */
  Pixmap pixmap;		/* Label widget's background pixmap */
  unsigned int width, height;	/* dimensions */
} XData;

extern XData _xdata;

/* Font types */
typedef enum 
{ 
  F_STANDARD, F_STROKE, F_DEVICE_SPECIFIC
} font_type;

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
  double x_font_size;		/* X font size, in pixels */
  unsigned long font_raw_ascent; /* what ascent wd. be for a 1000-pixel font */
  unsigned long font_raw_descent; /* what descent wd. be for a 1000-pixel font */
  double font_pixmatrix[4];	/* pixel matrix, parsed from font name */
  Boolean native_positioning;	/* if set, can use XDrawString() etc. */
  GC gc;			/* graphics context, for drawing */
  XFontStruct *x_font_struct;	/* font structure (used in alabel.c) */
  unsigned long x_bgcolor;	/* background color (pixel value) */
  unsigned long x_fgcolor;	/* foreground color (pixel value) */
  unsigned long x_fillcolor;	/* fill color (pixel value) */
  Boolean x_fgcolor_defined;	/* whether x fgcolor has been computed */
  Boolean x_fillcolor_defined;	/* whether x fillcolor has been computed */
/* pointer to previous state */
  struct lib_state *previous;
} State;

/* default drawing state, and pointer to state at head of linked list */

extern State _default_drawstate, *_drawstate;

/* initializations for default values of state variables when space() is
   called; note that X supports zero-width lines */

#define DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH (14.0/(8.5*72.0))
#define DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH 0.0

/* essentially the same two quantities over again, because savestate()
   needs them, at a time when space() hasn't yet been called.  aargh.  */

#define CROCK_DEFAULT_FONT_SIZE_IN_PIXELS 14
#define CROCK_DEFAULT_LINE_WIDTH_IN_PIXELS 0

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
#define PS_RADICAL_WIDTH 0.52
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

/* This elides the argument prototypes if the compiler does not
   support them. The name protarg is chosen in hopes that it will not
   collide with any others. */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif

extern Boolean _clean_iso_string P__ ((unsigned char *s));
extern Boolean _retrieve_font_internal P__ ((const char *name, double size, double rotation));
extern Boolean _x_set_font P__ ((const char *name));
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
extern void _evaluate_x_fgcolor P__ ((void));
extern void _evaluate_x_fillcolor P__ ((void));
extern void _fakearc P__ ((Point p0, Point p1, int arc_type, const double m[4]));
extern void _handle_x_events P__ ((void));
extern void _matrix_product P__ ((const double m[6], const double n[6], double product[6]));
extern void _prepare_chord_table P__ ((double sagitta));
extern void _retrieve_font P__ ((void));

#undef P__
