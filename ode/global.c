/*
 * definitions of global variables for ode.
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/* defined and initialized */

char    *progname	= "ode";
int	prec	= -1;
long	it	= 0;
double	hmin	= HMIN;
double	hmax	= HMAX;
double	ssmin	= 1e-11;
double	ssmax	= 1e-8;
double	abmin	= 1e-36;
double	abmax	= 1e36;
double	acmax	= 1e36;
struct	sym	*symtab = NULL;
struct	sym	*fsp	= NULL;
struct	sym	*dqueue	= NULL;
struct	prt	*pqueue = NULL;
struct	expr	exprzero  = 
{
  O_CONST,
  0.,
  NULL,
  NULL,
};
struct	expr	exprone = 
{
  O_CONST,
  1.,
  NULL,
  NULL,
};
integration_type algorithm = A_RUNGE_KUTTA_FEHLBERG;

/* defined but not initialized */

char	    *filename;
Boolean     printnum;
int         sawstep, sawprint, sawevery, sawfrom;
int         tflag, pflag, sflag, eflag, rflag, hflag, conflag;

jmp_buf mark;
int     prerr;
int	fwd;
int     tevery;
double  tstart, tstop, tstep, tfrom;
