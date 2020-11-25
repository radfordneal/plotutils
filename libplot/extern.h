/* This is the chief include file for GNU libplot/libplotter.  It
   supplements the include files ../include/sys-defines.h,
   ../include/plot.h and ../include/plotter.h.  plot.h is libplot-specific,
   but plotter.h is included both in libplot and libplotter.  plotter.h
   defines what a Plotter object is (a struct for libplot, but a class for
   libplotter). */

/* This file contains many #defines and declarations of data structures.
   More importantly, it contains declarations of all the Plotter methods.
   They are declared differently, depending on whether we are compiling
   libplot (signalled if NOT_LIBPLOTTER is #defined) or libplotter.

   In libplot, the plotter operations are implemented as global functions
   that are members of the Plotter struct.  They are set up differently for
   the different types of Plotter; for example, the `openpl' slot in the
   struct contains the method _g_openpl for generic [i.e. base] Plotters,
   the method _m_openpl for MetaPlotters, etc.  The files ?_defplot.c
   contain the initializations that are used for the different types of
   Plotter.  In this file, if NOT_LIBPLOTTER is defined then each of these
   many methods is declared as a global function.

   In libplotter, the different types of Plotter are implemented as
   distinct classes, which are derived from the generic [i.e. base] Plotter
   class.  This file contains a great many #defines that are appropriate to
   that situation.  For example, _m_openpl is #defined to be
   MetaPlotter::openpl if NOT_LIBPLOTTER is not defined.  The MetaPlotter
   class, like all other Plotter classes, is defined in plotter.h. */


/*************************************************************************/
/* INCLUDE FILES                                         */
/*************************************************************************/

/* 0. OUR OWN INCLUDE FILES */

/* Determine which of libplot/libplotter this is, and include plot.h if the
   former. */
#ifndef LIBPLOTTER
#include "plot.h"
#define NOT_LIBPLOTTER
#endif

/* Always include plotter.h.  (If NOT_LIBPLOTTER is defined, it's an
   internal header file rather than a definition file for the Plotter
   class.) */
#include "plotter.h"

/* 1. SYSTEM INCLUDE FILES FOR THE X WINDOW SYSTEM */

#ifndef X_DISPLAY_MISSING
#include <X11/Xatom.h>
#include <X11/Xlib.h>		/* included also in plotter.h */
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#include <X11/extensions/multibuf.h>
#endif
#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#include <X11/extensions/Xdbe.h>
#endif
#ifdef USE_MOTIF
#include <Xm/Label.h>
#else
#include <X11/Xaw/Label.h>
#endif
#endif /* not X_DISPLAY_MISSING */


/*************************************************************************/
/* DEFINITIONS RELATED TO OUR FONT DATABASE (g_fontdb.c and g_fontd2.c)  */
/*************************************************************************/

/* The types of font we support.  The final type (`other') is a catchall,
   currently used for any user-specified font, with a name not contained in
   our font database, that can be retrieved from an X server. */
#define F_HERSHEY 0
#define F_POSTSCRIPT 1
#define F_PCL 2
#define F_STICK 3
#define F_OTHER 4

/* NUM_PS_FONTS and NUM_PCL_FONTS should agree with the number of fonts of
   those two types found in g_fontdb.c/g_fontd2.c.  These are also defined
   in plotter.h. */
#define NUM_PS_FONTS 35
#define NUM_PCL_FONTS 45

/* Default fonts, of each type.  Any Plotter has a `default_font_type'
   field, and the appropriate values are copied into the Plotter drawing
   state when the Plotter is first opened (see g_savestate.c).  The
   typeface and font indices index into the tables in
   g_fontdb.c/g_fontd2.c.  DEFAULT_HERSHEY_FONT is also used as a backup by
   some Plotters if no scalable (or anamorphically transformed, etc.) font
   can be retrieved; see e.g. f_retrieve.c and x_retrieve.c. */

#define DEFAULT_HERSHEY_FONT "HersheySerif"
#define DEFAULT_HERSHEY_TYPEFACE_INDEX 0
#define DEFAULT_HERSHEY_FONT_INDEX 1

#define DEFAULT_POSTSCRIPT_FONT "Helvetica"
#define DEFAULT_POSTSCRIPT_TYPEFACE_INDEX 0
#define DEFAULT_POSTSCRIPT_FONT_INDEX 1

#define DEFAULT_PCL_FONT "Arial-Roman"
#define DEFAULT_PCL_TYPEFACE_INDEX 5
#define DEFAULT_PCL_FONT_INDEX 1

#define DEFAULT_STICK_FONT "Stick"
#define DEFAULT_STICK_TYPEFACE_INDEX 3
#define DEFAULT_STICK_FONT_INDEX 1

/* HERSHEY FONTS */

/* our information about each of the 22 Hershey vector fonts in g_fontdb.c,
   and the typefaces they belong to */
struct hershey_font_info_struct 
{
  const char *name;		/* font name */
  const char *othername;	/* an alias (for backward compatibility) */
  const char *orig_name;	/* Allen Hershey's original name for it */
  short chars[256];		/* array of vector glyphs */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
  bool visible;			/* whether font is visible, i.e. not internal*/
};

extern const struct hershey_font_info_struct _hershey_font_info[];

/* This numbering should agree with the numbering of Hershey fonts in the
   _hershey_font_info[] array in g_fontdb.c. */
#define HERSHEY_SERIF 0
#define HERSHEY_SERIF_ITALIC 1
#define HERSHEY_SERIF_BOLD 2
#define HERSHEY_CYRILLIC 4
#define HERSHEY_HIRAGANA 6	/* hidden font */
#define HERSHEY_KATAKANA 7	/* hidden font */
#define HERSHEY_EUC 8
#define HERSHEY_GOTHIC_GERMAN 16
#define HERSHEY_SERIF_SYMBOL 18

/* accented character information (used in constructing Hershey ISO-Latin-1
   accented characters, see table in g_fontdb.c) */
struct accented_char_info_struct
{
  unsigned char composite, character, accent;
};

extern const struct accented_char_info_struct _hershey_accented_char_info[];

/* types of accent, for a composite character in a Hershey font */
#define ACC0 (16384 + 0)	/* superimpose on character */
#define ACC1 (16384 + 1)	/* elevate by 7 Hershey units */
#define ACC2 (16384 + 2)	/* same, also shift right by 2 units */

/* a flag in a Hershey glyph number indicating a `small Kana' */
#define KS 8192			/* i.e. 0x200 */

/* HERSHEY VECTOR GLYPHS */

/* arrays of Hershey vector glyphs in g_her_glyph.c */
extern const char * const _occidental_hershey_glyphs[];
extern const char * const _oriental_hershey_glyphs[];

/* position of `undefined character' symbol (a bundle of horizontal lines)
   in the Hershey _occidental_hershey_glyphs[] array */
#define UNDE 4023

/* POSTSCRIPT FONTS */

/* our information about each of the 35 standard PS fonts in g_fontdb.c,
   and the typefaces they belong to */
struct ps_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
  const char *ps_name_alt;	/* alternative PS font name, if non-NULL */
  const char *x_name;		/* the X Windows font name */
  const char *x_name_alt;	/* alternative X Windows font name */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=variable */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  int fig_id;			/* Fig's font id */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern const struct ps_font_info_struct _ps_font_info[];

/* PCL FONTS */

/* our information about each of the 45 PCL fonts in g_fontdb.c, and the
   typefaces they belong to */
struct pcl_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
  const char *substitute_ps_name; /* alt. name when in a PS file, if non-NULL*/
  const char *x_name;		/* the X Windows font name */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=variable */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern const struct pcl_font_info_struct _pcl_font_info[];

/* STICK FONTS */

/* our information about each of the Stick fonts (i.e., vector fonts
   resident in HP's devices) listed in g_fontdb.c, and the typefaces they
   belong to */
struct stick_font_info_struct 
{
  const char *ps_name;		/* the postscript font name */
  bool basic;			/* basic stick font (supp. on all devices)? */
  int pcl_typeface;		/* the PCL typeface number */
  int pcl_spacing;		/* 0=fixed width, 1=variable */
  int pcl_posture;		/* 0=upright, 1=italic, etc. */
  int pcl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int pcl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1 */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int raster_width_lower;	/* width of abstract raster (lower half) */
  int raster_height_lower;	/* height of abstract raster (lower half) */
  int raster_width_upper;	/* width of abstract raster (upper half) */
  int raster_height_upper;	/* height of abstract raster (upper half) */
  int hpgl_charset_lower;	/* old HP character set number (lower half) */
  int hpgl_charset_upper;	/* old HP character set number (upper half) */
  int kerning_table_lower;	/* number of a kerning table (lower half) */
  int kerning_table_upper;	/* number of a kerning table (upper half) */
  char width[256];		/* per-character width information */
  int offset;			/* left edge (applies to all chars) */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* encoding is iso8859-1? (after reencoding) */
};

extern const struct stick_font_info_struct _stick_font_info[];

/* Device-resident kerning data (`spacing table' in HP documentation),
   indexed by `right edge character class' and `left edge character class',
   i.e., `row class' and `column class'.  There are three such spacing
   tables, shared among old-style HP character sets of size 128, and hence
   among our Stick fonts.  See the article by L. W. Hennessee et al. in the
   Nov. 1981 issue of the Hewlett-Packard Journal. */
struct stick_spacing_table_struct
{
  int rows, cols;
  const short *kerns;
};

extern const struct stick_spacing_table_struct _stick_spacing_tables[];

/* Kerning tables for 128-character halves of our Stick fonts.  A kerning
   table for the lower or upper half of one of our 256-character fonts
   specifies a spacing table (see above), and maps each character in the
   half-font to the appropriate row and column class. */
struct stick_kerning_table_struct
{
  int spacing_table;
  char row[128], col[128];	/* we use char's as very short int's */
};

extern const struct stick_kerning_table_struct _stick_kerning_tables[];

/* TYPEFACES */

/* typeface information, applicable to all four sorts of font in our font
   database (Hershey, PS, PCL, Stick) */

#define FONTS_PER_TYPEFACE 10	/* maximum */

struct typeface_info_struct
{
  int numfonts;
  int fonts[FONTS_PER_TYPEFACE];
};

extern const struct typeface_info_struct _hershey_typeface_info[];
extern const struct typeface_info_struct _ps_typeface_info[];
extern const struct typeface_info_struct _pcl_typeface_info[];
extern const struct typeface_info_struct _stick_typeface_info[];


/***********************************************************************/
/* GENERAL DEFINITIONS, TYPEDEFS, & EXTERNAL VARIABLES                 */
/***********************************************************************/

/* miscellaneous data types */

typedef Point Vector;

