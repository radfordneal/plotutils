/* This is plot.h, the user-level include file for the several versions of
   GNU libplot.  stdio.h must be included by the user before this file is
   included. */

#ifndef _plot_h_
#define _plot_h_

/* support C++ */
#ifdef __cplusplus
#define externMod extern "C"
#else
#define externMod extern
#endif

/* This elides the argument prototypes if the compiler does not support
   them. The name P__ is chosen in hopes that it will not collide with any
   others. */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif

/* functions in traditional libplot */
externMod int arc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
externMod int box P__ ((int x0, int y0, int x1, int y1));
externMod int circle P__ ((int x, int y, int r));
externMod int closepl P__ ((void));
externMod int cont P__ ((int x, int y));
externMod int dot P__ ((int x, int y, int dx, int n, char *pattern));
externMod int erase P__ ((void));
externMod int label P__ ((char *s));
externMod int line P__ ((int x0, int y0, int x1, int y1));
externMod int linemod P__ ((char *s));
externMod int move P__ ((int x, int y));
externMod int openpl P__ ((void));
externMod int point P__ ((int x, int y));
externMod int space P__ ((int x0, int y0, int x1, int y1));

/* additional functions in GNU libplot */
externMod int alabel P__ ((int x_justify, int y_justify, char *s));
externMod int arcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
externMod int boxrel P__ ((int dx0, int dy0, int dx1, int dy1));
externMod int capmod P__ ((char *s));
externMod int circlerel P__ ((int dx, int dy, int r));
externMod int color P__ ((int red, int green, int blue));
externMod int colorname P__ ((char *name));
externMod int contrel P__ ((int x, int y));
externMod int ellarc P__ ((int xc, int yc, int x0, int y0, int x1, int y1));
externMod int ellarcrel P__ ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
externMod int ellipse P__ ((int x, int y, int rx, int ry, int angle));
externMod int ellipserel P__ ((int dx, int dy, int rx, int ry, int angle));
externMod int endpoly P__((void));
externMod int fill P__ ((int level));
externMod int fillcolor P__ ((int red, int green, int blue));
externMod int fillcolorname P__ ((char *name));
externMod int flushpl P__ ((void));
externMod int fontname P__ ((char *s));
externMod int fontsize P__ ((int size));
externMod int joinmod P__ ((char *s));
externMod int labelwidth P__ ((char *s));
externMod int linerel P__ ((int dx0, int dy0, int dx1, int dy1));
externMod int linewidth P__ ((int size));
externMod int marker P__ ((int x, int y, int type, int size));
externMod int markerrel P__ ((int dx, int dy, int type, int size));
externMod int moverel P__ ((int x, int y));
externMod FILE* outfile P__((FILE* newstream)); /* defined in stdio.h */
externMod int pencolor P__ ((int red, int green, int blue));
externMod int pencolorname P__ ((char *name));
externMod int pointrel P__ ((int dx, int dy));
externMod int restorestate P__((void));
externMod int savestate P__((void));
externMod int space2 P__ ((int x0, int y0, int x1, int y1, int x2, int y2));
externMod int textangle P__ ((int angle));

/* floating point counterparts to the above (all GNU extensions) */
externMod double falabel P__ ((int x_justify, int y_justify, char *s));
externMod double ffontname P__ ((char *s));
externMod double ffontsize P__ ((double size));
externMod double flabelwidth P__ ((char *s));
externMod double ftextangle P__ ((double angle));
externMod int farc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
externMod int farcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
externMod int fbox P__ ((double x0, double y0, double x1, double y1));
externMod int fboxrel P__ ((double dx0, double dy0, double dx1, double dy1));
externMod int fcircle P__ ((double x, double y, double r));
externMod int fcirclerel P__ ((double dx, double dy, double r));
externMod int fcont P__ ((double x, double y));
externMod int fcontrel P__ ((double x, double y));
externMod int fellarc P__ ((double xc, double yc, double x0, double y0, double x1, double y1));
externMod int fellarcrel P__ ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
externMod int fellipse P__ ((double x, double y, double rx, double ry, double angle));
externMod int fellipserel P__ ((double dx, double dy, double rx, double ry, double angle));
externMod int fline P__ ((double x0, double y0, double x1, double y1));
externMod int flinerel P__ ((double dx0, double dy0, double dx1, double dy1));
externMod int flinewidth P__ ((double size));
externMod int fmarker P__ ((double x, double y, int type, double size));
externMod int fmarkerrel P__ ((double dx, double dy, int type, double size));
externMod int fmove P__ ((double x, double y));
externMod int fmoverel P__ ((double x, double y));
externMod int fpoint P__ ((double x, double y));
externMod int fpointrel P__ ((double dx, double dy));
externMod int fspace P__ ((double x0, double y0, double x1, double y1));
externMod int fspace2 P__ ((double x0, double y0, double x1, double y1, double x2, double y2));

