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

/* 1. OUR OWN INCLUDE FILE */

/* Determine which of libplot/libplotter this is. */
#ifndef LIBPLOTTER
#define NOT_LIBPLOTTER
#endif

/* Always include plotter.h.  (If NOT_LIBPLOTTER is defined, it's a C-style
   header file, declaring the Plotter struct, rather than a declaration
   file for the Plotter class.) */
#include "plotter.h"

/* 2. INCLUDE FILES FOR THE X WINDOW SYSTEM */

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

#define DEFAULT_PCL_FONT "Univers"
#define DEFAULT_PCL_TYPEFACE_INDEX 0
#define DEFAULT_PCL_FONT_INDEX 1

#define DEFAULT_STICK_FONT "Stick"
#define DEFAULT_STICK_TYPEFACE_INDEX 3
#define DEFAULT_STICK_FONT_INDEX 1

/* HERSHEY FONTS */

/* our information about each of the 22 Hershey vector fonts in g_fontdb.c,
   and the typefaces they belong to */
struct plHersheyFontInfoStruct
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

extern const struct plHersheyFontInfoStruct _hershey_font_info[];

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
struct plHersheyAccentedCharInfoStruct
{
  unsigned char composite, character, accent;
};

extern const struct plHersheyAccentedCharInfoStruct _hershey_accented_char_info[];

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
struct plPSFontInfoStruct
{
  const char *ps_name;		/* the postscript font name */
  const char *ps_name_alt;	/* alternative PS font name, if non-NULL */
  const char *ps_name_alt2;	/* 2nd alternative PS font name, if non-NULL */
  const char *x_name;		/* the X Windows font name */
  const char *x_name_alt;	/* alternative X Windows font name */
  const char *css_family;	/* CSS font family */
  const char *css_generic_family; /* CSS generic font family */
  const char *css_style;	/* CSS font style */
  const char *css_weight;	/* CSS font weight */
  const char *css_stretch;	/* CSS font stretch */
  int pcl_typeface;		/* the PCL typeface number */
  int hpgl_spacing;		/* 0=fixed width, 1=variable */
  int hpgl_posture;		/* 0=upright, 1=italic, etc. */
  int hpgl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int hpgl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int font_cap_height;		/* the font's cap height */
  int font_x_height;		/* the font's x height */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  int fig_id;			/* Fig's font id */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern const struct plPSFontInfoStruct _ps_font_info[];

/* PCL FONTS */

/* our information about each of the 45 PCL fonts in g_fontdb.c, and the
   typefaces they belong to.  (The `substitute_ps_name' field is present
   only to support the Tidbits-is-Wingdings botch.) */
struct plPCLFontInfoStruct
{
  const char *ps_name;		/* the postscript font name */
  const char *ps_name_alt;	/* alternative PS font name, if non-NULL */
  const char *substitute_ps_name; /* replacement name (for use in a PS file) */
  const char *x_name;		/* the X Windows font name */
  const char *css_family;	/* CSS font family */
  const char *css_generic_family; /* CSS generic font family */
  const char *css_style;	/* CSS font style */
  const char *css_weight;	/* CSS font weight */
  const char *css_stretch;	/* CSS font stretch */
  int pcl_typeface;		/* the PCL typeface number */
  int hpgl_spacing;		/* 0=fixed width, 1=variable */
  int hpgl_posture;		/* 0=upright, 1=italic, etc. */
  int hpgl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int hpgl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int font_cap_height;		/* the font's cap height */
  int font_x_height;		/* the font's x height */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

extern const struct plPCLFontInfoStruct _pcl_font_info[];

/* STICK FONTS */

/* our information about each of the Stick fonts (i.e., vector fonts
   resident in HP's devices) listed in g_fontdb.c, and the typefaces they
   belong to */
struct plStickFontInfoStruct
{
  const char *ps_name;		/* the postscript font name */
  bool basic;			/* basic stick font (supp. on all devices)? */
  int pcl_typeface;		/* the PCL typeface number */
  int hpgl_spacing;		/* 0=fixed width, 1=variable */
  int hpgl_posture;		/* 0=upright, 1=italic, etc. */
  int hpgl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int hpgl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1 */
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

extern const struct plStickFontInfoStruct _stick_font_info[];

/* Device-resident kerning data (`spacing table' in HP documentation),
   indexed by `right edge character class' and `left edge character class',
   i.e., `row class' and `column class'.  There are three such spacing
   tables, shared among old-style HP character sets of size 128, and hence
   among our Stick fonts.  See the article by L. W. Hennessee et al. in the
   Nov. 1981 issue of the Hewlett-Packard Journal. */
struct plStickCharSpacingTableStruct
{
  int rows, cols;
  const short *kerns;
};

extern const struct plStickCharSpacingTableStruct _stick_spacing_tables[];

/* Kerning tables for 128-character halves of our Stick fonts.  A kerning
   table for the lower or upper half of one of our 256-character fonts
   specifies a spacing table (see above), and maps each character in the
   half-font to the appropriate row and column class. */
struct plStickFontSpacingTableStruct
{
  int spacing_table;
  char row[128], col[128];	/* we use char's as very short int's */
};

extern const struct plStickFontSpacingTableStruct _stick_kerning_tables[];

/* TYPEFACES */

/* typeface information, applicable to all four sorts of font in our font
   database (Hershey, PS, PCL, Stick) */

#define FONTS_PER_TYPEFACE 10	/* maximum */

struct plTypefaceInfoStruct
{
  int numfonts;
  int fonts[FONTS_PER_TYPEFACE];
};

extern const struct plTypefaceInfoStruct _hershey_typeface_info[];
extern const struct plTypefaceInfoStruct _ps_typeface_info[];
extern const struct plTypefaceInfoStruct _pcl_typeface_info[];
extern const struct plTypefaceInfoStruct _stick_typeface_info[];


/***********************************************************************/
/* GENERAL DEFINITIONS, TYPEDEFS, & EXTERNAL VARIABLES                 */
/***********************************************************************/

/* miscellaneous data types */

typedef plPoint plVector;
typedef plIntPoint plIntVector;

/* Initializations for default values of Plotter data members, performed
   when space() is first called.  Latter doesn't apply to Plotters whose
   device models have type DISP_DEVICE_COORS_INTEGER_LIBXMI; the default
   for such Plotters is to use zero-width (i.e. Bresenham) lines.  See
   g_space.c. */
#define DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE (1.0/50.0)
#define DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE (1.0/850.0)

/* horizontal justification types for labels (our numbering, but it's the
   same as xfig's; don't alter without checking the code) */
#define JUST_LEFT 0
#define JUST_CENTER 1
#define JUST_RIGHT 2

/* vertical justification types for labels (our numbering) */
#define JUST_TOP 0
#define JUST_HALF 1
#define JUST_BASE 2
#define JUST_BOTTOM 3
#define JUST_CAP 4

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
} plLineStyle;

extern const plLineStyle _line_styles[NUM_LINE_STYLES];

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

/* A Plotter type is first classified according to its `display device
   model', i.e., according to whether the display device to which the user
   frame is mapped is physical or virtual.

   A `physical' display device is one for which the viewport is located on
   a page of known type and size (e.g. "letter", "a4").  I.e. the Plotter
   with a physical display device is one for which the PAGESIZE parameter
   is meaningful.  A Plotter with a `virtual' display device is one for
   which it normally is not: the viewport size that the Plotter uses may be
   fixed (as is the case for a CGM Plotter), or set in a Plotter-dependent
   way (e.g. via the BITMAPSIZE parameter).  */

enum { DISP_MODEL_PHYSICAL, DISP_MODEL_VIRTUAL };

/* Any Plotter is also classified according to the coordinate type it uses
   when writing output (i.e. when writing to its display device, if it has
   one).  A Plotter may use real coordinates (e.g., a generic, Metafile, PS
   or AI Plotter).  A Plotter may also use integer coordinates.  There are
   two subtypes of the latter: one in which a bitmap is produced using
   libxmi or compatible scan-conversion routines (e.g., Bitmap, PNM, PNG,
   GIF, X, X Drawable Plotters), and one in which graphics with integer
   coordinates are drawn by other means (e.g., Fig, HPGL, PCL, ReGIS, and
   Tektronix Plotters).  The only significant distinction is that in vector
   graphics drawn with libxmi, zero-width lines are visible: by convention,
   zero-width lines are interpreted as Bresenham lines. */

enum { DISP_DEVICE_COORS_REAL, DISP_DEVICE_COORS_INTEGER_LIBXMI, DISP_DEVICE_COORS_INTEGER_NON_LIBXMI };

/* The user->device coordinate transformation */

/* X, Y Device: transform user coordinates to device coordinates */
#define XD(x,y) XD_INTERNAL((x),(y),_plotter->drawstate->transform.m)
#define YD(x,y) YD_INTERNAL((x),(y),_plotter->drawstate->transform.m)

#ifdef __GNUC__
#define XD_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[4] + _x * _m[0] + _y * _m[2]; _retval; })
#define YD_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[5] + _x * _m[1] + _y * _m[3]; _retval; })
#else
#define XD_INTERNAL(x,y,m) ((m)[4] + (x) * (m)[0] + (y) * (m)[2])
#define YD_INTERNAL(x,y,m) ((m)[5] + (x) * (m)[1] + (y) * (m)[3])
#endif

/* X,Y Device Vector: transform user vector to device vector */
#define XDV(x,y) XDV_INTERNAL((x),(y),_plotter->drawstate->transform.m)
#define YDV(x,y) YDV_INTERNAL((x),(y),_plotter->drawstate->transform.m)

#ifdef __GNUC__
#define XDV_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[0] * _x + _m[2] * _y; _retval; })
#define YDV_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[1] * _x + _m[3] * _y; _retval; })
#else
#define XDV_INTERNAL(x,y,m) ((m)[0] * (x) + (m)[2] * (y))
#define YDV_INTERNAL(x,y,m) ((m)[1] * (x) + (m)[3] * (y))
#endif

/* X, Y User Vector: transform device vector back to user vector 
   (used by X11 driver only) */
#ifdef __GNUC__
#define XUV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; double _retval = (m[3] * _x - m[2] * _y) / (m[0] * m[3] - m[1] * m[2]); _retval; })
#define YUV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; double _retval = (- m[1] * _x + m[0] * _y) / (m[0] * m[3] - m[1] * m[2]); _retval; })
#else
#define XUV(x,y) ((_plotter->drawstate->transform.m[3] * (x) - _plotter->drawstate->transform.m[2] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))
#define YUV(x,y) ((- _plotter->drawstate->transform.m[1] * (x) + _plotter->drawstate->transform.m[0] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))
#endif


/*************************************************************************/
/* MISC. DEFS on POLYLINES and PATHS(relevant to all or most display devices)*/
/*************************************************************************/

/* Default value for the miter limit (see comments in g_miter.c).  This
   is the value used by X11: it chops off all mitered line joins if the
   join angle is less than 11 degrees. */
#define DEFAULT_MITER_LIMIT 10.4334305246

/* Default length an unfilled path (stored in the path buffer's segment
   list) is allowed to grow to, before it is flushed out by an automatic
   invocation of endpath().  (We don't flush filled paths, since they need
   to be preserved as discrete objects if filling is to be performed
   properly). */
#define MAX_UNFILLED_PATH_LENGTH 500
#define MAX_UNFILLED_PATH_LENGTH_STRING "500"


/************************************************************************/
/* DEFINITIONS & EXTERNALS SPECIFIC TO INDIVIDUAL DEVICE DRIVERS */
/************************************************************************/

/************************************************************************/
/* Metafile device driver */
/************************************************************************/

/* string with which to begin each metafile, must begin with '#' to permit
   parsing by our plot filters */
#define PLOT_MAGIC "#PLOT"

/* bit fields for specifying, via a mask, which libplot attributes should
   be updated (see m_attribs.c) */
#define PL_ATTR_POSITION (1<<0)
#define PL_ATTR_TRANSFORMATION_MATRIX (1<<1)
#define PL_ATTR_PEN_COLOR (1<<2)
#define PL_ATTR_FILL_COLOR (1<<3)
#define PL_ATTR_BG_COLOR (1<<4)
#define PL_ATTR_PEN_TYPE (1<<5)
#define PL_ATTR_FILL_TYPE (1<<6)
#define PL_ATTR_LINE_STYLE (1<<7) /* line mode and/or dash array */
#define PL_ATTR_LINE_WIDTH (1<<8)
#define PL_ATTR_FILL_RULE (1<<9)
#define PL_ATTR_JOIN_STYLE (1<<10)
#define PL_ATTR_CAP_STYLE (1<<11)
#define PL_ATTR_MITER_LIMIT (1<<12)
#define PL_ATTR_ORIENTATION (1<<13)
#define PL_ATTR_FONT_NAME (1<<14)
#define PL_ATTR_FONT_SIZE (1<<15)
#define PL_ATTR_TEXT_ANGLE (1<<16)

/************************************************************************/
/* ReGIS (remote graphics instruction set) device driver */
/************************************************************************/

/* For a ReGIS device we clip to the rectangular physical display
   [0..767]x[0..479], not to the square libplot graphics display
   [144..623]x[0..479], which is specified in r_defplot.c.  Note: ReGIS
   uses a flipped-y convention. */

#define REGIS_DEVICE_X_MIN 0
#define REGIS_DEVICE_X_MAX 767
#define REGIS_DEVICE_Y_MIN 0
#define REGIS_DEVICE_Y_MAX 479

#define REGIS_CLIP_FUZZ 0.0000001
#define REGIS_DEVICE_X_MIN_CLIP (REGIS_DEVICE_X_MIN - 0.5 + REGIS_CLIP_FUZZ)
#define REGIS_DEVICE_X_MAX_CLIP (REGIS_DEVICE_X_MAX + 0.5 - REGIS_CLIP_FUZZ)
#define REGIS_DEVICE_Y_MIN_CLIP (REGIS_DEVICE_Y_MIN - 0.5 + REGIS_CLIP_FUZZ)
#define REGIS_DEVICE_Y_MAX_CLIP (REGIS_DEVICE_Y_MAX + 0.5 - REGIS_CLIP_FUZZ)


/************************************************************************/
/* Tektronix device driver */
/************************************************************************/

/* For a Tektronix device we clip to the rectangular physical display
   [0..4095]x[0..3119], not to the square libplot graphics display
   [488..3607]x[0..3119], which is specified in t_defplot.c.  Note:
   Tektronix displays do not use a flipped-y convention. */

#define TEK_DEVICE_X_MIN 0
#define TEK_DEVICE_X_MAX 4095
#define TEK_DEVICE_Y_MIN 0
#define TEK_DEVICE_Y_MAX 3119

#define TEK_CLIP_FUZZ 0.0000001
#define TEK_DEVICE_X_MIN_CLIP (TEK_DEVICE_X_MIN - 0.5 + TEK_CLIP_FUZZ)
#define TEK_DEVICE_X_MAX_CLIP (TEK_DEVICE_X_MAX + 0.5 - TEK_CLIP_FUZZ)
#define TEK_DEVICE_Y_MIN_CLIP (TEK_DEVICE_Y_MIN - 0.5 + TEK_CLIP_FUZZ)
#define TEK_DEVICE_Y_MAX_CLIP (TEK_DEVICE_Y_MAX + 0.5 - TEK_CLIP_FUZZ)

/* Tektronix modes (our private numbering, values are not important but
   order is, see t_tek_md.c) */
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
extern const plColor _kermit_stdcolors[KERMIT_NUM_STD_COLORS];
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

/* HP-GL/2 pen types, i.e. screening types: the type of area fill to be
   applied to wide pen strokes.  (HP-GL/2 numbering, as used in the `SV'
   [screened vectors] instruction.  Screened vectors are supported only on
   HP-GL/2 devices that are not pen plotters.) */
#define HPGL_PEN_SOLID 0
#define HPGL_PEN_SHADED 1
#define HPGL_PEN_PREDEFINED_CROSSHATCH 21 /* imported from PCL or RTL */

/* HP-GL and HP-GL/2 fill types.  (Their numbering, as used in the `FT'
   instruction.) */
#define HPGL_FILL_SOLID_BI 1
#define HPGL_FILL_SOLID_UNI 2
#define HPGL_FILL_PARALLEL_LINES 3
#define HPGL_FILL_CROSSHATCHED_LINES 4
#define HPGL_FILL_SHADED 10
#define HPGL_FILL_PREDEFINED_CROSSHATCH 21 /* imported from PCL or RTL */

/* HP-GL/2 character rendering types, as used in the `CF' [character fill
   mode] instruction.  By default the current pen is used for edging, and
   for filling too, if filling is requested.  Some fill types [set with the
   `FT' command'] include color information, in which case the current pen
   is not used for filling.  Types 0,1,2 allow specification of an edge pen
   which may be different from the present pen.  (At least for type 0,
   specifying edge pen 0 seems to turn off edging.  For types 1 and 3, edge
   pen 0 may request white edging [on color devices].)  Note that there are
   three kinds of font: bitmap, stick, and outline, which are treated
   slightly differently: bitmap and stick chars are filled, not edged, so
   edging doesn't apply to them. */

/* Default rendering is type 0, with edge pen 0, which as just mentioned
   turns off edging. */

#define HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE 0
#define HPGL_CHAR_EDGE 1	/* bitmap, stick chars are filled instead */
#define HPGL_CHAR_FILL 2	/* i.e. with current fill type */
#define HPGL_CHAR_FILL_AND_EDGE 3 /* i.e. with current fill type */

/* HP-GL object types (our numbering), which we use when passing an
   argument to an HPGLPlotter's internal _h_set_pen_color() method, letting
   it know the type of object that will be drawn.  Passing the libplot pen
   color down to the HP-GL/2 level, prior to drawing a label rather than a
   path, may involve changing the character rendition type. */
#define HPGL_OBJECT_PATH 0
#define HPGL_OBJECT_LABEL 1

/* Nominal pen width in native HP-GL units (so this is 0.3mm).  Used by our
   HP7550B-style cross-hatching algorithm, which we employ when emulating
   shading (if HPGL_VERSION is 1 or 1.5, i.e. if there's no true shading).  */
#define HPGL_NOMINAL_PEN_WIDTH 12 

/* default values for HPGL_PENS environment variable, for HP-GL[/2]; this
   lists available pens and their positions in carousel */
#define DEFAULT_HPGL_PEN_STRING "1=black"
#define DEFAULT_HPGL2_PEN_STRING "1=black:2=red:3=green:4=yellow:5=blue:6=magenta:7=cyan"

/* PCL 5 font information: symbol set, i.e. encoding */
#define PCL_ISO_8859_1 14
#define PCL_ROMAN_8 277

/* PCL typeface number for default HP-GL/2 typeface */
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
   L_SOLID).  Units are Fig display units.  See f_path.c. */
extern const double _fig_dash_length[NUM_LINE_STYLES];

#define FIG_JOIN_MITER 0
#define FIG_JOIN_ROUND 1
#define FIG_JOIN_BEVEL 2

#define FIG_CAP_BUTT 0
#define FIG_CAP_ROUND 1
#define FIG_CAP_PROJECT 2

/* Fig join and cap styles, see f_path.c, indexed by our internal join and
   cap type numbers (miter/rd./bevel and butt/rd./project) */
extern const int _fig_join_style[], _fig_cap_style[];

/* these constants for Fig colors are hardcoded in xfig */

#define FIG_STD_COLOR_MIN 0	/* see f_color2.c for colors 0,...,31 */
#define C_BLACK 0		/* i.e. #0 in table in f_color2.c */
#define C_WHITE 7		/* i.e. #7 in table */
#define FIG_NUM_STD_COLORS 32
#define FIG_USER_COLOR_MIN 32
extern const plColor _fig_stdcolors[FIG_NUM_STD_COLORS];

/* xfig's depth attribute ranges from 0 to FIG_MAXDEPTH. */
#define FIG_MAXDEPTH 999

/* depth of the first object we'll draw (we make it a bit less than
   FIG_MAXDEPTH, since the user may wish to edit the drawing with xfig to
   include deeper, i.e. obscured objects) */
#define FIG_INITIAL_DEPTH 989


/************************************************************************/
/* CGM device driver */
/************************************************************************/

/* CGM output file `profiles', which are increasingly general (our
   numbering).  The most restrictive is the WebCGM profile.  We increment
   the profile number appropriately whenever anything noncompliant is
   seen. */

#define CGM_PROFILE_WEB 0
#define CGM_PROFILE_MODEL 1
#define CGM_PROFILE_NONE 2

/* Possible encodings of the CGM output file (our numbering).  Only the
   first (binary) is allowed by the WebCGM profile. */

#define CGM_ENCODING_BINARY 0	/* default */
#define CGM_ENCODING_CHARACTER 1 /* not supported by libplot */
#define CGM_ENCODING_CLEAR_TEXT 2

/* In the binary encoding, how many bytes we use to represent an integer
   parameter of a CGM command.  This determines the range over which
   integers (e.g., point coordinates) can vary, and hence the granularity
   of our quantization to integer coordinates in the output file.  This
   value should not be greater than the number of bytes used in the system
   representation for an integer (see comments in c_emit.c).

   Don't change this value unless you know what you're doing.  Some old
   [buggy] CGM interpreters can't handle any value except 2, or possibly 4.
   The old RALCGM viewer/translator partially breaks on a value of 3 or 4.
   (It can display binary-encoded CGM files that use the value of `3', but
   when it translates such files to the clear text encoding, it produces a
   bogus [zero] value for the metric scaling factor.) */

#define CGM_BINARY_BYTES_PER_INTEGER 2

/* In the binary encoding, how many bytes we use to represent an RGB color
   component.  In this is 1, then 24-bit color will be used; if 2, then
   48-bit color will be used.  Valid values are 1, 2, 3, 4, but our
   code in c_color.c assumes that the value is 1 or 2. */

#define CGM_BINARY_BYTES_PER_COLOR_COMPONENT 2

/* In the binary encoding, how many bytes we use to represent a string
   parameter of a CGM command.  (See c_emit.c.)  In the binary encoding, a
   string <= 254 bytes in length is represented literally, preceded by a
   1-byte count.  Any string > 254 bytes in length is partitioned: after
   the initial byte, there are one or more partitions.  Each partition
   contains an initial byte, and then up to CGM_STRING_PARTITION_SIZE bytes
   of the string.  According to the CGM spec, CGM_STRING_PARTITION_SIZE
   could be as large as 32767.  However, since we don't wish to overrun our
   output buffer, we keep it fairly small (see comment in g_outbuf.c). */

#define CGM_STRING_PARTITION_SIZE 2000
#define CGM_BINARY_BYTES_PER_STRING(len) \
((len) <= 254 ? (1 + (len)) : \
(1 + (len) + 2 * (1 + ((len) - 1) / CGM_STRING_PARTITION_SIZE)))

/* CGM's element classes (CGM numbering) */
#define CGM_DELIMITER_ELEMENT 0
#define CGM_METAFILE_DESCRIPTOR_ELEMENT 1
#define CGM_PICTURE_DESCRIPTOR_ELEMENT 2
#define CGM_CONTROL_ELEMENT 3
#define CGM_GRAPHICAL_PRIMITIVE_ELEMENT 4
#define CGM_ATTRIBUTE_ELEMENT 5
#define CGM_ESCAPE_ELEMENT 6	/* not used by libplot */
#define CGM_EXTERNAL_ELEMENT 7	/* not used by libplot */
#define CGM_SEGMENT_ELEMENT 8	/* not used by libplot */

/* tags for CGM data types within a CGM SDR (structured data record) */
#define CGM_SDR_DATATYPE_SDR 1
#define CGM_SDR_DATATYPE_COLOR_INDEX 2
#define CGM_SDR_DATATYPE_COLOR_DIRECT 3
#define CGM_SDR_DATATYPE_ENUM 5
#define CGM_SDR_DATATYPE_INTEGER 6
#define CGM_SDR_DATATYPE_INDEX 11
#define CGM_SDR_DATATYPE_REAL 12
#define CGM_SDR_DATATYPE_STRING 13
#define CGM_SDR_DATATYPE_STRING_FIXED 14
#define CGM_SDR_DATATYPE_VDC 16
#define CGM_SDR_DATATYPE_UNSIGNED_INTEGER_8BIT 18
#define CGM_SDR_DATATYPE_COLOR_LIST 21

/* CGM font properties, from the CGM spec.  (Value of each of these font
   props is an SDR, comprising a single datum of `index' type, except for
   the FAMILY prop, for which the datum is a string, and the DESIGN_GROUP
   prop, for which the SDR comprises three 8-bit unsigned integers.) */
#define CGM_FONT_PROP_INDEX 1
#define CGM_FONT_PROP_FAMILY 4
#define CGM_FONT_PROP_POSTURE 5
#define CGM_FONT_PROP_WEIGHT 6
#define CGM_FONT_PROP_WIDTH 7
#define CGM_FONT_PROP_DESIGN_GROUP 13
#define CGM_FONT_PROP_STRUCTURE 14

/* CGM line/edge types (CGM numbering; for custom dash arrays defined by
   linedash(), negative values are used) */
#define CGM_L_SOLID 1
#define CGM_L_DASHED 2
#define CGM_L_DOTTED 3
#define CGM_L_DOTDASHED 4
#define CGM_L_DOTDOTDASHED 5

/* CGM interior styles (CGM numbering) */
#define CGM_INT_STYLE_HOLLOW 0
#define CGM_INT_STYLE_SOLID 1
#define CGM_INT_STYLE_PATTERN 2
#define CGM_INT_STYLE_HATCH 3
#define CGM_INT_STYLE_EMPTY 4
#define CGM_INT_STYLE_GEOMETRIC_PATTERN 5
#define CGM_INT_STYLE_INTERPOLATED 6

/* CGM line/edge join styles (CGM numbering) */
#define CGM_JOIN_UNSPEC 1
#define CGM_JOIN_MITER 2
#define CGM_JOIN_ROUND 3
#define CGM_JOIN_BEVEL 4

/* CGM line/edge cap styles (CGM numbering) */
#define CGM_CAP_UNSPEC 1
#define CGM_CAP_BUTT 2
#define CGM_CAP_ROUND 3
#define CGM_CAP_PROJECTING 4
#define CGM_CAP_TRIANGULAR 5

/* CGM line/edge dash cap styles (CGM numbering) */
#define CGM_DASH_CAP_UNSPEC 1
#define CGM_DASH_CAP_BUTT 2
#define CGM_DASH_CAP_MATCH 3

/* CGM marker types (CGM numbering) */
#define CGM_M_DOT 1
#define CGM_M_PLUS 2
#define CGM_M_ASTERISK 3
#define CGM_M_CIRCLE 4
#define CGM_M_CROSS 5

/* CGM object types (our numbering) */
#define CGM_OBJECT_OPEN 0
#define CGM_OBJECT_CLOSED 1
#define CGM_OBJECT_MARKER 2
#define CGM_OBJECT_TEXT 3
#define CGM_OBJECT_OTHER 4

/* CGM horizontal justification types for labels (CGM numbering) */
#define CGM_ALIGN_NORMAL_HORIZONTAL 0
#define CGM_ALIGN_LEFT 1
#define CGM_ALIGN_CENTER 2
#define CGM_ALIGN_RIGHT 3

/* CGM vertical justification types for labels (CGM numbering) */
#define CGM_ALIGN_NORMAL_VERTICAL 0
#define CGM_ALIGN_TOP 1
#define CGM_ALIGN_CAP 2
#define CGM_ALIGN_HALF 3
#define CGM_ALIGN_BASE 4
#define CGM_ALIGN_BOTTOM 5

/* CGM `restricted text' types (CGM numbering) */
#define CGM_RESTRICTED_TEXT_TYPE_BASIC 1
#define CGM_RESTRICTED_TEXT_TYPE_BOXED_CAP 2

/* mappings from internal PS font number to CGM font id, as used in output
   file; see g_fontdb.c */
extern const int _ps_font_to_cgm_font_id[];
extern const int _cgm_font_id_to_ps_font[];

/* structure used to store the CGM properties for a font; see g_fontdb.c */
typedef struct
{
  const char *family;
  const char *extra_style;
  const char *style;
  int posture;			/* 1=upright, 2=oblique, 4=italic */
  int weight;			/* 4=semilight, 5=normal, 6=semibold, 7=bold */
  int proportionate_width;	/* 3=condensed, 5=medium */
  int design_group[3];
  int structure;		/* 1=filled, 2=outline */
} plCGMFontProperties;

extern const plCGMFontProperties _cgm_font_properties[];

/* structure used to store a user-defined line type; see g_attribs.c */
typedef struct plCGMCustomLineTypeStruct
{
  int *dashes;
  int dash_array_len;
  struct plCGMCustomLineTypeStruct *next;
} plCGMCustomLineType;

/* maximum number of line types a user can define, and the maximum dash
   array length a user can specify per line type, without violating the
   WebCGM or Model CGM profiles */
#define CGM_MAX_CUSTOM_LINE_TYPES 16
#define CGM_MAX_DASH_ARRAY_LENGTH 8


/************************************************************************/
/* Postscript/idraw device driver */
/************************************************************************/

/* minimum desired resolution in device frame (i.e. in printer's points) */
#define PS_MIN_RESOLUTION 0.05

#define POINT_PS_SIZE 0.5  /* em size (in printer's points) for a font in
			      which a `point' could appear as a symbol */

/* 16-bit line style brush arrays for idraw, see p_path.c/p_ellipse.c,
   indexed by our internal line style numbering (L_SOLID, L_DOTTED, etc.)  */
extern const long _idraw_brush_pattern[NUM_LINE_STYLES];

/* PS line join and line cap styles [also used by AI device driver] */

#define PS_LINE_JOIN_MITER 0
#define PS_LINE_JOIN_ROUND 1
#define PS_LINE_JOIN_BEVEL 2

#define PS_LINE_CAP_BUTT 0
#define PS_LINE_CAP_ROUND 1
#define PS_LINE_CAP_PROJECT 2

/* PS join and cap styles, see p_path.c, indexed by our internal join and
   cap type numbering (miter/rd./bevel/triangular and
   butt/rd./project/triangular) */
extern const int _ps_join_style[], _ps_cap_style[];

/* information on colors known to idraw, see p_color2.c */

#define IDRAW_NUM_STD_COLORS 12
extern const plColor _idraw_stdcolors[IDRAW_NUM_STD_COLORS];
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

/* numbering of our X GC's (graphics contexts); this is the numbering we
   use when passing an argument to _x_set_attributes() indicating which GC
   should be altered */
#define X_GC_FOR_DRAWING 0
#define X_GC_FOR_FILLING 1
#define X_GC_FOR_ERASING 2

#endif /* not X_DISPLAY_MISSING */


/***********************************************************************/
/* DRAWING STATE                                                       */
/***********************************************************************/

/* Default drawing state, defined in g_defstate.c.  This is used for
   initialization of the first state on the drawing state stack that every
   Plotter maintains; see g_savestate.c. */
extern const plDrawState _default_drawstate;

/*************************************************************************/
/* PLOTTER OBJECTS (structs for libplot, class instances for libplotter) */
/*************************************************************************/

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
   creation time (in apinewc.c, which is libplot-specific). */
extern const Plotter _g_default_plotter, _b_default_plotter, _m_default_plotter, _r_default_plotter, _t_default_plotter, _h_default_plotter, _q_default_plotter, _f_default_plotter, _c_default_plotter, _p_default_plotter, _a_default_plotter, _s_default_plotter, _i_default_plotter, _n_default_plotter, _z_default_plotter, _x_default_plotter, _y_default_plotter;

/* Similarly, in libplot this is the initialization of the function-pointer
   part of any PlotterParams object. */
extern const PlotterParams _default_plotter_params;
#endif /* not LIBPLOTTER */

/* The array used for storing the names of recognized Plotter parameters,
   and their default values.  (See g_params2.c.) */
struct plParamRecord
{
  const char *parameter;	/* parameter name */
  voidptr_t default_value;	/* default value (applies if string-valued) */
  bool is_string;		/* whether or not value must be a string */
};

extern const struct plParamRecord _known_params[NUM_PLOTTER_PARAMETERS];

/* A pointer to a distinguished (global) PlotterParams object, used by the
   old C and C++ bindings.  The function parampl() sets parameters in this
   object.  (This is one reason why the old bindings are non-thread-safe.
   The new bindings allow the programmer to instantiate and use more than a
   single PlotterParams object, so they are thread-safe.)  In libplotter,
   this pointer is declared as a static member of the Plotter class. */
#ifndef LIBPLOTTER
extern PlotterParams *_old_api_global_plotter_params;
#else
#define _old_api_global_plotter_params Plotter::_old_api_global_plotter_params
#endif


/**************************************************************************/
/* PROTOTYPES ETC. for libplot and libplotter */
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

/* Miscellaneous internal functions that aren't Plotter class methods, so
   they're declared the same for both libplot and libplotter. */

/* wrappers for malloc and friends */
extern voidptr_t _plot_xcalloc P___((size_t nmemb, size_t size));
extern voidptr_t _plot_xmalloc P___((size_t size));
extern voidptr_t _plot_xrealloc P___((voidptr_t p, size_t size));

/* misc. utility functions, mostly geometry-related */

extern plPoint _truecenter P___((plPoint p0, plPoint p1, plPoint pc));
extern plVector *_vscale P___((plVector *v, double newlen));
extern double _angle_of_arc P___((plPoint p0, plPoint p1, plPoint pc));
extern double _matrix_norm P___((const double m[6]));
extern double _xatan2 P___((double y, double x));
extern int _clip_line P___((double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip));
extern int _codestring_len P___((const unsigned short *codestring));
extern int _grayscale_approx P___((int red, int green, int blue));
extern void _matrix_product P___((const double m[6], const double n[6], double product[6]));
extern void _matrix_inverse P___((const double m[6], double inverse[6]));
extern void _matrix_sing_vals P___((const double m[6], double *min_sing_val, double *max_sing_val));
extern void _set_common_mi_attributes P___((plDrawState *drawstate, voidptr_t ptr));
extern voidptr_t _get_default_plot_param P___((const char *parameter)); 

/* plPlotterData methods */
/* lowest-level output routines used by Plotters */
extern void _write_byte P___((const plPlotterData *data, unsigned char c));
extern void _write_bytes P___((const plPlotterData *data, int n, const unsigned char *c));
extern void _write_string P___((const plPlotterData *data, const char *s));
/* other plPlotterData methods */
extern bool _compute_ndc_to_device_map P___((plPlotterData *data));
extern void _set_page_type P___((plPlotterData *data));
extern voidptr_t _get_plot_param P___((const plPlotterData *data, const char *parameter)); 

/* plPath methods (see g_subpaths.c) */
extern plPath * _flatten_path P___((const plPath *path));
extern plPath * _new_plPath P___((void));
extern plPath ** _merge_paths P___((const plPath **paths, int num_paths));
extern void _add_arc P___((plPath *path, plPoint pc, plPoint p1));
extern void _add_arc_as_bezier3 P___((plPath *path, plPoint pc, plPoint p1));
extern void _add_arc_as_lines P___((plPath *path, plPoint pc, plPoint p1));
extern void _add_bezier2 P___((plPath *path, plPoint pc, plPoint p));
extern void _add_bezier2_as_lines P___((plPath *path, plPoint pc, plPoint p));
extern void _add_bezier3 P___((plPath *path, plPoint pc, plPoint pd, plPoint p));
extern void _add_bezier3_as_lines P___((plPath *path, plPoint pc, plPoint pd, plPoint p));
extern void _add_box P___((plPath *path, plPoint p0, plPoint p1, bool clockwise));
extern void _add_box_as_lines P___((plPath *path, plPoint p0, plPoint p1, bool clockwise));
extern void _add_circle P___((plPath *path, plPoint pc, double radius, bool clockwise));
extern void _add_circle_as_bezier3s P___((plPath *path, plPoint pc, double radius, bool clockwise));
extern void _add_circle_as_ellarcs P___((plPath *path, plPoint pc, double radius, bool clockwise));
extern void _add_circle_as_lines P___((plPath *path, plPoint pc, double radius, bool clockwise));
extern void _add_ellarc P___((plPath *path, plPoint pc, plPoint p1));
extern void _add_ellarc_as_bezier3 P___((plPath *path, plPoint pc, plPoint p1));
extern void _add_ellarc_as_lines P___((plPath *path, plPoint pc, plPoint p1));
extern void _add_ellipse P___((plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise));
extern void _add_ellipse_as_bezier3s P___((plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise));
extern void _add_ellipse_as_ellarcs P___((plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise));
extern void _add_ellipse_as_lines P___((plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise));
extern void _add_line P___((plPath *path, plPoint p));
extern void _add_moveto P___((plPath *path, plPoint p));
extern void _delete_plPath P___((plPath *path));
extern void _reset_plPath P___((plPath *path));

/* plOutbuf methods (see g_outbuf.c) */
extern plOutbuf * _new_outbuf P___((void));
extern void _bbox_of_outbuf P___((plOutbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax));
extern void _bbox_of_outbufs P___((plOutbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax));
extern void _delete_outbuf P___((plOutbuf *outbuf));
extern void _freeze_outbuf P___((plOutbuf *outbuf));
extern void _reset_outbuf P___((plOutbuf *outbuf));
extern void _update_bbox P___((plOutbuf *bufp, double x, double y));
extern void _update_buffer P___((plOutbuf *outbuf));
extern void _update_buffer_by_added_bytes P___((plOutbuf *outbuf, int additional));

/* functions that update a device-frame bounding box for a page, as stored
   in a plOutbuf */
extern void _set_bezier2_bbox P___((plOutbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2, double device_line_width, double m[6]));
extern void _set_bezier3_bbox P___((plOutbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3, double device_line_width, double m[6]));
extern void _set_ellipse_bbox P___((plOutbuf *bufp, double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth, double m[6]));
extern void _set_line_end_bbox P___((plOutbuf *bufp, double x, double y, double xother, double yother, double linewidth, int capstyle, double m[6]));
extern void _set_line_join_bbox P___((plOutbuf *bufp, double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle, double miterlimit, double m[6]));

/* CGMPlotter-related functions, which write a CGM command, or an argument
   of same, alternatively to a plOutbuf or to a string (see c_emit.c) */
extern void _cgm_emit_command_header P___((plOutbuf *outbuf, int cgm_encoding, int element_class, int id, int data_len, int *byte_count, const char *op_code));
extern void _cgm_emit_color_component P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_enum P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count, const char *text_string)); 
extern void _cgm_emit_index P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_integer P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_point P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int y, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_points P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, const int *x, const int *y, int npoints, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_real_fixed_point P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, double x, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_real_floating_point P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, double x, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_string P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, const char *s, int string_length, bool use_double_quotes, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_unsigned_integer P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_unsigned_integer_8bit P___((plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count));
extern void _cgm_emit_command_terminator P___((plOutbuf *outbuf, int cgm_encoding, int *byte_count));

/* SVGPlotter-related functions */
extern const char * _libplot_color_to_svg_color P___((plColor color_48, char charbuf[8]));

/* plColorNameCache methods */
extern bool _string_to_color P___((const char *name, plColor *color_p, plColorNameCache *color_name_cache));
extern plColorNameCache * _create_color_name_cache P___((void));
extern void _delete_color_name_cache P___((plColorNameCache *color_cache));

/* Renaming of the global symbols in the libxmi scan conversion library,
   which we include in libplot/libplotter as a rendering module.  We
   prepend each name with one or two underscores.  Doing this prevents
   pollution of the user-level namespace, and allows an application to link
   with both libplot/libplotter and a separate version of libxmi. */

/* libxmi API functions get one underscore, since in libxmi itself, they
   get no underscores.  */

#define miClearPaintedSet _miClearPaintedSet
#define miCopyCanvas _miCopyCanvas
#define miCopyGC _miCopyGC
#define miCopyPaintedSetToCanvas _miCopyPaintedSetToCanvas
#define miDeleteCanvas _miDeleteCanvas
#define miDeleteEllipseCache _miDeleteEllipseCache
#define miDeleteGC _miDeleteGC
#define miDeletePaintedSet _miDeletePaintedSet
#define miDrawArcs_r _miDrawArcs_r
#define miDrawLines _miDrawLines
#define miDrawPoints _miDrawPoints
#define miDrawRectangles _miDrawRectangles
#define miFillArcs _miFillArcs
#define miFillPolygon _miFillPolygon
#define miFillRectangles _miFillRectangles
#define miNewCanvas _miNewCanvas
#define miNewEllipseCache _miNewEllipseCache
#define miNewGC _miNewGC
#define miNewPaintedSet _miNewPaintedSet
#define miSetCanvasStipple _miSetCanvasStipple
#define miSetCanvasTexture _miSetCanvasTexture
#define miSetGCAttrib _miSetGCAttrib
#define miSetGCAttribs _miSetGCAttribs
#define miSetGCDashes _miSetGCDashes
#define miSetGCMiterLimit _miSetGCMiterLimit
#define miSetGCPixels _miSetGCPixels
#define miSetPixelMerge2 _miSetPixelMerge2
#define miSetPixelMerge3 _miSetPixelMerge3

/* An external libxmi symbol; add an underscore to it, similarly. */
#define mi_libxmi_ver _mi_libxmi_ver

/* Internal libxmi functions get two underscores, since in libxmi itself,
   they get one underscore. */
#define mi_xmalloc __mi_xmalloc
#define mi_xcalloc __mi_xcalloc
#define mi_xrealloc __mi_xrealloc
#define miAddSpansToPaintedSet _miAddSpansToPaintedSet
#define miDrawArcs_r_internal __miDrawArcs_r_internal
#define miDrawArcs_internal __miDrawArcs_internal
#define miDrawLines_internal __miDrawLines_internal
#define miDrawRectangles_internal __miDrawRectangles_internal
#define miPolyArc_r __miPolyArc_r
#define miPolyArc __miPolyArc
#define miFillArcs_internal __miFillArcs_internal
#define miFillRectangles_internal __miFillRectangles_internal
#define miFillSppPoly __miFillSppPoly
#define miFillPolygon_internal __miFillPolygon_internal
#define miFillConvexPoly __miFillConvexPoly
#define miFillGeneralPoly __miFillGeneralPoly
#define miDrawPoints_internal __miDrawPoints_internal
#define miCreateETandAET __miCreateETandAET
#define miloadAET __miloadAET
#define micomputeWAET __micomputeWAET
#define miInsertionSort __miInsertionSort
#define miFreeStorage __miFreeStorage
#define miQuickSortSpansY __miQuickSortSpansY
#define miUniquifyPaintedSet __miUniquifyPaintedSet
#define miWideDash __miWideDash
#define miStepDash __miStepDash
#define miWideLine __miWideLine
#define miZeroPolyArc_r __miZeroPolyArc_r
#define miZeroPolyArc __miZeroPolyArc
#define miZeroLine __miZeroLine
#define miZeroDash __miZeroDash

/* Don't include unneeded non-reentrant libxmi functions, such as the
   function miPolyArc().  We use the reentrant version miPolyArc_r()
   instead, to avoid static data. */
#define NO_NONREENTRANT_POLYARC_SUPPORT

/* Internal functions that aren't Plotter class methods, but which need to
   be renamed in libplotter, so that both libplot and libplotter can be
   loaded simultaneously. */
#ifdef LIBPLOTTER
#define libplot_warning_handler libplotter_warning_handler
#define libplot_error_handler libplotter_error_handler
#endif

/* Declarations of forwarding functions used in libplot (not libplotter). */

/* These support the derivation of classes such as the PNMPlotter and the
   PNGPlotter classes from the BitmapPlotter class, the derivation of the
   PCLPlotter class from the HPGLPlotter class, and the derivation of the
   XPlotter class from the XDrawablePlotter class.  */

#ifndef LIBPLOTTER
extern int _maybe_output_image P___((Plotter *_plotter));
extern void _maybe_switch_to_hpgl P___((Plotter *_plotter));
extern void _maybe_switch_from_hpgl P___((Plotter *_plotter));
#ifndef X_DISPLAY_MISSING
extern void _maybe_get_new_colormap P___((Plotter *_plotter));
extern void _maybe_handle_x_events P___((Plotter *_plotter));
#endif /* not X_DISPLAY_MISSING */
#endif /* not LIBPLOTTER */

/* Declarations of the Plotter methods and the device-specific versions of
   same.  The initial letter indicates the Plotter class specificity:
   g=generic (i.e. base Plotter class), b=bitmap, m=metafile, t=Tektronix,
   r=ReGIS, h=HP-GL/2 and PCL 5, f=xfig, c=CGM, p=PS, a=Adobe Illustrator,
   s=SVG, i=GIF, n=PNM (i.e. PBM/PGM/PPM), z=PNG, x=X11 Drawable, y=X11.

   In libplot, these are declarations of global functions.  But in
   libplotter, we use #define and the double colon notation to make them
   function members of the appropriate Plotter classes. */

#ifndef LIBPLOTTER
/* Plotter public methods, for libplot */
#define _API_alabel pl_alabel_r
#define _API_arc pl_arc_r
#define _API_arcrel pl_arcrel_r
#define _API_bezier2 pl_bezier2_r
#define _API_bezier2rel pl_bezier2rel_r
#define _API_bezier3 pl_bezier3_r
#define _API_bezier3rel pl_bezier3rel_r
#define _API_bgcolor pl_bgcolor_r
#define _API_bgcolorname pl_bgcolorname_r
#define _API_box pl_box_r
#define _API_boxrel pl_boxrel_r
#define _API_capmod pl_capmod_r
#define _API_circle pl_circle_r
#define _API_circlerel pl_circlerel_r
#define _API_closepath pl_closepath_r
#define _API_closepl pl_closepl_r
#define _API_color pl_color_r
#define _API_colorname pl_colorname_r
#define _API_cont pl_cont_r
#define _API_contrel pl_contrel_r
#define _API_ellarc pl_ellarc_r
#define _API_ellarcrel pl_ellarcrel_r
#define _API_ellipse pl_ellipse_r
#define _API_ellipserel pl_ellipserel_r
#define _API_endpath pl_endpath_r
#define _API_endsubpath pl_endsubpath_r
#define _API_erase pl_erase_r
#define _API_farc pl_farc_r
#define _API_farcrel pl_farcrel_r
#define _API_fbezier2 pl_fbezier2_r
#define _API_fbezier2rel pl_fbezier2rel_r
#define _API_fbezier3 pl_fbezier3_r
#define _API_fbezier3rel pl_fbezier3rel_r
#define _API_fbox pl_fbox_r
#define _API_fboxrel pl_fboxrel_r
#define _API_fcircle pl_fcircle_r
#define _API_fcirclerel pl_fcirclerel_r
#define _API_fconcat pl_fconcat_r
#define _API_fcont pl_fcont_r
#define _API_fcontrel pl_fcontrel_r
#define _API_fellarc pl_fellarc_r
#define _API_fellarcrel pl_fellarcrel_r
#define _API_fellipse pl_fellipse_r
#define _API_fellipserel pl_fellipserel_r
#define _API_ffontname pl_ffontname_r
#define _API_ffontsize pl_ffontsize_r
#define _API_fillcolor pl_fillcolor_r
#define _API_fillcolorname pl_fillcolorname_r
#define _API_fillmod pl_fillmod_r
#define _API_filltype pl_filltype_r
#define _API_flabelwidth pl_flabelwidth_r
#define _API_fline pl_fline_r
#define _API_flinedash pl_flinedash_r
#define _API_flinerel pl_flinerel_r
#define _API_flinewidth pl_flinewidth_r
#define _API_flushpl pl_flushpl_r
#define _API_fmarker pl_fmarker_r
#define _API_fmarkerrel pl_fmarkerrel_r
#define _API_fmiterlimit pl_fmiterlimit_r
#define _API_fmove pl_fmove_r
#define _API_fmoverel pl_fmoverel_r
#define _API_fontname pl_fontname_r
#define _API_fontsize pl_fontsize_r
#define _API_fpoint pl_fpoint_r
#define _API_fpointrel pl_fpointrel_r
#define _API_frotate pl_frotate_r
#define _API_fscale pl_fscale_r
#define _API_fsetmatrix pl_fsetmatrix_r
#define _API_fspace pl_fspace_r
#define _API_fspace2 pl_fspace2_r
#define _API_ftextangle pl_ftextangle_r
#define _API_ftranslate pl_ftranslate_r
#define _API_havecap pl_havecap_r
#define _API_joinmod pl_joinmod_r
#define _API_label pl_label_r
#define _API_labelwidth pl_labelwidth_r
#define _API_line pl_line_r
#define _API_linedash pl_linedash_r
#define _API_linemod pl_linemod_r
#define _API_linerel pl_linerel_r
#define _API_linewidth pl_linewidth_r
#define _API_marker pl_marker_r
#define _API_markerrel pl_markerrel_r
#define _API_move pl_move_r
#define _API_moverel pl_moverel_r
#define _API_openpl pl_openpl_r
#define _API_orientation pl_orientation_r
#define _API_outfile pl_outfile_r /* OBSOLESCENT */
#define _API_pencolor pl_pencolor_r
#define _API_pencolorname pl_pencolorname_r
#define _API_pentype pl_pentype_r
#define _API_point pl_point_r
#define _API_pointrel pl_pointrel_r
#define _API_restorestate pl_restorestate_r
#define _API_savestate pl_savestate_r
#define _API_space pl_space_r
#define _API_space2 pl_space2_r
#define _API_textangle pl_textangle_r
extern FILE* _API_outfile P___((Plotter *_plotter, FILE* newstream));/* OBSOLESCENT */
extern double _API_ffontname P___((Plotter *_plotter, const char *s));
extern double _API_ffontsize P___((Plotter *_plotter, double size));
extern double _API_flabelwidth P___((Plotter *_plotter, const char *s));
extern double _API_ftextangle P___((Plotter *_plotter, double angle));
extern int _API_alabel P___((Plotter *_plotter, int x_justify, int y_justify, const char *s));
extern int _API_arc P___((Plotter *_plotter, int xc, int yc, int x0, int y0, int x1, int y1));
extern int _API_arcrel P___((Plotter *_plotter, int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _API_bezier2 P___((Plotter *_plotter, int x0, int y0, int x1, int y1, int x2, int y2));
extern int _API_bezier2rel P___((Plotter *_plotter, int dx0, int dy0, int dx1, int dy1, int dx2, int dy2));
extern int _API_bezier3 P___((Plotter *_plotter, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3));
extern int _API_bezier3rel P___((Plotter *_plotter, int dx0, int dy0, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3));
extern int _API_bgcolor P___((Plotter *_plotter, int red, int green, int blue));
extern int _API_bgcolorname P___((Plotter *_plotter, const char *name));
extern int _API_box P___((Plotter *_plotter, int x0, int y0, int x1, int y1));
extern int _API_boxrel P___((Plotter *_plotter, int dx0, int dy0, int dx1, int dy1));
extern int _API_capmod P___((Plotter *_plotter, const char *s));
extern int _API_circle P___((Plotter *_plotter, int x, int y, int r));
extern int _API_circlerel P___((Plotter *_plotter, int dx, int dy, int r));
extern int _API_closepath P___((Plotter *_plotter));
extern int _API_closepl P___((Plotter *_plotter));
extern int _API_color P___((Plotter *_plotter, int red, int green, int blue));
extern int _API_colorname P___((Plotter *_plotter, const char *name));
extern int _API_cont P___((Plotter *_plotter, int x, int y));
extern int _API_contrel P___((Plotter *_plotter, int x, int y));
extern int _API_ellarc P___((Plotter *_plotter, int xc, int yc, int x0, int y0, int x1, int y1));
extern int _API_ellarcrel P___((Plotter *_plotter, int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
extern int _API_ellipse P___((Plotter *_plotter, int x, int y, int rx, int ry, int angle));
extern int _API_ellipserel P___((Plotter *_plotter, int dx, int dy, int rx, int ry, int angle));
extern int _API_endpath P___((Plotter *_plotter));
extern int _API_endsubpath P___((Plotter *_plotter));
extern int _API_erase P___((Plotter *_plotter));
extern int _API_farc P___((Plotter *_plotter, double xc, double yc, double x0, double y0, double x1, double y1));
extern int _API_farcrel P___((Plotter *_plotter, double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _API_fbezier2 P___((Plotter *_plotter, double x0, double y0, double x1, double y1, double x2, double y2));
extern int _API_fbezier2rel P___((Plotter *_plotter, double dx0, double dy0, double dx1, double dy1, double dx2, double dy2));
extern int _API_fbezier3 P___((Plotter *_plotter, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3));
extern int _API_fbezier3rel P___((Plotter *_plotter, double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3));
extern int _API_fbox P___((Plotter *_plotter, double x0, double y0, double x1, double y1));
extern int _API_fboxrel P___((Plotter *_plotter, double dx0, double dy0, double dx1, double dy1));
extern int _API_fcircle P___((Plotter *_plotter, double x, double y, double r));
extern int _API_fcirclerel P___((Plotter *_plotter, double dx, double dy, double r));
extern int _API_fconcat P___((Plotter *_plotter, double m0, double m1, double m2, double m3, double m4, double m5));
extern int _API_fcont P___((Plotter *_plotter, double x, double y));
extern int _API_fcontrel P___((Plotter *_plotter, double x, double y));
extern int _API_fellarc P___((Plotter *_plotter, double xc, double yc, double x0, double y0, double x1, double y1));
extern int _API_fellarcrel P___((Plotter *_plotter, double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
extern int _API_fellipse P___((Plotter *_plotter, double x, double y, double rx, double ry, double angle));
extern int _API_fellipserel P___((Plotter *_plotter, double dx, double dy, double rx, double ry, double angle));
extern int _API_fillcolor P___((Plotter *_plotter, int red, int green, int blue));
extern int _API_fillcolorname P___((Plotter *_plotter, const char *name));
extern int _API_fillmod P___((Plotter *_plotter, const char *s));
extern int _API_filltype P___((Plotter *_plotter, int level));
extern int _API_fline P___((Plotter *_plotter, double x0, double y0, double x1, double y1));
extern int _API_flinedash P___((Plotter *_plotter, int n, const double *dashes, double offset));
extern int _API_flinerel P___((Plotter *_plotter, double dx0, double dy0, double dx1, double dy1));
extern int _API_flinewidth P___((Plotter *_plotter, double size));
extern int _API_flushpl P___((Plotter *_plotter));
extern int _API_fmarker P___((Plotter *_plotter, double x, double y, int type, double size));
extern int _API_fmarkerrel P___((Plotter *_plotter, double dx, double dy, int type, double size));
extern int _API_fmiterlimit P___((Plotter *_plotter, double limit));
extern int _API_fmove P___((Plotter *_plotter, double x, double y));
extern int _API_fmoverel P___((Plotter *_plotter, double x, double y));
extern int _API_fontname P___((Plotter *_plotter, const char *s));
extern int _API_fontsize P___((Plotter *_plotter, int size));
extern int _API_fpoint P___((Plotter *_plotter, double x, double y));
extern int _API_fpointrel P___((Plotter *_plotter, double dx, double dy));
extern int _API_frotate P___((Plotter *_plotter, double theta));
extern int _API_fscale P___((Plotter *_plotter, double x, double y));
extern int _API_fsetmatrix P___((Plotter *_plotter, double m0, double m1, double m2, double m3, double m4, double m5));
extern int _API_fspace P___((Plotter *_plotter, double x0, double y0, double x1, double y1));
extern int _API_fspace2 P___((Plotter *_plotter, double x0, double y0, double x1, double y1, double x2, double y2));
extern int _API_ftranslate P___((Plotter *_plotter, double x, double y));
extern int _API_havecap P___((Plotter *_plotter, const char *s));
extern int _API_joinmod P___((Plotter *_plotter, const char *s));
extern int _API_label P___((Plotter *_plotter, const char *s));
extern int _API_labelwidth P___((Plotter *_plotter, const char *s));
extern int _API_line P___((Plotter *_plotter, int x0, int y0, int x1, int y1));
extern int _API_linedash P___((Plotter *_plotter, int n, const int *dashes, int offset));
extern int _API_linemod P___((Plotter *_plotter, const char *s));
extern int _API_linerel P___((Plotter *_plotter, int dx0, int dy0, int dx1, int dy1));
extern int _API_linewidth P___((Plotter *_plotter, int size));
extern int _API_marker P___((Plotter *_plotter, int x, int y, int type, int size));
extern int _API_markerrel P___((Plotter *_plotter, int dx, int dy, int type, int size));
extern int _API_move P___((Plotter *_plotter, int x, int y));
extern int _API_moverel P___((Plotter *_plotter, int x, int y));
extern int _API_openpl P___((Plotter *_plotter));
extern int _API_orientation P___((Plotter *_plotter, int direction));
extern int _API_pencolor P___((Plotter *_plotter, int red, int green, int blue));
extern int _API_pencolorname P___((Plotter *_plotter, const char *name));
extern int _API_pentype P___((Plotter *_plotter, int level));
extern int _API_point P___((Plotter *_plotter, int x, int y));
extern int _API_pointrel P___((Plotter *_plotter, int dx, int dy));
extern int _API_restorestate P___((Plotter *_plotter));
extern int _API_savestate P___((Plotter *_plotter));
extern int _API_space P___((Plotter *_plotter, int x0, int y0, int x1, int y1));
extern int _API_space2 P___((Plotter *_plotter, int x0, int y0, int x1, int y1, int x2, int y2));
extern int _API_textangle P___((Plotter *_plotter, int angle));
/* Plotter protected methods, for libplot */
extern bool _g_begin_page P___((Plotter *_plotter));
extern bool _g_end_page P___((Plotter *_plotter));
extern bool _g_erase_page P___((Plotter *_plotter));
extern bool _g_flush_output P___((Plotter *_plotter));
extern bool _g_paint_marker P___((Plotter *_plotter, int type, double size));
extern bool _g_paint_paths P___((Plotter *_plotter));
extern bool _g_path_is_flushable P___((Plotter *_plotter));
extern bool _g_retrieve_font P___((Plotter *_plotter));
extern double _g_get_text_width P___((Plotter *_plotter, const unsigned char *s));
extern double _g_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int x_justify, int y_justify));
extern void _g_error P___((Plotter *_plotter, const char *msg));
extern void _g_initialize P___((Plotter *_plotter));
extern void _g_maybe_prepaint_segments P___((Plotter *_plotter, int prev_num_segments));
extern void _g_paint_path P___((Plotter *_plotter));
extern void _g_paint_point P___((Plotter *_plotter));
extern void _g_paint_text_string_with_escapes P___((Plotter *_plotter, const unsigned char *s, int x_justify, int y_justify));
extern void _g_pop_state P___((Plotter *_plotter));
extern void _g_push_state P___((Plotter *_plotter));
extern void _g_terminate P___((Plotter *_plotter));
extern void _g_warning P___((Plotter *_plotter, const char *msg));
/* undocumented public methods that provide access to the font tables
   within libplot/libplotter; for libplot */
extern voidptr_t pl_get_hershey_font_info P___((Plotter *_plotter));
extern voidptr_t pl_get_ps_font_info P___((Plotter *_plotter));
extern voidptr_t pl_get_pcl_font_info P___((Plotter *_plotter));
extern voidptr_t pl_get_stick_font_info P___((Plotter *_plotter));
/* private functions related to the drawing of text strings in Hershey
   fonts (defined in g_alab_her.c); for libplot */
extern double _alabel_hershey P___((Plotter *_plotter, const unsigned char *s, int x_justify, int y_justify));
extern double _flabelwidth_hershey P___((Plotter *_plotter, const unsigned char *s));
extern void _draw_hershey_glyph P___((Plotter *_plotter, int num, double charsize, int type, bool oblique));
extern void _draw_hershey_penup_stroke P___((Plotter *_plotter, double dx, double dy, double charsize, bool oblique));
extern void _draw_hershey_string P___((Plotter *_plotter, const unsigned short *string));
extern void _draw_hershey_stroke P___((Plotter *_plotter, bool pendown, double deltax, double deltay));
/* other private Plotter functions (a mixed bag), for libplot */
extern double _render_non_hershey_string P___((Plotter *_plotter, const char *s, bool do_render, int x_justify, int y_justify));
extern double _render_simple_string P___((Plotter *_plotter, const unsigned char *s, bool do_render, int h_just, int v_just));
extern unsigned short * _controlify P___((Plotter *_plotter, const unsigned char *));
extern void _copy_params_to_plotter P___((Plotter *_plotter, const PlotterParams *params));
extern void _create_first_drawing_state P___((Plotter *_plotter));
extern void _delete_first_drawing_state P___((Plotter *_plotter));
extern void _free_params_in_plotter P___((Plotter *_plotter));
extern void _maybe_replace_arc P___((Plotter *_plotter));
extern void _set_font P___((Plotter *_plotter));
/* other protected Plotter functions (a mixed bag), for libplot */
extern void _flush_plotter_outstreams P___((Plotter *_plotter));
#else  /* LIBPLOTTER */
/* static Plotter public method (libplotter only) */
#define parampl Plotter::parampl
/* Plotter public methods, for libplotter */
#define _API_alabel Plotter::alabel
#define _API_arc Plotter::arc
#define _API_arcrel Plotter::arcrel
#define _API_bezier2 Plotter::bezier2
#define _API_bezier2rel Plotter::bezier2rel
#define _API_bezier3 Plotter::bezier3
#define _API_bezier3rel Plotter::bezier3rel
#define _API_bgcolor Plotter::bgcolor
#define _API_bgcolorname Plotter::bgcolorname
#define _API_box Plotter::box
#define _API_boxrel Plotter::boxrel
#define _API_capmod Plotter::capmod
#define _API_circle Plotter::circle
#define _API_circlerel Plotter::circlerel
#define _API_closepath Plotter::closepath
#define _API_closepl Plotter::closepl
#define _API_color Plotter::color
#define _API_colorname Plotter::colorname
#define _API_cont Plotter::cont
#define _API_contrel Plotter::contrel
#define _API_ellarc Plotter::ellarc
#define _API_ellarcrel Plotter::ellarcrel
#define _API_ellipse Plotter::ellipse
#define _API_ellipserel Plotter::ellipserel
#define _API_endpath Plotter::endpath
#define _API_endsubpath Plotter::endsubpath
#define _API_erase Plotter::erase
#define _API_farc Plotter::farc
#define _API_farcrel Plotter::farcrel
#define _API_fbezier2 Plotter::fbezier2
#define _API_fbezier2rel Plotter::fbezier2rel
#define _API_fbezier3 Plotter::fbezier3
#define _API_fbezier3rel Plotter::fbezier3rel
#define _API_fbox Plotter::fbox
#define _API_fboxrel Plotter::fboxrel
#define _API_fcircle Plotter::fcircle
#define _API_fcirclerel Plotter::fcirclerel
#define _API_fconcat Plotter::fconcat
#define _API_fcont Plotter::fcont
#define _API_fcontrel Plotter::fcontrel
#define _API_fellarc Plotter::fellarc
#define _API_fellarcrel Plotter::fellarcrel
#define _API_fellipse Plotter::fellipse
#define _API_fellipserel Plotter::fellipserel
#define _API_ffontname Plotter::ffontname
#define _API_ffontsize Plotter::ffontsize
#define _API_fillcolor Plotter::fillcolor
#define _API_fillcolorname Plotter::fillcolorname
#define _API_fillmod Plotter::fillmod
#define _API_filltype Plotter::filltype
#define _API_flabelwidth Plotter::flabelwidth
#define _API_fline Plotter::fline
#define _API_flinedash Plotter::flinedash
#define _API_flinerel Plotter::flinerel
#define _API_flinewidth Plotter::flinewidth
#define _API_flushpl Plotter::flushpl
#define _API_fmarker Plotter::fmarker
#define _API_fmarkerrel Plotter::fmarkerrel
#define _API_fmiterlimit Plotter::fmiterlimit
#define _API_fmove Plotter::fmove
#define _API_fmoverel Plotter::fmoverel
#define _API_fontname Plotter::fontname
#define _API_fontsize Plotter::fontsize
#define _API_fpoint Plotter::fpoint
#define _API_fpointrel Plotter::fpointrel
#define _API_frotate Plotter::frotate
#define _API_fscale Plotter::fscale
#define _API_fsetmatrix Plotter::fsetmatrix
#define _API_fspace Plotter::fspace
#define _API_fspace2 Plotter::fspace2
#define _API_ftextangle Plotter::ftextangle
#define _API_ftranslate Plotter::ftranslate
#define _API_havecap Plotter::havecap
#define _API_joinmod Plotter::joinmod
#define _API_label Plotter::label
#define _API_labelwidth Plotter::labelwidth
#define _API_line Plotter::line
#define _API_linedash Plotter::linedash
#define _API_linemod Plotter::linemod
#define _API_linerel Plotter::linerel
#define _API_linewidth Plotter::linewidth
#define _API_marker Plotter::marker
#define _API_markerrel Plotter::markerrel
#define _API_move Plotter::move
#define _API_moverel Plotter::moverel
#define _API_openpl Plotter::openpl
#define _API_orientation Plotter::orientation
#define _API_outfile Plotter::outfile /* OBSOLESCENT */
#define _API_pencolor Plotter::pencolor
#define _API_pencolorname Plotter::pencolorname
#define _API_pentype Plotter::pentype
#define _API_point Plotter::point
#define _API_pointrel Plotter::pointrel
#define _API_restorestate Plotter::restorestate
#define _API_savestate Plotter::savestate
#define _API_space Plotter::space
#define _API_space2 Plotter::space2
#define _API_textangle Plotter::textangle
/* Plotter protected methods, for libplotter */
#define _g_begin_page Plotter::begin_page
#define _g_end_page Plotter::end_page
#define _g_erase_page Plotter::erase_page
#define _g_error Plotter::error
#define _g_paint_text_string_with_escapes Plotter::paint_text_string_with_escapes
#define _g_paint_text_string Plotter::paint_text_string
#define _g_get_text_width Plotter::get_text_width
#define _g_flush_output Plotter::flush_output
#define _g_initialize Plotter::initialize
#define _g_path_is_flushable Plotter::path_is_flushable
#define _g_maybe_prepaint_segments Plotter::maybe_prepaint_segments
#define _g_paint_marker Plotter::paint_marker
#define _g_paint_path Plotter::paint_path
#define _g_paint_paths Plotter::paint_paths
#define _g_paint_point Plotter::paint_point
#define _g_pop_state Plotter::pop_state
#define _g_push_state Plotter::push_state
#define _g_retrieve_font Plotter::retrieve_font
#define _g_terminate Plotter::terminate
#define _g_warning Plotter::warning
/* undocumented public methods that provide access to the font tables
   within libplot/libplotter; for libplotter */
#define pl_get_hershey_font_info Plotter::get_hershey_font_info
#define pl_get_ps_font_info Plotter::get_ps_font_info
#define pl_get_pcl_font_info Plotter::get_pcl_font_info
#define pl_get_stick_font_info Plotter::get_stick_font_info
/* private functions related to the drawing of text strings in Hershey
   fonts (defined in g_alab_her.c), for libplotter  */
#define _alabel_hershey Plotter::_alabel_hershey
#define _draw_hershey_glyph Plotter::_draw_hershey_glyph
#define _draw_hershey_penup_stroke Plotter::_draw_hershey_penup_stroke
#define _draw_hershey_string Plotter::_draw_hershey_string
#define _draw_hershey_stroke Plotter::_draw_hershey_stroke
#define _flabelwidth_hershey Plotter::_flabelwidth_hershey
/* other private functions (a mixed bag), for libplotter */
#define _controlify Plotter::_controlify
#define _copy_params_to_plotter Plotter::_copy_params_to_plotter
#define _create_first_drawing_state Plotter::_create_first_drawing_state
#define _delete_first_drawing_state Plotter::_delete_first_drawing_state
#define _free_params_in_plotter Plotter::_free_params_in_plotter
#define _maybe_replace_arc Plotter::_maybe_replace_arc
#define _render_non_hershey_string Plotter::_render_non_hershey_string
#define _render_simple_string Plotter::_render_simple_string
#define _set_font Plotter::_set_font
/* other protected functions (a mixed bag), for libplotter */
#define _flush_plotter_outstreams Plotter::_flush_plotter_outstreams
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* MetaPlotter protected methods, for libplot */
extern bool _m_begin_page P___((Plotter *_plotter));
extern bool _m_end_page P___((Plotter *_plotter));
extern bool _m_erase_page P___((Plotter *_plotter));
extern bool _m_paint_marker P___((Plotter *_plotter, int type, double size));
extern bool _m_paint_paths P___((Plotter *_plotter));
extern bool _m_path_is_flushable P___((Plotter *_plotter));
extern void _m_initialize P___((Plotter *_plotter));
extern void _m_maybe_prepaint_segments P___((Plotter *_plotter, int prev_num_segments));
extern void _m_paint_path P___((Plotter *_plotter));
extern void _m_paint_point P___((Plotter *_plotter));
extern void _m_paint_text_string_with_escapes P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern void _m_terminate P___((Plotter *_plotter));
/* MetaPlotter-specific internal functions, for libplot */
extern void _m_emit_integer P___((Plotter *_plotter, int x));
extern void _m_emit_float P___((Plotter *_plotter, double x));
extern void _m_emit_op_code P___((Plotter *_plotter, int c));
extern void _m_emit_string P___((Plotter *_plotter, const char *s));
extern void _m_emit_terminator P___((Plotter *_plotter));
extern void _m_paint_path_internal P___((Plotter *_plotter, const plPath *path));
extern void _m_set_attributes P___((Plotter *_plotter, unsigned int mask));
#else  /* LIBPLOTTER */
/* MetaPlotter protected methods, for libplotter */
#define _m_begin_page MetaPlotter::begin_page
#define _m_end_page MetaPlotter::end_page
#define _m_erase_page MetaPlotter::erase_page
#define _m_paint_text_string_with_escapes MetaPlotter::paint_text_string_with_escapes
#define _m_initialize MetaPlotter::initialize
#define _m_path_is_flushable MetaPlotter::path_is_flushable
#define _m_maybe_prepaint_segments MetaPlotter::maybe_prepaint_segments
#define _m_paint_marker MetaPlotter::paint_marker
#define _m_paint_path MetaPlotter::paint_path
#define _m_paint_paths MetaPlotter::paint_paths
#define _m_paint_point MetaPlotter::paint_point
#define _m_terminate MetaPlotter::terminate
/* MetaPlotter-specific internal functions, for libplotter */
#define _m_emit_integer MetaPlotter::_m_emit_integer
#define _m_emit_float MetaPlotter::_m_emit_float
#define _m_emit_op_code MetaPlotter::_m_emit_op_code
#define _m_emit_string MetaPlotter::_m_emit_string
#define _m_emit_terminator MetaPlotter::_m_emit_terminator
#define _m_paint_path_internal MetaPlotter::_m_paint_path_internal
#define _m_set_attributes MetaPlotter::_m_set_attributes
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* BitmapPlotter protected methods, for libplot */
extern bool _b_begin_page P___((Plotter *_plotter));
extern bool _b_end_page P___((Plotter *_plotter));
extern bool _b_erase_page P___((Plotter *_plotter));
extern bool _b_paint_paths P___((Plotter *_plotter));
extern void _b_initialize P___((Plotter *_plotter));
extern void _b_paint_path P___((Plotter *_plotter));
extern void _b_paint_point P___((Plotter *_plotter));
extern void _b_terminate P___((Plotter *_plotter));
/* BitmapPlotter internal functions, for libplot (overridden in subclasses) */
extern int _b_maybe_output_image P___((Plotter *_plotter));
/* other BitmapPlotter internal functions, for libplot */
extern void _b_delete_image P___((Plotter *_plotter));
extern void _b_draw_elliptic_arc P___((Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc));
extern void _b_draw_elliptic_arc_2 P___((Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc));
extern void _b_draw_elliptic_arc_internal P___((Plotter *_plotter, int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange));
extern void _b_new_image P___((Plotter *_plotter));
#else  /* LIBPLOTTER */
/* BitmapPlotter protected methods, for libplotter */
#define _b_begin_page BitmapPlotter::begin_page
#define _b_end_page BitmapPlotter::end_page
#define _b_erase_page BitmapPlotter::erase_page
#define _b_initialize BitmapPlotter::initialize
#define _b_paint_path BitmapPlotter::paint_path
#define _b_paint_paths BitmapPlotter::paint_paths
#define _b_paint_point BitmapPlotter::paint_point
#define _b_terminate BitmapPlotter::terminate
/* BitmapPlotter internal functions (overriden in subclasses) */
#define _b_maybe_output_image BitmapPlotter::_maybe_output_image
/* other BitmapPlotter internal functions, for libplotter */
#define _b_delete_image BitmapPlotter::_b_delete_image
#define _b_draw_elliptic_arc BitmapPlotter::_b_draw_elliptic_arc
#define _b_draw_elliptic_arc_2 BitmapPlotter::_b_draw_elliptic_arc_2
#define _b_draw_elliptic_arc_internal BitmapPlotter::_b_draw_elliptic_arc_internal
#define _b_new_image BitmapPlotter::_b_new_image 
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* TekPlotter protected methods, for libplot */
extern bool _t_begin_page P___((Plotter *_plotter));
extern bool _t_end_page P___((Plotter *_plotter));
extern bool _t_erase_page P___((Plotter *_plotter));
extern bool _t_path_is_flushable P___((Plotter *_plotter));
extern void _t_initialize P___((Plotter *_plotter));
extern void _t_maybe_prepaint_segments P___((Plotter *_plotter, int prev_num_segments));
extern void _t_paint_point P___((Plotter *_plotter));
extern void _t_terminate P___((Plotter *_plotter));
/* TekPlotter internal functions, for libplot */
extern void _t_set_attributes P___((Plotter *_plotter));
extern void _t_set_bg_color P___((Plotter *_plotter));
extern void _t_set_pen_color P___((Plotter *_plotter));
extern void _tek_mode P___((Plotter *_plotter, int newmode));
extern void _tek_move P___((Plotter *_plotter, int xx, int yy));
extern void _tek_vector P___((Plotter *_plotter, int xx, int yy));
extern void _tek_vector_compressed P___((Plotter *_plotter, int xx, int yy, int oldxx, int oldyy, bool force));
#else  /* LIBPLOTTER */
/* TekPlotter protected methods, for libplotter */
#define _t_begin_page TekPlotter::begin_page
#define _t_end_page TekPlotter::end_page
#define _t_erase_page TekPlotter::erase_page
#define _t_initialize TekPlotter::initialize
#define _t_path_is_flushable TekPlotter::path_is_flushable
#define _t_maybe_prepaint_segments TekPlotter::maybe_prepaint_segments
#define _t_paint_point TekPlotter::paint_point
#define _t_terminate TekPlotter::terminate
/* TekPlotter internal functions, for libplotter */
#define _t_set_attributes TekPlotter::_t_set_attributes
#define _t_set_bg_color TekPlotter::_t_set_bg_color
#define _t_set_pen_color TekPlotter::_t_set_pen_color
#define _tek_mode TekPlotter::_tek_mode
#define _tek_move TekPlotter::_tek_move
#define _tek_vector TekPlotter::_tek_vector
#define _tek_vector_compressed TekPlotter::_tek_vector_compressed
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* ReGISPlotter protected methods, for libplot */
extern bool _r_begin_page P___((Plotter *_plotter));
extern bool _r_end_page P___((Plotter *_plotter));
extern bool _r_erase_page P___((Plotter *_plotter));
extern bool _r_paint_paths P___((Plotter *_plotter));
extern bool _r_path_is_flushable P___((Plotter *_plotter));
extern void _r_initialize P___((Plotter *_plotter));
extern void _r_maybe_prepaint_segments P___((Plotter *_plotter, int prev_num_segments));
extern void _r_paint_path P___((Plotter *_plotter));
extern void _r_paint_point P___((Plotter *_plotter));
extern void _r_terminate P___((Plotter *_plotter));
/* ReGISPlotter internal functions, for libplot */
extern void _r_set_attributes P___((Plotter *_plotter));
extern void _r_set_bg_color P___((Plotter *_plotter));
extern void _r_set_fill_color P___((Plotter *_plotter));
extern void _r_set_pen_color P___((Plotter *_plotter));
extern void _regis_move P___((Plotter *_plotter, int xx, int yy));
#else  /* LIBPLOTTER */
/* ReGISPlotter protected methods, for libplotter */
#define _r_begin_page ReGISPlotter::begin_page
#define _r_end_page ReGISPlotter::end_page
#define _r_erase_page ReGISPlotter::erase_page
#define _r_initialize ReGISPlotter::initialize
#define _r_path_is_flushable ReGISPlotter::path_is_flushable
#define _r_maybe_prepaint_segments ReGISPlotter::maybe_prepaint_segments
#define _r_paint_path ReGISPlotter::paint_path
#define _r_paint_paths ReGISPlotter::paint_paths
#define _r_paint_point ReGISPlotter::paint_point
#define _r_terminate ReGISPlotter::terminate
/* ReGISPlotter internal functions, for libplotter */
#define _r_set_attributes ReGISPlotter::_r_set_attributes
#define _r_set_bg_color ReGISPlotter::_r_set_bg_color
#define _r_set_fill_color ReGISPlotter::_r_set_fill_color
#define _r_set_pen_color ReGISPlotter::_r_set_pen_color
#define _regis_move ReGISPlotter::_regis_move
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* HPGLPlotter/PCLPlotter protected methods, for libplot */
extern bool _h_begin_page P___((Plotter *_plotter));
extern bool _h_end_page P___((Plotter *_plotter));
extern bool _h_erase_page P___((Plotter *_plotter));
extern bool _h_paint_paths P___((Plotter *_plotter));
extern double _h_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern void _h_paint_point P___((Plotter *_plotter));
extern void _h_paint_path P___((Plotter *_plotter));
/* HPGLPlotter protected methods, for libplot */
extern void _h_initialize P___((Plotter *_plotter));
extern void _h_terminate P___((Plotter *_plotter));
/* PCLPlotter protected methods, for libplot */
extern void _q_initialize P___((Plotter *_plotter));
extern void _q_terminate P___((Plotter *_plotter));
/* HPGLPlotter/PCLPlotter internal functions, for libplot */
extern bool _hpgl2_maybe_update_font P___((Plotter *_plotter));
extern bool _hpgl_maybe_update_font P___((Plotter *_plotter));
extern bool _parse_pen_string P___((Plotter *_plotter, const char *pen_s));
extern int _hpgl_pseudocolor P___((Plotter *_plotter, int red, int green, int blue, bool restrict_white));
extern void _h_set_attributes P___((Plotter *_plotter));
extern void _h_set_fill_color P___((Plotter *_plotter, bool force_pen_color));
extern void _h_set_font P___((Plotter *_plotter));
extern void _h_set_pen_color P___((Plotter *_plotter, int hpgl_object_type));
extern void _h_set_position P___((Plotter *_plotter));
extern void _hpgl_shaded_pseudocolor P___((Plotter *_plotter, int red, int green, int blue, int *pen, double *shading));
extern void _set_hpgl_fill_type P___((Plotter *_plotter, int fill_type, double option1, double option2));
extern void _set_hpgl_pen P___((Plotter *_plotter, int pen));
extern void _set_hpgl_pen_type P___((Plotter *_plotter, int pen_type, double option1, double option2));
/* HPGLPlotter functions (overridden in PCLPlotter class), for libplotter */
extern void _h_maybe_switch_to_hpgl P___((Plotter *_plotter)); 
extern void _h_maybe_switch_from_hpgl P___((Plotter *_plotter)); 
/* PCLPlotter functions (overriding the above), for libplotter */
extern void _q_maybe_switch_to_hpgl P___((Plotter *_plotter)); 
extern void _q_maybe_switch_from_hpgl P___((Plotter *_plotter)); 
#else  /* LIBPLOTTER */
/* HPGLPlotter/PCLPlotter protected methods, for libplotter */
#define _h_begin_page HPGLPlotter::begin_page
#define _h_end_page HPGLPlotter::end_page
#define _h_erase_page HPGLPlotter::erase_page
#define _h_paint_text_string HPGLPlotter::paint_text_string
#define _h_paint_path HPGLPlotter::paint_path
#define _h_paint_paths HPGLPlotter::paint_paths
#define _h_paint_point HPGLPlotter::paint_point
/* HPGLPlotter protected methods, for libplotter */
#define _h_initialize HPGLPlotter::initialize
#define _h_terminate HPGLPlotter::terminate
/* PCLPlotter protected methods, for libplotter */
#define _q_initialize PCLPlotter::initialize
#define _q_terminate PCLPlotter::terminate
/* HPGLPlotter/PCLPlotter internal functions, for libplotter */
#define _hpgl_shaded_pseudocolor HPGLPlotter::_hpgl_shaded_pseudocolor
#define _hpgl2_maybe_update_font HPGLPlotter::_hpgl2_maybe_update_font
#define _hpgl_maybe_update_font HPGLPlotter::_hpgl_maybe_update_font
#define _hpgl_pseudocolor HPGLPlotter::_hpgl_pseudocolor
#define _parse_pen_string HPGLPlotter::_parse_pen_string
#define _h_set_attributes HPGLPlotter::_h_set_attributes
#define _h_set_fill_color HPGLPlotter::_h_set_fill_color
#define _h_set_font HPGLPlotter::_h_set_font
#define _h_set_pen_color HPGLPlotter::_h_set_pen_color
#define _h_set_position HPGLPlotter::_h_set_position
#define _set_hpgl_fill_type HPGLPlotter::_set_hpgl_fill_type
#define _set_hpgl_pen HPGLPlotter::_set_hpgl_pen
#define _set_hpgl_pen_type HPGLPlotter::_set_hpgl_pen_type
/* HPGLPlotter functions (overridden in PCLPlotter class), for libplotter */
#define _h_maybe_switch_to_hpgl HPGLPlotter::_maybe_switch_to_hpgl
#define _h_maybe_switch_from_hpgl HPGLPlotter::_maybe_switch_from_hpgl
/* PCLPlotter functions (overriding the above), for libplotter */
#define _q_maybe_switch_to_hpgl PCLPlotter::_maybe_switch_to_hpgl
#define _q_maybe_switch_from_hpgl PCLPlotter::_maybe_switch_from_hpgl
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* FigPlotter protected methods, for libplot */
extern bool _f_begin_page P___((Plotter *_plotter));
extern bool _f_end_page P___((Plotter *_plotter));
extern bool _f_erase_page P___((Plotter *_plotter));
extern bool _f_paint_paths P___((Plotter *_plotter));
extern bool _f_retrieve_font P___((Plotter *_plotter));
extern double _f_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern void _f_initialize P___((Plotter *_plotter));
extern void _f_paint_path P___((Plotter *_plotter));
extern void _f_paint_point P___((Plotter *_plotter));
extern void _f_terminate P___((Plotter *_plotter));
/* FigPlotter internal functions, for libplot */
extern int _fig_color P___((Plotter *_plotter, int red, int green, int blue));
extern void _f_compute_line_style P___((Plotter *_plotter, int *style, double *spacing));
extern void _f_draw_arc_internal P___((Plotter *_plotter, double xc, double yc, double x0, double y0, double x1, double y1));
extern void _f_draw_box_internal P___((Plotter *_plotter, plPoint p0, plPoint p1));
extern void _f_draw_ellipse_internal P___((Plotter *_plotter, double x, double y, double rx, double ry, double angle, int subtype));
extern void _f_set_fill_color P___((Plotter *_plotter));
extern void _f_set_pen_color P___((Plotter *_plotter));
#else  /* LIBPLOTTER */
/* FigPlotter protected methods, for libplotter */
#define _f_begin_page FigPlotter::begin_page
#define _f_end_page FigPlotter::end_page
#define _f_erase_page FigPlotter::erase_page
#define _f_paint_text_string FigPlotter::paint_text_string
#define _f_initialize FigPlotter::initialize
#define _f_paint_path FigPlotter::paint_path
#define _f_paint_paths FigPlotter::paint_paths
#define _f_paint_point FigPlotter::paint_point
#define _f_retrieve_font FigPlotter::retrieve_font
#define _f_terminate FigPlotter::terminate
/* FigPlotter internal functions, for libplotter */
#define _f_compute_line_style FigPlotter::_f_compute_line_style
#define _f_draw_arc_internal FigPlotter::_f_draw_arc_internal
#define _f_draw_box_internal FigPlotter::_f_draw_box_internal
#define _f_draw_ellipse_internal FigPlotter::_f_draw_ellipse_internal
#define _f_set_fill_color FigPlotter::_f_set_fill_color
#define _f_set_pen_color FigPlotter::_f_set_pen_color
#define _fig_color FigPlotter::_fig_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* CGMPlotter protected methods, for libplot */
extern bool _c_begin_page P___((Plotter *_plotter));
extern bool _c_end_page P___((Plotter *_plotter));
extern bool _c_erase_page P___((Plotter *_plotter));
extern bool _c_paint_marker P___((Plotter *_plotter, int type, double size));
extern bool _c_paint_paths P___((Plotter *_plotter));
extern double _c_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern void _c_initialize P___((Plotter *_plotter));
extern void _c_paint_path P___((Plotter *_plotter));
extern void _c_paint_point P___((Plotter *_plotter));
extern void _c_terminate P___((Plotter *_plotter));
/* CGMPlotter internal functions, for libplot */
extern void _c_set_attributes P___((Plotter *_plotter, int cgm_object_type));
extern void _c_set_bg_color P___((Plotter *_plotter));
extern void _c_set_fill_color P___((Plotter *_plotter, int cgm_object_type));
extern void _c_set_pen_color P___((Plotter *_plotter, int cgm_object_type));
#else  /* LIBPLOTTER */
/* CGMPlotter protected methods, for libplotter */
#define _c_begin_page CGMPlotter::begin_page
#define _c_end_page CGMPlotter::end_page
#define _c_erase_page CGMPlotter::erase_page
#define _c_paint_text_string CGMPlotter::paint_text_string
#define _c_initialize CGMPlotter::initialize
#define _c_paint_marker CGMPlotter::paint_marker
#define _c_paint_path CGMPlotter::paint_path
#define _c_paint_paths CGMPlotter::paint_paths
#define _c_paint_point CGMPlotter::paint_point
#define _c_terminate CGMPlotter::terminate
/* CGMPlotter internal functions, for libplotter */
#define _c_set_attributes CGMPlotter::_c_set_attributes
#define _c_set_bg_color CGMPlotter::_c_set_bg_color
#define _c_set_fill_color CGMPlotter::_c_set_fill_color
#define _c_set_pen_color CGMPlotter::_c_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* PSPlotter protected methods, for libplot */
extern bool _p_begin_page P___((Plotter *_plotter));
extern bool _p_end_page P___((Plotter *_plotter));
extern bool _p_erase_page P___((Plotter *_plotter));
extern bool _p_paint_paths P___((Plotter *_plotter));
extern double _p_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern void _p_initialize P___((Plotter *_plotter));
extern void _p_paint_path P___((Plotter *_plotter));
extern void _p_paint_point P___((Plotter *_plotter));
extern void _p_terminate P___((Plotter *_plotter));
/* PSPlotter internal functions, for libplot */
extern double _p_emit_common_attributes P___((Plotter *_plotter));
extern void _p_compute_idraw_bgcolor P___((Plotter *_plotter));
extern void _p_fellipse_internal P___((Plotter *_plotter, double x, double y, double rx, double ry, double angle, bool circlep));
extern void _p_set_fill_color P___((Plotter *_plotter));
extern void _p_set_pen_color P___((Plotter *_plotter));
#else  /* LIBPLOTTER */
/* PSPlotter protected methods, for libplotter */
#define _p_begin_page PSPlotter::begin_page
#define _p_end_page PSPlotter::end_page
#define _p_erase_page PSPlotter::erase_page
#define _p_paint_text_string PSPlotter::paint_text_string
#define _p_initialize PSPlotter::initialize
#define _p_paint_path PSPlotter::paint_path
#define _p_paint_paths PSPlotter::paint_paths
#define _p_paint_point PSPlotter::paint_point
#define _p_terminate PSPlotter::terminate
/* PSPlotter internal functions, for libplotter */
#define _p_compute_idraw_bgcolor PSPlotter::_p_compute_idraw_bgcolor
#define _p_emit_common_attributes PSPlotter::_p_emit_common_attributes
#define _p_fellipse_internal PSPlotter::_p_fellipse_internal
#define _p_set_fill_color PSPlotter::_p_set_fill_color
#define _p_set_pen_color PSPlotter::_p_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* AIPlotter protected methods, for libplot */
extern bool _a_begin_page P___((Plotter *_plotter));
extern bool _a_end_page P___((Plotter *_plotter));
extern bool _a_erase_page P___((Plotter *_plotter));
extern bool _a_paint_paths P___((Plotter *_plotter));
extern double _a_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern void _a_initialize P___((Plotter *_plotter));
extern void _a_paint_path P___((Plotter *_plotter));
extern void _a_paint_point P___((Plotter *_plotter));
extern void _a_terminate P___((Plotter *_plotter));
/* AIPlotter internal functions, for libplot */
extern void _a_set_attributes P___((Plotter *_plotter));
extern void _a_set_fill_color P___((Plotter *_plotter, bool force_pen_color));
extern void _a_set_pen_color P___((Plotter *_plotter));
#else /* LIBPLOTTER */
/* AIPlotter protected methods, for libplotter */
#define _a_begin_page AIPlotter::begin_page
#define _a_end_page AIPlotter::end_page
#define _a_erase_page AIPlotter::erase_page
#define _a_paint_text_string AIPlotter::paint_text_string
#define _a_initialize AIPlotter::initialize
#define _a_paint_path AIPlotter::paint_path
#define _a_paint_paths AIPlotter::paint_paths
#define _a_paint_point AIPlotter::paint_point
#define _a_terminate AIPlotter::terminate
/* AIPlotter internal functions, for libplotter */
#define _a_set_attributes AIPlotter::_a_set_attributes
#define _a_set_fill_color AIPlotter::_a_set_fill_color
#define _a_set_pen_color AIPlotter::_a_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* SVGPlotter protected methods, for libplot */
extern bool _s_begin_page P___((Plotter *_plotter));
extern bool _s_end_page P___((Plotter *_plotter));
extern bool _s_erase_page P___((Plotter *_plotter));
extern bool _s_paint_paths P___((Plotter *_plotter));
extern double _s_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern void _s_initialize P___((Plotter *_plotter));
extern void _s_paint_path P___((Plotter *_plotter));
extern void _s_paint_point P___((Plotter *_plotter));
extern void _s_terminate P___((Plotter *_plotter));
/* PSPlotter internal functions, for libplot */
extern void _s_set_matrix P___((Plotter *_plotter, const double m_base[6], const double m_local[6]));
#else  /* LIBPLOTTER */
/* SVGPlotter protected methods, for libplotter */
#define _s_begin_page SVGPlotter::begin_page
#define _s_end_page SVGPlotter::end_page
#define _s_erase_page SVGPlotter::erase_page
#define _s_paint_text_string SVGPlotter::paint_text_string
#define _s_initialize SVGPlotter::initialize
#define _s_paint_path SVGPlotter::paint_path
#define _s_paint_paths SVGPlotter::paint_paths
#define _s_paint_point SVGPlotter::paint_point
#define _s_terminate SVGPlotter::terminate
/* SVGPlotter internal functions, for libplotter */
#define _s_set_matrix SVGPlotter::_s_set_matrix
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
/* PNMPlotter protected methods, for libplot */
extern void _n_initialize P___((Plotter *_plotter));
extern void _n_terminate P___((Plotter *_plotter));
/* PNMPlotter internal functions (which override BitmapPlotter functions) */
extern int _n_maybe_output_image P___((Plotter *_plotter));
/* other PNMPlotter internal functions, for libplot */
extern void _n_write_pnm P___((Plotter *_plotter));
extern void _n_write_pbm P___((Plotter *_plotter));
extern void _n_write_pgm P___((Plotter *_plotter));
extern void _n_write_ppm P___((Plotter *_plotter));
#else  /* LIBPLOTTER */
/* PNMPlotter protected methods, for libplotter */
#define _n_initialize PNMPlotter::initialize
#define _n_terminate PNMPlotter::terminate
/* PNMPlotter internal methods (which override BitmapPlotter methods) */
#define _n_maybe_output_image PNMPlotter::_maybe_output_image
/* other PNMPlotter internal functions, for libplotter */
#define _n_write_pnm PNMPlotter::_n_write_pnm
#define _n_write_pbm PNMPlotter::_n_write_pbm
#define _n_write_pgm PNMPlotter::_n_write_pgm
#define _n_write_ppm PNMPlotter::_n_write_ppm
#endif /* LIBPLOTTER */

#ifdef INCLUDE_PNG_SUPPORT
#ifndef LIBPLOTTER
/* PNGPlotter protected methods, for libplot */
extern void _z_initialize P___((Plotter *_plotter));
extern void _z_terminate P___((Plotter *_plotter));
/* PNGPlotter internal functions (which override BitmapPlotter functions) */
extern int _z_maybe_output_image P___((Plotter *_plotter));
#else  /* LIBPLOTTER */
/* PNGPlotter protected methods, for libplotter */
#define _z_initialize PNGPlotter::initialize
#define _z_terminate PNGPlotter::terminate
/* PNGPlotter internal methods (which override BitmapPlotter methods) */
#define _z_maybe_output_image PNGPlotter::_maybe_output_image
#endif /* LIBPLOTTER */
#endif /* INCLUDE_PNG_SUPPORT */

#ifndef LIBPLOTTER
/* GIFPlotter protected methods, for libplot */
extern bool _i_begin_page P___((Plotter *_plotter));
extern bool _i_end_page P___((Plotter *_plotter));
extern bool _i_erase_page P___((Plotter *_plotter));
extern bool _i_paint_paths P___((Plotter *_plotter));
extern void _i_initialize P___((Plotter *_plotter));
extern void _i_paint_path P___((Plotter *_plotter));
extern void _i_paint_point P___((Plotter *_plotter));
extern void _i_terminate P___((Plotter *_plotter));
/* GIFPlotter internal functions, for libplot */
extern int _i_scan_pixel P___((Plotter *_plotter));
extern unsigned char _i_new_color_index P___((Plotter *_plotter, int red, int green, int blue));
extern void _i_delete_image P___((Plotter *_plotter));
extern void _i_draw_elliptic_arc P___((Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc));
extern void _i_draw_elliptic_arc_2 P___((Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc));
extern void _i_draw_elliptic_arc_internal P___((Plotter *_plotter, int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange));
extern void _i_new_image P___((Plotter *_plotter));
extern void _i_set_bg_color P___((Plotter *_plotter));
extern void _i_set_fill_color P___((Plotter *_plotter));
extern void _i_set_pen_color P___((Plotter *_plotter));
extern void _i_start_scan P___((Plotter *_plotter));
extern void _i_write_gif_header P___((Plotter *_plotter));
extern void _i_write_gif_image P___((Plotter *_plotter));
extern void _i_write_gif_trailer P___((Plotter *_plotter));
extern void _i_write_short_int P___((Plotter *_plotter, unsigned int i));
#else  /* LIBPLOTTER */
/* GIFPlotter protected methods, for libplotter */
#define _i_begin_page GIFPlotter::begin_page
#define _i_end_page GIFPlotter::end_page
#define _i_erase_page GIFPlotter::erase_page
#define _i_initialize GIFPlotter::initialize
#define _i_paint_path GIFPlotter::paint_path
#define _i_paint_paths GIFPlotter::paint_paths
#define _i_paint_point GIFPlotter::paint_point
#define _i_terminate GIFPlotter::terminate
/* GIFPlotter internal functions, for libplotter */
#define _i_scan_pixel GIFPlotter::_i_scan_pixel
#define _i_new_color_index GIFPlotter::_i_new_color_index
#define _i_delete_image GIFPlotter::_i_delete_image
#define _i_draw_elliptic_arc GIFPlotter::_i_draw_elliptic_arc
#define _i_draw_elliptic_arc_2 GIFPlotter::_i_draw_elliptic_arc_2
#define _i_draw_elliptic_arc_internal GIFPlotter::_i_draw_elliptic_arc_internal
#define _i_new_image GIFPlotter::_i_new_image 
#define _i_set_bg_color GIFPlotter::_i_set_bg_color
#define _i_set_fill_color GIFPlotter::_i_set_fill_color
#define _i_set_pen_color GIFPlotter::_i_set_pen_color
#define _i_start_scan GIFPlotter::_i_start_scan
#define _i_write_gif_header GIFPlotter::_i_write_gif_header
#define _i_write_gif_image GIFPlotter::_i_write_gif_image
#define _i_write_gif_trailer GIFPlotter::_i_write_gif_trailer
#define _i_write_short_int GIFPlotter::_i_write_short_int
#endif /* LIBPLOTTER */

#ifndef X_DISPLAY_MISSING
#ifndef LIBPLOTTER
/* XDrawablePlotter/XPlotter protected methods, for libplot */
extern bool _x_begin_page P___((Plotter *_plotter));
extern bool _x_end_page P___((Plotter *_plotter));
extern bool _x_erase_page P___((Plotter *_plotter));
extern bool _x_flush_output P___((Plotter *_plotter));
extern bool _x_paint_paths P___((Plotter *_plotter));
extern bool _x_path_is_flushable P___((Plotter *_plotter));
extern bool _x_retrieve_font P___((Plotter *_plotter));
extern double _x_paint_text_string P___((Plotter *_plotter, const unsigned char *s, int h_just, int v_just));
extern double _x_get_text_width P___((Plotter *_plotter, const unsigned char *s));
extern void _x_initialize P___((Plotter *_plotter));
extern void _x_maybe_prepaint_segments P___((Plotter *_plotter, int prev_num_segments));
extern void _x_paint_path P___((Plotter *_plotter));
extern void _x_paint_point P___((Plotter *_plotter));
extern void _x_pop_state P___((Plotter *_plotter));
extern void _x_push_state P___((Plotter *_plotter));
extern void _x_terminate P___((Plotter *_plotter));
/* XDrawablePlotter/XPlotter internal functions, for libplot */
extern bool _x_retrieve_color P___((Plotter *_plotter, XColor *rgb_ptr));
extern bool _x_select_font P___((Plotter *_plotter, const char *name, bool is_zero[4], const unsigned char *s));
extern bool _x_select_font_carefully P___((Plotter *_plotter, const char *name, bool is_zero[4], const unsigned char *s));
extern bool _x_select_xlfd_font_carefully P___((Plotter *_plotter, const char *x_name, const char *x_name_alt, double user_size, double rotation));
extern void _x_add_gcs_to_first_drawing_state P___((Plotter *_plotter));
extern void _x_delete_gcs_from_first_drawing_state P___((Plotter *_plotter));
extern void _x_draw_elliptic_arc P___((Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc));
extern void _x_draw_elliptic_arc_2 P___((Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc));
extern void _x_draw_elliptic_arc_internal P___((Plotter *_plotter, int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange));
extern void _x_set_attributes P___((Plotter *_plotter, int x_gc_type));
extern void _x_set_bg_color P___((Plotter *_plotter));
extern void _x_set_fill_color P___((Plotter *_plotter));
extern void _x_set_font_dimensions P___((Plotter *_plotter, bool is_zero[4]));
extern void _x_set_pen_color P___((Plotter *_plotter));
/* XDrawablePlotter internal functions, for libplot */
extern void _x_maybe_get_new_colormap P___((Plotter *_plotter));
extern void _x_maybe_handle_x_events P___((Plotter *_plotter));
/* XPlotter protected methods, for libplot */
extern bool _y_begin_page P___((Plotter *_plotter));
extern bool _y_end_page P___((Plotter *_plotter));
extern bool _y_erase_page P___((Plotter *_plotter));
extern void _y_initialize P___((Plotter *_plotter));
extern void _y_terminate P___((Plotter *_plotter));
/* XPlotter internal functions, for libplot */
extern void _y_flush_plotter_outstreams P___((Plotter *_plotter));
extern void _y_maybe_get_new_colormap P___((Plotter *_plotter));
extern void _y_maybe_handle_x_events P___((Plotter *_plotter));
extern void _y_set_data_for_quitting P___((Plotter *_plotter));
#else  /* LIBPLOTTER */
/* XDrawablePlotter/XPlotter protected methods, for libplotter */
#define _x_begin_page XDrawablePlotter::begin_page
#define _x_end_page XDrawablePlotter::end_page
#define _x_erase_page XDrawablePlotter::erase_page
#define _x_paint_text_string XDrawablePlotter::paint_text_string
#define _x_get_text_width XDrawablePlotter::get_text_width
#define _x_flush_output XDrawablePlotter::flush_output
#define _x_path_is_flushable XDrawablePlotter::path_is_flushable
#define _x_maybe_prepaint_segments XDrawablePlotter::maybe_prepaint_segments
#define _x_paint_path XDrawablePlotter::paint_path
#define _x_paint_paths XDrawablePlotter::paint_paths
#define _x_paint_point XDrawablePlotter::paint_point
#define _x_pop_state XDrawablePlotter::pop_state
#define _x_push_state XDrawablePlotter::push_state
#define _x_retrieve_font XDrawablePlotter::retrieve_font
/* XDrawablePlotter protected methods (overridden in XPlotter class) */
#define _x_initialize XDrawablePlotter::initialize
#define _x_terminate XDrawablePlotter::terminate
/* XPlotter protected methods (which override the preceding) */
#define _y_begin_page XPlotter::begin_page
#define _y_end_page XPlotter::end_page
#define _y_erase_page XPlotter::erase_page
#define _y_initialize XPlotter::initialize
#define _y_terminate XPlotter::terminate
/* XDrawablePlotter/XPlotter internal functions, for libplotter */
#define _x_add_gcs_to_first_drawing_state XDrawablePlotter::_x_add_gcs_to_first_drawing_state
#define _x_delete_gcs_from_first_drawing_state XDrawablePlotter::_x_delete_gcs_from_first_drawing_state
#define _x_draw_elliptic_arc XDrawablePlotter::_x_draw_elliptic_arc
#define _x_draw_elliptic_arc_2 XDrawablePlotter::_x_draw_elliptic_arc_2
#define _x_draw_elliptic_arc_internal XDrawablePlotter::_x_draw_elliptic_arc_internal
#define _x_retrieve_color XDrawablePlotter::_x_retrieve_color
#define _x_select_font XDrawablePlotter::_x_select_font
#define _x_select_font_carefully XDrawablePlotter::_x_select_font_carefully
#define _x_select_xlfd_font_carefully XDrawablePlotter::_x_select_xlfd_font_carefully
#define _x_set_attributes XDrawablePlotter::_x_set_attributes
#define _x_set_bg_color XDrawablePlotter::_x_set_bg_color
#define _x_set_fill_color XDrawablePlotter::_x_set_fill_color
#define _x_set_font_dimensions XDrawablePlotter::_x_set_font_dimensions
#define _x_set_pen_color XDrawablePlotter::_x_set_pen_color
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

/* Declarations of the PlotterParams methods.  In libplot, these are
   declarations of global functions.  But in libplotter, we use #define and
   the double colon notation to make them function members of the
   PlotterParams class. */

#ifndef LIBPLOTTER
/* PlotterParams public methods, for libplot */
extern int _setplparam P___((PlotterParams *_plotter_params, const char *parameter, voidptr_t value));
extern int _pushplparams P___((PlotterParams *_plotter_params));
extern int _popplparams P___((PlotterParams *_plotter_params));
#else /* LIBPLOTTER */
/* PlotterParams public methods, for libplotter */
#define _setplparam PlotterParams::setplparam
#define _pushplparams PlotterParams::pushplparams
#define _popplparams PlotterParams::popplparams
#endif /* LIBPLOTTER */

#undef P___
