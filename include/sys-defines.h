#include <config.h>		/* built by autoconf */

#ifdef HAVE_VOID		/* defined in config.h  */
typedef void *Voidptr;
#else
#define NO_VOID_SUPPORT
typedef char *Voidptr;
#define void int
#endif /* HAVE_VOID */

#ifdef const			/* may be defined to empty in config.h */
#define NO_CONST_SUPPORT
#endif

/* __P is a macro used to wrap function prototypes, so that compilers that
   don't understand ANSI C prototypes still work, and ANSI C compilers can
   issue warnings about type mismatches. */
#ifdef __P
#undef __P
#endif
#ifdef _HAVE_PROTOS
#undef _HAVE_PROTOS
#endif
#if defined (__STDC__) || defined (_AIX) \
	|| (defined (__mips) && defined (_SYSTYPE_SVR4)) \
	|| defined(WIN32) || defined(__cplusplus)
#define __P(protos) protos
#define _HAVE_PROTOS 1
#else
#define __P(protos) ()
#endif

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
extern char *getenv __P((const char *name));
extern int atoi __P((const char *nptr));
extern double atof __P((const char *nptr));

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
extern int strcasecmp _P((const char *s1, const char *s2));
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#else
extern Voidptr malloc();
extern Voidptr realloc();
extern Voidptr calloc();
extern void free __P((void));
#endif /* HAVE_MALLOC_H */

#endif	/* STDC_HEADERS */

#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
          
#ifndef __cplusplus
#ifdef __STDC__
typedef enum { false = 0, true = 1 } bool;
#else
typedef int bool;
#define false 0
#define true 1
#endif
#endif
  
#ifndef MAXINT
#define MAXINT ((int)(~(1U << (8 * (int)sizeof(int) - 1))))
#endif
#ifndef MAXLONG
#define MAXLONG ((long int)(~(1U << (8 * (long int)sizeof(long int) - 1))))
#endif
#ifndef MAXDOUBLE   /* make a very conservative (Vax-like) assumption */
#define MAXDOUBLE   (1.701411834604692293e+38) 
#endif
#ifndef MAXFLOAT    /* make a very conservative (Vax-like) assumption */
#define MAXFLOAT    (1.7014117331926443e+38)
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

/* gamma() and lgamma() both compute the log of the gamma function.  There
   are old systems out there which do not have lgamma (the name was
   introduced after BSD 4.2), but which do have gamma.  Also, Apollos have
   lgamma but not gamma.  Some systems, old and new (e.g. cygwin32) have
   neither.  Also at least one vendor's gamma() is buggy, so we allow the
   installer to do -DNO_SYSTEM_GAMMA to prevent the use of vendor code. */
#ifdef _AIX
#define NO_SYSTEM_GAMMA	/* AIX gamma support in libm.a is buggy */
#endif
#ifdef NO_SYSTEM_GAMMA
#define F_LGAMMA f_lgamma	/* our own version, see ode/specfun.c */
#else  /* not NO_SYSTEM_GAMMA */
#ifdef HAVE_LGAMMA
#define F_LGAMMA lgamma
#else
#ifdef HAVE_GAMMA
#define F_LGAMMA gamma
#else
#define F_LGAMMA f_lgamma
#define NO_SYSTEM_GAMMA
#endif
#endif
#endif /* NO_SYSTEM_GAMMA */

#ifdef __GNUC__
#define IMAX(a,b) ({int _a = (a), _b = (b); _a > _b ? _a : _b; })
#define IMIN(a,b) ({int _a = (a), _b = (b); _a < _b ? _a : _b; })
#define DMAX(a,b) ({double _a = (a), _b = (b); _a > _b ? _a : _b; })
#define DMIN(a,b) ({double _a = (a), _b = (b); _a < _b ? _a : _b; })
#define IROUND(x) ({double _x = (x); int _i; \
                    if (_x >= MAXINT) _i = MAXINT; \
                    else if (_x <= -(MAXINT)) _i = -(MAXINT); \
                    else _i = (_x > 0.0 ? (int)(_x + 0.5) : (int)(_x - 0.5)); \
                    _i;})
#define FROUND(x) ({double _x = (x); float _f; \
                    if (_x >= MAXFLOAT) _f = MAXFLOAT; \
                    else if (_x <= -(MAXFLOAT)) _f = -(MAXFLOAT); \
                    else _f = _x; \
                    _f;})
#else
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define DMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ((a) < (b) ? (a) : (b))
#define DMIN(a,b) ((a) < (b) ? (a) : (b))
#define IROUND(x) ((int) ((x) > 0 ? (x) + 0.5 : (x) - 0.5))
#define FROUND(x) ((float)(x))
#endif
