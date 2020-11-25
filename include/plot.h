/* This is plot.h, the user-level include file for release 1.1 of 
   GNU libplot, a shared library for 2-dimensional vector graphics.  

   Release 1.1 of libplot, and this file, are distributed as part of
   release 2.2 of the GNU plotting utilities package. */

/* stdio.h must be included before this file is included. */

/* This file is written for ANSI C compilers.  If you use it with a
   pre-ANSI C compiler that does not support the `const' keyword, such as
   the `cc' compiler supplied with SunOS (i.e., Solaris 1.x), you should
   use the -DNO_CONST_SUPPORT option when compiling your code. */

#ifndef _PLOT_H_
#define _PLOT_H_ 1

/* support C++ */
#ifdef __BEGIN_DECLS
#undef __BEGIN_DECLS
#endif
#ifdef __END_DECLS
#undef __END_DECLS
#endif
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS		/* empty */
# define __END_DECLS		/* empty */
#endif
     
/* ___P is a macro used to wrap function prototypes, so that compilers that
   don't understand ANSI C prototypes still work, and ANSI C compilers can
   issue warnings about type mismatches. */
#ifdef ___P
#undef ___P
#endif
#if defined (__STDC__) || defined (_AIX) \
	|| (defined (__mips) && defined (_SYSTYPE_SVR4)) \
	|| defined(WIN32) || defined(__cplusplus)
#define ___P(protos) protos
#else
#define ___P(protos) ()
#endif

/* For old compilers (e.g. SunOS) */
#ifdef ___const
#undef ___const
#endif
#ifdef NO_CONST_SUPPORT
#define ___const
#else
#define ___const const
#endif

__BEGIN_DECLS

/* THE C API */

/* THE GLOBAL FUNCTIONS IN GNU LIBPLOT */
/* There are 81 basic functions, plus 4 that are specific to the C binding */

/* functions in traditional (pre-GNU) libplot */
int arc ___P ((int xc, int yc, int x0, int y0, int x1, int y1));
int box ___P ((int x0, int y0, int x1, int y1));
int circle ___P ((int x, int y, int r));
int closepl ___P ((void));
int cont ___P ((int x, int y));
int erase ___P ((void));
int label ___P ((___const char *s));
int line ___P ((int x0, int y0, int x1, int y1));
int linemod ___P ((___const char *s));
int move ___P ((int x, int y));
int openpl ___P ((void));
int point ___P ((int x, int y));
int space ___P ((int x0, int y0, int x1, int y1));

/* additional functions in GNU libplot */
int alabel ___P ((int x_justify, int y_justify, ___const char *s));
int arcrel ___P ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
int bgcolor ___P ((int red, int green, int blue));
int bgcolorname ___P ((___const char *name));
int boxrel ___P ((int dx0, int dy0, int dx1, int dy1));
int capmod ___P ((___const char *s));
int circlerel ___P ((int dx, int dy, int r));
int color ___P ((int red, int green, int blue));
int colorname ___P ((___const char *name));
int contrel ___P ((int x, int y));
int ellarc ___P ((int xc, int yc, int x0, int y0, int x1, int y1));
int ellarcrel ___P ((int dxc, int dyc, int dx0, int dy0, int dx1, int dy1));
int ellipse ___P ((int x, int y, int rx, int ry, int angle));
int ellipserel ___P ((int dx, int dy, int rx, int ry, int angle));
int endpath ___P((void));
int filltype ___P ((int level));
int fillcolor ___P ((int red, int green, int blue));
int fillcolorname ___P ((___const char *name));
int flushpl ___P ((void));
int fontname ___P ((___const char *s));
int fontsize ___P ((int size));
int havecap ___P ((___const char *s));
int joinmod ___P ((___const char *s));
int labelwidth ___P ((___const char *s));
int linerel ___P ((int dx0, int dy0, int dx1, int dy1));
int linewidth ___P ((int size));
int marker ___P ((int x, int y, int type, int size));
int markerrel ___P ((int dx, int dy, int type, int size));
int moverel ___P ((int x, int y));
FILE* outfile ___P((FILE* newstream));
int pencolor ___P ((int red, int green, int blue));
int pencolorname ___P ((___const char *name));
int pointrel ___P ((int dx, int dy));
int restorestate ___P((void));
int savestate ___P((void));
int space2 ___P ((int x0, int y0, int x1, int y1, int x2, int y2));
int textangle ___P ((int angle));