typedef struct
{
  const char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Colornameinfo;

/* Initializations for default values of Plotter data members, performed
   when space() is first called.  Latter doesn't apply to Plotters whose
   displays are of type DISP_INTEGER; the default for such Plotters is to
   use zero-width (i.e. Bresenham) lines. */
#define DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE (1.0/50.0)
#define DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE (1.0/850.0)

/* justification types for labels (our numbering is the same as xfig's;
   don't alter without checking the code) */
#define JUST_LEFT 0
#define JUST_CENTER 1
#define JUST_RIGHT 2

/* fill rules (our internal numbering) */
#define FILL_ODD_WINDING 0	/* i.e. `even-odd' fill */
#define FILL_NONZERO_WINDING 1

/* canonical line styles (our internal numbering, must agree with the
   ordering in g_dash2.c) */
#define NUM_LINE_STYLES 7
#define L_SOLID 0
#define L_DOTTED 1
#define L_DOTDASHED 2
#define L_SHORTDASHED 3
#define L_LONGDASHED 4
#define L_DOTDOTDASHED 5
#define L_DOTDOTDOTDASHED 6

/* maximum length of dash array for our canonical line styles (see line
   style database in g_dash2.c; for example "dotted" corresponds to
   length-2 dash array [ 1 3 ] ) */
#define MAX_DASH_ARRAY_LEN 8

typedef struct
{
  const char *name;		/* user-level name (e.g. "dotted") */
  int type;			/* internal number (e.g. L_DOTTED) */
  int dash_array_len;		/* length of dash array for this style */
  int dash_array[MAX_DASH_ARRAY_LEN]; /* dash array for this style */
} LineStyle;

extern const LineStyle _line_styles[NUM_LINE_STYLES];

/* when using a canonical line style, numbers appearing in the dash array,
   specifying dash/gap distances, mean multiples of the line width, except
   the following floor is put on the line width */
#define MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE (1.0/576.0)

/* cap and join types (our internal numbering) */

#define JOIN_MITER 0
#define JOIN_ROUND 1
#define JOIN_BEVEL 2
#define JOIN_TRIANGULAR 3

#define CAP_BUTT 0
#define CAP_ROUND 1
#define CAP_PROJECT 2
#define CAP_TRIANGULAR 3

/* Display type: a Plotter's display is classified as one of these.

   An `integer' display uses integer coordinates, the correspondence to
   physical distances being unknown.  A `physical' display is one whose
   size is specified as one of the known page types (e.g. "letter", "a4").
   Physical displays may also use integer coordinates.  For example,
   Fig Plotters do, but PS Plotters don't. */

#define DISP_UNKNOWN 0
#define DISP_INTEGER 1
#define DISP_PHYSICAL 2

/* Structure used for specifying a known page type (e.g. "letter", "a4";
   see our database of known page types in g_pagetype.h) */
typedef struct
{
  const char *name;		/* page type, e.g. "letter" */
  bool metric;			/* metric vs. Imperial, advisory only */
  Displaycoors fig, ps, hpgl;	/* coors of corners of display (device-dep.) */
} Pagedata;

/* The user->device coordinate transformation */

/* X, Y Device: transform user coordinates to device coordinates */
#ifdef __GNUC__
#define XD(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; m[4] + _x * m[0] + _y * m[2]; })
#define YD(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; m[5] + _x * m[1] + _y * m[3]; })
#else
#define XD(x,y) (_plotter->drawstate->transform.m[4] + (x) * _plotter->drawstate->transform.m[0] + (y) * _plotter->drawstate->transform.m[2])
#define YD(x,y) (_plotter->drawstate->transform.m[5] + (x) * _plotter->drawstate->transform.m[1] + (y) * _plotter->drawstate->transform.m[3])
#endif

/* X,Y Device Vector: transform user vector to device vector */
#ifdef __GNUC__
#define XDV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; m[0] * _x + m[2] * _y; })
#define YDV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; m[1] * _x + m[3] * _y; })
#else
#define XDV(x,y) (_plotter->drawstate->transform.m[0] * (x) + _plotter->drawstate->transform.m[2] * (y))
#define YDV(x,y) (_plotter->drawstate->transform.m[1] * (x) + _plotter->drawstate->transform.m[3] * (y))
#endif

/* X, Y User Vector: transform device vector back to user vector 
   (used by X11 driver only) */
#ifdef __GNUC__
#define XUV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; (m[3] * _x - m[2] * _y) / (m[0] * m[3] - m[1] * m[2]); })
#define YUV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; (- m[1] * _x + m[0] * _y) / (m[0] * m[3] - m[1] * m[2]); })
#else
#define XUV(x,y) ((_plotter->drawstate->transform.m[3] * (x) - _plotter->drawstate->transform.m[2] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))
#define YUV(x,y) ((- _plotter->drawstate->transform.m[1] * (x) + _plotter->drawstate->transform.m[0] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))
#endif


/*************************************************************************/
/* MISC. DEFS on POLYLINES and ARCS (relevant to all or most display devices)*/
/*************************************************************************/

/* Default value for the miter limit (see comments in g_miter.c).  This
   is the value used by X11: it chops off all mitered line joins if the
   join angle is less than 11 degrees. */
#define DEFAULT_MITER_LIMIT 10.4334305246

/* Default length an unfilled polyline is allowed to grow to, before it is
   flushed out by an automatic invocation of endpath(), provided
   _plotter->flush_long_polylines is true.  (We don't flush filled
   polylines, since they need to be preserved as discrete objects if the
   filling is to be performed properly). */
#define MAX_UNFILLED_POLYLINE_LENGTH 500
#define MAX_UNFILLED_POLYLINE_LENGTH_STRING "500"

/* Maximum number of times a circular or elliptic arc is recursively
   subdivided, when it is being approximated by an inscribed polyline.  The
   polyline will contain no more than 2**MAX_ARC_SUBDIVISIONS line
   segments.  MAX_ARC_SUBDIVISIONS must be no longer than
   TABULATED_ARC_SUBDIVISIONS below (see g_arc.h). */
#define MAX_ARC_SUBDIVISIONS 5 	/* to avoid buffer overflow on HP7550[A|B] */

/* Types of circular/elliptic arc (which may or may not be drawn via
   recursive subdivision).  These index into the table in g_arc.h. */
#define NUM_ARC_TYPES 4

#define QUARTER_ARC 0
#define HALF_ARC 1
#define THREE_QUARTER_ARC 2
#define USER_DEFINED_ARC 3

#define TABULATED_ARC_SUBDIVISIONS 15	/* length of each table entry */

/* A `path' in libplot/libplotter is a continuous sequence of line
   segments, circular arc segments, elliptic arc segments, quadratic Bezier
   segments, or cubic Bezier segments.  It is represented internally as a
   list of GeneralizedPoints (see plotter.h).  These are allowed values for
   the type field. */

#define S_LINE 0
#define S_ARC 1
#define S_ELLARC 2
#define S_QUAD 3
#define S_CUBIC 4

/* Values for data members `allowed_{arc|ellarc|quad|cubic}_scaling' of any
   Plotter.  They specify which sorts of user->device frame transformation
   are allowed, if an arc or Bezier curve that is part of a path is to be
   output as a primitive rather than approximated as a polyline.

   When the user draws a path incrementally, we store an arc or Bezier in
   our storage buffer only if the current transformation agrees with this
   constraint.  If it doesn't, we store an approximation: an inscribed
   polyline.

   The values AS_UNIFORM and AS_AXES_PRESERVED are used for
   circular/elliptic arcs; not for Beziers.  For example, X Plotters can
   drawing elliptic arcs as primitives only if the axes of the ellipse are
   aligned with the coordinate axes in device space.  So in an X Plotter,
   the allowed_{arc|ellarc}_scaling fields are set to AS_AXES_PRESERVED. */

#define AS_NONE 0		/* primitive not supported at all */
#define AS_UNIFORM 1		/* supported only if transf. is uniform  */
#define AS_AXES_PRESERVED 2	/* supported only if transf. preserves axes */
#define AS_ANY 3		/* supported irrespective of transformation */


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
   display [488..3607]x[0..3119].  See g_clipper.c. */

#define CLIP_FUZZ 0.0000001
#define TEK_DEVICE_X_MIN_CLIP (-0.5 + CLIP_FUZZ)
#define TEK_DEVICE_X_MAX_CLIP (4095.5 - CLIP_FUZZ)
#define TEK_DEVICE_Y_MIN_CLIP  (-0.5 + CLIP_FUZZ)
#define TEK_DEVICE_Y_MAX_CLIP (3119.5 - CLIP_FUZZ)

/* Tektronix modes (our private numbering, values are not important but
   order definitely is, see t_tek_md.c) */

#define MODE_ALPHA 0
#define MODE_PLOT 1
#define MODE_POINT 2
#define MODE_INCREMENTAL 3	/* currently not used */

/* Tektronix display types */
#define D_GENERIC 0
#define D_KERMIT 1
#define D_XTERM 2

/* colors supported by MS-DOS kermit Tek emulation, see t_color2.c */

#define KERMIT_NUM_STD_COLORS 16
extern const Color _kermit_stdcolors[KERMIT_NUM_STD_COLORS];
extern const char * const _kermit_fgcolor_escapes[KERMIT_NUM_STD_COLORS];
extern const char * const _kermit_bgcolor_escapes[KERMIT_NUM_STD_COLORS];
/* must agree with the ordering in t_color2.c */
#define ANSI_SYS_BLACK   0
#define ANSI_SYS_GRAY30  8
#define ANSI_SYS_GRAY55  7
#define ANSI_SYS_WHITE  15


/************************************************************************/
/* HP-GL device driver */
/************************************************************************/

/* An HPGLPlotter plots using virtual device coordinates: not the native
   device coordinates, but rather scaled coordinates in which the graphics
   display is [0,10000]x[0,10000].  To arrange this, in the initialization
   code in h_defplot.c we move the HP-GL `scaling points' to the lower left
   and upper right corners of our graphics display, and use the HP-GL `SC'
   instruction to set up a scaled set of coordinates. */
#define HPGL_SCALED_DEVICE_LEFT 0
#define HPGL_SCALED_DEVICE_RIGHT 10000
#define HPGL_SCALED_DEVICE_BOTTOM 0
#define HPGL_SCALED_DEVICE_TOP 10000

#define HPGL_UNITS_PER_INCH 1016 /* 1 HP-GL unit = 1/40 mm */

/* HP-GL line attribute types (HP-GL numbering; see h_attribs.c) */
#define HPGL_L_SOLID (-100)	/* no numeric parameter at all */
#define HPGL_L_DOTTED 1
#define HPGL_L_DOTDASHED 5
#define HPGL_L_SHORTDASHED 2
#define HPGL_L_LONGDASHED 3
#define HPGL_L_DOTDOTDASHED 6
#define HPGL_L_DOTDOTDOTDASHED (-10) /* pseudo */

#define HPGL_JOIN_MITER 1	/* miter length is clamped by miter limit */
#define HPGL_JOIN_MITER_BEVEL 2	/* miter or bevel, based on miter limit */
#define HPGL_JOIN_TRIANGULAR 3
#define HPGL_JOIN_ROUND 4
#define HPGL_JOIN_BEVEL 5

#define HPGL_CAP_BUTT 1
#define HPGL_CAP_PROJECT 2
#define HPGL_CAP_TRIANGULAR 3
#define HPGL_CAP_ROUND 4

/* HP-GL and HP-GL/2 fill types (HP-GL/2 numbering) */
#define HPGL_FILL_SOLID_BI 1
#define HPGL_FILL_SOLID_UNI 2
#define HPGL_FILL_SHADING 10

/* Maximum number of pens, or logical pens, for an HP-GL/2 device.  Some
   such devices permit as many as 256, but all should permit at least 32.
   Our pen numbering will range over 0..HPGL2_MAX_NUM_PENS-1. */
#define HPGL2_MAX_NUM_PENS 32

/* default values for HPGL_PENS environment variable, for HP-GL[/2]; this
   lists available pens and their positions in carousel */
#define DEFAULT_HPGL_PEN_STRING "1=black"
#define DEFAULT_HPGL2_PEN_STRING "1=black:2=red:3=green:4=yellow:5=blue:6=magenta:7=cyan"

/* PCL 5 typeface information: symbol set, i.e. encoding */
#define PCL_ISO_8859_1 14
#define PCL_ROMAN_8 277

/* Default HP-GL/2 typeface */
#define STICK_TYPEFACE 48

/* Old (pre-HP-GL/2) 7-bit HP-GL character sets */
#define HP_ASCII 0
#define HP_ROMAN_EXTENSIONS 7

/* The nominal HP-GL/2 fontsize we use for drawing a label (for fixed-width
   and proportional fonts, respectively).  We retrieve fonts in the size
   specified by whichever of the two following parameters is relevant, and
   then rescale it as needed before drawing the label. */
#define NOMINAL_CHARS_PER_INCH 8.0
#define NOMINAL_POINT_SIZE 18

/* Spacing characteristic of the PCL and Stick fonts, in HP-GL/2 */
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
   dashes, except in the DASHDOTTED case).  */
