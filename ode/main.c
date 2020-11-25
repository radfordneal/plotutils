/*
 * main() routine for ode, including command-line parser.
 * Copyright (C) 1996-1997 Free Software Foundation, Inc.
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"
#include "getopt.h"

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] =
{
  {"absolute-error-bound",	ARG_REQUIRED,	NULL, 'e'}, /* 1 or 2 */
  {"input-file",		ARG_REQUIRED,	NULL, 'f'},
  {"step-size-bound",		ARG_REQUIRED,	NULL, 'h'}, /* 1 or 2 */
  {"precision",			ARG_REQUIRED,	NULL, 'p'},
  {"relative-error-bound",	ARG_REQUIRED,	NULL, 'r'}, /* 1 or 2 */
  {"suppress-error-bound",	ARG_NONE,	NULL, 's'},
  {"title",			ARG_NONE,	NULL, 't'},
  {"adams-moulton",		ARG_OPTIONAL,	NULL, 'A'}, /* 0 or 1 */
  {"euler",			ARG_OPTIONAL,	NULL, 'E'}, /* 0 or 1 */
  {"runge-kutta",		ARG_OPTIONAL,	NULL, 'R'}, /* 0 or 1 */
  /* Long options with no equivalent short option alias */
  {"version",			ARG_NONE,	NULL, 'V' << 8},
  {"help",			ARG_NONE,	NULL, 'h' << 8},
  {NULL, 0, 0, 0}
};

#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
void display_usage P__((void));
void display_version P__((void));
void fatal P__((char *s));
#undef P__

void
display_usage ()
{
  int i;
  int col = 0;
  
  fprintf (stderr, "Usage: %s", progname);
  col += (strlen (progname) + 7);
  for (i = 0; long_options[i].name; i++)
    {
      int option_len;
      
      option_len = strlen (long_options[i].name);
      if (col >= 80 - (option_len + 16))
	{
	  fprintf (stderr, "\n\t");
	  col = 8;
	}
      fprintf (stderr, " [--%s", long_options[i].name);
      col += (option_len + 4);
      if ((unsigned int)(long_options[i].val) < 256)
	{
	  fprintf (stderr, " | -%c", long_options[i].val);
	  col += 5;
	}
      if (long_options[i].has_arg == ARG_REQUIRED)
	{
	  fprintf (stderr, " arg]");
	  col += 5;
	}
      else if (long_options[i].has_arg == ARG_OPTIONAL)
	{
	  fprintf (stderr, " [arg]]");
	  col += 10;
	}
      else
	{
	  fprintf (stderr, "]");
	  col++;
	}
    }
  fprintf (stderr, "\n");
}

void
display_version ()
{
  fprintf (stderr, "%s (GNU plotutils) %s\n", progname, VERSION);
  fprintf (stderr, 
	   "Copyright (C) 1982-97 Free Software Foundation, Inc.,\nand Nicholas B. Tufillaro.\n");
  fprintf (stderr, 
	   "The GNU plotutils come with NO WARRANTY, to the extent permitted by law.\n");
  fprintf (stderr, "You may redistribute copies of the GNU plotutils\n");
  fprintf (stderr, "under the terms of the GNU General Public License.\n");
}

/*
 * fatal error message
 */
void
fatal (s)
     char *s;
{
  fprintf (stderr, "%s: %s\n", progname, s);
  exit (1);
}

