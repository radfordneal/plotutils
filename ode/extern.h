/*
 * declarations of external variables and functions for ode
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

/*
 * external variable declarations
 */

/* variables defined and initted in global.c */
extern char           *progname;
extern int            prec;
extern long           it;
extern double         hmin, hmax, ssmin, ssmax, abmin, abmax, acmax;
extern struct sym     *symtab, *fsp;
extern struct sym     *dqueue;
extern struct prt     *pqueue;
extern struct expr    exprzero, exprone;

/* variables defined but not initted in global.c */
extern char    *filename;
extern jmp_buf mark;
extern int     prerr;
extern int     sawstep, sawprint, sawevery, sawfrom;
extern int     printnum;
extern int     tflag, pflag, sflag, eflag, rflag, hflag, conflag;
extern int     fwd;
extern int     tevery;
extern double  tstart, tstop, tstep, tfrom;
extern integration_type	algorithm;

/* in parser */
extern FILE    *yyin;

/* in scanner */
extern int     curline;

/*
 * external function declarations
 */

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif

extern Boolean check P__((void));
extern Boolean hierror P__((void));
extern Boolean intpr P__((double t));
extern Boolean lowerror P__((void));
extern Boolean xatof P__((char *s, double *dval));
extern Boolean xatol P__((char *s, long int *ival));
extern Voidptr xmalloc P__((unsigned int length));
extern char * xstrdup P__((const char *s));
extern double eval P__((struct expr *ep));
extern void am P__((void));
extern void ama P__((void));
extern void args P__((int ac, char **av));
extern void defalt P__((void));
extern void eu P__((void));
extern void efree P__((struct expr *ep));
extern void field P__((void));
extern void maxerr P__((void));
extern void panic P__((char *s));
extern void panicn P__((char *fmt, int n));
extern void pfree P__((struct prt *pp));
extern void printq P__((void));
extern void prval P__((double x));
extern void maxerror P__((void));
extern void resetflt P__((void));
extern void rk P__((void));
extern void rka P__((void));
extern void rterror P__((char *s));
extern void rterrors P__((char *fmt, char *s));
extern void rtsquawks P__((char *fmt, char *s));
extern void setflt P__((void));
extern void sfree P__((struct sym *sp));
extern void solve P__((void));
extern void startstep P__((void));
extern void title P__((void));
extern void trivial P__((void));
extern struct expr * ealloc P__((void));
extern struct prt * palloc P__((void));
extern struct sym * lookup P__((char *nam));
extern struct sym * salloc P__((void));
extern RETSIGTYPE fptrap P__((int sig));

/* in scanner or parser */
extern int yyparse P__((void));
extern int yywrap P__((void));
extern struct lex * lalloc P__((void));
extern void concat P__((struct expr *e0, struct expr *e1));
extern void lfree P__((struct lex *lp));
extern void prexq P__((struct expr *ep));

/* math library exception handling */
#ifdef HAVE_MATHERR
extern int matherr P__((struct exception *x));
#endif

/* math functions in bessel.c and specfun.c */
#ifndef HAVE_J0
extern double j0 P__((double x));
extern double j1 P__((double x));
extern double y0 P__((double x));
extern double y1 P__((double x));
#endif
#ifdef NO_GAMMA_SUPPORT
extern double f_lgamma P__((double x));
#endif
extern double f_gamma P__((double x));
#ifndef HAVE_ERF
extern double erf P__((double x));
extern double erfc P__((double x));
#endif
extern double ibeta P__((double a, double b, double x));
extern double igamma P__((double a, double x));
extern double inverf P__((double p));
extern double invnorm P__((double p));
extern double norm P__((double x));

#undef P__
