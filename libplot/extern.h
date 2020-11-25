/* This file defines what a Plotter object is, and includes external
   declarations of miscellaneous support routines and various databases.

   What a Plotter object is, depends on its type (there are many
   device-specific fields).  There are some device-specific parameters
   defined in g_params.h, also. */

#ifndef X_DISPLAY_MISSING
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#ifdef USE_MOTIF
#include <Xm/Label.h>
#else
#include <X11/Xaw/Label.h>
#endif
#endif /* X_DISPLAY_MISSING */

/*************************************************************************/
/* TYPEDEFS FOR, AND EXTERNAL VARIABLES IN, OUR FONT DATABASE (g_fontdb.c) */
/*************************************************************************/

/* our information about the 35 standard fonts in g_fontdb.c, and the
   typefaces they belong to */

/* Note: NUM_PS_FONTS should agree with the number of PS fonts in
   g_fontdb.c. */
#define NUM_PS_FONTS 35

struct ps_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
  const char *x_name;		/* the X Windows font name */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  short width[256];		/* the font width information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  int fig_id;			/* Fig's font id */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern const struct ps_font_info_struct _ps_font_info[];

/* our information about the 45 PCL fonts in g_fontdb.c, and the typefaces
   they belong to */

struct pcl_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=variable */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1 */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int cap_height;		/* the font's cap height */
  double hp_width_factor;	/* HP's width adjustment factor (why?) */
  short width[256];		/* the font width information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
};

extern const struct pcl_font_info_struct _pcl_font_info[];

/* our information about the vector fonts in g_fontdb.c, and the typefaces
   they belong to */

struct vector_font_info_struct 
{
  const char *name;		/* font name */
  const char *othername;	/* alternative font name */
  short chars[256];		/* array of vector glyphs */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
  bool visible;		/* whether font is visible, i.e. not internal */
};

extern const struct vector_font_info_struct _vector_font_info[];

/* accented character information (used in constructing Hershey ISO-Latin-1
   accented characters, see table in g_fontdb.c) */

struct accented_char_info_struct
{
  unsigned char composite, character, accent;
};

extern const struct accented_char_info_struct _vector_accented_char_info[];

/* types of accent, for a composite character */

#define ACC0 (16384 + 0)	/* superimpose on character */
#define ACC1 (16384 + 1)	/* elevate by 7 Hershey units */
#define ACC2 (16384 + 2)	/* same, also shift right by 2 units */

/* a flag in the glyph number indicating a small Kana */

#define KS 8192

/* arrays of Hershey vector glyphs */

extern const char *_occidental_vector_glyphs[];
extern const char *_oriental_vector_glyphs[];

/* typeface information, see g_fontdb.c */

#define FONTS_PER_TYPEFACE 10

struct typeface_info_struct
{
  int numfonts;
  int fonts[FONTS_PER_TYPEFACE];
};

extern const struct typeface_info_struct _ps_typeface_info[];
extern const struct typeface_info_struct _pcl_typeface_info[];
extern const struct typeface_info_struct _vector_typeface_info[];

/***********************************************************************/
/* GENERAL DEFINITIONS, TYPEDEFS, & EXTERNAL VARIABLES                 */
/***********************************************************************/

/* Initializations for default values of state variables when space() is
   called */
#define DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH (14.0/(8.5*72.0))
#define DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH (1.0/(8.5*100.0))

/* Justification types for labels (our numbering is the same as xfig's;
   don't alter without checking the code) */
#define JUST_LEFT 0
#define JUST_CENTER 1
#define JUST_RIGHT 2

/* Line types (our internal numbering) */
#define L_SOLID 0
#define L_DOTTED 1
#define L_DOTDASHED 2
#define L_SHORTDASHED 3
#define L_LONGDASHED 4

/* Join and cap types (our internal numbering) */

#define JOIN_MITER 0
#define JOIN_ROUND 1
#define JOIN_BEVEL 2

#define CAP_BUTT 0
#define CAP_ROUND 1
#define CAP_PROJECT 2

/* Data structures for points and vectors */

typedef struct
{
  double x,y;
} Point;

typedef Point Vector;

typedef struct
{
  int x,y;
} IntPoint;

typedef struct
{
  int red;
  int green;
  int blue;
} Color;

/* structure used in colorname.c for storing names of colors we recognize */

