#include <config.h>		/* built by autoconf */

#ifdef HAVE_VOID		/* defined in config.h  */
typedef void *Voidptr;
#else
#define NO_VOID_SUPPORT
typedef char *Voidptr;
#define void int
#endif /* not HAVE_VOID */

#ifdef const			/* may be defined to empty in config.h */
#define NO_CONST_SUPPORT
#endif

#ifdef __cplusplus
#ifdef RETSIGTYPE
#undef RETSIGTYPE		/* `configure' may get this wrong in C++ */
#endif
#define RETSIGTYPE void
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

#include <errno.h>
#ifndef HAVE_STRERROR
extern char * strerror __P((int errnum));
#endif

#ifdef __DJGPP__
/* must specify -lm will be used, according to mdruiter@cs.vu.nl */
#define _USE_LIBM_MATH_H
#endif
#include <math.h>

#ifdef HAVE_FLOAT_H
#include <float.h>		/* for DBL_MAX, FLT_MAX */
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>		/* for INT_MAX */
#endif
#ifdef HAVE_VALUES_H
#include <values.h>		/* for MAXDOUBLE, MAXFLOAT, MAXINT (backups) */
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>		/* for getenv, atoi, atof, etc. */
#include <string.h>		/* for memcpy, strchr etc. */

#else  /* not STC_HEADERS */

/* functions in stdlib.h */
extern char *getenv __P((const char *name));
extern int atoi __P((const char *nptr));
extern double atof __P((const char *nptr));

/* definitions in stdlib.h */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

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
#endif /* not HAVE_STRCHR */

#ifndef HAVE_MEMCPY
#define memcpy(d, s, n) bcopy ((s), (d), (n))
#endif

#ifndef HAVE_STRCASECMP		/* will use local version */
extern int strcasecmp __P((const char *s1, const char *s2));
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#else
extern Voidptr malloc();
extern Voidptr realloc();
extern Voidptr calloc();
extern void free __P((void));
#endif /* not HAVE_MALLOC_H */

#endif /* not STC_HEADERS */

#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
          
/* we are logical */
#ifndef __cplusplus
#ifdef __STDC__
typedef enum { false = 0, true = 1 } bool;
#else
typedef int bool;
#define false 0
#define true 1
#endif
#endif
  
#ifndef INT_MAX
#ifdef MAXINT
#define INT_MAX MAXINT
#else
#define INT_MAX ((int)(~(1U << (8 * (int)sizeof(int) - 1))))
#endif
#endif /* not INT_MAX */

#ifndef DBL_MAX
#ifdef MAXDOUBLE
#define DBL_MAX MAXDOUBLE
#else
/* make a very conservative (Vax-like) assumption */
#define DBL_MAX (1.701411834604692293e+38) 
#endif
#endif /* not DBL_MAX */

#ifndef FLT_MAX
#ifdef MAXFLOAT
#define FLT_MAX MAXFLOAT
#else
/* make a very conservative (Vax-like) assumption */
#define FLT_MAX (1.7014117331926443e+38)
#endif
#endif /* not FLT_MAX */

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
#define NO_SYSTEM_GAMMA		/* AIX gamma support in libm.a is buggy */
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
#endif /* not NO_SYSTEM_GAMMA */

/* IBM's definition of INT_MAX is bizarre, in AIX 4.1 at least, and using
   IROUND() below will yield a warning message unless we repair it */
#ifdef _AIX
#ifdef __GNUC__
#undef INT_MAX
#define INT_MAX ((int)(~(1U << (8 * (int)sizeof(int) - 1))))
#endif
#endif

#ifdef __GNUC__
#define IMAX(a,b) ({int _a = (a), _b = (b); _a > _b ? _a : _b; })
#define IMIN(a,b) ({int _a = (a), _b = (b); _a < _b ? _a : _b; })
#define DMAX(a,b) ({double _a = (a), _b = (b); _a > _b ? _a : _b; })
#define DMIN(a,b) ({double _a = (a), _b = (b); _a < _b ? _a : _b; })
#define IROUND(x) ({double _x = (x); int _i; \
                    if (_x >= INT_MAX) _i = INT_MAX; \
                    else if (_x <= -(INT_MAX)) _i = -(INT_MAX); \
                    else _i = (_x > 0.0 ? (int)(_x + 0.5) : (int)(_x - 0.5)); \
                    _i;})
#define FROUND(x) ({double _x = (x); float _f; \
                    if (_x >= FLT_MAX) _f = FLT_MAX; \
                    else if (_x <= -(FLT_MAX)) _f = -(FLT_MAX); \
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
