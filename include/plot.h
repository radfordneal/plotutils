/* This is plot.h, the user-level include file for GNU libplot, a shared
   library for 2-dimensional vector graphics. */

/* stdio.h must be included before this file is included. */

/* This file is written for ANSI C compilers.  If you use it with a
   pre-ANSI C compiler that does not support the `const' keyword, such as
   the `cc' compiler supplied with SunOS (i.e., Solaris 1.x), you should
   use the -DNO_CONST_SUPPORT option when compiling your code. */

#ifndef _PLOT_H_
#define _PLOT_H_ 1

/* This version of plot.h accompanies GNU libplot version 1.6. */
#define LIBPLOT_VERSION "1.6"

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

/* 13 functions in traditional (pre-GNU) libplot */
int arc ___P ((int xc, int yc, int x0, int y0, int x1, int y1));
int box ___P ((int x0, int y0, int x1, int y1)); /* no op code, originally */
int circle ___P ((int x, int y, int r));
int closepl ___P ((void));	/* no op code, originally */
int cont ___P ((int x, int y));
int erase ___P ((void));
int label ___P ((___const char *s));
int line ___P ((int x0, int y0, int x1, int y1));
int linemod ___P ((___const char *s));
int move ___P ((int x, int y));
int openpl ___P ((void));	/* no op code, originally */
int point ___P ((int x, int y));
int space ___P ((int x0, int y0, int x1, int y1));

/* 37 additional functions in GNU libplot */
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

/* 27 floating point counterparts to some of the above (all GNU additions) */
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

/* 4 floating point operations with no integer counterpart (GNU additions) */
int fconcat ___P ((double m0, double m1, double m2, double m3, double m4, double m5));
int frotate ___P ((double theta));
int fscale ___P ((double x, double y));
int ftranslate ___P ((double x, double y));

/* 4 functions specific to the C binding (for construction/destruction of
   Plotters, and setting of Plotter parameters) */
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

/* Symbol types for the marker() function, extending over the range 0..31
   (1 through 5 are as in GKS).  

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

/* ONE-BYTE OP CODES FOR GNU METAFILE FORMAT
   These are now defined as enums rather than ints.  Cast them to ints if
   necessary. */

/* There are 68 currently used op codes, including 25 that are used only in
   binary metafiles, not in portable metafiles.  Several obsolete op codes
   are listed below only because the `plot' filter still supports them. 
   They will be removed in a later release. */

/* 10 op codes for primitive graphics operations, as in Unix plot(5) format. */
enum
{  
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
  
  /* 32 op codes that are GNU extensions [plus an obsolete one] */
  O_ALABEL	=	'T',
  O_ARCREL	=	'A',
  O_BGCOLOR	=	'~',
  O_BOX		=	'B',	/* not a separate op code in Unix plot(5) */
  O_BOXREL	=	'H',
  O_CAPMOD	=	'K',
  O_CIRCLEREL	=	'G',
  O_CLOSEPL	=	'x',
  O_COLOR	=	'C',	/* obsolete, to be removed */
  O_COMMENT	=	'#',
  O_CONTREL	=	'N',
  O_ELLARC	=	'?',
  O_ELLARCREL	=	'/',
  O_ELLIPSE	=	'+',
  O_ELLIPSEREL	=	'=',
  O_ENDPATH	=	'E',
  O_FILLTYPE	=	'L',
  O_FILLCOLOR	=	'D',
  O_FONTNAME	=	'F',
  O_FONTSIZE	=	'S',
  O_JOINMOD	=	'J',
  O_LINEREL	=	'I',
  O_LINEWIDTH	=	'W',
  O_MARKER	=	'Y',
  O_MARKERREL	=	'Z',
  O_MOVEREL	=	'M',
  O_OPENPL	=	'o',
  O_PENCOLOR	=	'-',
  O_POINTREL	=	'P',
  O_RESTORESTATE=	'O',
  O_SAVESTATE	=	'U',
  O_SPACE2	=	':',
  O_TEXTANGLE	=	'R',

/* 25 floating point counterparts to some of the above.  Used only in
     binary GNU metafile format, not in portable GNU metafile format. */
  O_FARC	=	'1',
  O_FARCREL	=	'2',
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

/* 1 op code for a floating point operation with no integer counterpart
   [plus 3 obsolete ones] */
  O_FCONCAT	=	'\\',
  O_FROTATE	=	'V',	/* obsolete, to be removed */
  O_FSCALE	=	'X',	/* obsolete, to be removed */
  O_FTRANSLATE	=	'Q'	/* obsolete, to be removed */
};

#endif /* _PLOT_H_ */