typedef struct
{
  const char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Colornameinfo;

typedef struct
{
  double left, right, bottom, top; /* device-frame coordinates, in inches */
  double extra;			/* plot length (hpgl only) */
}
Displaycoors;

typedef struct
{
  const char *name;
  bool metric;			/* metric vs. Imperial, advisory only */
  Displaycoors fig, ps, hpgl;	/* coordinates for several devices */
} Pagedata;

extern const Pagedata _pagedata[]; /* array of known page types */

/* An output buffer that may easily be resized; see outbuf.c. */

typedef struct
{
  char *base;
  int len;
  char *current;
  int contents;
}
Outbuffer;

/* a struct specifying a user->device coordinate transformation (each
   drawing state has such a struct as an element) */
typedef struct 
{ 
  /* the two components of an affine transformation */
  double m[6];			/* 1. a linear transformation (4 elements) */
  				/* 2. a translation (2 elements) */
  /* related data kept here for convenience */
  bool uniform;		/* transf. scaling is uniform? */
  bool axes_preserved;	/* transf. preserves axis directions? */
  bool nonreflection;	/* transf. doesn't involve a reflection? */
  bool is_raster;		/* does device use ints or floats as coors? */
} Transform;

typedef enum 
{ 
  /* The types of font we support (see database of font metrics in
     g_fontdb.c).  The fourth type (`other') is a catchall, currently used
     for any user-specified font, with an unrecognized name, that can be
     successfully retrieved from an X server. */
  F_HERSHEY, F_POSTSCRIPT, F_PCL, F_OTHER
} our_font_type;

/* X Device: transform user x coordinate to device x coordinate */
#define XD(x,y) (_plotter->drawstate->transform.m[4] + (x) * _plotter->drawstate->transform.m[0] + (y) * _plotter->drawstate->transform.m[2])

/* Y Device: transform user y coordinate to device y coordinate */
#define YD(x,y) (_plotter->drawstate->transform.m[5] + (x) * _plotter->drawstate->transform.m[1] + (y) * _plotter->drawstate->transform.m[3])

/* X Device Vector: transform user vector to device vector's x component */
#define XDV(x,y) (_plotter->drawstate->transform.m[0] * (x) + _plotter->drawstate->transform.m[2] * (y))

/* Y Device Vector: transform user vector to device vector's y component */
#define YDV(x,y) (_plotter->drawstate->transform.m[1] * (x) + _plotter->drawstate->transform.m[3] * (y))

/* X User Vector: transform device vector to user vector's x component
   (used by X11 driver only) */
#define XUV(x,y) ((_plotter->drawstate->transform.m[3] * (x) - _plotter->drawstate->transform.m[2] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))

/* Y User Vector: transform device vector to user vector's y component
   (used by X11 driver only) */
#define YUV(x,y) ((- _plotter->drawstate->transform.m[1] * (x) + _plotter->drawstate->transform.m[0] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))

/*************************************************************************/
/* MISC. DEFS ON POLYLINES and ARCS (relevant to all or most display devices)*/
/*************************************************************************/

/* Default length an unfilled polyline is allowed to grow to, before it is
   flushed out (we put no [small] maximum on the length of filled
   polylines, since they need to be preserved as discrete objects if the
   filling is to be performed properly). */
#define MAX_UNFILLED_POLYLINE_LENGTH 500
#define MAX_UNFILLED_POLYLINE_LENGTH_STRING "500"

/* Maximum number of times an arc is recursively subdivided, when it is
   being approximated by an inscribed polyline.  The polyline will contain
   no more than 2**MAX_ARC_SUBDIVISIONS line segments.  MAX_ARC_SUBDIVISIONS
   must be no longer than TABULATED_ARC_SUBDIVISIONS below (see g_fakearc.h)*/
#define MAX_ARC_SUBDIVISIONS 6 	/* to avoid buffer overflow on HP7550[A|B] */

/* Types of arc (which may or may not be drawn via recursive subdivision).
   These index into the table in g_fakearc.h. */
#define NUM_ARC_TYPES 4

#define QUARTER_ARC 0
#define HALF_ARC 1
#define THREE_QUARTER_ARC 2
#define USER_DEFINED_ARC 3

#define TABULATED_ARC_SUBDIVISIONS 15	/* length of each table entry */

/************************************************************************/
/* DEFINITIONS, TYPEDEFS, & EXTERNALS SPECIFIC TO INDIVIDUAL DEVICE DRIVERS */
/************************************************************************/

/************************************************************************/
/* Metafile device driver */
/************************************************************************/

/* string with which to begin each metafile, must begin with '#' to permit
   parsing by our plot filters */

#define PLOT_MAGIC "#PLOT"

/************************************************************************/
/* Tektronix device driver */
/************************************************************************/

/* For a Tektronix device we clip to [0..4095]x[0..3119], not the graphics
   display [488..3607]x[0..3119].  See t_clipper.c. */

#define CLIP_FUZZ 0.0000001
#define TEK_DEVICE_X_MIN_CLIP (-0.5 + CLIP_FUZZ)
#define TEK_DEVICE_X_MAX_CLIP (4095.5 - CLIP_FUZZ)
#define TEK_DEVICE_Y_MIN_CLIP  (-0.5 + CLIP_FUZZ)
#define TEK_DEVICE_Y_MAX_CLIP (3119.5 - CLIP_FUZZ)

/* Tektronix modes (our private numbering, values are not important but
   order definitely is, see tek_mode.c) */

#define MODE_ALPHA 0
#define MODE_PLOT 1
#define MODE_POINT 2
#define MODE_INCREMENTAL 3	/* currently not used */

typedef enum 
{
  D_GENERIC, D_KERMIT, D_XTERM
} tek_display_type;

/* colors supported by MS-DOS kermit Tek emulation, see colordb2.c */

#define KERMIT_NUM_STD_COLORS 16
extern const Color _kermit_stdcolors[KERMIT_NUM_STD_COLORS];
extern const char *_kermit_fgcolor_escapes[KERMIT_NUM_STD_COLORS];
extern const char *_kermit_bgcolor_escapes[KERMIT_NUM_STD_COLORS];
/* must agree with the ordering in colordb2.c */
#define ANSI_SYS_BLACK   0
#define ANSI_SYS_GRAY30  8
#define ANSI_SYS_GRAY55  7
#define ANSI_SYS_WHITE  15

/************************************************************************/
/* HP-GL device driver */
/************************************************************************/

/* An HPGLPlotter plots using virtual device coordinates: not the native
   device coordinates, but rather scaled coordinates in which the graphics
   display is [0,10000]x[0,10000].  To arrange this, in h_closepl.c we move
   the HP-GL `scaling points' to the lower left and upper right corners of
   our graphics display, and use the HP-GL `SC' instruction to set up a
   scaled set of coordinates. */
#define HPGL_SCALED_DEVICE_LEFT 0
#define HPGL_SCALED_DEVICE_RIGHT 10000
#define HPGL_SCALED_DEVICE_BOTTOM 0
#define HPGL_SCALED_DEVICE_TOP 10000

#define HPGL_UNITS_PER_INCH 1016 /* 1 HP-GL unit = 1/40 mm */

/* HP-GL line attribute types (HP-GL numbering; see h_attribs.c) */
#define HPGL_L_SOLID (-100)	/* no numeric parameter at all */
#define HPGL_L_DOTTED 1
#define HPGL_L_DOTDASHED 4
#define HPGL_L_SHORTDASHED 2
#define HPGL_L_LONGDASHED 3

#define HPGL_JOIN_MITER 1
#define HPGL_JOIN_ROUND 4
#define HPGL_JOIN_BEVEL 5

#define HPGL_CAP_BUTT 1
#define HPGL_CAP_ROUND 4
#define HPGL_CAP_PROJECT 2

/* HP-GL and HP-GL/2 fill types (HP-GL/2 numbering) */
#define HPGL_FILL_SOLID_BI 1
#define HPGL_FILL_SOLID_UNI 2
#define HPGL_FILL_SHADING 10

/* Maximum number of pens, or logical pens, for an HP-GL/2 device.  Some
   such devices permit as many as 256, but all should permit at least as
   many as 32.  Our pen numbering will range over 0..MAX_NUM_PENS-1. */
#define MAX_NUM_PENS 32

/* default values for HPGL_PENS environment variable, for HP-GL[/2]; this
   lists available pens and their positions in carousel */
#define DEFAULT_HPGL_PEN_STRING "1=black"
#define DEFAULT_HPGL2_PEN_STRING "1=black:2=red:3=green:4=yellow:5=blue:6=magenta:7=cyan"

/* PCL 5 typeface information: symbol set, i.e. encoding */
#define PCL_ISO_8859_1 14
#define PCL_ROMAN_8 277

/* Default HP-GL/2 typeface */
#define STICK_TYPEFACE 48

/* The nominal HP-GL/2 fontsize we use for drawing a label (for fixed-width
   and proportional fonts, respectively).  We retrieve a font in the
   specified size, and rescale it linearly before drawing the label. */
#define NOMINAL_CHARS_PER_INCH 8.0
#define NOMINAL_POINT_SIZE 18

/* Spacing characteristic of the PCL fonts */
#define FIXED_SPACING 0
#define PROPORTIONAL_SPACING 1

/************************************************************************/
/* xfig device driver */
/************************************************************************/

/* Standard Fig unit size in v. 3.0+ */
#define FIG_UNITS_PER_INCH 1200

/* device units <-> printer's points; number of points per inch == 72 */
#define FIG_UNITS_TO_POINTS(size) ((size)*72.0/FIG_UNITS_PER_INCH)
#define POINTS_TO_FIG_UNITS(size) ((size)*((double)FIG_UNITS_PER_INCH)/72.0)

/* xfig specifies line widths in `Fig display units' rather than `Fig units'
   (there are 80 of the former per inch). */
#define FIG_UNITS_TO_FIG_DISPLAY_UNITS(width) ((width)*80.0/FIG_UNITS_PER_INCH)

/* For historical reasons, xfig scales the fonts down by a factor
   FONT_SCALING, i.e., (80.0)/(72.0).  So we have to premultiply font sizes
   by the same factor.  The current release of xfig unfortunately can't
   handle font sizes that aren't integers, so it rounds them.  Ah well. */
#define FIG_FONT_SCALING ((80.0)/(72.0))

/* Fig supported line styles.  DOTTED and DASHED line styles are specified
   by (respectively) the length of the gap between successive dots, and the
   length of each dash (equal to the length of the gap between successive
   dashes).  */

#define FIG_L_DEFAULT (-1)
#define FIG_L_SOLID 0
#define FIG_L_DASHED 1
#define FIG_L_DOTTED 2

/* Fig's line styles, indexed into by internal line number
   (L_SOLID/L_DOTTED/ L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED.  Fig format
   supports only dotted lines (isolated dots, with a specified inter-dot
   distance) and dashed lines (on/off dashes, the lengths of the on and off
   segments being equal).  We map our canonical five line types into Fig
   line types as best we can. */
extern const int _fig_line_style[];

/* Fig's `style value', i.e. inter-dot length or on-and-off segment length,
   indexed into by internal line number (L_SOLID/L_DOTTED/
   L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED; dash length ignored for
   L_SOLID).  Units are Fig display units.  See f_endpath.c. */
extern const double _fig_dash_length[];

#define FIG_JOIN_MITER 0
#define FIG_JOIN_ROUND 1
#define FIG_JOIN_BEVEL 2

#define FIG_CAP_BUTT 0
#define FIG_CAP_ROUND 1
#define FIG_CAP_PROJECT 2

/* Fig join and cap styles, see f_endpath.c, indexed by our internal join
   and cap type numbers (miter/rd./bevel and butt/rd./project) */
extern const int _fig_join_style[], _fig_cap_style[];

/* these constants for Fig colors are hardcoded in xfig */

#define FIG_STD_COLOR_MIN 0	/* see colordb2.c for colors 0,...,31 */
#define C_BLACK 0		/* i.e. #0 in table in colordb2.c */
#define C_WHITE 7		/* i.e. #7 in table */
#define FIG_NUM_STD_COLORS 32
#define FIG_USER_COLOR_MIN 32
extern const Color _fig_stdcolors[FIG_NUM_STD_COLORS];

/* xfig's depth attribute ranges from 0 to FIG_MAXDEPTH.  See DEPTH.doc. */
#define FIG_MAXDEPTH 999

/* depth of the first object we'll draw (we make it a bit less than
   FIG_MAXDEPTH, since the user may wish to edit the drawing with xfig to
   include deeper, i.e. obscured objects) */
#define FIG_INITIAL_DEPTH 989

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

#define FIG_MAX_NUM_USER_COLORS 512

/* used in place of anamorphically scaled PS fonts (which xfig currently
   doesn't support) */
#define FIG_DEFAULT_HERSHEY_FONT "HersheySerif"

/************************************************************************/
/* Postscript/idraw device driver */
/************************************************************************/

/* minimum desired resolution in device frame (i.e. in printer's points) */
#define PS_MIN_RESOLUTION 0.05

/* arrays, see p_endpath.c, indexed by line type */

extern const long _ps_line_type_bit_vector[]; /* idraw linestyle, 16-bit brush */
extern const char *_ps_line_type_setdash[]; /* dasharray arg, for PS `setdash' */

/* PS line join and line cap styles */

#define PS_JOIN_MITER 0
#define PS_JOIN_ROUND 1
#define PS_JOIN_BEVEL 2

#define PS_CAP_BUTT 0
#define PS_CAP_ROUND 1
#define PS_CAP_PROJECT 2

/* PS join and cap styles, see p_endpath.c, indexed by our internal join
   and cap type numbers (miter/rd./bevel and butt/rd./project) */
extern const int _ps_join_style[], _ps_cap_style[];

/* information on colors known to idraw, see colordb2.c */

#define IDRAW_NUM_STD_COLORS 12

extern const Color _idraw_stdcolors[IDRAW_NUM_STD_COLORS];
extern const char *_idraw_stdcolornames[IDRAW_NUM_STD_COLORS];

/* information on shadings known to idraw, see colordb2.c */

#define IDRAW_NUM_STD_SHADINGS 5

extern const double _idraw_stdshadings[IDRAW_NUM_STD_SHADINGS];

/************************************************************************/
/* X11 and `X11 Drawable' device drivers */
/************************************************************************/

#ifndef X_DISPLAY_MISSING
/* We keep track of which fonts we request from an X server, in any
   connection, by constructing a linked list of these records.  A linked
   list is good enough if we don't have huge numbers of font changes. */
typedef struct lib_fontrecord
{
  char *name;			/* font name, e.g. XLFD name */
  XFontStruct *x_font_struct;	/* font structure */
  struct lib_fontrecord *next;	/* most previously retrieved font */
} Fontrecord;
#endif

/* used as a backup, if we can't retrieve an X font */
#define X_DEFAULT_HERSHEY_FONT "HersheySerif"

/* sixteen-bit restriction on X11 protocol parameters */
#define XOOB_UNSIGNED(x) ((x) > (int)0xffff)
#define XOOB_INT(x) ((x) > (int)0x7fff || (x) < (int)(-0x8000))

/***********************************************************************/
/* DRAWING STATE AND DEVICE STATE */
/***********************************************************************/

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
  bool arc_stashed;		/* whether arc has been stored */
  bool arc_immediate;		/* whether to output stored arc immediately */
  bool arc_polygonal;		/* whether to output as polyline (advisory) */
  double axc, ayc, ax0, ay0, ax1, ay1; /* stored arc parameters */
/* modal drawing attributes */
  char *line_mode;		/* line mode */
  int line_type;		/* one of L_*, determined by line mode */
  char *join_mode;		/* join mode */
  int join_type;		/* one of J_*, determined by join mode */
  char *cap_mode;		/* cap mode */
  int cap_type;			/* one of C_*, determined by cap mode */
  double line_width;		/* width of lines in user coordinates */
  int fill_level;		/* fill level */
  char *font_name;		/* font name */
  double font_size;		/* font size in user coordinates */
  double true_font_size;	/* true font size (as retrieved) */
  double font_ascent;		/* font ascent (as retrieved) */
  double font_descent;		/* font descent (as retrieved) */
  our_font_type font_type;	/* F_HERSHEY / F_PS / F_PCL / F_OTHER */
  int typeface_index;		/* typeface index (in g_fontdb.h table) */
  int font_index;		/* font index, within typeface */
  bool font_is_iso8859;		/* whether font has an iso8859 character set */
  double text_rotation;		/* degrees counterclockwise, for labels */
  Color fgcolor;		/* foreground color */
  Color fillcolor;		/* fill color */
  Color bgcolor;		/* background color */
  bool points_are_connected;	/* if not set, polyline displayed as points */
  double device_line_width;	/* line width in device coordinates */
  int quantized_device_line_width; /* line width, quantized to integer */
  bool suppress_polyline_flushout; /* suppress endpath while drawing a path? */
/* elements specific to the HP-GL drawing state */
  double hpgl_pen_width;	/* pen width (frac of diag dist betw P1,P2) */
/* elements specific to the fig drawing state */
  int fig_font_point_size;	/* font size in fig's idea of points */
  int fig_fill_level;		/* fig's fill level */
  int fig_fgcolor;		/* fig's foreground color */
  int fig_fillcolor;		/* fig's fill color */
/* elements specific to the PS drawing state */
  double ps_fgcolor_red;	/* RGB for fgcolor, each in [0.0,1.0] */
  double ps_fgcolor_green;
  double ps_fgcolor_blue;
  double ps_fillcolor_red;	/* RGB for fillcolor, each in [0.0,1.0] */
  double ps_fillcolor_green;
  double ps_fillcolor_blue;
  int idraw_fgcolor;		/* index of idraw fgcolor in table */
  int idraw_bgcolor;		/* index of idraw bgcolor in table */
  int idraw_shading;		/* index of idraw shading in table */
#ifndef X_DISPLAY_MISSING
/* elements specific to the X11 and X11 Drawable drawing states */
  double font_pixmatrix[4];	/* pixel matrix, parsed from font name */
  bool native_positioning;	/* if set, can use XDrawString() etc. */
  GC gc;			/* graphics context, for drawing */
  XFontStruct *x_font_struct;	/* font structure (used in alabel.c) */
  unsigned long x_bgcolor;	/* background color (pixel value) */
  unsigned long x_fgcolor;	/* foreground color (pixel value) */
  unsigned long x_fillcolor;	/* fill color (pixel value) */
#endif /* X_DISPLAY_MISSING */
/* pointer to previous state */
  struct lib_state *previous;
} State;

