/* This is "plotter.h", the public header file for the C++ Plotter class
   provided by GNU libplotter, a shared class library for 2-dimensional
   vector graphics.  

   This file should be included by any code that uses the Plotter class.
   If the Plotter class was installed without X Window System support, be
   sure to do "#define X_DISPLAY_MISSING" before including this file.

   From the base Plotter class, the MetaPlotter, TekPlotter, HPGLPlotter,
   FigPlotter, PSPlotter, AIPlotter, GIFPlotter, PNMPlotter, and
   XDrawablePlotter classes are derived.  The PCLPlotter class is derived
   from the HPGLPlotter class, and XPlotter class is derived from the
   XDrawablePlotter class. */

/* If NOT_LIBPLOTTER is defined, this file magically becomes an internal
   header file used in GNU libplot, the C version of libplotter.  libplot
   has its own public header file, called "plot.h". */

/* Implementation note: In libplot, a Plotter is a typedef'd structure
   rather than a class instance.  Because of the need to support both
   libplot and libplotter, i.e. C and C++, all data members of derived
   Plotter classes are declared in this file twice: once in the Plotter
   structure (for C), and once in each derived class declaration (for C++). */

#ifndef _PLOTTER_H_
#define _PLOTTER_H_ 1

/***********************************************************************/

/* This version of plotter.h accompanies GNU libplot/libplotter version 2.0. */
#define LIBPLOT_VERSION "2.0"

/* Support ancient C compilers, best not explained further. */
#ifndef void
#define Voidptr void *
#endif

/* Include X header files needed by the class definition. */
#ifndef X_DISPLAY_MISSING
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#endif /* not X_DISPLAY_MISSING */

/* Include iostream, stdio support if this is libplotter rather than
   libplot. */
#ifndef NOT_LIBPLOTTER
#include <iostream.h>
#include <stdio.h>
#endif

/* THE GLOBAL VARIABLES IN GNU LIBPLOTTER */
/* There are two; both are user-settable error handlers. */
#ifndef NOT_LIBPLOTTER
extern int (*libplotter_warning_handler) (const char *msg);
extern int (*libplotter_error_handler) (const char *msg);
#endif


/***********************************************************************/

/* Structures for points and paths, which appear as data members of the
   Plotter class.  E.g., A `path' is a continuous sequence of line
   segments, circular arc segments, elliptic arc segments, quadratic Bezier
   segments, or cubic Bezier segments.  It is represented internally as a
   list of GeneralizedPoints.  Each GeneralizedPoint has a type field (the
   first GeneralizedPoint in the path is just a point; its type field is
   ignored). */

typedef struct
{
  double x, y;
} Point;

typedef struct
{
  int x, y;
} IntPoint;

typedef struct
{
  double x, y;			/* endpoint of segment */
  double xc, yc;		/* intermediate control point (if any) */
  double xd, yd;		/* additional control point (S_CUBIC only) */
  int type;			/* S_LINE, S_ARC, S_ELLARC, S_QUAD, S_CUBIC */
} GeneralizedPoint;


/**********************************************************************/

/* Structures for colors */

/* Used for either 24-bit or 48-bit RGB */
typedef struct
{
  int red;
  int green;
  int blue;
} Color;

/* Any bitmap-oriented Plotter writes into a pixmap that is made up of the
   following type of pixel.  We use a union, so the MI scan-conversion
   module, which does the writing, can be used both by GIF Plotters (in
   which pixel values are color indices) and by PNM Plotters (for which
   pixel values are 24-bit RGB values). */
typedef union
{
  unsigned char index;		/* for GIF Plotters */
  unsigned char rgb[3];		/* for PNM Plotters */
} miPixel;


/**********************************************************************/

/* Structures specifying the device frame and the user->device coordinate
   transformation.  These are members of the drawing state structure.  */

typedef struct
{
  double left, right, bottom, top; /* device-frame coordinates, in inches */
  double extra;			/* plot length (hpgl) or paper height (PS) */
} Displaycoors;

typedef struct 
{ 
  /* the two components of an affine transformation */
  double m[6];			/* 1. a linear transformation (4 elements) */
  				/* 2. a translation (2 elements) */
  /* related data kept here for convenience */
  bool uniform;			/* transf. scaling is uniform? */
  bool axes_preserved;		/* transf. preserves axis directions? */
  bool nonreflection;		/* transf. doesn't involve a reflection? */
} Transform;


/**********************************************************************/

/* Drawing state structure.  Includes all drawing attributes and the state
   of any uncompleted path object.  When open, i.e., when drawing a page of
   graphics, any Plotter maintains a stack of these things.  Many of the
   data members are specific to individual derived Plotter classes, but
   it's more efficient to keep them in a single structure.  The
   device-independent data members are listed first. */

typedef struct lib_state
{
/***************** DEVICE-INDEPENDENT PART ***************************/

/* affine transformation from user coordinates to device coordinates */
  Transform transform;		/* see definition of structure above */

/* graphics cursor position */
  Point pos;			/* graphics cursor position */

/* the state of any uncompleted path object */
  GeneralizedPoint *datapoints; /* list of points defining the path */
  int points_in_path;		/* number of points accumulated */
  int datapoints_len;		/* length of point storage buffer (bytes) */
  bool suppress_polyline_flushout; /* suppress endpath while drawing a path? */
  bool convex_path;		/* path to be drawn is convex? (endpath hint)*/

/* modal drawing attributes */
  /* path-related attributes */
  char *fill_rule;		/* fill rule */
  int fill_rule_type;		/* one of FILL_*, determined by fill rule */
  char *line_mode;		/* line mode */
  int line_type;		/* one of L_*, determined by line mode */
  bool points_are_connected;	/* if not set, polyline displayed as points */
  char *cap_mode;		/* cap mode */
  int cap_type;			/* one of CAP_*, determined by cap mode */
  char *join_mode;		/* join mode */
  int join_type;		/* one of JOIN_*, determined by join mode */
  double miter_limit;		/* miter limit for line joins */
  double line_width;		/* width of lines in user coordinates */
  double device_line_width;	/* line width in device coordinates */
  int quantized_device_line_width; /* line width, quantized to integer */
  double *dash_array;		/* array of dash on/off lengths  */
  int dash_array_len;		/* length of same */
  double dash_offset;		/* offset distance into dash array (`phase') */
  bool dash_array_in_effect;	/* dash array should override line mode? */
  /* text-related attributes */
  char *font_name;		/* font name */
  double font_size;		/* font size in user coordinates */
  double text_rotation;		/* degrees counterclockwise, for labels */
  double true_font_size;	/* true font size (as retrieved) */
  double font_ascent;		/* font ascent (as retrieved) */
  double font_descent;		/* font descent (as retrieved) */
  int font_type;		/* F_{HERSHEY|POSTSCRIPT|PCL|STICK|OTHER} */
  int typeface_index;		/* typeface index (in g_fontdb.h table) */
  int font_index;		/* font index, within typeface */
  bool font_is_iso8859_1;	/* whether font uses iso8859_1 encoding */
  /* color attributes (which affect both paths and text) */
  Color fgcolor;		/* foreground color */
  Color fillcolor;		/* fill color */
  Color bgcolor;		/* background color */
  int fill_level;		/* fill level */

/* default values for certain attributes, used when an out-of-range value
   is requested (these two are special because they're set by space())*/
  double default_line_width;	/* width of lines in user coordinates */
  double default_font_size;	/* font size in user coordinates */

/****************** DEVICE-DEPENDENT PART ***************************/

/* elements specific to the HPGL Plotter drawing state */
  double hpgl_pen_width;	/* pen width (frac of diag dist betw P1,P2) */

/* elements specific to the Fig Plotter drawing state */
  int fig_font_point_size;	/* font size in fig's idea of points */
  int fig_fill_level;		/* fig's fill level */
  int fig_fgcolor;		/* fig's foreground color */
  int fig_fillcolor;		/* fig's fill color */

/* elements specific to the PS Plotter drawing state */
  double ps_fgcolor_red;	/* RGB for fgcolor, each in [0.0,1.0] */
  double ps_fgcolor_green;
  double ps_fgcolor_blue;
  double ps_fillcolor_red;	/* RGB for fillcolor, each in [0.0,1.0] */
  double ps_fillcolor_green;
  double ps_fillcolor_blue;
  int ps_idraw_fgcolor;		/* index of idraw fgcolor in table */
  int ps_idraw_bgcolor;		/* index of idraw bgcolor in table */
  int ps_idraw_shading;		/* index of idraw shading in table */

/* elements specific to the GIF Plotter drawing state */
  Color i_pen_color;		/* pen color (24-bit RGB) */
  Color i_fill_color;		/* fill color (24-bit RGB) */
  Color i_bg_color;		/* background color (24-bit RGB) */
  unsigned char i_pen_color_index; /* pen color index */
  unsigned char i_fill_color_index; /* fill color index */
  unsigned char i_bg_color_index; /* bg color index */
  bool i_pen_color_status;	/* foreground color index is genuine? */
  bool i_fill_color_status;	/* fill color index is genuine? */
  bool i_bg_color_status;	/* background color index is genuine? */

#ifndef X_DISPLAY_MISSING
/* elements specific to the X Drawable Plotter and X Plotter drawing states */
  double x_font_pixmatrix[4];	/* pixel matrix, parsed from font name */
  bool x_native_positioning;	/* if set, can use XDrawString() etc. */
  XFontStruct *x_font_struct;	/* font structure (used in x_alab_x.c) */
  const unsigned char *x_label;	/* label (hint to font retrieval routine) */
  GC x_gc_fg;			/* graphics context, for drawing */
  GC x_gc_fill;			/* graphics context, for filling */
  GC x_gc_bg;			/* graphics context, for erasing */
  Color x_current_fgcolor;	/* foreground color (48-bit RGB) */
  Color x_current_fillcolor;	/* fill color (48-bit RGB) */
  Color x_current_bgcolor;	/* background color (48-bit RGB) */
  int x_current_fill_level;	/* fill level used in computing pixel val */
  unsigned long x_fgcolor;	/* foreground color (pixel value) */
  unsigned long x_fillcolor;	/* fill color (pixel value) */
  unsigned long x_bgcolor;	/* background color (pixel value) */
  bool x_fgcolor_status;	/* foreground pixel is genuine? */
  bool x_fillcolor_status;	/* fill pixel is genuine? */
  bool x_bgcolor_status;	/* background pixel is genuine? */
#endif /* not X_DISPLAY_MISSING */

/* pointer to previous drawing state */
  struct lib_state *previous;

} pl_DrawState;


