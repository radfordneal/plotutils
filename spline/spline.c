/* This program, spline, interpolates input data using splines with
   tension, including piecewise cubic (zero-tension) splines.  When acting
   as a real-time filter, it uses cubic Bessel interpolation instead.
   Written by Robert S. Maier <rsm@math.arizona.edu>, based on earlier work
   by Rich Murphey.  Copyright (C) 1989-1997 Free Software Foundation, Inc.

   References:

   D. Kincaid and [E.] W. Cheney [Jr.], Numerical Analysis, 2nd. ed., 1996,
   Section 6.4.

   C. de Boor, A Practical Guide to Splines, 1978, Chapter 4.

   The tension in a spline is set with the -t (i.e., --tension) option.  By
   definition, a spline with tension satisfies the differential equation
   y''''=sgn(tension)*(tension**2)y''.  The default value for the tension
   is zero.  If tension=0 then a spline with tension reduces to a
   conventional piecewise cubic spline.  In the limits tension->+infinity
   and tension->-infinity, a spline with tension reduces to a piecewise
   linear (`broken line') interpolation.

   To oversimplify a bit, 1.0/tension is the maximum abscissa range over
   which the spline likes to curve, at least when tension>0.  So increasing
   the tension far above zero tends to make the spline contain short curved
   sections, separated by sections that are almost straight.  The curved
   sections will be centered on the user-specified data points.  The
   behavior of the spline when tension<0 is altogether different: it will
   tend to oscillate, though as tension->-infinity the oscillations are
   damped out.

   Tension is a `dimensionful' quantity.  If tension=0 (the cubic spline
   case), then the computation of the spline is scale-invariant.  But if
   the tension is nonzero, then when the abscissa values are multiplied by
   some common positive factor, the tension should be divided by the same
   factor to obtain a scaled version of the original spline.

   The algorithms of Kincaid and Cheney have been extended to include
   support for periodicity.  To obtain a periodic spline, with or without
   tension, the user uses the -p (i.e., --periodic) option and supplies
   input data satisfying y[n]=y[0].  Also, in the non-periodic case the
   algorithms have been extended to include support for a parameter k,
   which appears in the two boundary conditions y''[0]=ky''[1] and
   y''[n]=ky''[n-1].  The default value of k is 1.0.  The parameter k,
   which is specified with the -k (i.e. --boundary-condition) option, is
   ignored for periodic splines (using the -k option with the -p option
   will elicit a warning).

   If the -f option is specified, then an altogether different (real-time)
   algorithm for generating interpolating points will be used, so that this
   program can be used as a real-time filter.  If -f is specified then the
   -x option, otherwise optional, must also be used.  (I.e., the minimum
   and maximum abscissa values for the interpolating points must be
   specified, and optionally the spacing between them as well.  If the
   spacing is not specified on the command line, then the interval
   [xmin,xmax] will be subdivided into a default number of intervals [100],
   unless the default number of intervals is overridden with the -n option.

   The real-time algorithm that is used when the -f option is specified is
   cubic Bessel interpolation.  (The -t, -p, and -k options are ignored
   when -f is specified; using them will elicit a warning.)  Interpolation
   in this case is piecewise cubic, and the slopes at either end of each
   sub-interval are found by fitting a parabola through each successive
   triple of points.  That is, the slope at x_n is found by fitting a
   parabola through x_(n-1), x_n, and x_(n+1).  This interpolation scheme
   yields a spline that is only once, rather than twice, continuously
   differentiable.  However, it has the feature that all computations are
   local rather than global, so it is suitable for real-time work. */

#include "sys-defines.h"
#include "getopt.h"

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

/* states for cubic Bessel DFA; occupancy of data point queue */
enum 
{
  STATE_ZERO, STATE_ONE, STATE_TWO, STATE_THREE
};

#define FUZZ 0.0000001		/* potential roundoff error */

struct option long_options[] =
{
  {"auto-abscissa",	ARG_OPTIONAL,	NULL, 'a'}, /* 0 or 1 or 2 */
  {"boundary-condition",ARG_REQUIRED,	NULL, 'k'},
  {"tension",		ARG_REQUIRED, 	NULL, 't'},
  {"no-of-intervals",	ARG_REQUIRED,	NULL, 'n'},
  {"filter",		ARG_NONE,	NULL, 'f'},
  {"periodic",		ARG_NONE,	NULL, 'p'},
  {"x-limits",		ARG_REQUIRED,	NULL, 'x'}, /* 1 or 2 or 3 */
  {"precision",		ARG_REQUIRED,	NULL, 'P'},
  /* ascii or double */
  {"input-type",	ARG_REQUIRED,	NULL, 'I'},
  {"output-type",	ARG_REQUIRED,	NULL, 'O'},
  /* Long options with no equivalent short option alias */
  {"version",		ARG_NONE,	NULL, 'V' << 8},
  {"help",		ARG_NONE,	NULL, 'h' << 8},
  {NULL, 		0, 		0,     0}
};

typedef enum
{
  T_DOUBLE, T_ASCII
}
data_type;

data_type input_type = T_ASCII;
data_type output_type = T_ASCII;

char *progname = "spline";	/* Program name */

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
Boolean do_bessel P__ ((FILE *input, Boolean auto_abscissa, double *auto_x, double auto_delta, double first_x, double last_x, double spacing_x, int precision));
Boolean is_monotonic P__ ((int n, double *x));
Boolean read_data P__ ((FILE *input, int *len, int *used, Boolean auto_abscissa, double *auto_x, double auto_delta, double **t, double **y, double **z));
Boolean read_float P__((FILE *input, double *dptr));
Boolean skip_whitespace P__ ((FILE *stream));
Boolean write_floats P__((double x, double y, int precision));
Voidptr xmalloc P__ ((unsigned int length));
Voidptr xrealloc P__ ((Voidptr p, unsigned int length));
double interpolate P__ ((int n, double *t, double *y, double *z, double x, double tension, Boolean periodic));
int get_point P__ ((FILE *input, double *t, double *y, Boolean auto_abscissa, double *auto_x, double auto_delta));
void display_usage P__ ((void));
void display_version P__ ((void));
void do_bessel_range P__ ((double abscissa0, double abscissa1, double value0, double value1, double slope0, double slope1, double first_x, double last_x, double spacing_x, int precision, Boolean endit));
void do_spline P__ ((int used, int len, double **t, double **y, double **z, double tension, Boolean periodic, Boolean use_boundary_condition, double boundary_condition, int precision, double first_x, double last_x, double spacing_x, int no_of_intervals, Boolean spec_first_x, Boolean spec_last_x, Boolean spec_spacing_x, Boolean spec_no_of_intervals));
void fit P__ ((int n, double *t, double *y, double *z, double k, double tension, Boolean periodic));
void non_monotonic_error P__((void));
void output_dataset_separator P__ ((void));
void set_format_type P__ ((char *s, data_type *typep));
#undef P__