/* default drawing states for supported plotter types */
extern State _meta_default_drawstate, _tek_default_drawstate, _hpgl_default_drawstate, _fig_default_drawstate, _ps_default_drawstate, _X_default_drawstate;

/* supported plotter types */
#define PL_META 0		/* GNU graphics metafile */
#define PL_TEK 1		/* Tektronix 4014 with EGM */
#define PL_HPGL 2		/* HP-GL and HP-GL/2 */
#define PL_FIG 3		/* xfig 3.x */
#define PL_PS 4			/* Postscript */
#define PL_X11 5		/* X11 */
#define PL_X11_DRAWABLE 6	/* X11 Drawable */

/* recognized device driver parameters (key/value) are in g_params.h */
#define NUM_DEVICE_DRIVER_PARAMETERS 17

/* This elides the argument prototypes if the compiler does not support
   them. The name P__ is chosen in hopes that it will not collide with any
   others. */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif

/* a structure defining a plotter object */
typedef struct
{
  /* PUBLIC METHODS, as listed in the API */
  int (*alabel) P__ ((int x_justify, int y_justify, const char *s));
  int (*arc) P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
  int (*arcrel) P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
  int (*bgcolor) P__ ((int red, int green, int blue));
  int (*bgcolorname) P__ ((const char *name));
  int (*box) P__ ((int x0, int y0, int x1, int y1));
  int (*boxrel) P__ ((int dx0, int dy0, int dx1, int dy1));
  int (*capmod) P__ ((const char *s));
  int (*circle) P__ ((int x, int y, int r));
  int (*circlerel) P__ ((int dx, int dy, int r));
  int (*closepl) P__ ((void));
  int (*color) P__ ((int red, int green, int blue));
  int (*colorname) P__ ((const char *name));
  int (*cont) P__ ((int x, int y));
  int (*contrel) P__ ((int x, int y));
  int (*ellarc) P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
  int (*ellarcrel) P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
  int (*ellipse) P__ ((int x, int y, int rx, int ry, int angle));
  int (*ellipserel) P__ ((int dx, int dy, int rx, int ry, int angle));
  int (*endpath) P__((void));
  int (*erase) P__ ((void));
  int (*farc) P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
  int (*farcrel) P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
  int (*fbox) P__ ((double x0, double y0, double x1, double y1));
  int (*fboxrel) P__ ((double dx0, double dy0, double dx1, double dy1));
  int (*fcircle) P__ ((double x, double y, double r));
  int (*fcirclerel) P__ ((double dx, double dy, double r));
  int (*fconcat) P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
  int (*fcont) P__ ((double x, double y));
  int (*fcontrel) P__ ((double x, double y));
  int (*fellarc) P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
  int (*fellarcrel) P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
  int (*fellipse) P__ ((double x, double y, double rx, double ry, double angle));
  int (*fellipserel) P__ ((double dx, double dy, double rx, double ry, double angle));
  double (*ffontname) P__ ((const char *s));
  double (*ffontsize) P__ ((double size));
  int (*fillcolor) P__ ((int red, int green, int blue));
  int (*fillcolorname) P__ ((const char *name));
  int (*filltype) P__ ((int level));
  double (*flabelwidth) P__ ((const char *s));
  int (*fline) P__ ((double x0, double y0, double x1, double y1));
  int (*flinerel) P__ ((double dx0, double dy0, double dx1, double dy1));
  int (*flinewidth) P__ ((double size));
  int (*flushpl) P__ ((void));
  int (*fmarker) P__ ((double x, double y, int type, double size));
  int (*fmarkerrel) P__ ((double dx, double dy, int type, double size));
  int (*fmove) P__ ((double x, double y));
  int (*fmoverel) P__ ((double x, double y));
  int (*fontname) P__ ((const char *s));
  int (*fontsize) P__ ((int size));
  int (*fpoint) P__ ((double x, double y));
  int (*fpointrel) P__ ((double dx, double dy));
  int (*frotate) P__ ((double theta));
  int (*fscale) P__ ((double x, double y));
  int (*fspace) P__ ((double x0, double y0, double x1, double y1));
  int (*fspace2) P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
  double (*ftextangle) P__ ((double angle));
  int (*ftranslate) P__ ((double x, double y));
  int (*havecap) P__ ((const char *s));
  int (*joinmod) P__ ((const char *s));
  int (*label) P__ ((const char *s));
  int (*labelwidth) P__ ((const char *s));
  int (*line) P__ ((int x0, int y0, int x1, int y1));
  int (*linemod) P__ ((const char *s));
  int (*linerel) P__ ((int dx0, int dy0, int dx1, int dy1));
  int (*linewidth) P__ ((int size));
  int (*marker) P__ ((int x, int y, int type, int size));
  int (*markerrel) P__ ((int dx, int dy, int type, int size));
  int (*move) P__ ((int x, int y));
  int (*moverel) P__ ((int x, int y));
  int (*openpl) P__ ((void));
  FILE* (*outfile) P__((FILE* newstream));
  int (*pencolor) P__ ((int red, int green, int blue));
  int (*pencolorname) P__ ((const char *name));
  int (*point) P__ ((int x, int y));
  int (*pointrel) P__ ((int dx, int dy));
  int (*restorestate) P__((void));
  int (*savestate) P__((void));
  int (*space) P__ ((int x0, int y0, int x1, int y1));
  int (*space2) P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
  int (*textangle) P__ ((int angle));
  /* PRIVATE METHODS (may depend on plotter type) */
  /* Versions of the `falabel' and `flabelwidth' methods, one for each of
     the three sorts of non-Hershey font.  The argument h_just specifies
     the justification to be used when rendering (JUST_LEFT, RIGHT,
     CENTER).  If a display device supports non-default (i.e. non-LEFT)
     justification, the have_justification capability flag should be set. */
  double (*falabel_ps) P__ ((const unsigned char *s, int h_just));
  double (*falabel_pcl) P__ ((const unsigned char *s, int h_just));
  double (*falabel_other) P__ ((const unsigned char *s, int h_just));
  double (*flabelwidth_ps) P__((const unsigned char *s));
  double (*flabelwidth_pcl) P__((const unsigned char *s));
  double (*flabelwidth_other) P__((const unsigned char *s));
  /* private low-level `retrieve font' method */
  void (*retrieve_font) P__((void));
  /* private low-level `sync font' method */
  void (*set_font) P__((void));
  /* private `sync line attributes' method */
  void (*set_attributes) P__((void));
  /* private low-level `sync color' methods */
  void (*set_pen_color) P__((void));
  void (*set_fill_color) P__((void));  
  /* private low-level `sync position' method */
  void (*set_position) P__((void));
  /* error handlers */
  void (*warning) P__((const char *msg));
  void (*error) P__((const char *msg));

  /* PRIVATE DATA MEMBERS (not specific to any one device driver) */
  /* basic plotter variables */
  int type;			/* device type: one of PL_* above */
  bool open;			/* whether or not plotter is open */
  bool opened;			/* whether or not plotter has been opened */
  int page_number;		/* number of times it has been opened */
  FILE *instream;		/* input stream if any */
  FILE *outstream;		/* output stream if any */
  FILE *errstream;		/* error stream if any */
  /* low-level device driver parameters */
  void *params[NUM_DEVICE_DRIVER_PARAMETERS];
  /* capabilities */            /* capabilities: 0/1/2 = no/yes/maybe */
  int have_wide_lines;	
  int have_solid_fill;
  int have_settable_bg;
  int have_hershey_fonts;
  int have_ps_fonts;
  int have_pcl_fonts;
  int have_justification;
  long int hard_polyline_length_limit; /* a hard limit for all polylines */
  /* output buffer */
  Outbuffer outbuf;		/* output buffer (for non-realtime devices) */
  /* associated process id's */
  pid_t *pids;			/* list of pids of forked-off processes */
  int num_pids;			/* number of pids in list */
  /* drawing state(s) */
  State *drawstate;		/* pointer to top of drawing state stack */
  State *default_drawstate;	/* for initialization and resetting */
  /* dimensions */
  bool bitmap_device;		/* bitmap display device? */
  int imin, imax, jmin, jmax;	/* range of coordinates (for a bitmap device)*/
  Displaycoors display_coors;	/* same, for a physical device (in inches) */
  double device_units_per_inch;	/* units/inch for a physical device */
  bool flipped_y;		/* y increases downward? */
  /* elements used by more than one device driver, but not all */
  int max_unfilled_polyline_length; /* user-settable, for unfilled polylines */
  bool position_is_unknown;	/* cursor position is unknown? */
  IntPoint pos;			/* cursor position (for a bitmap device) */
  bool font_warning_issued;	/* issued warning on lack of sheared fonts */

  /* PRIVATE DATA MEMBERS (device driver-specific) */
  /* elements specific to the metafile device driver */
  bool portable_output;		/* portable, not binary output format? */
  /* elements specific to the Tektronix device driver */
  tek_display_type display_type; /* which sort of Tektronix? */
  int mode;			/* one of MODE_* */
  int line_type;		/* one of L_* */
  bool mode_is_unknown;
  bool line_type_is_unknown;
  int kermit_fgcolor;		/* kermit's foreground color */
  int kermit_bgcolor;		/* kermit's background color */
  /* elements specific to the HP-GL device driver */
  int hpgl_version;		/* version: 0=HP-GL, 1=HP7550A, 2=HP-GL/2 */
  int rotation;			/* HP-GL rotation angle */
  double p1x, p1y;		/* scaling point P1 in native HP-GL coors */
  double p2x, p2y;		/* scaling point P2 in native HP-GL coors */
  double plot_length;		/* plot length (for HP-GL/2 roll plotters) */
  int pen;			/* number of currently selected pen */
  bool pendown;			/* pen down rather than up? */
  double pen_width;		/* pen width (frac of diag dist betw P1,P2) */
  int hpgl_line_type;		/* line type (HP-GL numbering, solid = -100)*/
  int hpgl_cap_style;		/* cap style for lines (HP-GL/2 numbering) */
  int hpgl_join_style;		/* join style for lines (HP-GL/2 numbering) */
  int fill_type;		/* fill type (one of FILL_SOLID_UNI etc.) */
  double shading_level;		/* percent; used if fill_type=FILL_SHADING */
  bool monochrome;		/* have pen #1 only? */
  int free_pen;			/* pen to be assigned a color next */
  bool palette;			/* can assign pen colors? (HP-GL/2 only) */
  bool opaque_white;		/* white pen sh'd be opaque? (HP-GL/2 only) */
  int pcl_symbol_set;		/* encoding, 14=ISO-Latin-1,.. (HP-GL/2 only)*/
  int pcl_spacing;		/* font spacing, 0=fixed, 1=not(HP-GL/2 only)*/
  int pcl_posture;		/* posture, 0=upright, 1=italic(HP-GL/2 only)*/
  int pcl_stroke_weight;	/* weight,0=normal,3=bold, etc.(HP-GL/2 only)*/
  int pcl_typeface;		/* typeface, as in g_fontdb.c (HP-GL/2 only) */
  double relative_char_height;	/* char. ht., % of p2y-p1y (HP-GL/2 only) */
  double relative_char_width;	/* char. width, % of p2x-p1x (HP-GL/2 only) */
  double relative_label_rise;	/* label rise, % of p2y-p1y (HP-GL/2 only) */
  double relative_label_run;	/* label run, % of p2x-p1x (HP-GL/2 only) */
  double char_slant_tangent;	/* tangent of character slant (HP-GL/2 only)*/
  unsigned char label_terminator; /* character used as label terminator */
  /* Note: HP-GL driver also uses pen_color[] and pen_defined[] arrays below */
/* elements specific to the fig device driver */
  bool fig_use_metric;		/* whether xfig display should be in metric */
  int fig_drawing_depth;	/* fig's current value for `depth' attribute */
  int fig_last_priority;	/* drawing priority for last-drawn object */
  int fig_num_usercolors;	/* number of colors currently defined */
  /* Note: fig driver also uses the fig_usercolors[] array below */
/* elements specific to the PS device driver */
  double xrange_min, xrange_max, yrange_min, yrange_max; /* BoundingBox */
  /* Note: PS driver also uses the ps_font_used[] array below */
#ifndef X_DISPLAY_MISSING
/* elements specific to both the X11 and the X11 Drawable device drivers */
  Drawable drawable1;		/* an X drawable (e.g. a pixmap) */
  Drawable drawable2;		/* an X drawable (e.g. a window) */
  Fontrecord *fontlist;		/* head of list of retrieved X fonts */
  Display *dpy;			/* display */
/* elements specific to the X11 device driver */
  Colormap cmap;		/* colormap */
  XtAppContext app_con;		/* application context */
  Widget toplevel;		/* toplevel widget */
  Widget canvas;		/* Label widget */
  bool vanish_on_delete;	/* window(s) disappear on Plotter deletion? */
#endif /* X_DISPLAY_MISSING */
/* arrays used by various device drivers (see above), positioned at end,
   for ease of initialization */
  Color pen_color[MAX_NUM_PENS]; /* array of colors for pens / logical pens */
  int pen_defined[MAX_NUM_PENS]; /* 0=absent, 1=soft-defined, 2=hard-defined */
  long int fig_usercolors[FIG_MAX_NUM_USER_COLORS]; /* colors we've defined */
  bool ps_font_used[NUM_PS_FONTS]; /* whether or not each font has been used */
} Plotter;