/* floating point operations with no integer counterpart (GNU extensions) */
externMod int fconcat P__ ((double m0, double m1, double m2, double m3, double m4, double m5));
externMod int frotate P__ ((double theta));
externMod int fscale P__ ((double x, double y));
externMod int ftranslate P__ ((double x, double y));

#undef P__

/* symbol types for marker(), 1 through 5 are as in GKS */
#define M_NONE 0
#define M_DOT 1
#define M_PLUS 2
#define M_ASTERISK 3
#define M_CIRCLE 4
#define M_CROSS 5
#define M_SQUARE 6
#define M_TRIANGLE 7
#define M_DIAMOND 8
#define M_STAR 9
#define M_INVERTED_TRIANGLE 10
#define M_STARBURST 11
#define M_FANCY_PLUS 12
#define M_FANCY_CROSS 13
#define M_FANCY_SQUARE 14
#define M_FANCY_DIAMOND 15
#define M_FILLED_CIRCLE 16
#define M_FILLED_SQUARE 17
#define M_FILLED_TRIANGLE 18
#define M_FILLED_DIAMOND 19
#define M_FILLED_INVERTED_TRIANGLE 20
#define M_FILLED_FANCY_SQUARE 21
#define M_FILLED_FANCY_DIAMOND 22
#define M_HALF_FILLED_CIRCLE 23
#define M_HALF_FILLED_SQUARE 24
#define M_HALF_FILLED_TRIANGLE 25
#define M_HALF_FILLED_DIAMOND 26
#define M_HALF_FILLED_INVERTED_TRIANGLE 27
#define M_HALF_FILLED_FANCY_SQUARE 28
#define M_HALF_FILLED_FANCY_DIAMOND 29
#define M_OCTAGON 30
#define M_FILLED_OCTAGON 31

/* THE FOLLOWING EXTERNAL VARIABLES APPEAR IN ANY VERSION OF GNU LIBPLOT. */

/* 1. READ-ONLY EXTERNAL LIBPLOT VARIABLES */

/* The string _libplot_suffix distinguishes among the versions of libplot.
   It is the string appended to "plot" to form the name of the library.  It
   is "fig" for libplotfig, "" for raw libplot, etc. */

externMod char *_libplot_suffix;

/* The following four variables are capability variables (three dealing
   with fonts, and one dealing with lines).  Values are 0, 1, or 2, meaning
   YES/NO/MAYBE.  In raw libplot some of these variables may take values
   `MAYBE', since plot(5) format can be displayed on several sorts of
   graphic device, with different capabilities. */

/* HAVE_FONT_METRICS specifies whether or not definitive font metric
   information is available, for the default font, at least.  If so, the
   alabel() and labelwidth() functions will return the width of the string
   argument, in user units, and the fontname(), fontsize(), and textangle()
   functions will return the size of the font, in user units.  This is true
   for all versions of libplot except raw libplot. */

externMod int _libplot_have_font_metrics;

/* HAVE_VECTOR_FONTS specifies whether or not the standard Hershey fonts
   are directly supported (this is the case for all versions of libplot,
   except for raw libplot, which produces output in a GNU-enhanced version
   of the traditional plot(5) format, which must be translated with one of
   the plot filters). */

externMod int _libplot_have_vector_fonts;

/* HAVE_PS_FONTS specifies whether or not the 35 standard Postscript fonts
   are directly supported.  This is the case for libplotfig, libplotps, and
   libplotX, though not for libplottek, or raw libplot. */

externMod int _libplot_have_ps_fonts;