int
main (argc, argv)
     int argc;
     char *argv[];
{
  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered */
  Boolean show_version = FALSE;	/* remember to show version message */
  Boolean show_usage = FALSE;	/* remember to output usage message */

  /* parameters controlled by command line options: */
  Boolean auto_abscissa = FALSE; /* automatic generation of abscissa? */
  double x_start = 0.0;		/* start of auto abscissa */
  double delta_x = 1.0;		/* increment of auto abscissa */
  double running_x;		/* for storing auto abscissa */
  double local_x_start, local_delta_x;
  Boolean use_boundary_condition = FALSE;
  double boundary_condition = 1.0; /* force  y''_1 = k * y''_0, etc. */
  double tension = 0.0;
  Boolean filter = FALSE;	/* whether to act as a filter (cubic Bessel) */
  Boolean periodic = FALSE;	/* whether spline should be periodic */
  int precision = 6;		/* default no. of significant digits printed */
  int local_precision;
  double first_x = 0.0, last_x = 0.0, spacing_x = 0.0; /* values of limits */
  double local_first_x, local_last_x, local_spacing_x;
  Boolean spec_first_x = FALSE, spec_last_x = FALSE, spec_spacing_x = FALSE;
  Boolean spec_no_of_intervals = FALSE;
  int no_of_intervals = 100;	/* num. of intervals to divide x range into */
  Boolean more_points;
							  
  while (TRUE)
    {
      option = getopt_long (argc, argv, "fpI:O:P:k:n:x:t:a::", long_options, &opt_index);
      if (option == 0)
	option = long_options[opt_index].val;
      
      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 'p':		/* construct periodic, i.e., closed spline */
	  periodic = TRUE;
	  break;
	case 'f':		/* act as filter */
	  filter = TRUE;
	  break;
	case 'V' << 8:		/* Version */
	  show_version = TRUE;
	  break;
	case 'h' << 8:		/* Help */
	  show_usage = TRUE;
	  break;

	  /*--------------options with a single argument--------*/

	case 'I':
	  set_format_type (optarg, &input_type);
	  break;
	case 'O':
	  set_format_type (optarg, &output_type);
	  break;
	case 'k':
	  if (sscanf (optarg, "%lf", &boundary_condition) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: bad boundary condition argument `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  use_boundary_condition = TRUE;
	  break;
	case 't':
	  if (sscanf (optarg, "%lf", &tension) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: bad boundary condition argument `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'n':		/* number of intervals */
	  if (sscanf (optarg, "%d", &no_of_intervals) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: bad requested number of intervals `%s'\n", 
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    spec_no_of_intervals = TRUE;
	  break;
	case 'P':		/* precision */
	  if (sscanf (optarg, "%d", &local_precision) <= 0)
	    {
	      fprintf (stderr, "%s: error: bad requested precision `%s'\n", 
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    {
	      if (local_precision < 0)
		fprintf (stderr, 
			 "%s: warning: ignoring bad precision value `%s' (must be nonnegative)\n",
			 progname, optarg);
	      else
		precision = local_precision;
	    }
	  break;

	  /*------------options with 0 or more args ----------*/

	case 'a':		/* Auto-abscissa, ARG OPTIONAL [0,1,2] */
	  auto_abscissa = TRUE;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_delta_x) <= 0)
	    break;
	  delta_x = local_delta_x;
	  optind++;	/* tell getopt we recognized delta_x */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%lf", &local_x_start) <= 0)
	    break;
	  x_start = local_x_start;
	  optind++;	/* tell getopt we recognized x_start */
	  break;

	  /*--------------options with 1 or more arguments------*/

	case 'x':		/* X axis limits, ARG REQUIRED [1,2,3] */
	  if (sscanf (optarg, "%lf", &local_first_x) <= 0)
	    break;
	  first_x = local_first_x;
	  spec_first_x++;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%lf", &local_last_x) <= 0)
	    break;
	  last_x = local_last_x;
	  spec_last_x++;
	  optind++;	/* tell getopt we recognized last_x */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%lf", &local_spacing_x) <= 0)
	    break;
	  spacing_x = local_spacing_x;
	  spec_spacing_x++;
	  optind++;	/* tell getopt we recognized spacing_x */
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
    }
				/* endwhile */
  if (errcnt > 0)
    {
      fprintf (stderr, "Try `%s --help' for more information.\n", progname);
      return 0;
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

  if (no_of_intervals < 1)
    {
      fprintf (stderr, 
	       "%s: error: cannot subdivide an interval into %d subintervals\n", 
	       progname, no_of_intervals);
      return 1;
    }

  if (periodic)
    {
      if (use_boundary_condition)
	fprintf (stderr, 
		 "%s: warning: for periodic splines, boundary condition not supported\n", 
		 progname);
      boundary_condition = 0.0;
    }

  if (filter)
    /* acting as a filter, so use cubic Bessel interpolation */
    {
      if (!spec_first_x || !spec_last_x)
	{
	  fprintf (stderr,
		   "%s: error: acting as filter, must supply first x and last x\n",
		   progname);
	  return 1;
	}

      if (!spec_spacing_x) 
	spacing_x = (last_x - first_x) / no_of_intervals;
      else			/* user specified spacing */
	{
	  if (spec_no_of_intervals)
	    fprintf (stderr, "%s: warning: requested number of intervals ignored\n",
		     progname);
	  if ((last_x - first_x) * spacing_x < 0.0)
	    {
	      fprintf (stderr, "%s: warning: spacing of wrong sign, corrected\n",
		       progname);
	      spacing_x = -spacing_x;
	    }

	  /* N.B. if spacing specified, should optionally contract first_x and
	     last_x to make them integer multiples of spacing; cf. graph */
	}
      
      if (use_boundary_condition)
	fprintf (stderr, 
		 "%s: warning: acting as filter, boundary condition not supported\n",
		 progname);
      if (tension != 0.0)
	fprintf (stderr, 
		 "%s: warning: acting as filter, nonzero tension not supported\n",
		 progname);
      if (periodic)
	fprintf (stderr, 
		 "%s: warning: acting as filter, periodicity not supported\n",
		 progname);

      if (optind < argc)
	{
	  /* call do_bessel() on each file specified on the command line,
	     generating a spline from each dataset in the file */
	  for (; optind < argc; optind++)
	    {
	      FILE *data_file;
	      
	      data_file = fopen (argv[optind], "r");
	      if (data_file == NULL)
		{
		  fprintf (stderr, "%s: error: couldn't open file `%s'\n",
			   progname, argv[optind]);
		  return 1;
		}

	      /* loop through datasets in file (may be more than one) */
	      do
		{
		  running_x = x_start;	/* reset auto-abscissa */
		  more_points = do_bessel (data_file, 
					   auto_abscissa, &running_x, delta_x,
					   first_x, last_x, spacing_x, 
					   precision);
		  /* output a separator between successive splines,
		     i.e. between successive interpolated datasets */
		  if (more_points || (optind + 1 != argc))
		    output_dataset_separator();
		  
		} while (more_points);

	      if (fclose (data_file) < 0)
		{
		  fprintf (stderr, 
			   "%s: error: couldn't close file `%s'\n",
			   progname, argv[optind]);
		  return 1;
		}
	    }
	}
      else			/* no files spec'd, read stdin instead */
	/* loop through datasets read from stdin (may be more than one) */
	do
	  {
	    running_x = x_start;	/* reset auto-abscissa */
	    more_points = do_bessel (stdin, 
				     auto_abscissa, &running_x, delta_x,
				     first_x, last_x, spacing_x, precision);
	    
	    /* output a separator between successive splines
	       i.e. between successive interpolated datasets */
	    if (more_points)
	      output_dataset_separator();
	  }
	while (more_points);	/* keep going if no EOF yet */
    }

  else
    /* not acting as filter, so use spline interpolation (w/ tension) */
    {
      double *t, *y, *z;	/* ordinate, abscissa, 2nd derivative arrays */
      int len, used;

      if (optind < argc)	/* files spec'd on command line */
	{

	  /* call do_spline() on each file specified on the command line,
	     generating a spline from each dataset contained in the file */
	  for (; optind < argc; optind++)
	    {
	      FILE *data_file;
	      
	      /* loop through datasets in file (may be more than one) */
	      do
		{
		  len = 16;	/* initial value of storage length */
		  used = -1;	/* initial value of array size, minus 1 */
	      
		  t = (double *)xmalloc (sizeof(double) * len);
		  y = (double *)xmalloc (sizeof(double) * len);
		  z = (double *)xmalloc (sizeof(double) * len);
		  
		  running_x = x_start; /* reset auto-abscissa */
		  
		  data_file = fopen (argv[optind], "r");
		  if (data_file == NULL)
		    {
		      fprintf (stderr, "%s: error: couldn't open file `%s'\n",
			       progname, argv[optind]);
		      return 1;
		    }
		  more_points = read_data (data_file, &len, &used, 
					   auto_abscissa, &running_x, delta_x,
					   &t, &y, &z);
		  /* read_data() may reallocate t,y,z, and update len, used;
		     used + 1 is number of data points read in */

		  if (fclose (data_file) < 0)
		    {
		      fprintf (stderr, 
			       "%s: error: couldn't close file `%s'\n",
			       progname, argv[optind]);
		      return 1;
		    }
		  do_spline (used, len, 
			     &t, &y, &z, tension, periodic,
			     use_boundary_condition, boundary_condition, 
			     precision,
			     first_x, last_x, spacing_x, no_of_intervals,
			     spec_first_x, spec_last_x, spec_spacing_x, 
			     spec_no_of_intervals);

		  /* output a separator between successive splines */
		  if (more_points || (optind + 1 != argc))
		    output_dataset_separator();
		  
		  free (t);
		  free (y);
		  free (z);
		}
	      while (more_points);	/* keep going if no EOF yet */
	      
	    }
	}
      else			/* no files spec'd, read stdin instead */
	/* loop through datasets read from stdin (may be more than one) */
	do
	  {
	    len = 16;		/* initial value for array size */
	    used = -1;	/* initial number of stored points, minus 1 */
	    
	    t = (double *)xmalloc (sizeof(double) * len);
	    y = (double *)xmalloc (sizeof(double) * len);
	    z = (double *)xmalloc (sizeof(double) * len);
	    
	    running_x = x_start;	/* reset auto_abscissa */
	    more_points = read_data (stdin, &len, &used, 
				     auto_abscissa, &running_x, delta_x, 
				     &t, &y, &z);
	    /* after calling read_data(), used+1 is number of data points */
	    
	    do_spline (used, len, 
		       &t, &y, &z, tension, periodic,
		       use_boundary_condition, boundary_condition, precision,
		       first_x, last_x, spacing_x, no_of_intervals,
		       spec_first_x, spec_last_x, spec_spacing_x, 
		       spec_no_of_intervals);
	    
	    /* output a separator between successive splines */
	    if (more_points)
	      output_dataset_separator();
	    
	    free (t);
	    free (y);
	    free (z);
	  }
	while (more_points);	/* keep going if no EOF yet */
      
    }

  return 0;
}


void
display_version ()
{
  fprintf (stderr, "%s (GNU plotutils) %s\n", progname, VERSION);
  fprintf (stderr, "Copyright (C) 1997 Free Software Foundation, Inc.\n");
  fprintf (stderr, 
	   "The GNU plotutils come with NO WARRANTY, to the extent permitted by law.\n");
  fprintf (stderr, "You may redistribute copies of the GNU plotutils\n");
  fprintf (stderr, "under the terms of the GNU General Public License.\n");
}

void
display_usage ()
{
  int i;
  int col = 0;
  
  fprintf (stderr, "Usage: %s", progname);
  col += (strlen (progname) + 7);
  for (i = 0; long_options[i].name; i++)
    {
      int option_len = strlen (long_options[i].name);

      if (col >= 80 - (option_len + 14))
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
      else
	{
	  fprintf (stderr, "]");
	  col++;
	}
    }
  fprintf (stderr, "\n");
}


void
set_format_type (s, typep)
     char *s;
     data_type *typep;
{
  switch (s[0])
    {
    case 'a':
    case 'A':
      *typep = T_ASCII;
      break;
    case 'd':
    case 'D':
      *typep = T_DOUBLE;
      break;
    default:
      {
	fprintf (stderr, "%s: error: invalid data format type `%s'\n",
		 progname, s);
	exit (1);
      }
      break;
    }
}

Voidptr 
xmalloc (length)
     unsigned int length;
{
  Voidptr p;
  p = (Voidptr) malloc (length);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "%s: ", progname);
      perror ("malloc failed");
      exit (1);
    }
  return p;
}