#undef P__

/* pointer to our currently selected plotter */
extern Plotter *_plotter;

/* for initialization of plotters */
extern Plotter _meta_default_plotter, _tek_default_plotter, _hpgl_default_plotter, _fig_default_plotter, _ps_default_plotter, _X_default_plotter, _X_drawable_default_plotter;

/************************************************************************/
/* PROTOTYPES */
/************************************************************************/

/* This elides the argument prototypes if the compiler does not support
   them. The name P__ is chosen in hopes that it will not collide with any
   others. */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif

/* The following are declarations of miscellaneous support routines. */

/* innocuous, not Plotter class methods at all */
extern Point _truecenter P__ ((Point p0, Point p1, Point pc));
extern Vector *_vscale P__ ((Vector *v, double newlen));
extern Voidptr _plot_xcalloc P__ ((unsigned int nmemb, unsigned int size));
extern Voidptr _plot_xmalloc P__ ((unsigned int size));
extern Voidptr _plot_xrealloc P__ ((Voidptr p, unsigned int size));
extern bool _clean_iso_string P__ ((unsigned char *s));
extern bool _string_to_color P__ ((const char *name, const Colornameinfo **info_p));
extern const Pagedata * _pagetype P__ ((const char *name));
extern double _matrix_norm P__((const double m[6]));
extern double _xatan2 P__((double y, double x));
extern int _clip_line P__ ((double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip));
extern int _codestring_len P__((const unsigned short *codestring));
extern unsigned short * _controlify P__((const unsigned char *));
extern void _initialize_buffer P__ ((Outbuffer *bufp));
extern void _matrix_product P__ ((const double m[6], const double n[6], double product[6]));
extern void _reset_buffer P__((Outbuffer *bufp));
extern void _update_buffer P__((Outbuffer *bufp));