/**********************************************************************/

/* An output buffer that may easily be resized.  Used by Plotters that do
   not do real-time output, to store device code for all graphical objects
   plotted on a page, and bounding box and per-page font information too.
   (See e.g. g_outbuf.c.)  Plotters that wait until they are deleted before
   outputing graphics, e.g. PSPlotters, maintain not just one of these
   things but rather a linked list, one output buffer per page. */

/* NUM_PS_FONTS and NUM_PCL_FONTS should agree with the number of fonts of
   each type in g_fontdb.c.  These are also defined in libplot/extern.h. */
#define NUM_PS_FONTS 35
#define NUM_PCL_FONTS 45

typedef struct lib_outbuf
{
  char *base;			/* start of buffer */
  unsigned long len;		/* size of buffer */
  char *point;			/* current point (high-water mark) */
  char *reset_point;		/* point below which contents are frozen */
  unsigned long contents;	/* size of contents */
  unsigned long reset_contents;	/* size of frozen contents if any */
  double xrange_min;		/* bounding box, in device coordinates */
  double xrange_max;
  double yrange_min;
  double yrange_max;
  bool ps_font_used[NUM_PS_FONTS]; /* PS fonts used on page */
  bool pcl_font_used[NUM_PCL_FONTS]; /* PCL fonts used on page */
  struct lib_outbuf *next;	/* pointer to previous Outbuf in list if any */
} Outbuf;

#ifndef X_DISPLAY_MISSING
/* We keep track of which fonts we request from an X server, in any
   connection, by constructing a linked list of these records.  A linked
   list is good enough if we don't have huge numbers of font changes. */
typedef struct lib_fontrecord
{
  char *name;			/* font name, e.g. XLFD name */
  XFontStruct *x_font_struct;	/* font structure */
  double true_font_size;
  double font_pixmatrix[4];
  double font_ascent;
  double font_descent;
  bool native_positioning;
  bool font_is_iso8859_1;
  bool subset;			/* did we retrieve a subset of the font? */
  unsigned char subset_vector[32]; /* 256-bit vector, 1 bit per font char */
  struct lib_fontrecord *next;	/* most recently retrieved font */
} Fontrecord;

/* We keep track of allocated color cells similarly */
typedef struct lib_colorrecord
{
  XColor rgb;			/* RGB value and pixel value (if any) */
  bool allocated;		/* pixel value successfully allocated? */
  int frame;			/* frame that cell was most recently used in*/
  struct lib_colorrecord *next;	/* most recently retrieved color cell */
} Colorrecord;
#endif /* not X_DISPLAY_MISSING */


/***********************************************************************/

/* The Plotter class, and also its derived classes (in libplotter).  In
   libplot, a Plotter is a struct, and there are no derived classes; the
   data members of the derived classes are located inside the Plotter
   structure. */

/* Supported Plotter types.  These values are used in a `tag field', in
   libplot but not libplotter.  (C++ doesn't have such things, at least it
   didn't until RTTI was invented :-)). */
#ifdef NOT_LIBPLOTTER
typedef enum 
{
  PL_GENERIC,			/* instance of base Plotter class */
  PL_META,			/* GNU graphics metafile */
  PL_TEK,			/* Tektronix 4014 with EGM */
  PL_HPGL,			/* HP-GL and HP-GL/2 */
  PL_PCL,			/* PCL 5 (i.e. HP-GL/2 w/ header, trailer) */
  PL_FIG,			/* xfig 3.2 */
  PL_PS,			/* Postscript, with idraw support */
  PL_AI,			/* Adobe Illustrator 5 (or 3) */
  PL_GIF,			/* GIF 87a or 89a */
  PL_PNM			/* Portable Anymap Format (PBM/PGM/PPM) */
#ifndef X_DISPLAY_MISSING
  , PL_X11_DRAWABLE		/* X11 Drawable */
  , PL_X11			/* X11 (pops up, manages own window[s]) */
#endif /* not X_DISPLAY_MISSING */
} plotter_type;
#endif /* NOT_LIBPLOTTER */

/* Miscellaneous constants (may also be defined in libplot/extern.h) */

/* Number of recognized device driver parameters (see g_params.h). */
#define NUM_PLOTTER_PARAMETERS 32

/* Maximum number of pens, or logical pens, for an HP-GL/2 device.  Some
   such devices permit as many as 256, but all should permit at least 32.
   Our pen numbering will range over 0..HPGL2_MAX_NUM_PENS-1. */
#define HPGL2_MAX_NUM_PENS 32

/* Maximum number of non-builtin colors that can be specified in an
   xfig input file. */
#define FIG_MAX_NUM_USER_COLORS 512

/* The macro P___ elides argument prototypes if the compiler does not
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

/* The macro Q___ is used for declaring Plotter methods (as function
   pointers for libplot, and as virtual function members of the Plotter
   class, for libplotter). */
#ifdef Q___
#undef Q___
#endif
#ifdef NOT_LIBPLOTTER
#define Q___(rettype,f) rettype (*f)
#else
#define Q___(rettype,f) virtual rettype f
#endif

/* The Plotter class (or struct) definition.  There are many members! */