Voidptr 
xrealloc (p, length)
     Voidptr p;
     unsigned int length;
{
  p = (Voidptr) realloc (p, length);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "%s: ", progname);
      perror ("realloc failed");
      exit (1);
    }
  return p;
}


/* fit() computes the array z[] of second derivatives at the knots, i.e.,
   internal data points.  The abscissa array t[] and the ordinate array y[]
   are specified.  On entry, have n+1 >= 2 points in the t, y, z arrays,
   numbered 0..n.  The knots are numbered 1..n-1 as in Kincaid and Cheney.
   In the periodic case, the final knot, i.e., (t[n-1],y[n-1]), has the
   property that y[n-1]=y[0]; moreover, y[n]=y[1].  The number of points
   supplied by the user was n+1 in the non-periodic case, and n in the
   periodic case.  When this function is called, n>=1 in the non-periodic
   case, and n>=2 in the periodic case. */

/* Algorithm: the n-1 by n-1 tridiagonal matrix equation for the vector of
   2nd derivatives at the knots is reduced to upper diagonal form.  At that
   point the diagonal entries (pivots) of the upper diagonal matrix are in
   the vector u[], and the vector on the right-hand side is v[].  That is,
   the equation is of the form Ay'' = v, where a_(ii) = u[i], and a_(i,i+1)
   = alpha[i].  Here i=1..n-1 indexes the set of knots.  The matrix
   equation is solved by back-substitution. */