/* a function used in every driver */
extern void _draw_circular_arc __P((Point p0, Point p1, Point pc));

/* metafile driver */
/* private methods */
extern void _emit_float P__ ((double x)); 
extern void _emit_integer P__ ((int x)); 

/* Tek driver */
/* private methods */
extern void _tek_mode P__ ((int newmode));
extern void _tek_move P__ ((int xx, int yy));
extern void _tek_vector P__ ((int xx, int yy));
extern void _tek_vector_compressed P__((int xx, int yy, int oldxx, int oldyy));

/* HP-GL driver */
/* private methods */

/* Fig driver */
/* private methods */

/* PS driver */
/* private methods */
/* bounding box, a confusing situation: may all need to be private methods */
extern void _reset_range P__ ((void));
extern void _set_line_end_bbox P__((double x, double y, double xother, double yother, double linewidth, int capstyle));
extern void _set_line_join_bbox P__((double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle));
extern void _set_range P__ ((double x, double y));
extern void _set_ellipse_bbox P__ ((double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth));
extern void _get_range P__ ((double *xmin, double *xmax, double *ymin, double *ymax));

/* X driver */
/* private methods */
extern void _handle_x_events P__ ((void));

/* in api.c */
extern void * _get_plot_param P__ ((const char *parameter)); 

/* The following are declarations of the device-specific versions of
   plotter methods.  The initial letter indicates the device specificity.
   g=generic, m=metafile, t=Tektronix, h=HP-GL/2, f=xfig, p=PS, x=X11. */

extern FILE* _g_outfile P__((FILE* newstream));
extern double _g_falabel_hershey P__((int x_justify, int y_justify, const unsigned char *s));
extern double _g_ffontname P__ ((const char *s));
extern double _g_ffontsize P__ ((double size));
extern double _g_flabelwidth P__ ((const char *s));
extern double _g_flabelwidth_hershey P__((const unsigned char *s));
extern double _g_flabelwidth_ps P__((const unsigned char *s));
extern double _g_flabelwidth_pcl P__((const unsigned char *s));
extern double _g_ftextangle P__ ((double angle));
extern int _g_alabel P__ ((int x_justify, int y_justify, const char *s));
extern int _g_arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _g_arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _g_bgcolor P__ ((int red, int green, int blue));
extern int _g_bgcolorname P__ ((const char *name));
extern int _g_box P__ ((int x0, int y0, int x1, int y1));
extern int _g_boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _g_capmod P__ ((const char *s));
extern int _g_circle P__ ((int x, int y, int r));
extern int _g_circlerel P__ ((int dx, int dy, int r));
extern int _g_closepl P__ ((void));
extern int _g_color P__ ((int red, int green, int blue));
extern int _g_colorname P__ ((const char *name));
extern int _g_cont P__ ((int x, int y));
extern int _g_contrel P__ ((int x, int y));
extern int _g_ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _g_ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _g_ellipse P__ ((int x, int y, int rx, int ry, int angle));
extern int _g_ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
extern int _g_endpath P__((void));
extern int _g_erase P__ ((void));
extern int _g_farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _g_farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _g_fbox P__ ((double x0, double y0, double x1, double y1));
extern int _g_fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _g_fcircle P__ ((double x, double y, double r));
extern int _g_fcirclerel P__ ((double dx, double dy, double r));
extern int _g_fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _g_fcont P__ ((double x, double y));
extern int _g_fcontrel P__ ((double x, double y));
extern int _g_fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _g_fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _g_fellipse P__ ((double x, double y, double rx, double ry, double angle));
extern int _g_fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
extern int _g_fillcolor P__ ((int red, int green, int blue));
extern int _g_fillcolorname P__ ((const char *name));
extern int _g_filltype P__ ((int level));
extern int _g_fline P__ ((double x0, double y0, double x1, double y1));
extern int _g_flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _g_flinewidth P__ ((double size));
extern int _g_flushpl P__ ((void));
extern int _g_fmarker P__ ((double x, double y, int type, double size));
extern int _g_fmarkerrel P__ ((double dx, double dy, int type, double size));
extern int _g_fmove P__ ((double x, double y));
extern int _g_fmoverel P__ ((double x, double y));
extern int _g_fontname P__ ((const char *s));
extern int _g_fontsize P__ ((int size));
extern int _g_fpoint P__ ((double x, double y));
extern int _g_fpointrel P__ ((double dx, double dy));
extern int _g_frotate P__ ((double theta));
extern int _g_fscale P__ ((double x, double y));
extern int _g_fspace P__ ((double x0, double y0, double x1, double y1));
extern int _g_fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _g_ftranslate P__ ((double x, double y));
extern int _g_havecap P__ ((const char *s));
extern int _g_joinmod P__ ((const char *s));
extern int _g_label P__ ((const char *s));
extern int _g_labelwidth P__ ((const char *s));
extern int _g_line P__ ((int x0, int y0, int x1, int y1));
extern int _g_linemod P__ ((const char *s));
extern int _g_linerel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _g_linewidth P__ ((int size));
extern int _g_marker P__ ((int x, int y, int type, int size));
extern int _g_markerrel P__ ((int dx, int dy, int type, int size));
extern int _g_move P__ ((int x, int y));
extern int _g_moverel P__ ((int x, int y));
extern int _g_openpl P__ ((void));
extern int _g_pencolor P__ ((int red, int green, int blue));
extern int _g_pencolorname P__ ((const char *name));
extern int _g_point P__ ((int x, int y));
extern int _g_pointrel P__ ((int dx, int dy));
extern int _g_restorestate P__((void));
extern int _g_savestate P__((void));
extern int _g_space P__ ((int x0, int y0, int x1, int y1));
extern int _g_space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _g_textangle P__ ((int angle));
extern void _g_error P__((const char *msg));
extern void _g_warning P__((const char *msg));
extern void _g_retrieve_font P__((void));