int
main (argc, argv)
     int argc;
     char *argv[];
{
  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered */
  Boolean show_version = FALSE;	/* remember to show version message */
  Boolean show_usage = FALSE;	/* remember whether to output usage message. */
  double local_tstep, local_hmax;
  FILE *infile = NULL;

  while (TRUE)
    {
      option = getopt_long (argc, argv, "e:f:h:p:r:stA::E::R::V", long_options, &opt_index);
      if (option == 0)
	option = long_options[opt_index].val;

      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 's':		/* Suppress error bound, ARG NONE */
	  sflag = 1;
	  break;
	case 't':		/* Title, ARG NONE		*/
	  tflag = 1;
	  if (!pflag) 
	    {
	      prec = 6;
	      fwd = 13;
	    }
	  break;
	case 'V' << 8:		/* Version, ARG NONE		*/
	  show_version = TRUE;
	  break;
	case 'h' << 8:		/* Help, ARG NONE		*/
	  show_usage = TRUE;
	  break;

	  /*----------- options with a single argument --------------*/

	case 'f':		/* File name, ARG REQUIRED	*/
	  filename = xstrdup (optarg);
	  break;
	case 'p':		/* Precision, ARG REQUIRED 	*/
	  pflag = 1;
	  if (sscanf (optarg, "%d", &prec) <= 0)
	    fatal ("-p: bad argument");
	  prec--;
	  if (prec <= 0 || prec > 18)
	    fatal ("-p: argument out of range");
	  fwd = prec + 7;
	  if (fwd < 9)
	    fwd = 9;
	  break;

	  /*----------- options with 0 or 1 arguments --------------*/

	case 'A':		/* Adams-Moulton */
	  algorithm = A_ADAMS_MOULTON;
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv[optind], "%lf", &local_tstep) <= 0)
	    break;
	  tstep = local_tstep;
	  optind++;	/* tell getopt we recognized timestep */
	  conflag = 1;
	  break;
	case 'E':		/* Euler */
	  algorithm = A_EULER;
	  conflag = 1;
	  tstep = 0.1;
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv[optind], "%lf", &local_tstep) <= 0)
	    break;
	  tstep = local_tstep;
	  optind++;	/* tell getopt we recognized timestep */
	  break;
	case 'R':		/* Runge-Kutta-Fehlberg */
	  algorithm = A_RUNGE_KUTTA_FEHLBERG;
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv[optind], "%lf", &local_tstep) <= 0)
	    break;
	  tstep = local_tstep;
	  optind++;	/* tell getopt we recognized timestep */
	  conflag = 1;
	  break;

	  /*----------- options with 1 or 2 arguments --------------*/

	case 'h':		/* Step Size Bound(s) */
	  if (sscanf (optarg, "%lf", &hmin) <= 0)
	    fatal ("-h: bad argument");
	  if (hmin < HMIN)
	    fatal ("-h: value too small");
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv [optind], "%lf", &local_hmax) <= 0)
	    break;
	  hmax = local_hmax;
	  optind++;	/* tell getopt we recognized hmax */
	  hflag = 1;
	  break;

	case 'r':		/* Relative Error Bound(s) */
	  rflag = 1;
	  if (sscanf (optarg, "%lf", &ssmax) <= 0)
	    fatal ("-r: bad argument");
	  if (ssmax < HMIN)
	    fatal ("-r: max value too small");
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv [optind], "%lf", &ssmin) <= 0)
	    {
	      ssmin = ssmax * SCALE;	      
	      break;
	    }
	  optind++;	/* tell getopt we recognized ssmin */
	  break;

	case 'e':		/* Absolute Error Bound(s) */
	  eflag = 1;
	  if (sscanf (optarg, "%lf", &abmax) <= 0)
	    fatal ("-e: bad argument");
	  if (abmax < HMIN)
	    fatal ("-e: max value too small");
	  if (optind >= argc)
	  /* try to parse next arg as a float */
	    break;
	  if (sscanf (argv [optind], "%lf", &abmin) <= 0)
	    {
	      abmin = abmax * SCALE;
	      break;
	    }
	  optind++;	/* tell getopt we recognized abmin */
	  break;

	  /*---------------- End of options ----------------*/

	default:		/* Default, unknown option */
	  errcnt++;
	  break;
	}			/* endswitch */

      if ((option == EOF))
	{
	  errcnt--;
	  break;		/* break out of option processing */
	}
    }				/* endwhile */

  if (optind < argc)		/* too many arguments */
    {
      fprintf (stderr, "%s: too many arguments\n", progname);
      errcnt++;
    }
  
  if (errcnt > 0)
    {
      fprintf (stderr, "Try `%s --help' for more information.\n", progname);
      return 1;
    }
  if (show_version)
    {
      display_version ();
      return 0;
    }
  if (show_usage)
    {
      display_usage ();
      return 0;
    }

  /* Some sanity checks on user-supplied options. */

  if (algorithm == A_EULER && (eflag || rflag))
    fatal ("-E [Euler] illegal with -e or -r");

  /* DO IT */

  if (filename != NULL)
    {
      infile = fopen (filename, "r");
      if (infile == NULL)
	{
	  fprintf (stderr, "%s: ", progname);
	  perror (filename);
	  return 1;
	}
      yyin = infile;
      /* will switch later to stdin, in yywrap() */
    }
  else
    {
      yyin = stdin;
      filename = "";
    }
  
  yyparse();
  return 0;
}