#define FIG_L_DEFAULT (-1)
#define FIG_L_SOLID 0
#define FIG_L_DASHED 1
#define FIG_L_DOTTED 2
#define FIG_L_DASHDOTTED 3
#define FIG_L_DASHDOUBLEDOTTED 4
#define FIG_L_DASHTRIPLEDOTTED 5

/* Fig's line styles, indexed into by internal line style number
   (L_SOLID/L_DOTTED/ L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED. */
extern const int _fig_line_style[NUM_LINE_STYLES];

/* Fig's `style value', i.e. inter-dot length or dash length, indexed into
   by internal line style number (L_SOLID/L_DOTTED/
   L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED; dash length ignored for
   L_SOLID).  Units are Fig display units.  See f_endpath.c. */
extern const double _fig_dash_length[NUM_LINE_STYLES];

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

#define FIG_STD_COLOR_MIN 0	/* see f_color2.c for colors 0,...,31 */
#define C_BLACK 0		/* i.e. #0 in table in f_color2.c */
#define C_WHITE 7		/* i.e. #7 in table */
#define FIG_NUM_STD_COLORS 32
#define FIG_USER_COLOR_MIN 32
#define FIG_MAX_NUM_USER_COLORS 512 /* also defined in plotter.h */
extern const Color _fig_stdcolors[FIG_NUM_STD_COLORS];

/* xfig's depth attribute ranges from 0 to FIG_MAXDEPTH. */
#define FIG_MAXDEPTH 999

/* depth of the first object we'll draw (we make it a bit less than
   FIG_MAXDEPTH, since the user may wish to edit the drawing with xfig to
   include deeper, i.e. obscured objects) */
#define FIG_INITIAL_DEPTH 989


/************************************************************************/
/* Postscript/idraw device driver */
/************************************************************************/

/* minimum desired resolution in device frame (i.e. in printer's points) */
#define PS_MIN_RESOLUTION 0.05

#define POINT_PS_SIZE 0.5  /* em size (in printer's points) for a font in
			      which a `point' could appear as a symbol */

/* 16-bit line style brush arrays for idraw, see p_endpath.c/p_ellipse.c, 
   indexed by our internal line style numbering (L_SOLID, L_DOTTED, etc.) */
extern const long _idraw_brush_pattern[NUM_LINE_STYLES];

/* PS line join and line cap styles [also used by AI device driver] */

#define PS_JOIN_MITER 0
#define PS_JOIN_ROUND 1
#define PS_JOIN_BEVEL 2

#define PS_CAP_BUTT 0
#define PS_CAP_ROUND 1
#define PS_CAP_PROJECT 2

/* PS join and cap styles, see p_endpath.c, indexed by our internal join
   and cap type numbering (miter/rd./bevel/triangular and
   butt/rd./project/triangular) */
extern const int _ps_join_style[], _ps_cap_style[];

/* information on colors known to idraw, see p_color2.c */

#define IDRAW_NUM_STD_COLORS 12
extern const Color _idraw_stdcolors[IDRAW_NUM_STD_COLORS];
extern const char * const _idraw_stdcolornames[IDRAW_NUM_STD_COLORS];

/* information on shadings known to idraw, see p_color2.c */

#define IDRAW_NUM_STD_SHADINGS 5
extern const double _idraw_stdshadings[IDRAW_NUM_STD_SHADINGS];


/************************************************************************/
/* Adobe Illustrator device driver */
/************************************************************************/

/* types of Illustrator file format that an Illustrator Plotter can emit */
#define AI_VERSION_3 0
#define AI_VERSION_5 1

#define POINT_AI_SIZE 0.5  /* em size (in printer's points) for a font in
			      which a `point' could appear as a symbol */

/* AI fill rule types (in AI version 5 and later) */
#define AI_FILL_NONZERO_WINDING 0
#define AI_FILL_ODD_WINDING 1


/************************************************************************/
/* XDrawable and X device drivers */
/************************************************************************/

#ifndef X_DISPLAY_MISSING 

/* X11 colormap types (XDrawable Plotters use only the first of these) */
#define CMAP_ORIG 0
#define CMAP_NEW 1
#define CMAP_BAD 2  /* colormap full, can't allocate new colors */

/* sixteen-bit restriction on X11 protocol parameters */
#define XOOB_UNSIGNED(x) ((x) > (int)0xffff)
#define XOOB_INT(x) ((x) > (int)0x7fff || (x) < (int)(-0x8000))

/* double buffering types, used in XDrawablePlotter `x_double_buffering'
   data member */
#define DBL_NONE 0
#define DBL_BY_HAND 1
#define DBL_MBX 2		/* X11 MBX extension */
#define DBL_DBE 3		/* X11 DBE extension */

#endif /* not X_DISPLAY_MISSING */


/***********************************************************************/
/* DRAWING STATE                                                       */
/***********************************************************************/

/* Default drawing state, defined in g_defstate.c.  This is used for
   initialization of the first state on the drawing state stack that every
   Plotter maintains; see g_savestate.c. */
extern const pl_DrawState _default_drawstate;

/*************************************************************************/
/* PLOTTER OBJECTS (structs for libplot, class instances for libplotter) */
/*************************************************************************/

/* In libplot, "_plotter" is a pointer to the currently selected Plotter
   (after startup, one is always selected; see the libplot-specific file
   api.c).  But in libplotter, it's an alias for "this", since it's used
   only within methods of the base Plotter class or its derived classes. */

#ifndef LIBPLOTTER
extern Plotter *_plotter;
#else
#define _plotter this
#endif

/* "_plotters" is a sparse array containing pointers to all Plotter
   instances, of size "_plotters_len".  In libplot, they're globals, but in
   in libplotter, they're static data members of the base Plotter class.
   In both libraries, they're defined in g_defplot.c.

   Similarly, "_xplotters" is a sparse array containing pointers to all
   XPlotters instances, of size "_xplotters_len".  In libplot, they're
   globals, but in libplotter, they're static data members of the XPlotter
   class.  In both libraries, they're defined in y_defplot.c. */

#ifndef LIBPLOTTER
extern Plotter **_plotters;
extern int _plotters_len;
#define XPlotter Plotter	/* crock, needed by code in y_defplot.c */
extern XPlotter **_xplotters;
extern int _xplotters_len;
#else
#define _plotters Plotter::_plotters
#define _plotters_len Plotter::_plotters_len
#define _xplotters XPlotter::_xplotters
#define _xplotters_len XPlotter::_xplotters_len
#endif

#ifndef LIBPLOTTER
/* In libplot, these are the initializations of the function-pointer parts
   of the different types of Plotter.  They are copied to the Plotter at
   creation time (in api.c, which is libplot-specific). */
extern const Plotter _g_default_plotter, _m_default_plotter, _t_default_plotter, _h_default_plotter, _q_default_plotter, _f_default_plotter, _p_default_plotter, _a_default_plotter, _i_default_plotter, _n_default_plotter, _x_default_plotter, _y_default_plotter;
#endif /* not LIBPLOTTER */


/**************************************************************************/
/* PROTOTYPES for libplot and libplotter */
/**************************************************************************/

/* The P___ macro elides the argument prototypes if the compiler does not
   support them. */
#ifdef P___
#undef P___
#endif
#if defined (__STDC__) || defined (_AIX) \
	|| (defined (__mips) && defined (_SYSTYPE_SVR4)) \
	|| defined(WIN32) || defined(__cplusplus)
#define P___(protos) protos
#else
#define P___(protos) ()
#endif

/* Miscellaneous internal functions, which aren't Plotter class methods, so
   they're declared the same for both libplot and libplotter. */

extern Outbuf * _new_outbuf P___((void));
extern Point _truecenter P___((Point p0, Point p1, Point pc));
extern Vector *_vscale P___((Vector *v, double newlen));
extern Voidptr _get_default_plot_param P___((const char *parameter)); 
extern Voidptr _plot_xcalloc P___((unsigned int nmemb, unsigned int size));
extern Voidptr _plot_xmalloc P___((unsigned int size));
extern Voidptr _plot_xrealloc P___((Voidptr p, unsigned int size));
extern bool _clean_iso_string P___((unsigned char *s));
extern bool _string_to_color P___((const char *name, const Colornameinfo **info_p));
extern const Pagedata * _pagetype P___((const char *name));
extern double _matrix_norm P___((const double m[6]));
extern double _xatan2 P___((double y, double x));
extern int _clip_line P___((double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip));
extern int _codestring_len P___((const unsigned short *codestring));
extern void _bbox_of_outbuf P___((Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax));
extern void _bbox_of_outbufs P___((Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax));
extern void _delete_outbuf P___((Outbuf *outbuf));
extern void _freeze_outbuf P___((Outbuf *outbuf));
extern void _matrix_product P___((const double m[6], const double n[6], double product[6]));
extern void _matrix_sing_vals P___((const double m[6], double *min_sing_val, double *max_sing_val));
extern void _reset_outbuf P___((Outbuf *outbuf));
extern void _update_bbox P___((Outbuf *bufp, double x, double y));
extern void _update_buffer P___((Outbuf *outbuf));