void
fit (n, t, y, z, k, tension, periodic)
     int n;
     double *t, *y, *z;
     double k;			/* y''_1 = k y''_0, etc. */
     double tension;
     Boolean periodic;
{
  double *h, *b, *u, *v, *alpha, *beta;
  double *uu = NULL, *vv = NULL, *s = NULL;
  int i;

  if (n == 1)			/* exactly 2 points, use straight line */
    {
      z[0] = z[1] = 0.0;
      return;
    }

  h = (double *)xmalloc (sizeof(double) * n);
  b = (double *)xmalloc (sizeof(double) * n);
  u = (double *)xmalloc (sizeof(double) * n);
  v = (double *)xmalloc (sizeof(double) * n);
  alpha = (double *)xmalloc (sizeof(double) * n);
  beta = (double *)xmalloc (sizeof(double) * n);

  if (periodic)
    {
      s = (double *)xmalloc (sizeof(double) * n); 
      uu = (double *)xmalloc (sizeof(double) * n); 
      vv = (double *)xmalloc (sizeof(double) * n); 
    }

  for (i = 0; i <= n - 1 ; ++i)
    {
      h[i] = t[i + 1] - t[i];
      b[i] = 6.0 * (y[i + 1] - y[i]) / h[i]; /* for computing RHS */
    }

  if (tension < 0.0)		/* must rule out sin(tension * h[i]) = 0 */
    {
      for (i = 0; i <= n - 1 ; ++i)
	if (sin (tension * h[i]) == 0.0)
	  {
	    fprintf (stderr, "%s: error: tension value is singular\n", progname);
	    exit (1);
	  }
    }
  if (tension == 0.0)
    {
      for (i = 0; i <= n - 1 ; ++i)
	{
	  alpha[i] = h[i];	/* off-diagonal = alpha[i] to right */
	  beta[i] = 2.0 * h[i];	/* diagonal = beta[i-1] + beta[i] */
	}
    }
  else
    if (tension > 0.0)
      {
	for (i = 0; i <= n - 1 ; ++i)
	  {
	    alpha[i] = ((6.0 / (tension * tension))
			* ((1.0 / h[i]) - tension / sinh (tension * h[i])));
	    beta[i] = ((6.0 / (tension * tension))
		       * (tension / tanh (tension * h[i]) - (1.0 / h[i])));
	  }
      }
    else				/* parameter < 0 */
      {
	for (i = 0; i <= n - 1 ; ++i)
	  {
	    alpha[i] = ((6.0 / (tension * tension))
			* ((1.0 / h[i]) - tension / sin (tension * h[i])));
	    beta[i] = ((6.0 / (tension * tension))
		       * (tension / tan (tension * h[i]) - (1.0 / h[i])));
	  }
      }
  
  if (!periodic && n == 2)
      u[1] = beta[0] + beta[1] + 2 * k * alpha[0];
  else
    u[1] = beta[0] + beta[1] + k * alpha[0];

  v[1] = b[1] - b[0];
  
  if (u[1] == 0.0)
    {
      fprintf (stderr, 
	       "%s: error: as posed, problem of computing spline is singular\n",
	       progname);
      exit (1);
    }

  if (periodic)
    {
      s[1] = alpha[0];
      uu[1] = 0.0;
      vv[1] = 0.0;
    }

  for (i = 2; i <= n - 1 ; ++i)
    {
      u[i] = (beta[i] + beta[i - 1]
	      - alpha[i - 1] * alpha[i - 1] / u[i - 1]
	      + (i == n - 1 ? k * alpha[n - 1] : 0.0));

      if (u[i] == 0.0)
	{
	  fprintf (stderr, 
		   "%s: error: as posed, problem of computing spline is singular\n",
		   progname);
	  exit (1);
	}


      v[i] = b[i] - b[i - 1] - alpha[i - 1] * v[i - 1] / u[i - 1];

      if (periodic)
	{
	  s[i] = - s[i-1] * alpha[i-1] / u[i-1];
	  uu[i] = uu[i-1] - s[i-1] * s[i-1] / u[i-1];
	  vv[i] = vv[i-1] - v[i-1] * s[i-1] / u[i-1];
	}
    }
      
  if (!periodic)
    {
      /* fill in 2nd derivative array */
      z[n] = 0.0;
      for (i = n - 1; i >= 1; --i)
	z[i] = (v[i] - alpha[i] * z[i + 1]) / u[i];
      z[0] = 0.0;
      
      /* modify to include boundary condition */
      z[0] = k * z[1];
      z[n] = k * z[n - 1];
    }
  else		/* periodic */
    {
      z[n-1] = (v[n-1] + vv[n-1]) / (u[n-1] + uu[n-1] + 2 * s[n-1]);
      for (i = n - 2; i >= 1; --i)
	z[i] = ((v[i] - alpha[i] * z[i + 1]) - s[i] * z[n-1]) / u[i];

      z[0] = z[n-1];
      z[n] = z[1];
    }

  free (h);
  free (b);
  free (u);
  free (v);
  free (alpha);
  free (beta);
  if (periodic)
    {
      free (s);
      free (uu);
      free (vv);
    }
}