/* floating point counterparts to the above (all GNU additions) */
double ffontname ___P ((___const char *s));
double ffontsize ___P ((double size));
double flabelwidth ___P ((___const char *s));
double ftextangle ___P ((double angle));
int farc ___P ((double xc, double yc, double x0, double y0, double x1, double y1));
int farcrel ___P ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
int fbox ___P ((double x0, double y0, double x1, double y1));
int fboxrel ___P ((double dx0, double dy0, double dx1, double dy1));
int fcircle ___P ((double x, double y, double r));
int fcirclerel ___P ((double dx, double dy, double r));
int fcont ___P ((double x, double y));
int fcontrel ___P ((double x, double y));
int fellarc ___P ((double xc, double yc, double x0, double y0, double x1, double y1));
int fellarcrel ___P ((double dxc, double dyc, double dx0, double dy0, double dx1, double dy1));
int fellipse ___P ((double x, double y, double rx, double ry, double angle));
int fellipserel ___P ((double dx, double dy, double rx, double ry, double angle));
int fline ___P ((double x0, double y0, double x1, double y1));
int flinerel ___P ((double dx0, double dy0, double dx1, double dy1));
int flinewidth ___P ((double size));
int fmarker ___P ((double x, double y, int type, double size));
int fmarkerrel ___P ((double dx, double dy, int type, double size));
int fmove ___P ((double x, double y));
int fmoverel ___P ((double x, double y));
int fpoint ___P ((double x, double y));
int fpointrel ___P ((double dx, double dy));
int fspace ___P ((double x0, double y0, double x1, double y1));
int fspace2 ___P ((double x0, double y0, double x1, double y1, double x2, double y2));

/* floating point operations with no integer counterpart (GNU additions) */
int fconcat ___P ((double m0, double m1, double m2, double m3, double m4, double m5));
int frotate ___P ((double theta));
int fscale ___P ((double x, double y));
int ftranslate ___P ((double x, double y));

/* functions specific to the C binding (for construction/destruction of
   Plotters, and setting of low-level Plotter parameters ) */
int newpl ___P((___const char *type, FILE *instream, FILE *outstream, FILE *errstream));
int selectpl ___P ((int handle));
int deletepl ___P ((int handle));
#ifdef NO_VOID_SUPPORT
int parampl ___P ((___const char *parameter, char *value));
#else
int parampl ___P ((___const char *parameter, void *value));
#endif

__END_DECLS

/* THE GLOBAL VARIABLES IN GNU LIBPLOT */
/* There are two; both are user-settable error handlers. */
extern int (*libplot_warning_handler) ___P ((___const char *msg));
extern int (*libplot_error_handler) ___P ((___const char *msg));

#undef ___const
#undef ___P

/* A temporary kludge.  In plotutils-2.x, endpoly() and fill() were renamed
   endpath() and filltype().  Also, falabel() and the undocumented function
   dot() were dropped, and the return value for alabel() was changed.  We
   can't do anything to work around the latter changes, but for the next
   couple of releases we'll at least support endpoly() and fill() by
   including the following preprocessor definitions. */

#ifndef endpoly
#define endpoly() endpath()
#endif
#ifndef fill
#define fill(arg) filltype(arg)
#endif

/* symbol types for the marker() function, 1 through 5 are as in GKS */

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


/* ONE-BYTE OP CODES FOR GNU METAFILE FORMAT */

/* Op codes for primitive graphics operations, as in Unix plot(5) format. */
#define ARC		'a'
#define BOX		'B'	/* not a separate op code in Unix plot(5) */
#define CIRCLE		'c'
#define CONT		'n'
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
#define BGCOLOR		'~'
#define BOXREL		'H'
#define CAPMOD		'K'
#define CIRCLEREL	'G'
#define CLOSEPL		'x'
#define COLOR		'C'	/* obsolete, to be removed */
#define COMMENT		'#'
#define CONTREL		'N'
#define ELLARC		'?'
#define ELLARCREL	'/'
#define ELLIPSE		'+'
#define ELLIPSEREL	'='
#define ENDPATH		'E'
#define FILLTYPE	'L'
#define FILLCOLOR	'D'
#define FONTNAME	'F'
#define FONTSIZE	'S'
#define JOINMOD		'J'
#define LINEREL		'I'
#define LINEWIDTH	'W'
#define MARKER		'Y'
#define MARKERREL	'Z'
#define MOVEREL		'M'
#define OPENPL		'o'
#define PENCOLOR	'-'
#define POINTREL	'P'
#define RESTORESTATE	'O'
#define SAVESTATE	'U'
#define SPACE2		':'
#define TEXTANGLE	'R'

/* Floating point counterparts to the above.  Used only in binary metafile
   format, not in portable metafile format. */
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

#endif /* _PLOT_H_ */