/* Renamings of global functions in the MI scan conversion module (defined
   in the files g_mi*.c) to ensure they all begin with an underscore.  Not
   necessary, but it's wise not to fill up the user-level namespace. */
#define miAppendSpans _miAppendSpans
#define miCreateETandAET _miCreateETandAET
#define miFillConvexPoly _miFillConvexPoly
#define miFillGeneralPoly _miFillGeneralPoly
#define miFillPolygon _miFillPolygon
#define miFillSpanGroup _miFillSpanGroup
#define miFillSpans _miFillSpans
#define miFillSppPoly _miFillSppPoly
#define miFillUniqueSpanGroup _miFillUniqueSpanGroup
#define miFreeSpanGroup _miFreeSpanGroup
#define miFreeStorage _miFreeStorage
#define miInitSpanGroup _miInitSpanGroup
#define miInsertionSort _miInsertionSort
#define miPolyArc _miPolyArc
#define miPolyFillArc _miPolyFillArc
#define miPolyFillRect _miPolyFillRect
#define miPolyPoint _miPolyPoint
#define miStepDash _miStepDash
#define miWideDash _miWideDash
#define miWideLine _miWideLine
#define miZeroDash _miZeroDash
#define miZeroLine _miZeroLine
#define miZeroPolyArc _miZeroPolyArc
#define micomputeWAET _micomputeWAET
#define miloadAET _miloadAET

/* Internal functions that aren't Plotter class methods, but which need to
   be renamed in libplotter, so that both libplot and libplotter can be
   loaded simultaneously. */
#ifdef LIBPLOTTER
#define libplot_warning_handler libplotter_warning_handler
#define libplot_error_handler libplotter_error_handler
#endif

/* Declarations of forwarding functions defined in api.c, used in libplot
   (not libplotter).  These support the derivation of the PCLPlotter class
   from the HPGLPlotter class, the MetaPlotter class from the Plotter
   class, and the XPlotter class from the XDrawablePlotter class. */

#ifndef LIBPLOTTER
extern void _maybe_switch_to_hpgl P___((void));
extern void _maybe_switch_from_hpgl P___((void));
extern void _recompute_device_line_width P___((void));
#ifndef X_DISPLAY_MISSING
extern void _maybe_get_new_colormap P___((void));
extern void _maybe_handle_x_events P___((void));
#endif /* not X_DISPLAY_MISSING */
#endif /* not LIBPLOTTER */

/* Declarations of the Plotter methods and the device-specific versions of
   same.  The initial letter indicates the Plotter subclass specificity.
   g=generic (i.e. base Plotter class), m=metafile, t=Tektronix, h=HP-GL/2
   and PCL 5, f=xfig, p=PS, a=Adobe Illustrator, i=GIF, n=PNM
   (i.e. PBM/PGM/PPM), x=X11 Drawable, y=X11.

   In libplot, these are definitions of global functions.  But in
   libplotter, we use #define and the double colon notation to make them
   function members of the appropriate classes. */