/* interpolate() computes an approximate ordinate value for a given
   abscissa value, given an array of data points (stored in t[] and y[],
   containing abscissa and ordinate values respectively), and z[], the
   array of 2nd derivatives at the knots (i.e. internal data points).
   
   On entry, have n+1 >= 2 points in the t, y, z arrays, numbered 0..n.
   The number of knots (i.e. internal data points) is n-1; they are
   numbered 1..n-1 as in Kincaid and Cheney.  In the periodic case, the
   final knot, i.e., (t[n-1],y[n-1]), has the property that y[n-1]=y[0];
   also, y[n]=y[1].  The number of data points supplied by the user was n+1
   in the non-periodic case, and n in the periodic case.  When this
   function is called, n>=1 in the non-periodic case, and n>=2 in the
   periodic case. */

double
interpolate (n, t, y, z, x, tension, periodic)
     int n;
     double *t, *y, *z, x;
     double tension;
     Boolean periodic;
{
  double diff, updiff, h;
  double value;
  int is_ascending = (t[n-1] < t[n]);
  int i = 0, k;

  /* in periodic case, map x to t[0] <= x < t[n] */
  if (periodic && (x - t[0]) * (x - t[n]) > 0.0)
    x -= ((int)floor( (x - t[0]) / (t[n] - t[0]) ) * (t[n] - t[0]));

  /* do binary search to find interval */
  for (k = n - i; k > 1;)
    {
      if (is_ascending ? x >= t[i + (k>>1)] : x <= t[i + (k>>1)])
	{
	  i = i + (k>>1);
	  k = k - (k>>1);
	}
      else
	k = k>>1;
    }

  /* at this point, x is between t[i] and t[i+1] */
  diff = x - t[i];
  updiff = t[i+1] - x;
  h = t[i + 1] - t[i];

  if (tension == 0.0)
  /* evaluate cubic polynomial in nested form */
    value =  y[i] 
      + diff
	* ((y[i + 1] - y[i]) / h - h * (z[i + 1] + z[i] * 2.0) / 6.0
	   + diff * (0.5 * z[i] + diff * (z[i + 1] - z[i]) / (6.0 * h)));
  
  else if (tension > 0.0)
    /* ``positive'' (really real) tension, use sinh's */
    value = (((z[i] * sinh (tension * updiff) + z[i + 1] * sinh (tension * diff))
	      / (tension * tension * sinh (tension * h)))
	     + (y[i] - z[i] / (tension * tension)) * (updiff / h)
	     + (y[i + 1] - z[i + 1] / (tension * tension)) * (diff / h));
  else
    /* ``negative'' (really imaginary) tension, use sin's */
    value = (((z[i] * sin (tension * updiff) + z[i + 1] * sin (tension * diff))
	      / (tension * tension * sin (tension * h)))
	     + (y[i] - z[i] / (tension * tension)) * (updiff / h)
	     + (y[i + 1] - z[i + 1] / (tension * tension)) * (diff / h));
  
  return value;
}

/* is_monotonic() check whether an array of data points, read in by
   read_data(), has monotonic abscissa values. */

Boolean
is_monotonic (n, x)
     int n;			/* array size n+1 */
     double *x;
{
  int is_ascending = (x[n-1] < x[n]);

  while (n>0)
    {
      n--;
      if (is_ascending ? x[n] > x[n+1] : x[n] < x[n+1])
	return FALSE;
    };
  return TRUE;
}


/* read_float reads a single floating point quantity from an input stream
   (in either ascii or double format).  Return value indicates whether it
   was read successfully. */

Boolean 
read_float (input, dptr)
     FILE *input;
     double *dptr;
{
  int num_read;
  double dval;

  switch (input_type)
    {
    case T_ASCII:
    default:
      num_read = fscanf (input, "%lf", &dval);
      break;
    case T_DOUBLE:
      num_read = fread ((Voidptr) &dval, sizeof (dval), 1, input);
      break;
    }
  if (num_read <= 0)
    return FALSE;
  else
    {
      *dptr = dval;
      return TRUE;
    }
}

