#include <config.h>		/* built by autoconf */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#ifdef HAVE_VALUES_H
#include <values.h>		/* for MAXDOUBLE, MAXINT */
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>		/* for getenv, atoi, atof, etc. */
#include <string.h>		/* for memcpy, strchr etc. */

#else

/* functions in stdlib.h */
extern char *getenv();
extern int atoi();
extern double atof();

#ifdef HAVE_STRCHR
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#else  /* don't have strchr, prefer strings.h */
#ifdef HAVE_STRINGS_H
#include <strings.h>
#else
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#endif
#define strchr index
#define strrchr rindex
#endif /* HAVE_STRCHR */

#ifndef HAVE_MEMCPY
#define memcpy(d, s, n) bcopy ((s), (d), (n))
#endif

#ifndef HAVE_STRCASECMP		/* will use local version */
extern int strcasecmp();
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#else
extern void *malloc();
extern void *realloc();
extern void *calloc();
extern void free();
#endif /* HAVE_MALLOC_H */

#endif	/* STDC_HEADERS */

#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
          
/* Note: other Unix-specific files are #inc'd by libs/libplotps/closepl.c */

#define Boolean int	/* also defined in libs/libplotX/extern.h */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#if _STDC_
typedef void *Voidptr;
#else
#ifdef NOVOIDPTR		/* has not been tested */
typedef char *Voidptr;     
#else
typedef void *Voidptr;
#endif
#endif

#ifndef MAXINT
#define MAXINT ((int)(~(1U << (8 * (int)sizeof(int) - 1))))
#endif
#ifndef MAXDOUBLE
#ifdef HUGE
#define MAXDOUBLE HUGE
#else  /* make a very conservative (Vax-like) assumption */
#define MAXDOUBLE (1.701411834604692293e+38) 
#endif
#endif
#ifndef M_PI
#define M_PI        3.14159265358979323846264
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT3
#define M_SQRT3	    1.73205080756887719
#endif

/* gamma() and lgamma() both compute the log of the gamma function.  There are
   old systems out there which do not have lgamma (the name was introduced
   after BSD 4.2), but which do have gamma.  Also, Apollos have lgamma but not
   gamma. */
#ifdef HAVE_LGAMMA
#define F_LGAMMA lgamma
#else
#ifdef HAVE_GAMMA
#define F_LGAMMA gamma
#else
#define F_LGAMMA f_lgamma	/* our own version */
#define NO_GAMMA_SUPPORT
#endif
#endif

#ifdef __GNUC__
#define IMAX(a,b) ({int _a = (a), _b = (b); _a > _b ? _a : _b; })
#define IMIN(a,b) ({int _a = (a), _b = (b); _a < _b ? _a : _b; })
#define DMAX(a,b) ({double _a = (a), _b = (b); _a > _b ? _a : _b; })
#define DMIN(a,b) ({double _a = (a), _b = (b); _a < _b ? _a : _b; })
#define IROUND(x) ({double _x = (x); \
		      _x > 0.0 ? (int)(_x + 0.5) : (int)(_x - 0.5); })
#else
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define DMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ((a) < (b) ? (a) : (b))
#define DMIN(a,b) ((a) < (b) ? (a) : (b))
#define IROUND(x) ((int) ((x) > 0 ? (x) + 0.5 : (x) - 0.5))
#endif