#ifndef LIBPLOTTER
/* Plotter public methods, for libplot */
extern FILE* _g_outfile P___((FILE* newstream));/* OBSOLESCENT */
extern double _g_ffontname P___((const char *s));
extern double _g_ffontsize P___((double size));
extern double _g_flabelwidth P___((const char *s));
extern double _g_ftextangle P___((double angle));
extern int _g_alabel P___((int x_justify, int y_justify, const char *s));
extern int _g_arc P___((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _g_arcrel P___((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _g_bezier2 P___((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _g_bezier2rel P___((int dx0, int dy0, int dx1, int dy1, int dx2, int dy2));
extern int _g_bezier3 P___((int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3));
extern int _g_bezier3rel P___((int dx0, int dy0, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3));
extern int _g_bgcolor P___((int red, int green, int blue));
extern int _g_bgcolorname P___((const char *name));
extern int _g_box P___((int x0, int y0, int x1, int y1));
extern int _g_boxrel P___((int dx0, int dy0, int dx1, int dy1));
extern int _g_capmod P___((const char *s));
extern int _g_circle P___((int x, int y, int r));
extern int _g_circlerel P___((int dx, int dy, int r));
extern int _g_closepl P___((void));
extern int _g_color P___((int red, int green, int blue));
extern int _g_colorname P___((const char *name));
extern int _g_cont P___((int x, int y));
extern int _g_contrel P___((int x, int y));
extern int _g_ellarc P___((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _g_ellarcrel P___((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _g_ellipse P___((int x, int y, int rx, int ry, int angle));
extern int _g_ellipserel P___((int dx, int dy, int rx, int ry, int angle));
extern int _g_endpath P___((void));
extern int _g_erase P___((void));
extern int _g_farc P___((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _g_farcrel P___((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _g_fbezier2 P___((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _g_fbezier2rel P___((double dx0, double dy0, double dx1, double dy1, double dx2, double dy2));
extern int _g_fbezier3 P___((double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3));
extern int _g_fbezier3rel P___((double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3));
extern int _g_fbox P___((double x0, double y0, double x1, double y1));
extern int _g_fboxrel P___((double dx0, double dy0, double dx1, double dy1));
extern int _g_fcircle P___((double x, double y, double r));
extern int _g_fcirclerel P___((double dx, double dy, double r));
extern int _g_fconcat P___((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _g_fcont P___((double x, double y));
extern int _g_fcontrel P___((double x, double y));
extern int _g_fellarc P___((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _g_fellarcrel P___((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _g_fellipse P___((double x, double y, double rx, double ry, double angle));
extern int _g_fellipserel P___((double dx, double dy, double rx, double ry, double angle));
extern int _g_fillcolor P___((int red, int green, int blue));
extern int _g_fillcolorname P___((const char *name));
extern int _g_fillmod P___((const char *s));
extern int _g_filltype P___((int level));
extern int _g_fline P___((double x0, double y0, double x1, double y1));
extern int _g_flinedash P___((int n, const double *dashes, double offset));
extern int _g_flinerel P___((double dx0, double dy0, double dx1, double dy1));
extern int _g_flinewidth P___((double size));
extern int _g_flushpl P___((void));
extern int _g_fmarker P___((double x, double y, int type, double size));
extern int _g_fmarkerrel P___((double dx, double dy, int type, double size));
extern int _g_fmiterlimit P___((double limit));
extern int _g_fmove P___((double x, double y));
extern int _g_fmoverel P___((double x, double y));
extern int _g_fontname P___((const char *s));
extern int _g_fontsize P___((int size));
extern int _g_fpoint P___((double x, double y));
extern int _g_fpointrel P___((double dx, double dy));
extern int _g_frotate P___((double theta));
extern int _g_fscale P___((double x, double y));
extern int _g_fspace P___((double x0, double y0, double x1, double y1));
extern int _g_fspace2 P___((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _g_ftranslate P___((double x, double y));
extern int _g_havecap P___((const char *s));
extern int _g_joinmod P___((const char *s));
extern int _g_label P___((const char *s));
extern int _g_labelwidth P___((const char *s));
extern int _g_line P___((int x0, int y0, int x1, int y1));
extern int _g_linedash P___((int n, const int *dashes, int offset));
extern int _g_linemod P___((const char *s));
extern int _g_linerel P___((int dx0, int dy0, int dx1, int dy1));
extern int _g_linewidth P___((int size));
extern int _g_marker P___((int x, int y, int type, int size));
extern int _g_markerrel P___((int dx, int dy, int type, int size));
extern int _g_move P___((int x, int y));
extern int _g_moverel P___((int x, int y));
extern int _g_openpl P___((void));
extern int _g_pencolor P___((int red, int green, int blue));
extern int _g_pencolorname P___((const char *name));
extern int _g_point P___((int x, int y));
extern int _g_pointrel P___((int dx, int dy));
extern int _g_restorestate P___((void));
extern int _g_savestate P___((void));
extern int _g_space P___((int x0, int y0, int x1, int y1));
extern int _g_space2 P___((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _g_textangle P___((int angle));
/* Plotter protected methods, for libplot */
extern void _g_initialize P___((void));
extern void _g_terminate P___((void));
extern double _g_falabel_hershey P___((const unsigned char *s, int x_justify, int y_justify));
extern double _g_falabel_ps P___((const unsigned char *s, int h_just));
extern double _g_falabel_pcl P___((const unsigned char *s, int h_just));
extern double _g_falabel_stick P___((const unsigned char *s, int h_just));
extern double _g_falabel_other P___((const unsigned char *s, int h_just));
extern double _g_flabelwidth_hershey P___((const unsigned char *s));
extern double _g_flabelwidth_pcl P___((const unsigned char *s));
extern double _g_flabelwidth_ps P___((const unsigned char *s));
extern double _g_flabelwidth_stick P___((const unsigned char *s));
extern double _g_flabelwidth_other P___((const unsigned char *s));
extern void _g_error P___((const char *msg));
extern void _g_retrieve_font P___((void));
extern void _g_set_font P___((void));
extern void _g_set_attributes P___((void));
extern void _g_set_pen_color P___((void));
extern void _g_set_fill_color P___((void));
extern void _g_set_bg_color P___((void));
extern void _g_set_position P___((void));
extern void _g_warning P___((const char *msg));
extern void _g_write_byte P___((unsigned char c));
extern void _g_write_bytes P___((int n, const unsigned char *c));
extern void _g_write_string P___((const char *s));
/* Plotter internal functions overridden in MetaPlotter class, for libplot */
extern void _g_recompute_device_line_width P___((void));
/* other Plotter internal functions, for libplot */
extern Voidptr _get_plot_param P___((const char *parameter)); 
extern bool _match_pcl_font P___((void));
extern bool _match_ps_font P___((void));
extern double _label_width_hershey P___((const unsigned short *label));
extern double _render_non_hershey_string P___((const char *s, bool do_render, int x_justify, int y_justify));
extern double _render_simple_non_hershey_string P___((const unsigned char *s, bool do_render, int h_just));
extern unsigned short * _controlify P___((const unsigned char *));
extern void _copy_params_to_plotter P___((void));
extern void _draw_bezier2 P___((Point p0, Point p1, Point p2));
extern void _draw_bezier3 P___((Point p0, Point p1, Point p2, Point p3));
extern void _draw_circular_arc P___((Point p0, Point p1, Point pc));
extern void _draw_elliptic_arc P___((Point p0, Point p1, Point pc));
extern void _draw_hershey_glyph P___((int num, double charsize, int type, bool oblique));
extern void _draw_hershey_penup_stroke P___((double dx, double dy, double charsize, bool oblique));
extern void _draw_hershey_string P___((const unsigned short *string));
extern void _draw_hershey_stroke P___((bool pendown, double deltax, double deltay));
extern void _draw_stroke P___((bool pendown, double deltax, double deltay));
extern void _fakearc P___((Point p0, Point p1, int arc_type, const double m[4]));
extern void _fakebezier2 P___((Point p0, Point p1, Point p2));
extern void _fakebezier3 P___((Point p0, Point p1, Point p2, Point p3));
extern void _flush_plotter_outstreams P___((void));
extern void _free_params_in_plotter P___((void));
extern void _maybe_replace_arc P___((void));
extern void _set_bezier2_bbox  P___((Outbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2));
extern void _set_bezier3_bbox P___((Outbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3));
extern void _set_common_mi_attributes P___((Voidptr ptr));
extern void _set_ellipse_bbox P___((Outbuf *bufp, double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth));
extern void _set_line_end_bbox P___((Outbuf *bufp, double x, double y, double xother, double yother, double linewidth, int capstyle));
extern void _set_line_join_bbox P___((Outbuf *bufp, double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle, double miterlimit));
#else  /* LIBPLOTTER */
/* static Plotter public method (libplotter only) */
#define parampl Plotter::parampl
/* Plotter public methods, for libplotter */
#define _g_alabel Plotter::alabel
#define _g_arc Plotter::arc
#define _g_arcrel Plotter::arcrel
#define _g_bezier2 Plotter::bezier2
#define _g_bezier2rel Plotter::bezier2rel
#define _g_bezier3 Plotter::bezier3
#define _g_bezier3rel Plotter::bezier3rel
#define _g_bgcolor Plotter::bgcolor
#define _g_bgcolorname Plotter::bgcolorname
#define _g_box Plotter::box
#define _g_boxrel Plotter::boxrel
#define _g_capmod Plotter::capmod
#define _g_circle Plotter::circle
#define _g_circlerel Plotter::circlerel
#define _g_closepl Plotter::closepl
#define _g_color Plotter::color
#define _g_colorname Plotter::colorname
#define _g_cont Plotter::cont
#define _g_contrel Plotter::contrel
#define _g_ellarc Plotter::ellarc
#define _g_ellarcrel Plotter::ellarcrel
#define _g_ellipse Plotter::ellipse
#define _g_ellipserel Plotter::ellipserel
#define _g_endpath Plotter::endpath
#define _g_erase Plotter::erase
#define _g_farc Plotter::farc
#define _g_farcrel Plotter::farcrel
#define _g_fbezier2 Plotter::fbezier2
#define _g_fbezier2rel Plotter::fbezier2rel
#define _g_fbezier3 Plotter::fbezier3
#define _g_fbezier3rel Plotter::fbezier3rel
#define _g_fbox Plotter::fbox
#define _g_fboxrel Plotter::fboxrel
#define _g_fcircle Plotter::fcircle
#define _g_fcirclerel Plotter::fcirclerel
#define _g_fconcat Plotter::fconcat
#define _g_fcont Plotter::fcont
#define _g_fcontrel Plotter::fcontrel
#define _g_fellarc Plotter::fellarc
#define _g_fellarcrel Plotter::fellarcrel
#define _g_fellipse Plotter::fellipse
#define _g_fellipserel Plotter::fellipserel
#define _g_ffontname Plotter::ffontname
#define _g_ffontsize Plotter::ffontsize
#define _g_fillcolor Plotter::fillcolor
#define _g_fillcolorname Plotter::fillcolorname
#define _g_fillmod Plotter::fillmod
#define _g_filltype Plotter::filltype
#define _g_flabelwidth Plotter::flabelwidth
#define _g_fline Plotter::fline
#define _g_flinedash Plotter::flinedash
#define _g_flinerel Plotter::flinerel
#define _g_flinewidth Plotter::flinewidth
#define _g_flushpl Plotter::flushpl
#define _g_fmarker Plotter::fmarker
#define _g_fmarkerrel Plotter::fmarkerrel
#define _g_fmiterlimit Plotter::fmiterlimit
#define _g_fmove Plotter::fmove
#define _g_fmoverel Plotter::fmoverel
#define _g_fontname Plotter::fontname
#define _g_fontsize Plotter::fontsize
#define _g_fpoint Plotter::fpoint
#define _g_fpointrel Plotter::fpointrel
#define _g_frotate Plotter::frotate
#define _g_fscale Plotter::fscale
#define _g_fspace Plotter::fspace
#define _g_fspace2 Plotter::fspace2
#define _g_ftextangle Plotter::ftextangle
#define _g_ftranslate Plotter::ftranslate
#define _g_havecap Plotter::havecap
#define _g_joinmod Plotter::joinmod
#define _g_label Plotter::label
#define _g_labelwidth Plotter::labelwidth
#define _g_line Plotter::line
#define _g_linedash Plotter::linedash
#define _g_linemod Plotter::linemod
#define _g_linerel Plotter::linerel
#define _g_linewidth Plotter::linewidth
#define _g_marker Plotter::marker
#define _g_markerrel Plotter::markerrel
#define _g_move Plotter::move
#define _g_moverel Plotter::moverel
#define _g_openpl Plotter::openpl
#define _g_outfile Plotter::outfile /* OBSOLESCENT */
#define _g_pencolor Plotter::pencolor
#define _g_pencolorname Plotter::pencolorname
#define _g_point Plotter::point
#define _g_pointrel Plotter::pointrel
#define _g_restorestate Plotter::restorestate
#define _g_savestate Plotter::savestate
#define _g_space Plotter::space
#define _g_space2 Plotter::space2
#define _g_textangle Plotter::textangle
/* Plotter protected methods, for libplotter */
#define _g_error Plotter::error
#define _g_falabel_hershey Plotter::falabel_hershey
#define _g_falabel_other Plotter::falabel_other
#define _g_falabel_pcl Plotter::falabel_pcl
#define _g_falabel_ps Plotter::falabel_ps
#define _g_falabel_stick Plotter::falabel_stick
#define _g_flabelwidth_hershey Plotter::flabelwidth_hershey
#define _g_flabelwidth_other Plotter::flabelwidth_other
#define _g_flabelwidth_pcl Plotter::flabelwidth_pcl
#define _g_flabelwidth_ps Plotter::flabelwidth_ps
#define _g_flabelwidth_stick Plotter::flabelwidth_stick
#define _g_initialize Plotter::initialize
#define _g_retrieve_font Plotter::retrieve_font
#define _g_set_attributes Plotter::set_attributes
#define _g_set_bg_color Plotter::set_bg_color
#define _g_set_fill_color Plotter::set_fill_color
#define _g_set_font Plotter::set_font
#define _g_set_pen_color Plotter::set_pen_color
#define _g_set_position Plotter::set_position
#define _g_terminate Plotter::terminate
#define _g_warning Plotter::warning
#define _g_write_byte Plotter::write_byte
#define _g_write_bytes Plotter::write_bytes
#define _g_write_string Plotter::write_string
/* Plotter internal functions overridden in MetaPlotterclass, for libplotter */
#define _g_recompute_device_line_width Plotter::_recompute_device_line_width
/* other Plotter internal functions, for libplotter */
#define _controlify Plotter::_controlify
#define _copy_params_to_plotter Plotter::_copy_params_to_plotter
#define _draw_bezier2 Plotter::_draw_bezier2 
#define _draw_bezier3 Plotter::_draw_bezier3 
#define _draw_circular_arc Plotter::_draw_circular_arc 
#define _draw_elliptic_arc Plotter::_draw_elliptic_arc 
#define _draw_hershey_glyph Plotter::_draw_hershey_glyph
#define _draw_hershey_penup_stroke Plotter::_draw_hershey_penup_stroke
#define _draw_hershey_string Plotter::_draw_hershey_string
#define _draw_hershey_stroke Plotter::_draw_hershey_stroke
#define _draw_stroke Plotter::_draw_stroke
#define _fakearc Plotter::_fakearc
#define _fakebezier2 Plotter::_fakebezier2
#define _fakebezier3 Plotter::_fakebezier3
#define _flush_plotter_outstreams Plotter::_flush_plotter_outstreams
#define _free_params_in_plotter Plotter::_free_params_in_plotter
#define _get_plot_param Plotter::_get_plot_param
#define _label_width_hershey Plotter::_label_width_hershey
#define _match_pcl_font Plotter::_match_pcl_font
#define _match_ps_font Plotter::_match_ps_font
#define _maybe_replace_arc Plotter::_maybe_replace_arc
#define _render_non_hershey_string Plotter::_render_non_hershey_string
#define _render_simple_non_hershey_string Plotter::_render_simple_non_hershey_string
#define _set_bezier2_bbox Plotter::_set_bezier2_bbox
#define _set_bezier3_bbox Plotter::_set_bezier3_bbox
#define _set_common_mi_attributes Plotter::_set_common_mi_attributes
#define _set_ellipse_bbox Plotter::_set_ellipse_bbox
#define _set_line_end_bbox Plotter::_set_line_end_bbox
#define _set_line_join_bbox Plotter::_set_line_join_bbox
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* MetaPlotter public methods, for libplot */
extern int _m_alabel P___((int x_justify, int y_justify, const char *s));
extern int _m_arc P___((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _m_arcrel P___((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _m_bezier2 P___((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _m_bezier2rel P___((int dx0, int dy0, int dx1, int dy1, int dx2, int dy2));
extern int _m_bezier3 P___((int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3));
extern int _m_bezier3rel P___((int dx0, int dy0, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3));
extern int _m_bgcolor P___((int red, int green, int blue));
extern int _m_box P___((int x0, int y0, int x1, int y1));
extern int _m_boxrel P___((int dx0, int dy0, int dx1, int dy1));
extern int _m_capmod P___((const char *s));
extern int _m_circle P___((int x, int y, int r));
extern int _m_circlerel P___((int dx, int dy, int r));
extern int _m_closepl P___((void));
extern int _m_cont P___((int x, int y));
extern int _m_contrel P___((int x, int y));
extern int _m_ellarc P___((int xc, int yc, int x0, int y0, int x1, int y1));
extern int _m_ellarcrel P___((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _m_ellipse P___((int x, int y, int rx, int ry, int angle));
extern int _m_ellipserel P___((int dx, int dy, int rx, int ry, int angle));
extern int _m_endpath P___((void));
extern int _m_erase P___((void));
extern int _m_farc P___((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _m_farcrel P___((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _m_fbezier2 P___((double x0, double y0, double x1, double y1, double x2, double y2));
extern int _m_fbezier2rel P___((double dx0, double dy0, double dx1, double dy1, double dx2, double dy2));
extern int _m_fbezier3 P___((double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3));
extern int _m_fbezier3rel P___((double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3));
extern int _m_fbox P___((double x0, double y0, double x1, double y1));
extern int _m_fboxrel P___((double dx0, double dy0, double dx1, double dy1));
extern int _m_fcircle P___((double x, double y, double r));
extern int _m_fcirclerel P___((double dx, double dy, double r));
extern int _m_fconcat P___((double m0, double m1, double m2, double m3, double m4, double m5));
extern int _m_fcont P___((double x, double y));
extern int _m_fcontrel P___((double x, double y));
extern int _m_fellarc P___((double xc, double yc, double x0, double y0, double x1, double y1));
extern int _m_fellarcrel P___((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _m_fellipse P___((double x, double y, double rx, double ry, double angle));
extern int _m_fellipserel P___((double dx, double dy, double rx, double ry, double angle));
extern double _m_ffontname P___((const char *s));
extern double _m_ffontsize P___((double size));
extern int _m_fillcolor P___((int red, int green, int blue));
extern int _m_fillmod P___((const char *s));
extern int _m_filltype P___((int level));
extern int _m_fline P___((double x0, double y0, double x1, double y1));
extern int _m_flinedash P___((int n, const double *dashes, double offset));
extern int _m_flinerel P___((double dx0, double dy0, double dx1, double dy1));
extern int _m_flinewidth P___((double size));
extern int _m_fmarker P___((double x, double y, int type, double size));
extern int _m_fmarkerrel P___((double dx, double dy, int type, double size));
extern int _m_fmiterlimit P___((double limit));
extern int _m_fmove P___((double x, double y));
extern int _m_fmoverel P___((double x, double y));
extern int _m_fontname P___((const char *s));
extern int _m_fontsize P___((int size));
extern int _m_fpoint P___((double x, double y));
extern int _m_fpointrel P___((double dx, double dy));
extern int _m_fspace P___((double x0, double y0, double x1, double y1));
extern int _m_fspace2 P___((double x0, double y0, double x1, double y1, double x2, double y2));
extern double _m_ftextangle P___((double angle));
extern int _m_joinmod P___((const char *s));
extern int _m_label P___((const char *s));
extern int _m_line P___((int x0, int y0, int x1, int y1));
extern int _m_linedash P___((int n, const int *dashes, int offset));
extern int _m_linemod P___((const char *s));
extern int _m_linerel P___((int dx0, int dy0, int dx1, int dy1));
extern int _m_linewidth P___((int size));
extern int _m_marker P___((int x, int y, int type, int size));
extern int _m_markerrel P___((int dx, int dy, int type, int size));
extern int _m_move P___((int x, int y));
extern int _m_moverel P___((int x, int y));
extern int _m_openpl P___((void));
extern int _m_pencolor P___((int red, int green, int blue));
extern int _m_point P___((int x, int y));
extern int _m_pointrel P___((int dx, int dy));
extern int _m_restorestate P___((void));
extern int _m_savestate P___((void));
extern int _m_space P___((int x0, int y0, int x1, int y1));
extern int _m_space2 P___((int x0, int y0, int x1, int y1, int x2, int y2));
extern int _m_textangle P___((int angle));
/* MetaPlotter protected methods, for libplot */
extern void _m_initialize P___((void));
extern void _m_terminate P___((void));
/* MetaPlotter internal functions that override Plotter internal functions */
extern void _m_recompute_device_line_width P___((void));
/* other MetaPlotter internal functions, for libplot */
extern void _meta_emit_byte P___((int c));
extern void _meta_emit_float P___((double x)); 
extern void _meta_emit_integer P___((int x)); 
extern void _meta_emit_string P___((const char *s));
extern void _meta_emit_terminator P___((void));
extern bool _set_initial_font_size P___((double x0, double y0, double x1, double y1, double x2, double y2));
#else  /* LIBPLOTTER */
/* MetaPlotter public methods, for libplotter */
#define _m_alabel MetaPlotter::alabel
#define _m_arc MetaPlotter::arc
#define _m_arcrel MetaPlotter::arcrel
#define _m_bezier2 MetaPlotter::bezier2
#define _m_bezier2rel MetaPlotter::bezier2rel
#define _m_bezier3 MetaPlotter::bezier3
#define _m_bezier3rel MetaPlotter::bezier3rel
#define _m_bgcolor MetaPlotter::bgcolor
#define _m_box MetaPlotter::box
#define _m_boxrel MetaPlotter::boxrel
#define _m_capmod MetaPlotter::capmod
#define _m_circle MetaPlotter::circle
#define _m_circlerel MetaPlotter::circlerel
#define _m_closepl MetaPlotter::closepl
#define _m_cont MetaPlotter::cont
#define _m_contrel MetaPlotter::contrel
#define _m_ellarc MetaPlotter::ellarc
#define _m_ellarcrel MetaPlotter::ellarcrel
#define _m_ellipse MetaPlotter::ellipse
#define _m_ellipserel MetaPlotter::ellipserel
#define _m_endpath MetaPlotter::endpath
#define _m_erase MetaPlotter::erase
#define _m_farc MetaPlotter::farc
#define _m_farcrel MetaPlotter::farcrel
#define _m_fbezier2 MetaPlotter::fbezier2
#define _m_fbezier2rel MetaPlotter::fbezier2rel
#define _m_fbezier3 MetaPlotter::fbezier3
#define _m_fbezier3rel MetaPlotter::fbezier3rel
#define _m_fbox MetaPlotter::fbox
#define _m_fboxrel MetaPlotter::fboxrel
#define _m_fcircle MetaPlotter::fcircle
#define _m_fcirclerel MetaPlotter::fcirclerel
#define _m_fconcat MetaPlotter::fconcat
#define _m_fcont MetaPlotter::fcont
#define _m_fcontrel MetaPlotter::fcontrel
#define _m_fellarc MetaPlotter::fellarc
#define _m_fellarcrel MetaPlotter::fellarcrel
#define _m_fellipse MetaPlotter::fellipse
#define _m_fellipserel MetaPlotter::fellipserel
#define _m_ffontname MetaPlotter::ffontname
#define _m_ffontsize MetaPlotter::ffontsize
#define _m_fillcolor MetaPlotter::fillcolor
#define _m_fillmod MetaPlotter::fillmod
#define _m_filltype MetaPlotter::filltype
#define _m_fline MetaPlotter::fline
#define _m_flinedash MetaPlotter::flinedash
#define _m_flinerel MetaPlotter::flinerel
#define _m_flinewidth MetaPlotter::flinewidth
#define _m_fmarker MetaPlotter::fmarker
#define _m_fmarkerrel MetaPlotter::fmarkerrel
#define _m_fmiterlimit MetaPlotter::fmiterlimit
#define _m_fmove MetaPlotter::fmove
#define _m_fmoverel MetaPlotter::fmoverel
#define _m_fontname MetaPlotter::fontname
#define _m_fontsize MetaPlotter::fontsize
#define _m_fpoint MetaPlotter::fpoint
#define _m_fpointrel MetaPlotter::fpointrel
#define _m_fspace MetaPlotter::fspace
#define _m_fspace2 MetaPlotter::fspace2
#define _m_ftextangle MetaPlotter::ftextangle
#define _m_joinmod MetaPlotter::joinmod
#define _m_label MetaPlotter::label
#define _m_line MetaPlotter::line
#define _m_linedash MetaPlotter::linedash
#define _m_linemod MetaPlotter::linemod
#define _m_linerel MetaPlotter::linerel
#define _m_linewidth MetaPlotter::linewidth
#define _m_marker MetaPlotter::marker
#define _m_markerrel MetaPlotter::markerrel
#define _m_move MetaPlotter::move
#define _m_moverel MetaPlotter::moverel
#define _m_openpl MetaPlotter::openpl
#define _m_pencolor MetaPlotter::pencolor
#define _m_point MetaPlotter::point
#define _m_pointrel MetaPlotter::pointrel
#define _m_restorestate MetaPlotter::restorestate
#define _m_savestate MetaPlotter::savestate
#define _m_space MetaPlotter::space
#define _m_space2 MetaPlotter::space2
#define _m_textangle MetaPlotter::textangle
/* MetaPlotter protected methods, for libplotter */
#define _m_initialize MetaPlotter::initialize
#define _m_terminate MetaPlotter::terminate
/* MetaPlotter internal functions that override Plotter internal functions */
#define _m_recompute_device_line_width MetaPlotter::_recompute_device_line_width
/* MetaPlotter internal functions, for libplotter */
#define _meta_emit_byte MetaPlotter::_meta_emit_byte
#define _meta_emit_float MetaPlotter::_meta_emit_float
#define _meta_emit_integer MetaPlotter::_meta_emit_integer
#define _meta_emit_string MetaPlotter::_meta_emit_string
#define _meta_emit_terminator MetaPlotter::_meta_emit_terminator
#define _set_initial_font_size MetaPlotter::_set_initial_font_size
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* TekPlotter public methods, for libplot */
extern int _t_closepl P___((void));
extern int _t_erase P___((void));
extern int _t_fcont P___((double x, double y));
extern int _t_fpoint P___((double x, double y));
extern int _t_openpl P___((void));
/* TekPlotter protected methods, for libplot */
extern void _t_initialize P___((void));
extern void _t_terminate P___((void));
extern void _t_set_attributes P___((void));
extern void _t_set_bg_color P___((void));
extern void _t_set_pen_color P___((void));
/* TekPlotter internal functions, for libplot */
extern void _tek_mode P___((int newmode));
extern void _tek_move P___((int xx, int yy));
extern void _tek_vector P___((int xx, int yy));
extern void _tek_vector_compressed P___((int xx, int yy, int oldxx, int oldyy, bool force));
#else  /* LIBPLOTTER */
/* TekPlotter public methods, for libplotter */
#define _t_closepl TekPlotter::closepl
#define _t_erase TekPlotter::erase
#define _t_fcont TekPlotter::fcont
#define _t_fpoint TekPlotter::fpoint
#define _t_openpl TekPlotter::openpl
/* TekPlotter protected methods, for libplotter */
#define _t_initialize TekPlotter::initialize
#define _t_terminate TekPlotter::terminate
#define _t_set_attributes TekPlotter::set_attributes
#define _t_set_bg_color TekPlotter::set_bg_color
#define _t_set_pen_color TekPlotter::set_pen_color
/* TekPlotter internal functions, for libplotter */
#define _tek_mode TekPlotter::_tek_mode
#define _tek_move TekPlotter::_tek_move
#define _tek_vector TekPlotter::_tek_vector
#define _tek_vector_compressed TekPlotter::_tek_vector_compressed
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* HPGLPlotter/PCLPlotter public methods, for libplot */
extern int _h_closepl P___((void));
extern int _h_endpath P___((void));
extern int _h_fbox P___((double x0, double y0, double x1, double y1));
extern int _h_fcircle P___((double x, double y, double r));
extern int _h_flinewidth P___((double size));
extern int _h_fpoint P___((double x, double y));
extern int _h_openpl P___((void));
/* HPGLPlotter/PCLPlotter protected methods, for libplot */
extern double _h_falabel_ps P___((const unsigned char *s, int h_just));
extern double _h_falabel_pcl P___((const unsigned char *s, int h_just));
extern double _h_falabel_stick P___((const unsigned char *s, int h_just));
extern void _h_set_attributes P___((void));
extern void _h_set_fill_color P___((void));
extern void _h_set_font P___((void));
extern void _h_set_pen_color P___((void));
extern void _h_set_position P___((void));
/* HPGLPlotter protected methods, for libplot */
extern void _h_initialize P___((void));
extern void _h_terminate P___((void));
/* PCLPlotter protected methods, for libplot */
extern void _q_initialize P___((void));
extern void _q_terminate P___((void));
/* HPGLPlotter/PCLPlotter internal functions, for libplot */
extern bool _hpgl2_maybe_update_font P___((void));
extern bool _hpgl_maybe_update_font P___((void));
extern bool _parse_pen_string P___((const char *pen_s));
extern double _angle_of_arc P___((Point p0, Point p1, Point pc));
extern int _hpgl_pseudocolor P___((int red, int green, int blue, bool restrict_white));
extern void _compute_pseudo_fillcolor P___((int red, int green, int blue, int *pen, double *shading));
extern void _set_hp_fill_type P___((int fill_type, double option1));
extern void _set_hp_pen P___((int pen));
/* HPGLPlotter functions (overridden in PCLPlotter class), for libplotter */
extern void _h_maybe_switch_to_hpgl P___((void)); 
extern void _h_maybe_switch_from_hpgl P___((void)); 
/* PCLPlotter functions (overriding the above), for libplotter */
extern void _q_maybe_switch_to_hpgl P___((void)); 
extern void _q_maybe_switch_from_hpgl P___((void)); 
#else  /* LIBPLOTTER */
/* HPGLPlotter/PCLPlotter public methods, for libplotter */
#define _h_closepl HPGLPlotter::closepl
#define _h_endpath HPGLPlotter::endpath
#define _h_fbox HPGLPlotter::fbox
#define _h_fcircle HPGLPlotter::fcircle
#define _h_flinewidth HPGLPlotter::flinewidth
#define _h_fpoint HPGLPlotter::fpoint
#define _h_openpl HPGLPlotter::openpl
/* HPGLPlotter/PCLPlotter protected methods, for libplotter */
#define _h_falabel_ps HPGLPlotter::falabel_ps
#define _h_falabel_pcl HPGLPlotter::falabel_pcl
#define _h_falabel_stick HPGLPlotter::falabel_stick
#define _h_set_attributes HPGLPlotter::set_attributes
#define _h_set_fill_color HPGLPlotter::set_fill_color
#define _h_set_font HPGLPlotter::set_font
#define _h_set_pen_color HPGLPlotter::set_pen_color
#define _h_set_position HPGLPlotter::set_position
/* HPGLPlotter protected methods, for libplotter */
#define _h_initialize HPGLPlotter::initialize
#define _h_terminate HPGLPlotter::terminate
/* PCLPlotter protected methods, for libplotter */
#define _q_initialize PCLPlotter::initialize
#define _q_terminate PCLPlotter::terminate
/* HPGLPlotter/PCLPlotter internal functions, for libplotter */
#define _angle_of_arc HPGLPlotter::_angle_of_arc
#define _compute_pseudo_fillcolor HPGLPlotter::_compute_pseudo_fillcolor
#define _hpgl2_maybe_update_font HPGLPlotter::_hpgl2_maybe_update_font
#define _hpgl_maybe_update_font HPGLPlotter::_hpgl_maybe_update_font
#define _hpgl_pseudocolor HPGLPlotter::_hpgl_pseudocolor
#define _parse_pen_string HPGLPlotter::_parse_pen_string
#define _set_hp_fill_type HPGLPlotter::_set_hp_fill_type
#define _set_hp_pen HPGLPlotter::_set_hp_pen
/* HPGLPlotter functions (overridden in PCLPlotter class), for libplotter */
#define _h_maybe_switch_to_hpgl HPGLPlotter::_maybe_switch_to_hpgl
#define _h_maybe_switch_from_hpgl HPGLPlotter::_maybe_switch_from_hpgl
/* PCLPlotter functions (overriding the above), for libplotter */
#define _q_maybe_switch_to_hpgl PCLPlotter::_maybe_switch_to_hpgl
#define _q_maybe_switch_from_hpgl PCLPlotter::_maybe_switch_from_hpgl
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* FigPlotter public methods, for libplot */
extern int _f_closepl P___((void));
extern int _f_endpath P___((void));
extern int _f_erase P___((void));
extern int _f_fbox P___((double x0, double y0, double x1, double y1));
extern int _f_fcircle P___((double x, double y, double r));
extern int _f_fellipse P___((double x, double y, double rx, double ry, double angle));
extern int _f_flinewidth P___((double size));
extern int _f_fpoint P___((double x, double y));
extern int _f_openpl P___((void));
/* FigPlotter protected methods, for libplot */
extern void _f_initialize P___((void));
extern void _f_terminate P___((void));
extern double _f_falabel_ps P___((const unsigned char *s, int h_just));
extern void _f_retrieve_font P___((void));
extern void _f_set_fill_color P___((void));
extern void _f_set_pen_color P___((void));
/* FigPlotter internal functions, for libplot */
extern int _f_draw_ellipse_internal P___((double x, double y, double rx, double ry, double angle, int subtype));
extern int _fig_color P___((int red, int green, int blue));
extern void _f_compute_line_style P___((int *style, double *spacing));
extern void _f_emit_arc P___((double xc, double yc, double x0, double y0, double x1, double y1));
#else  /* LIBPLOTTER */
/* FigPlotter public methods, for libplotter */
#define _f_closepl FigPlotter::closepl
#define _f_endpath FigPlotter::endpath
#define _f_erase FigPlotter::erase
#define _f_fbox FigPlotter::fbox
#define _f_fcircle FigPlotter::fcircle
#define _f_fellipse FigPlotter::fellipse
#define _f_flinewidth FigPlotter::flinewidth
#define _f_fpoint FigPlotter::fpoint
#define _f_openpl FigPlotter::openpl
/* FigPlotter protected methods, for libplotter */
#define _f_falabel_ps FigPlotter::falabel_ps
#define _f_initialize FigPlotter::initialize
#define _f_retrieve_font FigPlotter::retrieve_font
#define _f_set_fill_color FigPlotter::set_fill_color
#define _f_set_pen_color FigPlotter::set_pen_color
#define _f_terminate FigPlotter::terminate
/* FigPlotter internal functions, for libplotter */
#define _f_compute_line_style FigPlotter::_f_compute_line_style
#define _f_draw_ellipse_internal FigPlotter::_f_draw_ellipse_internal
#define _f_emit_arc FigPlotter::_f_emit_arc
#define _fig_color FigPlotter::_fig_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* PSPlotter public methods, for libplot */
extern int _p_closepl P___((void));
extern int _p_endpath P___((void));
extern int _p_erase P___((void));
extern int _p_fcircle P___((double x, double y, double r));
extern int _p_fellipse P___((double x, double y, double rx, double ry, double angle));
extern int _p_fpoint P___((double x, double y));
extern int _p_openpl P___((void));
/* PSPlotter protected methods, for libplot */
extern void _p_initialize P___((void));
extern void _p_terminate P___((void));
extern double _p_falabel_ps P___((const unsigned char *s, int h_just));
extern double _p_falabel_pcl P___((const unsigned char *s, int h_just));
extern void _p_set_fill_color P___((void));
extern void _p_set_pen_color P___((void));
/* PSPlotter internal functions, for libplot */
extern double _p_emit_common_attributes P___((void));
extern void _p_compute_idraw_bgcolor P___((void));
extern void _p_fellipse_internal P___((double x, double y, double rx, double ry, double angle, bool circlep));
#else  /* LIBPLOTTER */
/* PSPlotter public methods, for libplotter */
#define _p_closepl PSPlotter::closepl
#define _p_endpath PSPlotter::endpath
#define _p_erase PSPlotter::erase
#define _p_fcircle PSPlotter::fcircle
#define _p_fellipse PSPlotter::fellipse
#define _p_fpoint PSPlotter::fpoint
#define _p_openpl PSPlotter::openpl
/* PSPlotter protected methods, for libplotter */
#define _p_initialize PSPlotter::initialize
#define _p_falabel_ps PSPlotter::falabel_ps
#define _p_falabel_pcl PSPlotter::falabel_pcl
#define _p_set_fill_color PSPlotter::set_fill_color
#define _p_set_pen_color PSPlotter::set_pen_color
#define _p_terminate PSPlotter::terminate
/* PSPlotter internal functions, for libplotter */
#define _p_compute_idraw_bgcolor PSPlotter::_p_compute_idraw_bgcolor
#define _p_emit_common_attributes PSPlotter::_p_emit_common_attributes
#define _p_fellipse_internal PSPlotter::_p_fellipse_internal
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* AIPlotter public methods, for libplot */
extern int _a_closepl P___((void));
extern int _a_endpath P___((void));
extern int _a_fpoint P___((double x, double y));
extern int _a_openpl P___((void));
/* AIPlotter protected methods, for libplot */
extern void _a_initialize P___((void));
extern void _a_terminate P___((void));
extern double _a_falabel_ps P___((const unsigned char *s, int h_just));
extern double _a_falabel_pcl P___((const unsigned char *s, int h_just));
extern void _a_set_attributes P___((void));
extern void _a_set_fill_color P___((void));
extern void _a_set_pen_color P___((void));
#else /* LIBPLOTTER */
/* AIPlotter public methods, for libplotter */
#define _a_closepl AIPlotter::closepl
#define _a_endpath AIPlotter::endpath
#define _a_fpoint AIPlotter::fpoint
#define _a_openpl AIPlotter::openpl
/* AIPlotter protected methods, for libplotter */
#define _a_falabel_ps AIPlotter::falabel_ps
#define _a_falabel_pcl AIPlotter::falabel_pcl
#define _a_initialize AIPlotter::initialize
#define _a_set_attributes AIPlotter::set_attributes
#define _a_set_fill_color AIPlotter::set_fill_color
#define _a_set_pen_color AIPlotter::set_pen_color
#define _a_terminate AIPlotter::terminate
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* PNMPlotter public methods, for libplot */
extern int _n_closepl P___((void));
extern int _n_endpath P___((void));
extern int _n_erase P___((void));
extern int _n_fellipse P___((double x, double y, double rx, double ry, double angle));
extern int _n_fpoint P___((double x, double y));
extern int _n_openpl P___((void));
/* PNMPlotter protected methods, for libplot */
extern void _n_initialize P___((void));
extern void _n_terminate P___((void));
/* PNMPlotter internal functions, for libplot */
extern void _n_delete_image P___((void));
extern void _n_draw_elliptic_mi_arc_internal P___((int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange));
extern void _n_new_image P___((void));
extern void _n_write_pnm P___((void));
extern void _n_write_pbm P___((void));
extern void _n_write_pgm P___((void));
extern void _n_write_ppm P___((void));
#else  /* LIBPLOTTER */
/* PNMPlotter public methods, for libplotter */
#define _n_closepl PNMPlotter::closepl
#define _n_erase PNMPlotter::erase
#define _n_endpath PNMPlotter::endpath
#define _n_fellipse PNMPlotter::fellipse
#define _n_fpoint PNMPlotter::fpoint
#define _n_openpl PNMPlotter::openpl
/* PNMPlotter protected methods, for libplotter */
#define _n_initialize PNMPlotter::initialize
#define _n_terminate PNMPlotter::terminate
/* PNMPlotter internal functions, for libplotter */
#define _n_delete_image PNMPlotter::_n_delete_image
#define _n_draw_elliptic_mi_arc_internal PNMPlotter::_n_draw_elliptic_mi_arc_internal
#define _n_new_image PNMPlotter::_n_new_image 
#define _n_write_pnm PNMPlotter::_n_write_pnm
#define _n_write_pbm PNMPlotter::_n_write_pbm
#define _n_write_pgm PNMPlotter::_n_write_pgm
#define _n_write_ppm PNMPlotter::_n_write_ppm
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* GIFPlotter public methods, for libplot */
extern int _i_closepl P___((void));
extern int _i_endpath P___((void));
extern int _i_erase P___((void));
extern int _i_fellipse P___((double x, double y, double rx, double ry, double angle));
extern int _i_fpoint P___((double x, double y));
extern int _i_openpl P___((void));
/* GIFPlotter protected methods, for libplot */
extern void _i_initialize P___((void));
extern void _i_terminate P___((void));
extern void _i_set_bg_color P___((void));
extern void _i_set_fill_color P___((void));
extern void _i_set_pen_color P___((void));
/* GIFPlotter internal functions, for libplot */
extern int _i_scan_pixel P___((void));
extern unsigned char _i_new_color_index P___((int red, int green, int blue));
extern void _i_delete_image P___((void));
extern void _i_draw_elliptic_mi_arc_internal P___((int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange));
extern void _i_new_image P___((void));
extern void _i_start_scan P___((void));
extern void _i_write_gif_header P___((void));
extern void _i_write_gif_image P___((void));
extern void _i_write_gif_trailer P___((void));
extern void _i_write_short_int P___((unsigned int i));
#else  /* LIBPLOTTER */
/* GIFPlotter public methods, for libplotter */
#define _i_closepl GIFPlotter::closepl
#define _i_erase GIFPlotter::erase
#define _i_endpath GIFPlotter::endpath
#define _i_fellipse GIFPlotter::fellipse
#define _i_fpoint GIFPlotter::fpoint
#define _i_openpl GIFPlotter::openpl
/* GIFPlotter protected methods, for libplotter */
#define _i_initialize GIFPlotter::initialize
#define _i_terminate GIFPlotter::terminate
#define _i_set_bg_color GIFPlotter::set_bg_color
#define _i_set_fill_color GIFPlotter::set_fill_color
#define _i_set_pen_color GIFPlotter::set_pen_color
/* GIFPlotter internal functions, for libplotter */
#define _i_scan_pixel GIFPlotter::_i_scan_pixel
#define _i_new_color_index GIFPlotter::_i_new_color_index
#define _i_delete_image GIFPlotter::_i_delete_image
#define _i_draw_elliptic_mi_arc_internal GIFPlotter::_i_draw_elliptic_mi_arc_internal
#define _i_new_image GIFPlotter::_i_new_image 
#define _i_start_scan GIFPlotter::_i_start_scan
#define _i_write_gif_header GIFPlotter::_i_write_gif_header
#define _i_write_gif_image GIFPlotter::_i_write_gif_image
#define _i_write_gif_trailer GIFPlotter::_i_write_gif_trailer
#define _i_write_short_int GIFPlotter::_i_write_short_int
#endif /* LIBPLOTTER */

#ifndef X_DISPLAY_MISSING
#ifndef LIBPLOTTER
/* XDrawablePlotter/XPlotter public methods, for libplot */
extern int _x_endpath P___((void));
extern int _x_erase P___((void));
extern int _x_fbox P___((double x0, double y0, double x1, double y1));
extern int _x_fcont P___((double x, double y));
extern int _x_fellipse P___((double x, double y, double rx, double ry, double angle));
extern int _x_flushpl P___((void));
extern int _x_fpoint P___((double x, double y));
extern int _x_restorestate P___((void));
extern int _x_savestate P___((void));
/* XDrawablePlotter public methods, for libplot */
extern int _x_closepl P___((void));
extern int _x_erase P___((void));
extern int _x_openpl P___((void));
/* XPlotter public methods, for libplot */
extern int _y_closepl P___((void));
extern int _y_erase P___((void));
extern int _y_openpl P___((void));
/* XDrawablePlotter/XPlotter protected methods, for libplot */
extern void _x_initialize P___((void));
extern void _x_terminate P___((void));
extern double _x_falabel_ps P___((const unsigned char *s, int h_just));
extern double _x_falabel_pcl P___((const unsigned char *s, int h_just));
extern double _x_falabel_other P___((const unsigned char *s, int h_just));
extern double _x_flabelwidth_ps P___((const unsigned char *s));
extern double _x_flabelwidth_pcl P___((const unsigned char *s));
extern double _x_flabelwidth_other P___((const unsigned char *s));
extern void _x_retrieve_font P___((void));
extern void _x_set_attributes P___((void));
extern void _x_set_bg_color P___((void));
extern void _x_set_fill_color P___((void));
extern void _x_set_pen_color P___((void));
/* XDrawablePlotter/XPlotter internal functions, for libplot */
extern bool _retrieve_X_color P___((XColor *rgb_ptr));
extern bool _retrieve_X_font_internal P___((const char *name, double size, double rotation));
extern bool _select_X_font P___((const char *name, bool is_zero[4], const unsigned char *s));
extern bool _select_X_font_carefully P___((const char *name, bool is_zero[4], const unsigned char *s));
extern void _draw_elliptic_X_arc P___((Point p0, Point p1, Point pc));
extern void _draw_elliptic_X_arc_2 P___((Point p0, Point p1, Point pc));
extern void _draw_elliptic_X_arc_internal P___((int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange));
extern void _set_X_font_dimensions P___((bool is_zero[4]));
/* XDrawablePlotter internal functions, for libplot */
extern void _x_maybe_get_new_colormap P___((void));
extern void _x_maybe_handle_x_events P___((void));
/* XPlotter protected methods, for libplot */
extern void _y_initialize P___((void));
extern void _y_terminate P___((void));
/* XPlotter internal functions, for libplot */
extern void _y_flush_plotter_outstreams P___((void));
extern void _y_maybe_get_new_colormap P___((void));
extern void _y_maybe_handle_x_events P___((void));
extern void _y_set_data_for_quitting P___((void));
#else  /* LIBPLOTTER */
/* XDrawablePlotter/XPlotter public methods, for libplotter */
#define _x_endpath XDrawablePlotter::endpath
#define _x_fbox XDrawablePlotter::fbox
#define _x_fcont XDrawablePlotter::fcont
#define _x_fellipse XDrawablePlotter::fellipse
#define _x_flushpl XDrawablePlotter::flushpl
#define _x_fpoint XDrawablePlotter::fpoint
#define _x_restorestate XDrawablePlotter::restorestate
#define _x_savestate XDrawablePlotter::savestate
/* more XDrawablePlotter public methods (overridden in XPlotter class) */
#define _x_closepl XDrawablePlotter::closepl
#define _x_erase XDrawablePlotter::erase
#define _x_openpl XDrawablePlotter::openpl
/* XPlotter public methods (which override the preceding) */
#define _y_closepl XPlotter::closepl
#define _y_erase XPlotter::erase
#define _y_openpl XPlotter::openpl
/* XDrawablePlotter/XPlotter protected methods, for libplotter */
#define _x_falabel_ps XDrawablePlotter::falabel_ps
#define _x_falabel_pcl XDrawablePlotter::falabel_pcl
#define _x_falabel_other XDrawablePlotter::falabel_other
#define _x_flabelwidth_ps XDrawablePlotter::flabelwidth_ps
#define _x_flabelwidth_pcl XDrawablePlotter::flabelwidth_pcl
#define _x_flabelwidth_other XDrawablePlotter::flabelwidth_other
#define _x_retrieve_font XDrawablePlotter::retrieve_font
#define _x_set_attributes XDrawablePlotter::set_attributes
#define _x_set_bg_color XDrawablePlotter::set_bg_color
#define _x_set_fill_color XDrawablePlotter::set_fill_color
#define _x_set_pen_color XDrawablePlotter::set_pen_color
/* XDrawablePlotter protected methods (overridden in XPlotter class) */
#define _x_initialize XDrawablePlotter::initialize
#define _x_terminate XDrawablePlotter::terminate
/* XPlotter protected methods (which override the preceding) */
#define _y_initialize XPlotter::initialize
#define _y_terminate XPlotter::terminate
/* XDrawablePlotter/XPlotter internal functions, for libplotter */
#define _draw_elliptic_X_arc XDrawablePlotter::_draw_elliptic_X_arc
#define _draw_elliptic_X_arc_2 XDrawablePlotter::_draw_elliptic_X_arc_2
#define _draw_elliptic_X_arc_internal XDrawablePlotter::_draw_elliptic_X_arc_internal
#define _retrieve_X_color XDrawablePlotter::_retrieve_X_color
#define _retrieve_X_font_internal XDrawablePlotter::_retrieve_X_font_internal
#define _select_X_font XDrawablePlotter::_select_X_font
#define _select_X_font_carefully XDrawablePlotter::_select_X_font_carefully
#define _set_X_font_dimensions XDrawablePlotter::_set_X_font_dimensions
/* XDrawablePlotter internal functions (overridden in XPlotter class) */
#define _x_maybe_get_new_colormap XDrawablePlotter::_maybe_get_new_colormap
#define _x_maybe_handle_x_events XDrawablePlotter::_maybe_handle_x_events
/* XPlotter internal functions (which override the preceding) */
#define _y_maybe_get_new_colormap XPlotter::_maybe_get_new_colormap
#define _y_maybe_handle_x_events XPlotter::_maybe_handle_x_events
/* other XPlotter internal functions, for libplotter */
#define _y_flush_plotter_outstreams XPlotter::_y_flush_plotter_outstreams
#define _y_set_data_for_quitting XPlotter::_y_set_data_for_quitting
#endif  /* LIBPLOTTER */
#endif /* not X_DISPLAY_MISSING */

#undef P___