Boolean 
write_floats (x, y, precision)
     double x, y;
     int precision;
{
  int num_written = 0;

  switch (output_type)
    {
    case T_DOUBLE:
      num_written += fwrite ((Voidptr) &x, sizeof (x), 1, stdout);
      num_written += fwrite ((Voidptr) &y, sizeof (y), 1, stdout);
      break;
    case T_ASCII:
    default:
      num_written = printf ("%.*g %.*g\n", precision, x, precision, y);
      break;
    }
  
  return (num_written > 0);	/* i.e. return successp */
}

/* get_point() attempts to read a data point from an input stream
   (auto-abscissa is supported, as are both ascii and double formats).
   Return value is 0 if a data point was read, 1 if no data point could be
   read (i.e. EOF or garbage in file).  A return value of 2 is special: it
   indicates that an explicit end-of-dataset indicator was seen in the input
   stream.  For an ascii stream this is two newlines in succession; for a
   double stream this is a MAXDOUBLE. */

int
get_point (input, t, y, auto_abscissa, auto_x, auto_delta)
     FILE *input;
     double *t, *y;
     Boolean auto_abscissa;
     double *auto_x, auto_delta;
{
  Boolean success;

  if (input_type == T_ASCII)
    {
      Boolean two_newlines;

      /* skip whitespace, up to but not including 2nd newline */
      two_newlines = skip_whitespace (input);
      if (two_newlines)
	/* end-of-dataset indicator */
	return 2;
    }
  if (feof (input))
    return 1;

  if (auto_abscissa)
    {
      success = read_float (input, y);
      if (!success)		/* e.g., EOF */
	return 1;
      if (output_type == T_DOUBLE && *y == MAXDOUBLE)
	/* end-of-dataset indicator */
	return 2;
      else
	{
	  *t = *auto_x;
	  *auto_x += auto_delta;
	  return 0;
	}
    }
  else
    {
      success = read_float (input, t);
      if (!success)		/* e.g., EOF */
	return 1;
      if (output_type == T_DOUBLE && *y == MAXDOUBLE)
	/* end-of-dataset indicator */
	return 2;
      success = read_float (input, y);
      if (!success)		/* effectively EOF (could be garbage) */
	{
	  fprintf (stderr, "%s: warning: premature EOF in input stream\n",
		   progname);
	  return 1;
	}
      return 0;
    }
}

/* read_data() reads a single dataset from an input stream, and stores it.
  If the stream is in ascii format, end-of-dataset is signalled by two
  newlines in succession.  If the stream is in double format, end-of-dataset
  is signalled by the occurrence of a MAXDOUBLE.

  Return value is TRUE if the dataset is ended by an explicit end-of-dataset
  signal, and FALSE if the dataset is terminated by EOF.  That is, return
  value indicates whether another dataset is expected to follow. */

Boolean
read_data (input, len, used, auto_abscissa, auto_x, auto_delta, t, y, z)
     FILE *input;
     int *len, *used;
     Boolean auto_abscissa;
     double *auto_x, auto_delta;
     double **t, **y, **z;
{
  int success;
  double tt, yy;

  while (TRUE)
    {
      if ((++ *used) >= *len)
	{
	  *len *= 2;
	  *t = (double *)xrealloc (*t, sizeof(double) * *len);
	  *y = (double *)xrealloc (*y, sizeof(double) * *len);
	  *z = (double *)xrealloc (*z, sizeof(double) * *len);
	}

      success = get_point (input, &tt, &yy, auto_abscissa, auto_x, auto_delta);

      switch (success)
	{
	case 0:			/* good data point */
	  (*t)[*used] = tt;
	  (*y)[*used] = yy;
	  break;
	case 1:			/* no more data points, EOF seen */
	  (*used)--;
	  return FALSE;
	case 2:			/* end of dataset, but input continues */
	  (*used)--;
	  return TRUE;
	  break;
	}
    }
}


/* do_spline() is the main routine for piecewise cubic spline
   interpolation, supporting both periodicity and a user-specified boundary
   condition parameter.  Nonzero tension may be specified, in which case
   the interpolate() routine, which this calls, will use not cubic
   polynomials but rather expressions involving hyperbolic sines.

   t[] and y[] are the arrays in which the abscissa and ordinate values of
   the user-specified data points are stored, and z[] is the array in which
   the 2nd derivatives at the knots (data points in the interior of the
   interval) will be stored.  used+1 is the effective size of each of these
   arrays.  The number of points supplied by the user was used+1 in the
   non-periodic case.  It was used+0 in the periodic case.  

   The reason that the number of elements is greater by one in the periodic
   case is that the first user-supplied data point occurs also at the end.
   In fact, in the periodic case this function will increment the size of
   the array once more, since periodic interpolation requires the first two
   data points, not just the first, to appear at the end. */