/* HAVE_WIDE_LINES specifies whether or not lines with a non-default width
   are supported, i.e. whether or not linewidth() is guaranteed to have an
   effect.  This is the case for libplotfig, libplotps, and libplotX,
   though not for libplottek, or raw libplot. */

externMod int _libplot_have_wide_lines;

/* 2. WRITABLE EXTERNAL LIBPLOT VARIABLES */

/* Maximum length an unfilled polyline is allowed to grow to, before it is
   flushed out (we put no [small] maximum on the length of filled
   polylines, since they need to be preserved as discrete objects if the
   filling is to be performed properly).  This is accessible to the user.
   Warning: this is not looked at by libplottek, since that library
   currently plots points as they are supplied. */

externMod int _libplot_max_unfilled_polyline_length;

/* The following two modifiable external variables are relevant only in raw
   libplot, since they determine the format of its plot(5) directives.  But
   some applications expect them always to be present, so they can twiddle
   them. */

/* OUTPUT_IS_ASCII specifies whether the output of libplot will be in a
   GNU-enhanced version of the traditional plot(5) format, or in an
   portable ASCII (non-binary) format.

   A value of 0 specifies traditional plot(5) format.  A nonzero value
   specifies an ASCII format. */

externMod int _libplot_output_is_ascii;

/* OUTPUT_HIGH_BYTE_FIRST specifies the byte order for two-byte integers
   written to the output stream, if a GNU-enhanced version of plot(5)
   format is used.

   A value of 1 specifies that the integers will be written high byte
   first.  A value of -1 specifies that they will be written low byte
   first.  A zero value specifies that the host (default) byte order and
   format is used.  The host format may not even be a two-byte format. */

externMod int _libplot_output_high_byte_first;

/* One-byte op codes for GNU-enhanced plot(5) format */

/* Op codes for the traditional graphics operations */
#define ARC		'a'
#define BOX		'B'	/* not a separate op code in Unix libplot */
#define CIRCLE		'c'
#define CONT		'n'
#define DOT		'd'	/* obsolete, see dot.c */
#define ERASE		'e'
#define LABEL		't'
#define LINEMOD		'f'
#define LINE		'l'
#define MOVE		'm'
#define POINT		'p'
#define SPACE		's'

/* Op codes for operations that are GNU extensions */
#define ALABEL		'T'
#define ARCREL		'A'
#define BOXREL		'H'
#define CAPMOD		'K'
#define CIRCLEREL	'G'
#define COLOR		'C'	/* obsolete, to be removed in v. 2.0 */
#define COMMENT		'#'
#define CONTREL		'N'
#define ELLARC		'?'
#define ELLARCREL	'/'
#define ELLIPSE		'+'
#define ELLIPSEREL	'='
#define ENDPOLY		'E'
#define FILL		'L'
#define FILLCOLOR	'D'
#define FONTNAME	'F'
#define FONTSIZE	'S'
#define JOINMOD		'J'
#define LINEREL		'I'
#define LINEWIDTH	'W'
#define MARKER		'Y'
#define MARKERREL	'Z'
#define MOVEREL		'M'
#define PENCOLOR	'-'
#define POINTREL	'P'
#define RESTORESTATE	'O'
#define SAVESTATE	'U'
#define SPACE2		':'
#define TEXTANGLE	'R'

/* Op codes for floating point counterparts to the above */
#define FARC		'1'
#define FARCREL		'2'
#define FBOX		'3'
#define FBOXREL		'4'
#define FCIRCLE		'5'
#define FCIRCLEREL	'6'
#define FCONT		')'
#define FCONTREL	'_'
#define FELLARC		'}'
#define FELLARCREL	'|'
#define FELLIPSE	'{'
#define FELLIPSEREL	'['
#define FFONTSIZE	'7'
#define FLINE		'8'
#define FLINEREL	'9'
#define FLINEWIDTH	'0'
#define FMARKER		'!'
#define FMARKERREL	'@'
#define FMOVE		'$'
#define FMOVEREL	'%'
#define FPOINT		'^'
#define FPOINTREL	'&'
#define FSPACE		'*'
#define FSPACE2		';'
#define FTEXTANGLE	'('

/* Op codes for floating point operations with no integer counterpart */
#define FCONCAT		'\\'
#define FROTATE		'V'
#define FSCALE		'X'
#define FTRANSLATE	'Q'

#endif /* _plot_h_ */