extern FILE* _f_outfile P__((FILE* newstream));
extern double _f_falabel_ps P__((const unsigned char *s, int h_just));
extern double _f_ffontname P__ ((const char *s));
extern double _f_ffontsize P__ ((double size));
extern double _f_ftextangle P__ ((double angle));
extern int _f_alabel P__ ((int x_justify, int y_justify, const char *s));
extern int _f_arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _f_arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _f_box P__ ((int x0, int y0, int x1, int y1));
extern int _f_boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _f_capmod P__ ((const char *s));
extern int _f_circle P__ ((int x, int y, int r));
extern int _f_circlerel P__ ((int dx, int dy, int r));
extern int _f_closepl P__ ((void));
extern int _f_color P__ ((int red, int green, int blue));
extern int _f_colorname P__ ((const char *name));
extern int _f_cont P__ ((int x, int y));
extern int _f_contrel P__ ((int x, int y));
extern int _f_ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _f_ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _f_ellipse P__ ((int x, int y, int rx, int ry, int angle));
extern int _f_ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
extern int _f_endpath P__((void));
extern int _f_erase P__ ((void));
extern int _f_farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _f_farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _f_fbox P__ ((double x0, double y0, double x1, double y1));
extern int _f_fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _f_fcircle P__ ((double x, double y, double r));
extern int _f_fcirclerel P__ ((double dx, double dy, double r));
extern int _f_fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _f_fcont P__ ((double x, double y));
extern int _f_fcontrel P__ ((double x, double y));
extern int _f_fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _f_fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _f_fellipse P__ ((double x, double y, double rx, double ry, double angle));
extern int _f_fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
extern int _f_fillcolor P__ ((int red, int green, int blue));
extern int _f_fillcolorname P__ ((const char *name));
extern int _f_filltype P__ ((int level));
extern int _f_fline P__ ((double x0, double y0, double x1, double y1));
extern int _f_flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _f_flinewidth P__ ((double size));
extern int _f_flushpl P__ ((void));
extern int _f_fmarker P__ ((double x, double y, int type, double size));
extern int _f_fmarkerrel P__ ((double dx, double dy, int type, double size));
extern int _f_fmove P__ ((double x, double y));
extern int _f_fmoverel P__ ((double x, double y));
extern int _f_fontname P__ ((const char *s));
extern int _f_fontsize P__ ((int size));
extern int _f_fpoint P__ ((double x, double y));
extern int _f_fpointrel P__ ((double dx, double dy));
extern int _f_frotate P__ ((double theta));
extern int _f_fscale P__ ((double x, double y));
extern int _f_fspace P__ ((double x0, double y0, double x1, double y1));
extern int _f_fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _f_ftranslate P__ ((double x, double y));
extern int _f_havecap P__ ((const char *s));
extern int _f_joinmod P__ ((const char *s));
extern int _f_label P__ ((const char *s));
extern int _f_labelwidth P__ ((const char *s));
extern int _f_line P__ ((int x0, int y0, int x1, int y1));
extern int _f_linemod P__ ((const char *s));
extern int _f_linerel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _f_linewidth P__ ((int size));
extern int _f_marker P__ ((int x, int y, int type, int size));
extern int _f_markerrel P__ ((int dx, int dy, int type, int size));
extern int _f_move P__ ((int x, int y));
extern int _f_moverel P__ ((int x, int y));
extern int _f_openpl P__ ((void));
extern int _f_pencolor P__ ((int red, int green, int blue));
extern int _f_pencolorname P__ ((const char *name));
extern int _f_point P__ ((int x, int y));
extern int _f_pointrel P__ ((int dx, int dy));
extern int _f_restorestate P__((void));
extern int _f_savestate P__((void));
extern int _f_space P__ ((int x0, int y0, int x1, int y1));
extern int _f_space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _f_textangle P__ ((int angle));
extern void _f_retrieve_font P__((void));
extern void _f_set_fill_color P__((void));
extern void _f_set_pen_color P__((void));

extern FILE* _h_outfile P__((FILE* newstream));
extern double _h_falabel_pcl P__((const unsigned char *s, int h_just));
extern double _h_ffontname P__ ((const char *s));
extern double _h_ffontsize P__ ((double size));
extern double _h_flabelwidth P__ ((const char *s));
extern double _h_ftextangle P__ ((double angle));
extern int _h_alabel P__ ((int x_justify, int y_justify, const char *s));
extern int _h_arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _h_arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _h_box P__ ((int x0, int y0, int x1, int y1));
extern int _h_boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _h_capmod P__ ((const char *s));
extern int _h_circle P__ ((int x, int y, int r));
extern int _h_circlerel P__ ((int dx, int dy, int r));
extern int _h_closepl P__ ((void));
extern int _h_color P__ ((int red, int green, int blue));
extern int _h_colorname P__ ((const char *name));
extern int _h_cont P__ ((int x, int y));
extern int _h_contrel P__ ((int x, int y));
extern int _h_ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _h_ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _h_ellipse P__ ((int x, int y, int rx, int ry, int angle));
extern int _h_ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
extern int _h_endpath P__((void));
extern int _h_erase P__ ((void));
extern int _h_farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _h_farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _h_fbox P__ ((double x0, double y0, double x1, double y1));
extern int _h_fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _h_fcircle P__ ((double x, double y, double r));
extern int _h_fcirclerel P__ ((double dx, double dy, double r));
extern int _h_fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _h_fcont P__ ((double x, double y));
extern int _h_fcontrel P__ ((double x, double y));
extern int _h_fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _h_fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _h_fellipse P__ ((double x, double y, double rx, double ry, double angle));
extern int _h_fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
extern int _h_fillcolor P__ ((int red, int green, int blue));
extern int _h_fillcolorname P__ ((const char *name));
extern int _h_filltype P__ ((int level));
extern int _h_fline P__ ((double x0, double y0, double x1, double y1));
extern int _h_flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _h_flinewidth P__ ((double size));
extern int _h_flushpl P__ ((void));
extern int _h_fmarker P__ ((double x, double y, int type, double size));
extern int _h_fmarkerrel P__ ((double dx, double dy, int type, double size));
extern int _h_fmove P__ ((double x, double y));
extern int _h_fmoverel P__ ((double x, double y));
extern int _h_fontname P__ ((const char *s));
extern int _h_fontsize P__ ((int size));
extern int _h_fpoint P__ ((double x, double y));
extern int _h_fpointrel P__ ((double dx, double dy));
extern int _h_frotate P__ ((double theta));
extern int _h_fscale P__ ((double x, double y));
extern int _h_fspace P__ ((double x0, double y0, double x1, double y1));
extern int _h_fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _h_ftranslate P__ ((double x, double y));
extern int _h_havecap P__ ((const char *s));
extern int _h_joinmod P__ ((const char *s));
extern int _h_label P__ ((const char *s));
extern int _h_labelwidth P__ ((const char *s));
extern int _h_line P__ ((int x0, int y0, int x1, int y1));
extern int _h_linemod P__ ((const char *s));
extern int _h_linerel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _h_linewidth P__ ((int size));
extern int _h_marker P__ ((int x, int y, int type, int size));
extern int _h_markerrel P__ ((int dx, int dy, int type, int size));
extern int _h_move P__ ((int x, int y));
extern int _h_moverel P__ ((int x, int y));
extern int _h_openpl P__ ((void));
extern int _h_pencolor P__ ((int red, int green, int blue));
extern int _h_pencolorname P__ ((const char *name));
extern int _h_point P__ ((int x, int y));
extern int _h_pointrel P__ ((int dx, int dy));
extern int _h_restorestate P__((void));
extern int _h_savestate P__((void));
extern int _h_space P__ ((int x0, int y0, int x1, int y1));
extern int _h_space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _h_textangle P__ ((int angle));
extern void _h_set_attributes P__((void));
extern void _h_set_fill_color P__((void));
extern void _h_set_font P__((void));
extern void _h_set_pen_color P__((void));
extern void _h_set_position P__((void));