void
do_spline (used, len, t, y, z, tension, periodic, use_boundary_condition, k, precision, first_x, last_x, spacing_x, no_of_intervals, spec_first_x, spec_last_x, spec_spacing_x, spec_no_of_intervals)
     int used;			/* used+1 elements stored in (*t)[] etc. */
     int len;			/* length of each array */
     double **t, **y, **z;	/* we use ** because may have to realloc */
     double tension;
     Boolean periodic;
     Boolean use_boundary_condition;
     double k;			/* boundary condition: y''_1 = k y''_0, etc. */
     int precision;
     double first_x, last_x, spacing_x; 
     int no_of_intervals;
     Boolean spec_first_x, spec_last_x, spec_spacing_x, spec_no_of_intervals;
{
  int range_count = 0;		/* number of req'd datapoints out of range */
  int lastval = 0;		/* last req'd point = 1st/last data point? */
  int i;

  if (used + 1 == 0)		/* zero data points in array */
    /* don't output anything (i.e. effectively output a null dataset) */
    return;

  if (used+1 == 1)		/* a single data point in array */
    {
      fprintf (stderr, 
	       "%s: warning: could not construct a spline from 1 data point\n", 
	       progname);
      /* don't output anything (i.e. effectively output a null dataset) */
      return;
    }

  if (!periodic && used+1 <= 2)
    {
      if (use_boundary_condition)
	fprintf (stderr, 
		 "%s: warning: only 2 data points, nonzero boundary condition is meaningless\n", 
		 progname);
      k = 0.0;
    }

  if (!is_monotonic (used, *t))
    non_monotonic_error();	/* self-explanatory */

  if (periodic)
    {
      if ((*y)[used] != (*y)[0])
	{
	  fprintf (stderr, "%s: warning: to ensure periodicity, final y value set equal to initial\n", 
		   progname); 
	  (*y)[used] = (*y)[0];
	}

      /* add pseudo-point at end (to accord with periodicity) */
      if (used + 1 >= len)
	{
	  len++;
	  *t = (double *)xrealloc (*t, sizeof(double) * len);
	  *y = (double *)xrealloc (*y, sizeof(double) * len);
	  *z = (double *)xrealloc (*z, sizeof(double) * len);
	}
      (*t)[used + 1] = (*t)[used] + ((*t)[1] - (*t)[0]);
      (*y)[used + 1] = (*y)[1];
    }

  /* compute z[], array of 2nd derivatives at each knot */
  fit (used + (periodic ? 1 : 0), /* include pseudo-point if any */
       *t, *y, *z, k, tension, periodic);

  if (!spec_first_x) 
    first_x = (*t)[0];
  if (!spec_last_x) 
    last_x = (*t)[used];	/* used+1 data points in all */
  if (!spec_spacing_x) 
    {
      if (no_of_intervals > 0)
	spacing_x = (last_x - first_x) / no_of_intervals;
      else
	spacing_x = 0;		/* won't happen */
    }
  else				/* user specified spacing */
    {
      if ((last_x - first_x) * spacing_x < 0.0)
	{
	  fprintf (stderr, "%s: warning: spacing of wrong sign, corrected\n",
		   progname);
	  spacing_x = -spacing_x;
	}
      if (spec_no_of_intervals)
	fprintf (stderr, "%s: requested number of intervals ignored\n",
		 progname);
      no_of_intervals = (int)(fabs((last_x - first_x) / spacing_x) + FUZZ);
    }

  if (last_x == (*t)[0])
    lastval = 1;
  else if (last_x == (*t)[used])
    lastval = 2;

  for (i = 0; i <= no_of_intervals; ++i)
    {
      double x;

      x = first_x + spacing_x * i;

      if (i == no_of_intervals)
	{
	  /* avoid numerical fuzz */
	  if (lastval == 1)	/* left end of input */
	    x = (*t)[0];
	  else if (lastval == 2) /* right end of input */
	    x = (*t)[used];
	}

      if (periodic || (x - (*t)[0]) * (x - (*t)[used]) <= 0)
	write_floats (x, 
		      interpolate (used, *t, *y, *z, x, tension, periodic),
		      precision);
      else
	range_count++;
    }

  switch (range_count)
    {
    case 0:
      break;
    case 1:
      fprintf (stderr, 
	       "%s: warning: 1 requested point could not be computed (out of data range)\n", 
	       progname);
      break;
    default:
      fprintf (stderr, 
	       "%s: warning: %d requested points could not be computed (out of data range)\n", 
	       progname, range_count);
      break;
    }
}

/* do_bessel() is the main routine for real-time cubic Bessel
   interpolation.  Its return value indicates whether the data points in
   the input stream ended with an explicit end-of-dataset indicator, i.e.,
   whether another dataset is expected to follow.  An end-of-dataset indicator
   is two newlines in succession for an ascii stream, and a MAXDOUBLE for a
   stream of doubles. */

Boolean
do_bessel (input, auto_abscissa, auto_x, auto_delta, first_x, last_x, spacing_x, precision)
     FILE *input;
     Boolean auto_abscissa;
     double *auto_x, auto_delta;     
     double first_x, last_x, spacing_x;
     int precision;
{
  int state = STATE_ZERO;
  double tt[4], yy[4];
  double s0 = 0.0, s1 = 0.0, s2 = 0.0;
  double t, y;
  int success;
  int direction = (last_x > first_x ? 1 : -1);

  while (TRUE)
    {
      success = get_point (input, &t, &y, auto_abscissa, auto_x, auto_delta);
      
      if (success == 0)		/* got a new data point */
	{
	  /* use our DFA to process the new data point */
	  switch (state)
	    {
	    case STATE_ZERO:	/* just store point */
	      tt[0] = t;
	      yy[0] = y;
	      state = STATE_ONE;
	      break;
	    case STATE_ONE:	/* just store point */
	      tt[1] = t;
	      if (direction * (tt[1] - tt[0]) <= 0)
		non_monotonic_error();
	      yy[1] = y;
	      state = STATE_TWO;
	      break;
	    case STATE_TWO:	/* store point, and process */
	      tt[2] = t;
	      if (direction * (tt[2] - tt[1]) <= 0)
		non_monotonic_error();
	      yy[2] = y;

	      /* fit parabola through points 0,1,2 to compute slopes at 0,1*/
	      s0 = (((tt[1]-tt[0]) * ((yy[0]-yy[2]) / (tt[0]-tt[2]))
		     + (tt[0]-tt[2]) * ((yy[1]-yy[0]) / (tt[1]-tt[0])))
		    / (tt[1]-tt[2]));
	      s1 = (((tt[2]-tt[1]) * ((yy[1]-yy[0]) / (tt[1]-tt[0]))
		     + (tt[1]-tt[0]) * ((yy[2]-yy[1]) / (tt[2]-tt[1])))
		    / (tt[2]-tt[0]));

	      /* output spline points in range between points 0, 1 */
	      do_bessel_range (tt[0], tt[1], yy[0], yy[1], s0, s1,
			       first_x, last_x, spacing_x, precision, FALSE);
	      
	      state = STATE_THREE;
	      break;
	    case STATE_THREE:	/* store point, and process */
	      tt[3] = t;
	      if (direction * (tt[3] - tt[2]) <= 0)
		non_monotonic_error();
	      yy[3] = y;

	      /* fit parabola through points 1,2,3 to compute slope at 2 */
	      s2 = (((tt[3]-tt[2]) * ((yy[2]-yy[1]) / (tt[2]-tt[1]))
		     + (tt[2]-tt[1]) * ((yy[3]-yy[2]) / (tt[3]-tt[2])))
		    / (tt[3]-tt[1]));
	      
	      /* output spline points in range between points 1, 2 */
	      do_bessel_range (tt[1], tt[2], yy[1], yy[2], s1, s2, 
			       first_x, last_x, spacing_x, precision, FALSE);
	      
	      /* shift points down */
	      tt[0] = tt[1];
	      tt[1] = tt[2];
	      tt[2] = tt[3];
	      yy[0] = yy[1];
	      yy[1] = yy[2];
	      yy[2] = yy[3];
	      /* shift down the only knot slope worth keeping */
	      s1 = s2;
	      break;
	    }
	}
      else		/* didn't get a point, so wind things up */
	{
	  switch (state)
	    {
	    case STATE_ZERO:
	      /* silently output a null dataset (i.e., don't output anything) */
	      break;
	    case STATE_ONE:
	      fprintf (stderr, "%s: warning: could not construct a spline from 1 data point\n", 
		       progname);
	      /* output a null dataset (i.e., don't output anything) */
	      break;
	    case STATE_TWO:
	      /* have two points: do linear interp between points 0, 1 */
	      s0 = s1 = (yy[1] - yy[0])/(tt[1]-tt[0]);
	      do_bessel_range (tt[0], tt[1], yy[0], yy[1], s0, s1, 
			       first_x, last_x, spacing_x, precision, TRUE);
	      break;
	    case STATE_THREE:
	      /* already did 1st of 2 intervals, so do 2nd one too */

	      /* fit parabola through points 0,1,2 to compute slope at 2 */
	      s2 = (((tt[0]-tt[2]) * ((yy[2]-yy[1]) / (tt[2]-tt[1]))
		     + (tt[2]-tt[1]) * ((yy[0]-yy[2]) / (tt[0]-tt[2])))
		    / (tt[0]-tt[1]));

	      /* output spline points in range between points 1, 2 */
	      do_bessel_range (tt[1], tt[2], yy[1], yy[2], s1, s2, 
			       first_x, last_x, spacing_x, precision, TRUE);
	      break;
	    }

	  /* return indication of whether end-of-dataset was seen in stream */
	  return (success == 2 ? TRUE : FALSE);
	}
    }
}