#ifndef NOT_LIBPLOTTER
class Plotter
#else
typedef struct
#endif
{
#ifndef NOT_LIBPLOTTER
 private:
  /* disallow copying and assignment */
  Plotter (const Plotter& oldplotter);  
  Plotter& operator= (const Plotter& oldplotter);

 public:
  /* PLOTTER CTORS AND DTOR */
  Plotter (FILE *infile, FILE *outfile, FILE *errfile);
  Plotter (FILE *outfile);
  Plotter (istream& in, ostream& out, ostream& err);
  Plotter (ostream& out);
  Plotter ();
  virtual ~Plotter ();

  /* PLOTTER PUBLIC METHOD (static) */
  static int parampl (const char *parameter, void *value);
#endif

  /* PLOTTER PUBLIC METHODS (all virtual) */
  Q___(int,alabel) P___((int x_justify, int y_justify, const char *s));
  Q___(int,arc) P___((int xc, int yc, int x0, int y0, int x1, int y1));
  Q___(int,arcrel) P___((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
  Q___(int,bezier2) P___((int x0, int y0, int x1, int y1, int x2, int y2));
  Q___(int,bezier2rel) P___((int dx0, int dy0, int dx1, int dy1, int dx2, int dy2));
  Q___(int,bezier3) P___((int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3));
  Q___(int,bezier3rel) P___((int dx0, int dy0, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3));
  Q___(int,bgcolor) P___((int red, int green, int blue));
  Q___(int,bgcolorname) P___((const char *name));
  Q___(int,box) P___((int x0, int y0, int x1, int y1));
  Q___(int,boxrel) P___((int dx0, int dy0, int dx1, int dy1));
  Q___(int,capmod) P___((const char *s));
  Q___(int,circle) P___((int x, int y, int r));
  Q___(int,circlerel) P___((int dx, int dy, int r));
  Q___(int,closepl) P___((void));
  Q___(int,color) P___((int red, int green, int blue));
  Q___(int,colorname) P___((const char *name));
  Q___(int,cont) P___((int x, int y));
  Q___(int,contrel) P___((int dx, int dy));
  Q___(int,ellarc) P___((int xc, int yc, int x0, int y0, int x1, int y1));
  Q___(int,ellarcrel) P___((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
  Q___(int,ellipse) P___((int x, int y, int rx, int ry, int angle));
  Q___(int,ellipserel) P___((int dx, int dy, int rx, int ry, int angle));
  Q___(int,endpath) P___((void));
  Q___(int,erase) P___((void));
  Q___(int,farc) P___((double xc, double yc, double x0, double y0, double x1, double y1));
  Q___(int,farcrel) P___((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
  Q___(int,fbezier2) P___((double x0, double y0, double x1, double y1, double x2, double y2));
  Q___(int,fbezier2rel) P___((double dx0, double dy0, double dx1, double dy1, double dx2, double dy2));
  Q___(int,fbezier3) P___((double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3));
  Q___(int,fbezier3rel) P___((double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3));
  Q___(int,fbox) P___((double x0, double y0, double x1, double y1));
  Q___(int,fboxrel) P___((double dx0, double dy0, double dx1, double dy1));
  Q___(int,fcircle) P___((double x, double y, double r));
  Q___(int,fcirclerel) P___((double dx, double dy, double r));
  Q___(int,fconcat) P___((double m0, double m1, double m2, double m3, double m4, double m5));
  Q___(int,fcont) P___((double x, double y));
  Q___(int,fcontrel) P___((double dx, double dy));
  Q___(int,fellarc) P___((double xc, double yc, double x0, double y0, double x1, double y1));
  Q___(int,fellarcrel) P___((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
  Q___(int,fellipse) P___((double x, double y, double rx, double ry, double angle));
  Q___(int,fellipserel) P___((double dx, double dy, double rx, double ry, double angle));
  Q___(double,ffontname) P___((const char *s));
  Q___(double,ffontsize) P___((double size));
  Q___(int,fillcolor) P___((int red, int green, int blue));
  Q___(int,fillcolorname) P___((const char *name));
  Q___(int,fillmod) P___((const char *s));
  Q___(int,filltype) P___((int level));
  Q___(double,flabelwidth) P___((const char *s));
  Q___(int,fline) P___((double x0, double y0, double x1, double y1));
  Q___(int,flinedash) P___((int n, const double *dashes, double offset));
  Q___(int,flinerel) P___((double dx0, double dy0, double dx1, double dy1));
  Q___(int,flinewidth) P___((double size));
  Q___(int,flushpl) P___((void));
  Q___(int,fmarker) P___((double x, double y, int type, double size));
  Q___(int,fmarkerrel) P___((double dx, double dy, int type, double size));
  Q___(int,fmiterlimit) P___((double limit));
  Q___(int,fmove) P___((double x, double y));
  Q___(int,fmoverel) P___((double dx, double dy));
  Q___(int,fontname) P___((const char *s));
  Q___(int,fontsize) P___((int size));
  Q___(int,fpoint) P___((double x, double y));
  Q___(int,fpointrel) P___((double dx, double dy));
  Q___(int,frotate) P___((double theta));
  Q___(int,fscale) P___((double x, double y));
  Q___(int,fspace) P___((double x0, double y0, double x1, double y1));
  Q___(int,fspace2) P___((double x0, double y0, double x1, double y1, double x2, double y2));
  Q___(double,ftextangle) P___((double angle));
  Q___(int,ftranslate) P___((double x, double y));
  Q___(int,havecap) P___((const char *s));
  Q___(int,joinmod) P___((const char *s));
  Q___(int,label) P___((const char *s));
  Q___(int,labelwidth) P___((const char *s));
  Q___(int,line) P___((int x0, int y0, int x1, int y1));
  Q___(int,linedash) P___((int n, const int *dashes, int offset));
  Q___(int,linemod) P___((const char *s));
  Q___(int,linerel) P___((int dx0, int dy0, int dx1, int dy1));
  Q___(int,linewidth) P___((int size));
  Q___(int,marker) P___((int x, int y, int type, int size));
  Q___(int,markerrel) P___((int dx, int dy, int type, int size));
  Q___(int,move) P___((int x, int y));
  Q___(int,moverel) P___((int dx, int dy));
  Q___(int,openpl) P___((void));
  Q___(FILE*,outfile) P___((FILE* newstream)); /* OBSOLESCENT */
  Q___(int,pencolor) P___((int red, int green, int blue));
  Q___(int,pencolorname) P___((const char *name));
  Q___(int,point) P___((int x, int y));
  Q___(int,pointrel) P___((int dx, int dy));
  Q___(int,restorestate) P___((void));
  Q___(int,savestate) P___((void));
  Q___(int,space) P___((int x0, int y0, int x1, int y1));
  Q___(int,space2) P___((int x0, int y0, int x1, int y1, int x2, int y2));
  Q___(int,textangle) P___((int angle));

#ifndef NOT_LIBPLOTTER
 protected:
#endif

  /* PLOTTER PROTECTED METHODS (all virtual) */

  /* Initialization (after creation) and termination (before deletion). */
  Q___(void,initialize) P___((void));
  Q___(void,terminate) P___((void));

  /* Versions of the `falabel' and `flabelwidth' methods, including one for
     each of the three sorts of non-Hershey font.  The argument h_just
     specifies the justification to be used when rendering (JUST_LEFT,
     JUST_RIGHT, or JUST_CENTER).  If a display device provides low-level
     support for non-default (i.e. non-LEFT) justification, the
     have_justification flag (see below) should be set. */
  Q___(double,falabel_hershey) P___((const unsigned char *s, int x_justify, int y_justify));
  Q___(double,falabel_ps) P___((const unsigned char *s, int h_just));
  Q___(double,falabel_pcl) P___((const unsigned char *s, int h_just));
  Q___(double,falabel_stick) P___((const unsigned char *s, int h_just));
  Q___(double,falabel_other) P___((const unsigned char *s, int h_just));
  Q___(double,flabelwidth_hershey) P___((const unsigned char *s));
  Q___(double,flabelwidth_ps) P___((const unsigned char *s));
  Q___(double,flabelwidth_pcl) P___((const unsigned char *s));
  Q___(double,flabelwidth_stick) P___((const unsigned char *s));
  Q___(double,flabelwidth_other) P___((const unsigned char *s));

  /* low-level `retrieve font' method */
  Q___(void,retrieve_font) P___((void));

  /* Low-level methods which, in a generic Plotter, do nothing and are
     never invoked.  Any derived Plotter class that wishes to use them for
     something (they have no specified semantics) should override the null
     versions, i.e. provide its own definitions. */
  Q___(void,set_font) P___((void));
  Q___(void,set_attributes) P___((void));
  Q___(void,set_pen_color) P___((void));
  Q___(void,set_fill_color) P___((void));  
  Q___(void,set_bg_color) P___((void));  
  Q___(void,set_position) P___((void));

  /* error handlers */
  Q___(void,warning) P___((const char *msg));
  Q___(void,error) P___((const char *msg));

  /* low-level output routines */
  Q___(void,write_byte) P___((unsigned char c));
  Q___(void,write_bytes) P___((int n, const unsigned char *c));
  Q___(void,write_string) P___((const char *s));

  /* PLOTTER INTERNAL FUNCTIONS */

  /* internal function overridden in the MetaPlotter class (a crock) */
  Q___(void,_recompute_device_line_width) P___((void));

#ifndef NOT_LIBPLOTTER
  /* other internal functions of the Plotter class (a mixed bag) */
  Voidptr _get_plot_param P___((const char *parameter)); 
  bool _match_pcl_font P___((void));
  bool _match_ps_font P___((void));
  double _label_width_hershey P___((const unsigned short *label));
  double _render_non_hershey_string P___((const char *s, bool do_render, int x_justify, int y_justify));
  double _render_simple_non_hershey_string P___((const unsigned char *s, bool do_render, int h_just));
  unsigned short * _controlify P___((const unsigned char *));
  void _copy_params_to_plotter P___((void));
  void _draw_bezier2 P___((Point p0, Point p1, Point p2));
  void _draw_bezier3 P___((Point p0, Point p1, Point p2, Point p3));
  void _draw_circular_arc P___((Point p0, Point p1, Point pc));
  void _draw_elliptic_arc P___((Point p0, Point p1, Point pc));
  void _draw_hershey_glyph P___((int num, double charsize, int type, bool oblique));
  void _draw_hershey_penup_stroke P___((double dx, double dy, double charsize, bool oblique));
  void _draw_hershey_string P___((const unsigned short *string));
  void _draw_hershey_stroke P___((bool pendown, double deltax, double deltay));
  void _draw_stroke P___((bool pendown, double deltax, double deltay));
  void _fakearc P___((Point p0, Point p1, int arc_type, const double m[4]));
  void _fakebezier2 P___((Point p0, Point p1, Point p2));
  void _fakebezier3 P___((Point p0, Point p1, Point p2, Point p3));
  void _flush_plotter_outstreams P___((void));
  void _free_params_in_plotter P___((void));
  void _maybe_replace_arc P___((void));
  void _set_bezier2_bbox P___((Outbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2));
  void _set_bezier3_bbox P___((Outbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3));
  void _set_common_mi_attributes P___((Voidptr ptr));
  void _set_ellipse_bbox P___((Outbuf *bufp, double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth));
  void _set_line_end_bbox P___((Outbuf *bufp, double x, double y, double xother, double yother, double linewidth, int capstyle));
  void _set_line_join_bbox P___((Outbuf *bufp, double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle, double miterlimit));
#endif /* NOT_LIBPLOTTER */

  /* PLOTTER DATA MEMBERS (not specific to any one device driver) */
  /* Some of these are constant over the usable lifetime of the Plotter,
     and are set at creation time, or at latest, when openpl() is first
     invoked.  They are just parameters.  Other data members may change.
     Each of the latter is flagged by "D:" (i.e. "dynamic") in its comment
     line. */

#ifdef NOT_LIBPLOTTER
  /* tag field */
  plotter_type type;		/* Plotter type: one of PL_* defined above */
#endif /* NOT_LIBPLOTTER */

  /* low-level I/O */
  FILE *infp;			/* stdio-style input stream if any */
  FILE *outfp;			/* stdio-style output stream if any */
  FILE *errfp;			/* stdio-style error stream if any */
#ifndef NOT_LIBPLOTTER
  istream *instream;		/* C++-style input stream if any */
  ostream *outstream;		/* C++-style output stream if any */
  ostream *errstream;		/* C++-style error stream if any */
#endif /* not NOT_LIBPLOTTER */

  /* device driver parameters (i.e., instance copies of class variables) */
  Voidptr params[NUM_PLOTTER_PARAMETERS];
  /* output buffers */
  Outbuf *page;			/* D: output buffer for current page */
  Outbuf *first_page;		/* D: first page (if a linked list is kept) */
  /* basic data members */
  bool open;			/* D: whether or not Plotter is open */
  bool opened;			/* D: whether or not Plotter has been opened */
  int page_number;		/* D: number of times it has been opened */
  bool space_invoked;		/* D: has space() been invoked on this page? */
  int frame_number;		/* D: number of frame in page */
  /* drawing state stack */
  pl_DrawState *drawstate;	/* D: pointer to top of drawing state stack */
  /* whether warning messages have been issued */
  bool font_warning_issued;	/* D: issued warning on font substitution */
  bool pen_color_warning_issued; /* D: issued warning on name substitution */
  bool fill_color_warning_issued; /* D: issued warning on name substitution */
  bool bg_color_warning_issued;	/* D: issued warning on name substitution */
  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  int have_wide_lines;	
  int have_dash_array;
  int have_solid_fill;
  int have_odd_winding_fill;
  int have_nonzero_winding_fill;
  int have_settable_bg;
  int have_hershey_fonts;
  int have_ps_fonts;
  int have_pcl_fonts;
  int have_stick_fonts;
  int have_extra_stick_fonts;
  /* text and font-related parameters (internal, not queryable by user) */
  int default_font_type;	/* F_{HERSHEY|POSTSCRIPT|PCL|STICK} */
  bool pcl_before_ps;		/* PCL fonts searched first? (if applicable) */
  bool have_justification;	/* display device can justify text? */
  bool kern_stick_fonts;	/* use pre-HP-GL/2 kerning for Stick fonts? */
  bool issue_font_warning;	/* issue warning on font substitution? */
  /* path and polyline-related parameters (also internal) */
  int max_unfilled_polyline_length; /* user-settable, for unfilled polylines */
  bool have_mixed_paths;	/* can mix arcs/Beziers and lines in paths? */
  int allowed_arc_scaling;	/* scaling allowed for circular arcs */
  int allowed_ellarc_scaling;	/* scaling allowed for elliptic arcs */
  int allowed_quad_scaling;	/* scaling allowed for quadratic Beziers */
  int allowed_cubic_scaling;	/* scaling allowed for cubic Beziers */
  bool flush_long_polylines;	/* invoke endpath() on too-long polylines? */
  long int hard_polyline_length_limit; /* a hard limit for all polylines */
  /* the device coordinate frame (dimensions etc.) */
  int display_type;		/* one of DISP_{UNKNOWN,INTEGER,PHYSICAL} */
  bool integer_device_coors;	/* display device uses integer coordinates? */
  int imin, imax, jmin, jmax;	/* coor ranges, for an `integer' display */
  Displaycoors display_coors;	/* ranges in inches, for a physical display */
  const char *page_type;	/* page type, for a physical display */
  double device_units_per_inch;	/* units/inch for a physical display */
  bool use_metric;		/* metric units? (for a physical display) */
  bool flipped_y;		/* y increases downward? */

#ifdef NOT_LIBPLOTTER
  /* PLOTTER DATA MEMBERS (device driver-specific). */
  /* In libplot, they appear here, i.e. in the Plotter struct.  But in
     libplotter, they don't appear here, i.e. they don't appear in the base
     Plotter class: they appear instead in the appropriate derived classes
     (for the definitions of which, see further below in this file). */

  /* Some of these are constant over the usable lifetime of the Plotter,
     and are set, at latest, in the first call to openpl().  They are just
     parameters.  Other data members may change.  Each of the latter is
     flagged by "D:" (i.e. "dynamic") in its comment line. */

  /* elements specific to Metafile Plotters */
  bool meta_portable_output;	/* portable, not binary output format? */
  /* elements specific to Tektronix Plotters */
  int tek_display_type;		/* which sort of Tektronix? (one of D_*) */
  int tek_mode;			/* D: one of MODE_* */
  int tek_line_type;		/* D: one of L_* */
  bool tek_mode_is_unknown;	/* D: tek mode unknown? */
  bool tek_line_type_is_unknown; /* D: tek line type unknown? */
  int tek_kermit_fgcolor;	/* D: kermit's foreground color */
  int tek_kermit_bgcolor;	/* D: kermit's background color */
  bool tek_position_is_unknown;	/* D: cursor position is unknown? */
  IntPoint tek_pos;		/* D: Tektronix cursor position */
  /* elements specific to HP-GL (and PCL) Plotters */
  int hpgl_version;		/* version: 0=HP-GL, 1=HP7550A, 2=HP-GL/2 */
  int rotation;			/* rotation angle (0, 90, 180, or 270) */
  Point p1;			/* scaling point P1 in native HP-GL coors */
  Point p2;			/* scaling point P2 in native HP-GL coors */
  double plot_length;		/* plot length (for HP-GL/2 roll plotters) */
  bool palette;			/* can assign pen colors? (HP-GL/2 only) */
  bool opaque_mode;		/* pen marks sh'd be opaque? (HP-GL/2 only) */
  Color pen_color[HPGL2_MAX_NUM_PENS]; /* D: color array for (logical) pens */
  int pen_defined[HPGL2_MAX_NUM_PENS]; /* D: 0=none, 1=soft-defd, 2=hard-defd*/
  int pen;			/* D: number of currently selected pen */
  int free_pen;			/* D: pen to be assigned a color next */
  bool bad_pen;			/* D: bad pen (advisory, see h_color.c) */
  bool pendown;			/* D: pen down rather than up? */
  double pen_width;		/* D: pen width(frac of diag dist betw P1,P2)*/
  int hpgl_line_type;		/* D: line type(HP-GL numbering,solid = -100)*/
  int hpgl_cap_style;		/* D: cap style for lines (HP-GL/2 numbering)*/
  int hpgl_join_style;		/* D: join style for lines(HP-GL/2 numbering)*/
  double hpgl_miter_limit;	/* D: miterlimit for line joins(HP-GL/2 only)*/
  int fill_type;		/* D: fill type (one of FILL_SOLID_UNI etc.) */
  double shading_level;		/* D: percent; used if fill_type=FILL_SHADING*/
  int pcl_symbol_set;		/* D: encoding, 14=ISO-Latin-1 (HP-GL/2 only)*/
  int pcl_spacing;		/* D: fontspacing,0=fixed,1=not(HP-GL/2 only)*/
  int pcl_posture;		/* D: posture,0=uprite,1=italic(HP-GL/2 only)*/
  int pcl_stroke_weight;	/* D: weight,0=normal,3=bold,..(HP-GL/2only)*/
  int pcl_typeface;		/* D: typeface, see g_fontdb.c(HP-GL/2 only) */
  int hpgl_charset_lower;	/* D: HP lower-half charset no. (pre-HP-GL/2)*/
  int hpgl_charset_upper;	/* D: HP upper-half charset no. (pre-HP-GL/2)*/
  double relative_char_height;	/* D: char ht., % of p2y-p1y (HP-GL/2 only) */
  double relative_char_width;	/* D: char width, % of p2x-p1x (HP-GL/2 only)*/
  double relative_label_rise;	/* D: label rise, % of p2y-p1y (HP-GL/2 only)*/
  double relative_label_run;	/* D: label run, % of p2x-p1x (HP-GL/2 only) */
  double char_slant_tangent;	/* D: tan of character slant (HP-GL/2 only)*/
  bool hpgl_position_is_unknown; /* D: HP-GL[/2] cursor position is unknown? */
  IntPoint hpgl_pos;		/* D: cursor position (integer HP-GL coors) */
/* elements specific to Fig Plotters */
  int fig_drawing_depth;	/* D: fig's curr value for `depth' attribute */
  int fig_num_usercolors;	/* D: number of colors currently defined */
  long int fig_usercolors[FIG_MAX_NUM_USER_COLORS]; /* D: colors we've def'd */
/* elements specific to Illustrator Plotters */
  int ai_version;		/* AI version 3 or AI version 5? */
  double ai_pen_cyan;		/* D: pen color (in CMYK space) */
  double ai_pen_magenta;
  double ai_pen_yellow;
  double ai_pen_black;
  double ai_fill_cyan;		/* D: fill color (in CMYK space) */
  double ai_fill_magenta;
  double ai_fill_yellow;
  double ai_fill_black;
  bool ai_cyan_used;		/* D: C, M, Y, K have been used? */
  bool ai_magenta_used;
  bool ai_yellow_used;
  bool ai_black_used;
  int ai_cap_style;		/* D: cap style for lines (PS numbering) */
  int ai_join_style;		/* D: join style for lines (PS numbering) */
  double ai_miter_limit;	/* D: miterlimit for line joins */
  int ai_line_type;		/* D: one of L_* */
  double ai_line_width;		/* D: line width in printer's points */
  int ai_fill_rule_type;	/* D: fill rule (FILL_{ODD|NONZERO}_WINDING) */
/* elements specific to PNM Plotters */
  bool n_portable_output;	/* portable, not binary output format? */
  int n_xn, n_yn;		/* bitmap dimensions */
  miPixel **n_bitmap;		/* D: bitmap */
/* elements specific to GIF Plotters */
  int i_xn, i_yn;		/* bitmap dimensions */
  int i_num_pixels;		/* total pixels (used by scanner) */
  bool i_animation;		/* animated (multi-image) GIF? */
  int i_iterations;		/* number of times GIF should be looped */
  int i_delay;			/* delay after image, in 1/100 sec units */
  bool i_interlace;		/* interlaced GIF? */
  bool i_transparent;		/* transparent GIF? */
  Color i_transparent_color;	/* if so, transparent color (24-bit RGB) */
  int i_transparent_index;	/* D: if so, transparent color index */
  miPixel **i_bitmap;		/* D: bitmap */
  Color i_colormap[256];	/* D: frame colormap (containing 24-bit RGBs)*/
  int i_num_color_indices;	/* D: number of color indices allocated */
  bool i_frame_nonempty;	/* D: something drawn in current frame? */
  int i_bit_depth;		/* D: bit depth (ceil(log2(num_indices))) */
  int i_pixels_scanned;		/* D: number that scanner has scanned */
  int i_pass;			/* D: scanner pass (used if interlacing) */
  IntPoint i_hot;		/* D: scanner hot spot */
  Color i_global_colormap[256];	/* D: colormap for first frame (stashed) */
  int i_num_global_color_indices;/* D: number of indices in global colormap */
  bool i_header_written;	/* D: GIF header written yet? */
#ifndef X_DISPLAY_MISSING
/* elements specific to X Drawable Plotters and X Plotters */
  Display *x_dpy;		/* display */
  Drawable x_drawable1;		/* an X drawable (e.g. a pixmap) */
  Drawable x_drawable2;		/* an X drawable (e.g. a window) */
  Drawable x_drawable3;		/* graphics buffer, if double buffering */
  int x_double_buffering;	/* double buffering type (if any) */
  Fontrecord *x_fontlist;	/* D: head of list of retrieved X fonts */
  Colorrecord *x_colorlist;	/* D: head of list of retrieved X color cells*/
  Colormap x_cmap;		/* D: colormap */
  int x_cmap_type;		/* D: colormap type (orig./copied/bad) */
  bool x_color_warning_issued;	/* D: issued warning on colormap filling up */
/* additional elements specific to X Plotters */
  XtAppContext y_app_con;	/* application context */
  Widget y_toplevel;		/* toplevel widget */
  Widget y_canvas;		/* Label widget */
  Drawable y_drawable4;		/* used for server-side double buffering */
  bool y_vanish_on_delete;	/* window(s) disappear on Plotter deletion? */
  pid_t *y_pids;		/* D: list of pids of forked-off processes */
  int y_num_pids;		/* D: number of pids in list */
#endif /* not X_DISPLAY_MISSING */
#endif /* NOT_LIBPLOTTER */

#ifndef NOT_LIBPLOTTER
  /* STATIC DATA MEMBERS. (In libplotter, these are used for maintaining a
     sparse array of pointers to Plotter instances.  In libplot, the
     corresponding static variables are globals, rather than members of the
     Plotter struct.  See comment in g_defplot.c.) */
  static Plotter **_plotters;	/* D: sparse array of Plotter instances */
  static int _plotters_len;	/* D: length of sparse array */
#endif /* NOT_LIBPLOTTER */

}
#ifdef NOT_LIBPLOTTER
Plotter;
#else  /* not NOT_LIBPLOTTER */
;
#endif /* not NOT_LIBPLOTTER */

#undef P___
#undef Q___


#ifndef NOT_LIBPLOTTER
/****************** DERIVED CLASSES (libplotter only) ********************/

/* The derived Plotter classes extensively override the generic Plotter
   methods.  Note that in libplot, this overriding is accomplished
   differently: `derived' Plotter structs are initialized to contain
   function pointers that may point to the non-generic methods.  The files
   ?_defplot.c contain the structures which, in libplot, are used to
   initialize the function-pointer part of the derived Plotter structs.

   The device-specific data members which, in libplot, all appear in every
   Plotter struct, are in libplotter spread among the derived Plotter
   classes, as they logically should be.  */

/* The MetaPlotter class, which produces GNU metafile output */
class MetaPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  MetaPlotter (const MetaPlotter& oldplotter);  
  MetaPlotter& operator= (const MetaPlotter& oldplotter);
 public:
  /* ctors, dtor */
  MetaPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  MetaPlotter (FILE *outfile);
  MetaPlotter (istream& in, ostream& out, ostream& err);
  MetaPlotter (ostream& out);
  MetaPlotter ();
  virtual ~MetaPlotter ();
  /* public methods (overriding Plotter methods) */
  int alabel (int x_justify, int y_justify, const char *s);
  int arc (int xc, int yc, int x0, int y0, int x1, int y1);
  int arcrel (int dxc, int dyc, int dx0, int dy0, int dx1, int dy1);
  int bezier2 (int x0, int y0, int x1, int y1, int x2, int y2);
  int bezier2rel (int dx0, int dy0, int dx1, int dy1, int dx2, int dy2);
  int bezier3 (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  int bezier3rel (int dx0, int dy0, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3);
  int bgcolor (int red, int green, int blue);
  int box (int x0, int y0, int x1, int y1);
  int boxrel (int dx0, int dy0, int dx1, int dy1);
  int capmod (const char *s);
  int circle (int x, int y, int r);
  int circlerel (int dx, int dy, int r);
  int closepl (void);
  int cont (int x, int y);
  int contrel (int x, int y);
  int ellarc (int xc, int yc, int x0, int y0, int x1, int y1);
  int ellarcrel (int dxc, int dyc, int dx0, int dy0, int dx1, int dy1);
  int ellipse (int x, int y, int rx, int ry, int angle);
  int ellipserel (int dx, int dy, int rx, int ry, int angle);
  int endpath (void);
  int erase (void);
  int farc (double xc, double yc, double x0, double y0, double x1, double y1);
  int farcrel (double dxc, double dyc, double dx0, double dy0, double dx1, double dy1);
  int fbezier2 (double x0, double y0, double x1, double y1, double x2, double y2);
  int fbezier2rel (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2);
  int fbezier3 (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);
  int fbezier3rel (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3);
  int fbox (double x0, double y0, double x1, double y1);
  int fboxrel (double dx0, double dy0, double dx1, double dy1);
  int fcircle (double x, double y, double r);
  int fcirclerel (double dx, double dy, double r);
  int fconcat (double m0, double m1, double m2, double m3, double m4, double m5);
  int fcont (double x, double y);
  int fcontrel (double x, double y);
  int fellarc (double xc, double yc, double x0, double y0, double x1, double y1);
  int fellarcrel (double dxc, double dyc, double dx0, double dy0, double dx1, double dy1);
  int fellipse (double x, double y, double rx, double ry, double angle);
  int fellipserel (double dx, double dy, double rx, double ry, double angle);
  double ffontname (const char *s);
  double ffontsize (double size);
  int fillcolor (int red, int green, int blue);
  int fillmod (const char *s);
  int filltype (int level);
  int fline (double x0, double y0, double x1, double y1);
  int flinedash (int n, const double *dashes, double offset);
  int flinerel (double dx0, double dy0, double dx1, double dy1);
  int flinewidth (double size);
  int fmarker (double x, double y, int type, double size);
  int fmarkerrel (double dx, double dy, int type, double size);
  int fmiterlimit (double limit);
  int fmove (double x, double y);
  int fmoverel (double x, double y);
  int fontname (const char *s);
  int fontsize (int size);
  int fpoint (double x, double y);
  int fpointrel (double dx, double dy);
  int fspace (double x0, double y0, double x1, double y1);
  int fspace2 (double x0, double y0, double x1, double y1, double x2, double y2);
  double ftextangle (double angle);
  int joinmod (const char *s);
  int label (const char *s);
  int line (int x0, int y0, int x1, int y1);
  int linedash (int n, const int *dashes, int offset);
  int linemod (const char *s);
  int linerel (int dx0, int dy0, int dx1, int dy1);
  int linewidth (int size);
  int marker (int x, int y, int type, int size);
  int markerrel (int dx, int dy, int type, int size);
  int move (int x, int y);
  int moverel (int x, int y);
  int openpl (void);
  int pencolor (int red, int green, int blue);
  int point (int x, int y);
  int pointrel (int dx, int dy);
  int restorestate (void);
  int savestate (void);
  int space (int x0, int y0, int x1, int y1);
  int space2 (int x0, int y0, int x1, int y1, int x2, int y2);
  int textangle (int angle);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  /* internal function that overrides a Plotter internal function (a crock) */
  void _recompute_device_line_width (void);
  /* other MetaPlotter-specific internal functions */
  void _meta_emit_byte (int c);
  void _meta_emit_float (double x); 
  void _meta_emit_integer (int x); 
  void _meta_emit_string (const char *s);
  void _meta_emit_terminator (void);
  bool _set_initial_font_size (double x0, double y0, double x1, double y1, double x2, double y2);
  /* MetaPlotter-specific data members */
  bool meta_portable_output;	/* portable, not binary output format? */
};

/* The TekPlotter class, which produces Tektronix output */
class TekPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  TekPlotter (const TekPlotter& oldplotter);  
  TekPlotter& operator= (const TekPlotter& oldplotter);
 public:
  /* ctors, dtor */
  TekPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  TekPlotter (FILE *outfile);
  TekPlotter (istream& in, ostream& out, ostream& err);
  TekPlotter (ostream& out);
  TekPlotter ();
  virtual ~TekPlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int erase (void);
  int fcont (double x, double y);
  int fpoint (double x, double y);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  void set_attributes (void);
  void set_bg_color (void);
  void set_pen_color (void);
  /* TekPlotter-specific internal functions */
  void _tek_mode (int newmode);
  void _tek_move (int xx, int yy);
  void _tek_vector (int xx, int yy);
  void _tek_vector_compressed (int xx, int yy, int oldxx, int oldyy, bool force);
  /* TekPlotter-specific data members */
  int tek_display_type;		/* which sort of Tektronix? */
  int tek_mode;			/* D: one of MODE_* */
  int tek_line_type;		/* D: one of L_* */
  bool tek_mode_is_unknown;	/* D: tek mode unknown? */
  bool tek_line_type_is_unknown; /* D: tek line type unknown? */
  int tek_kermit_fgcolor;	/* D: kermit's foreground color */
  int tek_kermit_bgcolor;	/* D: kermit's background color */
  bool tek_position_is_unknown;	/* D: cursor position is unknown? */
  IntPoint tek_pos;		/* D: Tektronix cursor position */
};

/* The HPGLPlotter class, which produces HP-GL or HP-GL/2 output */
class HPGLPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  HPGLPlotter (const HPGLPlotter& oldplotter);  
  HPGLPlotter& operator= (const HPGLPlotter& oldplotter);
 public:
  /* ctors, dtor */
  HPGLPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  HPGLPlotter (FILE *outfile);
  HPGLPlotter (istream& in, ostream& out, ostream& err);
  HPGLPlotter (ostream& out);
  HPGLPlotter ();
  virtual ~HPGLPlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int endpath (void);
  int fbox (double x0, double y0, double x1, double y1);
  int fcircle (double x, double y, double r);
  int flinewidth (double size);
  int fpoint (double x, double y);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods, overridden in PCLPlotter class */
  void initialize (void);
  void terminate (void);
  /* protected methods (overriding Plotter methods) */
  double falabel_ps (const unsigned char *s, int h_just);
  double falabel_pcl (const unsigned char *s, int h_just);
  double falabel_stick (const unsigned char *s, int h_just);
  void set_attributes (void);
  void set_fill_color (void);
  void set_font (void);
  void set_pen_color (void);
  void set_position (void);
  /* internal functions that are overridden in the PCLPlotter class */
  virtual void _maybe_switch_to_hpgl (void);
  virtual void _maybe_switch_from_hpgl (void);
  /* other HPGLPlotter-specific internal functions */
  bool _hpgl2_maybe_update_font (void);
  bool _hpgl_maybe_update_font (void);
  bool _parse_pen_string (const char *pen_s);
  double _angle_of_arc (Point p0, Point p1, Point pc);
  int _hpgl_pseudocolor (int red, int green, int blue, bool restrict_white);
  void _compute_pseudo_fillcolor (int red, int green, int blue, int *pen, double *shading);
  void _set_hp_fill_type (int fill_type, double option1);
  void _set_hp_pen (int pen);
  /* HPGLPlotter-specific data members */
  int hpgl_version;		/* version: 0=HP-GL, 1=HP7550A, 2=HP-GL/2 */
  int rotation;			/* rotation angle (0, 90, 180, or 270) */
  Point p1;			/* scaling point P1 in native HP-GL coors */
  Point p2;			/* scaling point P2 in native HP-GL coors */
  double plot_length;		/* plot length (for HP-GL/2 roll plotters) */
  bool palette;			/* can assign pen colors? (HP-GL/2 only) */
  bool opaque_mode;		/* pen marks sh'd be opaque? (HP-GL/2 only) */
  Color pen_color[HPGL2_MAX_NUM_PENS]; /* D: color array for (logical) pens */
  int pen_defined[HPGL2_MAX_NUM_PENS]; /* D: 0=none, 1=soft-defd, 2=hard-defd*/
  int pen;			/* D: number of currently selected pen */
  int free_pen;			/* D: pen to be assigned a color next */
  bool bad_pen;			/* D: bad pen (advisory, see h_color.c) */
  bool pendown;			/* D: pen down rather than up? */
  double pen_width;		/* D: pen width(frac of diag dist betw P1,P2)*/
  int hpgl_line_type;		/* D: line type(HP-GL numbering,solid = -100)*/
  int hpgl_cap_style;		/* D: cap style for lines (HP-GL/2 numbering)*/
  int hpgl_join_style;		/* D: join style for lines(HP-GL/2 numbering)*/
  double hpgl_miter_limit;	/* D: miterlimit for line joins(HP-GL/2 only)*/
  int fill_type;		/* D: fill type (one of FILL_SOLID_UNI etc.) */
  double shading_level;		/* D: percent; used if fill_type=FILL_SHADING*/
  int pcl_symbol_set;		/* D: encoding, 14=ISO-Latin-1 (HP-GL/2 only)*/
  int pcl_spacing;		/* D: fontspacing,0=fixed,1=not(HP-GL/2 only)*/
  int pcl_posture;		/* D: posture,0=uprite,1=italic(HP-GL/2 only)*/
  int pcl_stroke_weight;	/* D: weight,0=normal,3=bold,..(HP-GL/2only)*/
  int pcl_typeface;		/* D: typeface, see g_fontdb.c(HP-GL/2 only) */
  int hpgl_charset_lower;	/* D: HP lower-half charset no. (pre-HP-GL/2)*/
  int hpgl_charset_upper;	/* D: HP upper-half charset no. (pre-HP-GL/2)*/
  double relative_char_height;	/* D: char ht., % of p2y-p1y (HP-GL/2 only) */
  double relative_char_width;	/* D: char width, % of p2x-p1x (HP-GL/2 only)*/
  double relative_label_rise;	/* D: label rise, % of p2y-p1y (HP-GL/2 only)*/
  double relative_label_run;	/* D: label run, % of p2x-p1x (HP-GL/2 only) */
  double char_slant_tangent;	/* D: tan of character slant (HP-GL/2 only)*/
  bool hpgl_position_is_unknown; /* D: HP-GL[/2] cursor position is unknown? */
  IntPoint hpgl_pos;		/* D: cursor position (integer HP-GL coors) */
};

/* The PCLPlotter class, which produces PCL 5 output */
class PCLPlotter : public HPGLPlotter
{
 private:
  /* disallow copying and assignment */
  PCLPlotter (const PCLPlotter& oldplotter);  
  PCLPlotter& operator= (const PCLPlotter& oldplotter);
 public:
  /* ctors, dtor */
  PCLPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  PCLPlotter (FILE *outfile);
  PCLPlotter (istream& in, ostream& out, ostream& err);
  PCLPlotter (ostream& out);
  PCLPlotter ();
  virtual ~PCLPlotter ();
 protected:
  /* protected methods (overriding HPGLPlotter methods) */
  void initialize (void);
  void terminate (void);
  /* internal functions that override HPGLPlotter internal functions */
  void _maybe_switch_to_hpgl (void);
  void _maybe_switch_from_hpgl (void);
};

/* The FigPlotter class, which produces Fig-format output for xfig */
class FigPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  FigPlotter (const FigPlotter& oldplotter);  
  FigPlotter& operator= (const FigPlotter& oldplotter);
 public:
  /* ctors, dtor */
  FigPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  FigPlotter (FILE *outfile);
  FigPlotter (istream& in, ostream& out, ostream& err);
  FigPlotter (ostream& out);
  FigPlotter ();
  virtual ~FigPlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int endpath (void);
  int erase (void);
  int fbox (double x0, double y0, double x1, double y1);
  int fcircle (double x, double y, double r);
  int fellipse (double x, double y, double rx, double ry, double angle);
  int flinewidth (double size);
  int fpoint (double x, double y);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  double falabel_ps (const unsigned char *s, int h_just);
  void retrieve_font (void);
  void set_fill_color (void);
  void set_pen_color (void);
  /* FigPlotter-specific internal functions */
  int _f_draw_ellipse_internal (double x, double y, double rx, double ry, double angle, int subtype);
  int _fig_color (int red, int green, int blue);
  void _f_compute_line_style (int *style, double *spacing);
  void _f_emit_arc (double xc, double yc, double x0, double y0, double x1, double y1);
  /* FigPlotter-specific data members */
  int fig_drawing_depth;	/* D: fig's curr value for `depth' attribute */
  int fig_num_usercolors;	/* D: number of colors currently defined */
  long int fig_usercolors[FIG_MAX_NUM_USER_COLORS]; /* D: colors we've def'd */
};

/* The PSPlotter class, which produces idraw-editable PS output */
class PSPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  PSPlotter (const PSPlotter& oldplotter);  
  PSPlotter& operator= (const PSPlotter& oldplotter);
 public:
  /* ctors, dtor */
  PSPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  PSPlotter (FILE *outfile);
  PSPlotter (istream& in, ostream& out, ostream& err);
  PSPlotter (ostream& out);
  PSPlotter ();
  virtual ~PSPlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int endpath (void);
  int erase (void);
  int fcircle (double x, double y, double r);
  int fellipse (double x, double y, double rx, double ry, double angle);
  int fpoint (double x, double y);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  double falabel_ps (const unsigned char *s, int h_just);
  double falabel_pcl (const unsigned char *s, int h_just);
  void set_fill_color (void);
  void set_pen_color (void);
  /* PSPlotter-specific internal functions */
  double _p_emit_common_attributes (void);
  void _p_compute_idraw_bgcolor (void);
  void _p_fellipse_internal (double x, double y, double rx, double ry, double angle, bool circlep);
};

/* The AIPlotter class, which produces output editable by Adobe Illustrator */
class AIPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  AIPlotter (const AIPlotter& oldplotter);  
  AIPlotter& operator= (const AIPlotter& oldplotter);
 public:
  /* ctors, dtor */
  AIPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  AIPlotter (FILE *outfile);
  AIPlotter (istream& in, ostream& out, ostream& err);
  AIPlotter (ostream& out);
  AIPlotter ();
  virtual ~AIPlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int endpath (void);
  int fpoint (double x, double y);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  double falabel_ps (const unsigned char *s, int h_just);
  double falabel_pcl (const unsigned char *s, int h_just);
  void set_attributes (void);
  void set_fill_color (void);
  void set_pen_color (void);
  /* AIPlotter-specific data members */
  int ai_version;		/* AI3 or AI5? */
  double ai_pen_cyan;		/* D: pen color (in CMYK space) */
  double ai_pen_magenta;
  double ai_pen_yellow;
  double ai_pen_black;
  double ai_fill_cyan;		/* D: fill color (in CMYK space) */
  double ai_fill_magenta;
  double ai_fill_yellow;
  double ai_fill_black;
  bool ai_cyan_used;		/* D: C, M, Y, K have been used? */
  bool ai_magenta_used;
  bool ai_yellow_used;
  bool ai_black_used;
  int ai_cap_style;		/* D: cap style for lines (PS numbering)*/
  int ai_join_style;		/* D: join style for lines(PS numbering)*/
  double ai_miter_limit;	/* D: miterlimit for line joins */
  int ai_line_type;		/* D: one of L_* */
  double ai_line_width;		/* D: line width in printer's points */
  int ai_fill_rule_type;	/* D: fill rule (FILL_{ODD|NONZERO}_WINDING) */
};

/* The PNMPlotter class, which produces PBM/PGM/PPM output */
class PNMPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  PNMPlotter (const PNMPlotter& oldplotter);  
  PNMPlotter& operator= (const PNMPlotter& oldplotter);
 public:
  /* ctors, dtor */
  PNMPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  PNMPlotter (FILE *outfile);
  PNMPlotter (istream& in, ostream& out, ostream& err);
  PNMPlotter (ostream& out);
  PNMPlotter ();
  virtual ~PNMPlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int endpath (void);
  int erase (void);
  int fellipse (double x, double y, double rx, double ry, double angle);
  int fpoint (double x, double y);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  /* PNMPlotter-specific internal functions */
  void _n_delete_image (void);
  void _n_draw_elliptic_mi_arc_internal (int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange);
  void _n_new_image (void);
  void _n_write_pnm (void);
  void _n_write_pbm (void);
  void _n_write_pgm (void);
  void _n_write_ppm (void);
  /* PNMPlotter-specific data members */
  bool n_portable_output;	/* portable, not binary output format? */
  int n_xn, n_yn;		/* bitmap dimensions */
  miPixel **n_bitmap;		/* D: bitmap */
};

/* The GIFPlotter class, which produces pseudo-GIF output */
class GIFPlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  GIFPlotter (const GIFPlotter& oldplotter);  
  GIFPlotter& operator= (const GIFPlotter& oldplotter);
 public:
  /* ctors, dtor */
  GIFPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  GIFPlotter (FILE *outfile);
  GIFPlotter (istream& in, ostream& out, ostream& err);
  GIFPlotter (ostream& out);
  GIFPlotter ();
  virtual ~GIFPlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int endpath (void);
  int erase (void);
  int fellipse (double x, double y, double rx, double ry, double angle);
  int fpoint (double x, double y);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  void set_bg_color (void);
  void set_fill_color (void);
  void set_pen_color (void);
  /* GIFPlotter-specific internal functions */
  unsigned char _i_new_color_index (int red, int green, int blue);
  int _i_scan_pixel (void);
  void _i_delete_image (void);
  void _i_draw_elliptic_mi_arc_internal (int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange);
  void _i_new_image (void);
  void _i_start_scan (void);
  void _i_write_gif_header (void);
  void _i_write_gif_image (void);
  void _i_write_gif_trailer (void);
  void _i_write_short_int (unsigned int i);
  /* GIFPlotter-specific data members */
  int i_xn, i_yn;		/* bitmap dimensions */
  int i_num_pixels;		/* total pixels (used by scanner) */
  bool i_animation;		/* animated (multi-image) GIF? */
  int i_iterations;		/* number of times GIF should be looped */
  int i_delay;			/* delay after image, in 1/100 sec units */
  bool i_interlace;		/* interlaced GIF? */
  bool i_transparent;		/* transparent GIF? */
  Color i_transparent_color;	/* if so, transparent color (24-bit RGB) */
  int i_transparent_index;	/* D: if so, transparent color index */
  miPixel **i_bitmap;		/* D: bitmap */
  Color i_colormap[256];	/* D: frame colormap (containing 24-bit RGBs)*/
  int i_num_color_indices;	/* D: number of color indices allocated */
  bool i_frame_nonempty;	/* D: something drawn in current frame? */
  int i_bit_depth;		/* D: bit depth (ceil(log2(num_indices))) */
  int i_pixels_scanned;		/* D: number that scanner has scanned */
  int i_pass;			/* D: scanner pass (used if interlacing) */
  IntPoint i_hot;		/* D: scanner hot spot */
  Color i_global_colormap[256];	/* D: colormap for first frame (stashed) */
  int i_num_global_color_indices;/* D: number of indices in global colormap */
  bool i_header_written;	/* D: GIF header written yet? */
};

#ifndef X_DISPLAY_MISSING
/* The XDrawablePlotter class, which draws into one or two X drawables */
class XDrawablePlotter : public Plotter
{
 private:
  /* disallow copying and assignment */
  XDrawablePlotter (const XDrawablePlotter& oldplotter);  
  XDrawablePlotter& operator= (const XDrawablePlotter& oldplotter);
 public:
  /* ctors, dtor */
  XDrawablePlotter (FILE *infile, FILE *outfile, FILE *errfile);
  XDrawablePlotter (FILE *outfile);
  XDrawablePlotter (istream& in, ostream& out, ostream& err);
  XDrawablePlotter (ostream& out);
  XDrawablePlotter ();
  virtual ~XDrawablePlotter ();
  /* public methods (overriding Plotter methods) */
  int closepl (void);
  int endpath (void);
  int erase (void);
  int fbox (double x0, double y0, double x1, double y1);
  int fcont (double x, double y);
  int fellipse (double x, double y, double rx, double ry, double angle);
  int flushpl (void);
  int fpoint (double x, double y);
  int openpl (void);
  int restorestate (void);
  int savestate (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  double falabel_ps (const unsigned char *s, int h_just);
  double falabel_pcl (const unsigned char *s, int h_just);
  double falabel_other (const unsigned char *s, int h_just);
  double flabelwidth_ps (const unsigned char *s);
  double flabelwidth_pcl (const unsigned char *s);
  double flabelwidth_other (const unsigned char *s);
  void retrieve_font (void);
  void set_attributes (void);
  void set_bg_color (void);
  void set_fill_color (void);
  void set_pen_color (void);
  /* internal functions that are overridden in the XPlotter class (crocks) */
  virtual void _maybe_get_new_colormap (void);
  virtual void _maybe_handle_x_events (void);
  /* other XDrawablePlotter-specific internal functions */
  bool _retrieve_X_color (XColor *rgb_ptr);
  bool _retrieve_X_font_internal (const char *name, double size, double rotation);
  bool _select_X_font (const char *name, bool is_zero[4], const unsigned char *s);
  bool _select_X_font_carefully (const char *name, bool is_zero[4], const unsigned char *s);
  void _draw_elliptic_X_arc (Point p0, Point p1, Point pc);
  void _draw_elliptic_X_arc_2 (Point p0, Point p1, Point pc);
  void _draw_elliptic_X_arc_internal (int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange);
  void _set_X_font_dimensions (bool is_zero[4]);
  /* XDrawablePlotter-specific data members */
  Display *x_dpy;		/* display */
  Drawable x_drawable1;		/* an X drawable (e.g. a pixmap) */
  Drawable x_drawable2;		/* an X drawable (e.g. a window) */
  Drawable x_drawable3;		/* graphics buffer, if double buffering */
  int x_double_buffering;	/* double buffering type (if any) */
  Fontrecord *x_fontlist;	/* D: head of list of retrieved X fonts */
  Colorrecord *x_colorlist;	/* D: head of list of retrieved X color cells*/
  Colormap x_cmap;		/* D: colormap (dynamic only for XPlotters) */
  int x_cmap_type;		/* D: colormap type (orig./copied/bad) */
  bool x_color_warning_issued;	/* D: issued warning on colormap filling up */
};

/* The XPlotter class, which pops up a window and draws into it */
class XPlotter : public XDrawablePlotter
{
 private:
  /* disallow copying and assignment */
  XPlotter (const XPlotter& oldplotter);  
  XPlotter& operator= (const XPlotter& oldplotter);
 public:
  /* ctors, dtor */
  XPlotter (FILE *infile, FILE *outfile, FILE *errfile);
  XPlotter (FILE *outfile);
  XPlotter (istream& in, ostream& out, ostream& err);
  XPlotter (ostream& out);
  XPlotter ();
  virtual ~XPlotter ();
  /* public methods (overriding XDrawablePlotter methods) */
  int closepl (void);
  int erase (void);
  int openpl (void);
 protected:
  /* protected methods (overriding Plotter methods) */
  void initialize (void);
  void terminate (void);
  /* internal functions that override XDrawablePlotter functions (crocks) */
  void _maybe_get_new_colormap (void);
  void _maybe_handle_x_events (void);
  /* other XPlotter-specific internal functions */
  void _y_set_data_for_quitting (void);
  /* XPlotter-specific data members (non-static) */
  XtAppContext y_app_con;	/* application context */
  Widget y_toplevel;		/* toplevel widget */
  Widget y_canvas;		/* Label widget */
  Drawable y_drawable4;		/* used for server-side double buffering */
  bool y_vanish_on_delete;	/* window(s) disappear on Plotter deletion? */
  pid_t *y_pids;		/* D: list of pids of forked-off processes */
  int y_num_pids;		/* D: number of pids in list */
  /* XPlotter-specific data members (static) */
  static XPlotter **_xplotters;	/* D: sparse array of XPlotter instances */
  static int _xplotters_len;	/* D: length of sparse array */
};
#endif /* not X_DISPLAY_MISSING */
#endif /* not NOT_LIBPLOTTER */


/***********************************************************************/

#ifndef _PLOT_H_	/* allow inclusion of both plot.h, plotter.h */

/* Symbol types for the marker() function, extending over the range 0..31.
   (1 through 5 are the same as in the GKS [Graphical Kernel System].)

   These are now defined as enums rather than ints.  Cast them to ints if
   necessary. */
enum 
{ M_NONE, M_DOT, M_PLUS, M_ASTERISK, M_CIRCLE, M_CROSS, 
  M_SQUARE, M_TRIANGLE, M_DIAMOND, M_STAR, M_INVERTED_TRIANGLE, 
  M_STARBURST, M_FANCY_PLUS, M_FANCY_CROSS, M_FANCY_SQUARE, 
  M_FANCY_DIAMOND, M_FILLED_CIRCLE, M_FILLED_SQUARE, M_FILLED_TRIANGLE, 
  M_FILLED_DIAMOND, M_FILLED_INVERTED_TRIANGLE, M_FILLED_FANCY_SQUARE,
  M_FILLED_FANCY_DIAMOND, M_HALF_FILLED_CIRCLE, M_HALF_FILLED_SQUARE,
  M_HALF_FILLED_TRIANGLE, M_HALF_FILLED_DIAMOND,
  M_HALF_FILLED_INVERTED_TRIANGLE, M_HALF_FILLED_FANCY_SQUARE,
  M_HALF_FILLED_FANCY_DIAMOND, M_OCTAGON, M_FILLED_OCTAGON 
};

/* ONE-BYTE OPERATION CODES FOR GNU METAFILE FORMAT. These are now defined
   as enums rather than ints.  Cast them to ints if necessary.

   There are 80 currently used op codes, including 30 that are used only in
   binary metafiles, not in portable metafiles. */

enum
{  
/* 10 op codes for primitive graphics operations, as in Unix plot(5) format. */
  O_ARC		=	'a',  
  O_CIRCLE	=	'c',  
  O_CONT	=	'n',
  O_ERASE	=	'e',
  O_LABEL	=	't',
  O_LINEMOD	=	'f',
  O_LINE	=	'l',
  O_MOVE	=	'm',
  O_POINT	=	'p',
  O_SPACE	=	's',
  
/* 38 op codes that are GNU extensions */
  O_ALABEL	=	'T',
  O_ARCREL	=	'A',
  O_BEZIER2	=       'q',
  O_BEZIER2REL	=       'r',
  O_BEZIER3	=       'y',
  O_BEZIER3REL	=       'z',
  O_BGCOLOR	=	'~',
  O_BOX		=	'B',	/* not an op code in Unix plot(5) */
  O_BOXREL	=	'H',
  O_CAPMOD	=	'K',
  O_CIRCLEREL	=	'G',
  O_CLOSEPL	=	'x',	/* not an op code in Unix plot(5) */
  O_COMMENT	=	'#',
  O_CONTREL	=	'N',
  O_ELLARC	=	'?',
  O_ELLARCREL	=	'/',
  O_ELLIPSE	=	'+',
  O_ELLIPSEREL	=	'=',
  O_ENDPATH	=	'E',
  O_FILLTYPE	=	'L',
  O_FILLCOLOR	=	'D',
  O_FILLMOD	=	'g',
  O_FONTNAME	=	'F',
  O_FONTSIZE	=	'S',
  O_JOINMOD	=	'J',
  O_LINEDASH	= 	'd',
  O_LINEREL	=	'I',
  O_LINEWIDTH	=	'W',
  O_MARKER	=	'Y',
  O_MARKERREL	=	'Z',
  O_MOVEREL	=	'M',
  O_OPENPL	=	'o',	/* not an op code in Unix plot(5) */
  O_PENCOLOR	=	'-',
  O_POINTREL	=	'P',
  O_RESTORESTATE=	'O',
  O_SAVESTATE	=	'U',
  O_SPACE2	=	':',
  O_TEXTANGLE	=	'R',

/* 30 floating point counterparts to some of the above.  Used only in
   binary GNU metafile format, not in portable (human-readable) metafile
   format, so they are not even slightly mnemonic. */
  O_FARC	=	'1',
  O_FARCREL	=	'2',
  O_FBEZIER2	=       '`',
  O_FBEZIER2REL	=       '\'',
  O_FBEZIER3	=       ',',
  O_FBEZIER3REL	=       '.',
  O_FBOX	=	'3',
  O_FBOXREL	=	'4',
  O_FCIRCLE	=	'5',
  O_FCIRCLEREL	=	'6',
  O_FCONT	=	')',
  O_FCONTREL	=	'_',
  O_FELLARC	=	'}',
  O_FELLARCREL	=	'|',
  O_FELLIPSE	=	'{',
  O_FELLIPSEREL	=	'[',
  O_FFONTSIZE	=	'7',
  O_FLINE	=	'8',
  O_FLINEDASH	= 	'w',
  O_FLINEREL	=	'9',
  O_FLINEWIDTH	=	'0',
  O_FMARKER	=	'!',
  O_FMARKERREL	=	'@',
  O_FMOVE	=	'$',
  O_FMOVEREL	=	'%',
  O_FPOINT	=	'^',
  O_FPOINTREL	=	'&',
  O_FSPACE	=	'*',
  O_FSPACE2	=	';',
  O_FTEXTANGLE	=	'(',

/* 2 op codes for floating point operations with no integer counterpart */
  O_FCONCAT		=	'\\',
  O_FMITERLIMIT		=	'i'
};

#endif /* not _PLOT_H_ */

/***********************************************************************/

#endif /* not _PLOTTER_H_ */