extern FILE* _m_outfile P__((FILE* newstream));
extern double _m_ffontname P__ ((const char *s));
extern double _m_ffontsize P__ ((double size));
extern double _m_flabelwidth P__ ((const char *s));
extern double _m_ftextangle P__ ((double angle));
extern int _m_alabel P__ ((int x_justify, int y_justify, const char *s));
extern int _m_arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _m_arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _m_bgcolor P__ ((int red, int green, int blue));
extern int _m_box P__ ((int x0, int y0, int x1, int y1));
extern int _m_boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _m_capmod P__ ((const char *s));
extern int _m_circle P__ ((int x, int y, int r));
extern int _m_circlerel P__ ((int dx, int dy, int r));
extern int _m_closepl P__ ((void));
extern int _m_color P__ ((int red, int green, int blue));
extern int _m_colorname P__ ((const char *name));
extern int _m_cont P__ ((int x, int y));
extern int _m_contrel P__ ((int x, int y));
extern int _m_ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _m_ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _m_ellipse P__ ((int x, int y, int rx, int ry, int angle));
extern int _m_ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
extern int _m_endpath P__((void));
extern int _m_erase P__ ((void));
extern int _m_farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _m_farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _m_fbox P__ ((double x0, double y0, double x1, double y1));
extern int _m_fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _m_fcircle P__ ((double x, double y, double r));
extern int _m_fcirclerel P__ ((double dx, double dy, double r));
extern int _m_fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _m_fcont P__ ((double x, double y));
extern int _m_fcontrel P__ ((double x, double y));
extern int _m_fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _m_fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _m_fellipse P__ ((double x, double y, double rx, double ry, double angle));
extern int _m_fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
extern int _m_fillcolor P__ ((int red, int green, int blue));
extern int _m_fillcolorname P__ ((const char *name));
extern int _m_filltype P__ ((int level));
extern int _m_fline P__ ((double x0, double y0, double x1, double y1));
extern int _m_flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _m_flinewidth P__ ((double size));
extern int _m_flushpl P__ ((void));
extern int _m_fmarker P__ ((double x, double y, int type, double size));
extern int _m_fmarkerrel P__ ((double dx, double dy, int type, double size));
extern int _m_fmove P__ ((double x, double y));
extern int _m_fmoverel P__ ((double x, double y));
extern int _m_fontname P__ ((const char *s));
extern int _m_fontsize P__ ((int size));
extern int _m_fpoint P__ ((double x, double y));
extern int _m_fpointrel P__ ((double dx, double dy));
extern int _m_frotate P__ ((double theta));
extern int _m_fscale P__ ((double x, double y));
extern int _m_fspace P__ ((double x0, double y0, double x1, double y1));
extern int _m_fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _m_ftranslate P__ ((double x, double y));
extern int _m_havecap P__ ((const char *s));
extern int _m_joinmod P__ ((const char *s));
extern int _m_label P__ ((const char *s));
extern int _m_labelwidth P__ ((const char *s));
extern int _m_line P__ ((int x0, int y0, int x1, int y1));
extern int _m_linemod P__ ((const char *s));
extern int _m_linerel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _m_linewidth P__ ((int size));
extern int _m_marker P__ ((int x, int y, int type, int size));
extern int _m_markerrel P__ ((int dx, int dy, int type, int size));
extern int _m_move P__ ((int x, int y));
extern int _m_moverel P__ ((int x, int y));
extern int _m_openpl P__ ((void));
extern int _m_pencolor P__ ((int red, int green, int blue));
extern int _m_pencolorname P__ ((const char *name));
extern int _m_point P__ ((int x, int y));
extern int _m_pointrel P__ ((int dx, int dy));
extern int _m_restorestate P__((void));
extern int _m_savestate P__((void));
extern int _m_space P__ ((int x0, int y0, int x1, int y1));
extern int _m_space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _m_textangle P__ ((int angle));

extern FILE* _p_outfile P__((FILE* newstream));
extern double _p_falabel_ps P__((const unsigned char *s, int h_just));
extern double _p_ffontname P__ ((const char *s));
extern double _p_ffontsize P__ ((double size));
extern double _p_ftextangle P__ ((double angle));
extern int _p_alabel P__ ((int x_justify, int y_justify, const char *s));
extern int _p_arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _p_arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _p_box P__ ((int x0, int y0, int x1, int y1));
extern int _p_boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _p_capmod P__ ((const char *s));
extern int _p_circle P__ ((int x, int y, int r));
extern int _p_circlerel P__ ((int dx, int dy, int r));
extern int _p_closepl P__ ((void));
extern int _p_color P__ ((int red, int green, int blue));
extern int _p_colorname P__ ((const char *name));
extern int _p_cont P__ ((int x, int y));
extern int _p_contrel P__ ((int x, int y));
extern int _p_ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _p_ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _p_ellipse P__ ((int x, int y, int rx, int ry, int angle));
extern int _p_ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
extern int _p_endpath P__((void));
extern int _p_erase P__ ((void));
extern int _p_farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _p_farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _p_fbox P__ ((double x0, double y0, double x1, double y1));
extern int _p_fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _p_fcircle P__ ((double x, double y, double r));
extern int _p_fcirclerel P__ ((double dx, double dy, double r));
extern int _p_fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _p_fcont P__ ((double x, double y));
extern int _p_fcontrel P__ ((double x, double y));
extern int _p_fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _p_fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _p_fellipse P__ ((double x, double y, double rx, double ry, double angle));
extern int _p_fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
extern int _p_fillcolor P__ ((int red, int green, int blue));
extern int _p_fillcolorname P__ ((const char *name));
extern int _p_filltype P__ ((int level));
extern int _p_fline P__ ((double x0, double y0, double x1, double y1));
extern int _p_flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _p_flinewidth P__ ((double size));
extern int _p_flushpl P__ ((void));
extern int _p_fmarker P__ ((double x, double y, int type, double size));
extern int _p_fmarkerrel P__ ((double dx, double dy, int type, double size));
extern int _p_fmove P__ ((double x, double y));
extern int _p_fmoverel P__ ((double x, double y));
extern int _p_fontname P__ ((const char *s));
extern int _p_fontsize P__ ((int size));
extern int _p_fpoint P__ ((double x, double y));
extern int _p_fpointrel P__ ((double dx, double dy));
extern int _p_frotate P__ ((double theta));
extern int _p_fscale P__ ((double x, double y));
extern int _p_fspace P__ ((double x0, double y0, double x1, double y1));
extern int _p_fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _p_ftranslate P__ ((double x, double y));
extern int _p_havecap P__ ((const char *s));
extern int _p_joinmod P__ ((const char *s));
extern int _p_label P__ ((const char *s));
extern int _p_labelwidth P__ ((const char *s));
extern int _p_line P__ ((int x0, int y0, int x1, int y1));
extern int _p_linemod P__ ((const char *s));
extern int _p_linerel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _p_linewidth P__ ((int size));
extern int _p_marker P__ ((int x, int y, int type, int size));
extern int _p_markerrel P__ ((int dx, int dy, int type, int size));
extern int _p_move P__ ((int x, int y));
extern int _p_moverel P__ ((int x, int y));
extern int _p_openpl P__ ((void));
extern int _p_pencolor P__ ((int red, int green, int blue));
extern int _p_pencolorname P__ ((const char *name));
extern int _p_point P__ ((int x, int y));
extern int _p_pointrel P__ ((int dx, int dy));
extern int _p_restorestate P__((void));
extern int _p_savestate P__((void));
extern int _p_space P__ ((int x0, int y0, int x1, int y1));
extern int _p_space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _p_textangle P__ ((int angle));
extern void _p_set_fill_color P__((void));
extern void _p_set_pen_color P__((void));