void
non_monotonic_error ()
{
  fprintf (stderr, "%s: error: abscissa values not monotonic in correct direction\n",
	   progname);
  exit (1);
}


/* do_bessel_range() computes spline points within the abscissa interval
   abscissa0 <= x < abscissa1.  The ordinate values value0 and value1, and
   endpoint slopes slope0 and slope1, are specified.  If `endit' is set,
   then the interval stretches slightly farther than abscissa1, to
   compensate for roundoff error. */

void
do_bessel_range (abscissa0, abscissa1, value0, value1, slope0, slope1,
		 first_x, last_x, spacing_x, precision, endit)
     double abscissa0, abscissa1, value0, value1, slope0, slope1;
     double first_x, last_x, spacing_x;
     int precision;
     Boolean endit;		/* last interval to be treated */
{
  int direction = ((last_x > first_x) ? 1 : -1); /* sign of spacing_x */
  int i;
  int imin = (int)((abscissa0 - first_x) / spacing_x - 1);
  int imax = (int)((abscissa1 - first_x) / spacing_x + 1);

  for (i = imin; i <= imax; i++)
    {
      double x;

      x = first_x + i * spacing_x;

      if ((direction * x >= direction * abscissa0)
	  && ((direction * x 
	       < (direction 
		  * (abscissa1 
		     + (endit ? FUZZ * (abscissa1 - abscissa0) : 0.)))))
	  && (direction * x >= direction * first_x)
	  && (direction * x < (direction
			       * (last_x
				  + (endit ? FUZZ * (last_x - first_x) : 0.)))))
	{
	  double diff = x - abscissa0;
	  double updiff = abscissa1 - x;
	  double h = abscissa1 - abscissa0;
	  double y;
	  Boolean success;

	  /* should use a nested form */
	  y = (value1 * (-2 * diff * diff * diff / (h * h * h)
			 +3 * diff * diff / (h * h))
	       + value0 * (-2 * updiff * updiff * updiff / (h * h * h)
			   +3 * updiff * updiff / (h * h)))
	    + ((slope1 * (diff * diff * diff / (h * h) 
			  - diff * diff / h)
		- (slope0 * (updiff * updiff * updiff / (h * h) 
			     - updiff * updiff / h))));
	  
	  success = write_floats (x, y, precision);
	  if (!success)
	    {
	      fprintf (stderr, 
		       "%s: error: unable to write to stdout\n",
		       progname);
	      exit (1);
	    }
	}	  
    }
}

/* Output a separator between datasets.  For ascii-format output streams
   this is an extra newline (after the one that the spline ended with,
   yielding two newlines in succession).  For double-format output streams
   this is a MAXDOUBLE. */

void
output_dataset_separator()
{
  double dummy;
  
  dummy = MAXDOUBLE;
  switch (output_type)
    {
    case T_ASCII:
    default:
      printf ("\n");
      break;
    case T_DOUBLE:
      fwrite ((Voidptr) &dummy, 
	      sizeof(dummy), 1, stdout);
      break;
    }
}

/* skip_whitespace() skips whitespace in an ascii-format input stream,
   up to but not including a second newline.  Return value indicates
   whether or not two newlines were in fact seen.  (For ascii-format
   input streams, two newlines signals an end-of-dataset.) */

Boolean
skip_whitespace (stream)
     FILE *stream;
{
  int lookahead;
  int nlcount = 0;
  
  do 
    {
      lookahead = getc (stream);
      if (lookahead == (int)'\n')
	  nlcount++;
    }
  while (lookahead != EOF 
	 && isspace((unsigned char)lookahead)
	 && nlcount < 2);

  if (lookahead == EOF)
    return FALSE;
  
  ungetc (lookahead, stream);
  return (nlcount == 2);
}
