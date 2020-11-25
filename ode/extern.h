/*
 * declarations of external variables and functions for ode
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

/*
 * external variable declarations
 */

/* variables defined and initted in global.c */
extern const char     *progname;
extern int            prec;
extern long           it;
extern double         hmin, hmax, ssmin, ssmax, abmin, abmax, acmax;
extern struct sym     *symtab, *fsp;
extern struct sym     *dqueue;
extern struct prt     *pqueue;
extern struct expr    exprzero, exprone;
extern bool        sawstep, sawprint, sawevery, sawfrom;
extern bool        tflag, pflag, sflag, eflag, rflag, hflag, conflag;
extern integration_type	algorithm;

/* variables defined but not initted in global.c */
extern char    *filename;
extern jmp_buf mark;
extern int     fwd;
extern int     tevery;
extern double  tstart, tstop, tstep, tfrom;
extern bool printnum, prerr;

/* in parser */
extern FILE    *yyin;

/* in scanner */
extern int     curline;

/*
 * external function declarations
 */
bool check __P((void));
bool hierror __P((void));
bool intpr __P((double t));
bool lowerror __P((void));
bool xatof __P((char *s, double *dval));
bool xatol __P((char *s, long int *ival));
double eval __P((const struct expr *ep));
void am __P((void));
void ama __P((void));
void args __P((int ac, char **av));
void defalt __P((void));
void eu __P((void));
void efree __P((struct expr *ep));
void field __P((void));
void maxerr __P((void));
void panic __P((const char *s));
void panicn __P((const char *fmt, int n));
void pfree __P((struct prt *pp));
void printq __P((void));
void prval __P((double x));
void maxerror __P((void));
void resetflt __P((void));
void rk __P((void));
void rka __P((void));
void rterror __P((const char *s));
void rterrors __P((const char *fmt, const char *s));
void rtsquawks __P((const char *fmt, const char *s));
void setflt __P((void));
void sfree __P((struct sym *sp));
void solve __P((void));
void startstep __P((void));
void title __P((void));
void trivial __P((void));
struct expr * ealloc __P((void));
struct prt * palloc __P((void));
struct sym * lookup __P((char *nam));
struct sym * salloc __P((void));
RETSIGTYPE fptrap __P((int sig));

/* in scanner or parser */
int yyerror __P((const char *msg));
int yylex __P((void));
int yyparse __P((void));
struct lex * lalloc __P((void));
void concat __P((struct expr *e0, struct expr *e1));
void lfree __P((struct lex *lp));
void prexq __P((struct expr *ep));

/* math library exception handling */
#ifdef HAVE_MATHERR
int matherr __P((struct exception *x));
#endif

/* math functions in bessel.c and specfun.c */
#ifndef HAVE_J0
double j0 __P((double x));
double j1 __P((double x));
double y0 __P((double x));
double y1 __P((double x));
#endif
#ifdef NO_SYSTEM_GAMMA
double f_lgamma __P((double x));
#else  /* not NO_SYSTEM_GAMMA, we link in vendor code */
#ifdef HAVE_LGAMMA
extern double lgamma __P((double x)); /* declaration may be gratuitous */
#endif
#ifdef HAVE_GAMMA
extern double gamma __P((double x)); /* declaration may be gratuitous */
#endif
#endif
double f_gamma __P((double x));
#ifndef HAVE_ERF
double erf __P((double x));
double erfc __P((double x));
#endif
double ibeta __P((double a, double b, double x));
double igamma __P((double a, double x));
double inverf __P((double p));
double invnorm __P((double p));
double norm __P((double x));

/* in libcommon */
void display_usage __P((const char *progname, const int *omit_vals, bool files, bool fonts));
Voidptr xcalloc __P ((unsigned int nmemb, unsigned int size));
Voidptr xmalloc __P ((unsigned int size));
char *xstrdup __P ((const char *s));

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

__BEGIN_DECLS
int yywrap __P((void));
__END_DECLS     
