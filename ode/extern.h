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
bool check ____P((void));
bool hierror ____P((void));
bool intpr ____P((double t));
bool lowerror ____P((void));
bool xatof ____P((char *s, double *dval));
bool xatol ____P((char *s, long int *ival));
double eval ____P((const struct expr *ep));
void am ____P((void));
void ama ____P((void));
void args ____P((int ac, char **av));
void defalt ____P((void));
void eu ____P((void));
void efree ____P((struct expr *ep));
void field ____P((void));
void maxerr ____P((void));
void panic ____P((const char *s));
void panicn ____P((const char *fmt, int n));
void pfree ____P((struct prt *pp));
void printq ____P((void));
void prval ____P((double x));
void maxerror ____P((void));
void resetflt ____P((void));
void rk ____P((void));
void rka ____P((void));
void rterror ____P((const char *s));
void rterrors ____P((const char *fmt, const char *s));
void rtsquawks ____P((const char *fmt, const char *s));
void setflt ____P((void));
void sfree ____P((struct sym *sp));
void solve ____P((void));
void startstep ____P((void));
void title ____P((void));
void trivial ____P((void));
struct expr * ealloc ____P((void));
struct prt * palloc ____P((void));
struct sym * lookup ____P((char *nam));
struct sym * salloc ____P((void));
RETSIGTYPE fptrap ____P((int sig));

/* in scanner or parser */
int yyerror ____P((const char *msg));
int yylex ____P((void));
int yyparse ____P((void));
struct lex * lalloc ____P((void));
void concat ____P((struct expr *e0, struct expr *e1));
void lfree ____P((struct lex *lp));
void prexq ____P((struct expr *ep));

/* math library exception handling */
#ifdef HAVE_MATHERR
int matherr ____P((struct exception *x));
#endif

/* math functions in bessel.c and specfun.c */
#ifndef HAVE_J0
double j0 ____P((double x));
double j1 ____P((double x));
double y0 ____P((double x));
double y1 ____P((double x));
#endif
#ifdef NO_SYSTEM_GAMMA
double f_lgamma ____P((double x));
#else  /* not NO_SYSTEM_GAMMA, we link in vendor code */
#ifdef HAVE_LGAMMA
extern double lgamma ____P((double x)); /* declaration may be gratuitous */
#endif
#ifdef HAVE_GAMMA
extern double gamma ____P((double x)); /* declaration may be gratuitous */
#endif
#endif
double f_gamma ____P((double x));
#ifndef HAVE_ERF
double erf ____P((double x));
double erfc ____P((double x));
#endif
double ibeta ____P((double a, double b, double x));
double igamma ____P((double a, double x));
double inverf ____P((double p));
double invnorm ____P((double p));
double norm ____P((double x));

/* in libcommon */
void display_usage ____P((const char *progname, const int *omit_vals, const char *appendage, bool fonts));
Voidptr xcalloc ____P ((unsigned int nmemb, unsigned int size));
Voidptr xmalloc ____P ((unsigned int size));
char *xstrdup ____P ((const char *s));

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
int yywrap ____P((void));
__END_DECLS     