extern FILE* _t_outfile P__((FILE* newstream));
extern double _t_ffontname P__ ((const char *s));
extern double _t_ffontsize P__ ((double size));
extern double _t_flabelwidth P__ ((const char *s));
extern double _t_ftextangle P__ ((double angle));
extern int _t_alabel P__ ((int x_justify, int y_justify, const char *s));
extern int _t_arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _t_arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _t_box P__ ((int x0, int y0, int x1, int y1));
extern int _t_boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _t_capmod P__ ((const char *s));
extern int _t_circle P__ ((int x, int y, int r));
extern int _t_circlerel P__ ((int dx, int dy, int r));
extern int _t_closepl P__ ((void));
extern int _t_color P__ ((int red, int green, int blue));
extern int _t_colorname P__ ((const char *name));
extern int _t_cont P__ ((int x, int y));
extern int _t_contrel P__ ((int x, int y));
extern int _t_ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _t_ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _t_ellipse P__ ((int x, int y, int rx, int ry, int angle));
extern int _t_ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
extern int _t_endpath P__((void));
extern int _t_erase P__ ((void));
extern int _t_farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _t_farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _t_fbox P__ ((double x0, double y0, double x1, double y1));
extern int _t_fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _t_fcircle P__ ((double x, double y, double r));
extern int _t_fcirclerel P__ ((double dx, double dy, double r));
extern int _t_fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _t_fcont P__ ((double x, double y));
extern int _t_fcontrel P__ ((double x, double y));
extern int _t_fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _t_fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _t_fellipse P__ ((double x, double y, double rx, double ry, double angle));
extern int _t_fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
extern int _t_fillcolor P__ ((int red, int green, int blue));
extern int _t_fillcolorname P__ ((const char *name));
extern int _t_filltype P__ ((int level));
extern int _t_fline P__ ((double x0, double y0, double x1, double y1));
extern int _t_flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _t_flinewidth P__ ((double size));
extern int _t_flushpl P__ ((void));
extern int _t_fmarker P__ ((double x, double y, int type, double size));
extern int _t_fmarkerrel P__ ((double dx, double dy, int type, double size));
extern int _t_fmove P__ ((double x, double y));
extern int _t_fmoverel P__ ((double x, double y));
extern int _t_fontname P__ ((const char *s));
extern int _t_fontsize P__ ((int size));
extern int _t_fpoint P__ ((double x, double y));
extern int _t_fpointrel P__ ((double dx, double dy));
extern int _t_frotate P__ ((double theta));
extern int _t_fscale P__ ((double x, double y));
extern int _t_fspace P__ ((double x0, double y0, double x1, double y1));
extern int _t_fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _t_ftranslate P__ ((double x, double y));
extern int _t_havecap P__ ((const char *s));
extern int _t_joinmod P__ ((const char *s));
extern int _t_label P__ ((const char *s));
extern int _t_labelwidth P__ ((const char *s));
extern int _t_line P__ ((int x0, int y0, int x1, int y1));
extern int _t_linemod P__ ((const char *s));
extern int _t_linerel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _t_linewidth P__ ((int size));
extern int _t_marker P__ ((int x, int y, int type, int size));
extern int _t_markerrel P__ ((int dx, int dy, int type, int size));
extern int _t_move P__ ((int x, int y));
extern int _t_moverel P__ ((int x, int y));
extern int _t_openpl P__ ((void));
extern int _t_pencolor P__ ((int red, int green, int blue));
extern int _t_pencolorname P__ ((const char *name));
extern int _t_point P__ ((int x, int y));
extern int _t_pointrel P__ ((int dx, int dy));
extern int _t_restorestate P__((void));
extern int _t_savestate P__((void));
extern int _t_space P__ ((int x0, int y0, int x1, int y1));
extern int _t_space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _t_textangle P__ ((int angle));
extern void _t_set_attributes P__((void));
extern void _t_set_fill_color P__((void));
extern void _t_set_pen_color P__((void));

extern FILE* _x_outfile P__((FILE* newstream));
extern double _x_falabel_other P__((const unsigned char *s, int h_just));
extern double _x_flabelwidth_other P__((const unsigned char *s));
extern double _x_ffontname P__ ((const char *s));
extern double _x_ffontsize P__ ((double size));
extern double _x_flabelwidth P__ ((const char *s));
extern double _x_ftextangle P__ ((double angle));
extern int _x_alabel P__ ((int x_justify, int y_justify, const char *s));
extern int _x_arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _x_arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _x_bgcolor P__ ((int red, int green, int blue));
extern int _x_box P__ ((int x0, int y0, int x1, int y1));
extern int _x_boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _x_capmod P__ ((const char *s));
extern int _x_circle P__ ((int x, int y, int r));
extern int _x_circlerel P__ ((int dx, int dy, int r));
extern int _x_closepl P__ ((void));
extern int _x_color P__ ((int red, int green, int blue));
extern int _x_colorname P__ ((const char *name));
extern int _x_cont P__ ((int x, int y));
extern int _x_contrel P__ ((int x, int y));
extern int _x_ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _x_ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _x_ellipse P__ ((int x, int y, int rx, int ry, int angle));
extern int _x_ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
extern int _x_endpath P__((void));
extern int _x_erase P__ ((void));
extern int _x_farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _x_farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _x_fbox P__ ((double x0, double y0, double x1, double y1));
extern int _x_fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _x_fcircle P__ ((double x, double y, double r));
extern int _x_fcirclerel P__ ((double dx, double dy, double r));
extern int _x_fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _x_fcont P__ ((double x, double y));
extern int _x_fcontrel P__ ((double x, double y));
extern int _x_fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _x_fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _x_fellipse P__ ((double x, double y, double rx, double ry, double angle));
extern int _x_fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
extern int _x_fillcolor P__ ((int red, int green, int blue));
extern int _x_fillcolorname P__ ((const char *name));
extern int _x_filltype P__ ((int level));
extern int _x_fline P__ ((double x0, double y0, double x1, double y1));
extern int _x_flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
extern int _x_flinewidth P__ ((double size));
extern int _x_flushpl P__ ((void));
extern int _x_fmarker P__ ((double x, double y, int type, double size));
extern int _x_fmarkerrel P__ ((double dx, double dy, int type, double size));
extern int _x_fmove P__ ((double x, double y));
extern int _x_fmoverel P__ ((double x, double y));
extern int _x_fontname P__ ((const char *s));
extern int _x_fontsize P__ ((int size));
extern int _x_fpoint P__ ((double x, double y));
extern int _x_fpointrel P__ ((double dx, double dy));
extern int _x_frotate P__ ((double theta));
extern int _x_fscale P__ ((double x, double y));
extern int _x_fspace P__ ((double x0, double y0, double x1, double y1));
extern int _x_fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _x_ftranslate P__ ((double x, double y));
extern int _x_havecap P__ ((const char *s));
extern int _x_joinmod P__ ((const char *s));
extern int _x_label P__ ((const char *s));
extern int _x_labelwidth P__ ((const char *s));
extern int _x_line P__ ((int x0, int y0, int x1, int y1));
extern int _x_linemod P__ ((const char *s));
extern int _x_linerel P__ ((int dx0, int dy0, int dx1, int dy1));
extern int _x_linewidth P__ ((int size));
extern int _x_marker P__ ((int x, int y, int type, int size));
extern int _x_markerrel P__ ((int dx, int dy, int type, int size));
extern int _x_move P__ ((int x, int y));
extern int _x_moverel P__ ((int x, int y));
extern int _x_openpl P__ ((void));
extern int _x_pencolor P__ ((int red, int green, int blue));
extern int _x_pencolorname P__ ((const char *name));
extern int _x_point P__ ((int x, int y));
extern int _x_pointrel P__ ((int dx, int dy));
extern int _x_restorestate P__((void));
extern int _x_savestate P__((void));
extern int _x_space P__ ((int x0, int y0, int x1, int y1));
extern int _x_space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _x_textangle P__ ((int angle));
extern void _x_retrieve_font P__((void));
extern void _x_set_attributes P__ ((void));
extern void _x_set_fill_color P__((void));
extern void _x_set_pen_color P__((void));

extern int _y_openpl P__ ((void));
extern int _y_closepl P__ ((void));

#undef P__
