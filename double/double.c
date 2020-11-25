/* This program, double, is a filter for converting, scaling, cutting and
   joining data sets.  The data sets may be unformatted (i.e., in binary
   double precision format), or in ascii format.  Copyright (C) 1989-1998
   Free Software Foundation, Inc. */

#include "sys-defines.h"
#include "getopt.h"

/* type of data in input and output streams */
typedef enum
{
  T_ASCII, T_SINGLE, T_DOUBLE, T_INTEGER
}
data_type;

data_type input_type = T_ASCII;
data_type output_type = T_ASCII;

const char *progname = "double"; /* name of this program */

int precision = 8;		/* default no. of digits after decimal pt. */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] =
{
  /* string arg */
  {"input-type",		ARG_REQUIRED,	NULL, 'I'},
  {"output-type",		ARG_REQUIRED,	NULL, 'O'},
  {"precision",			ARG_REQUIRED,	NULL, 'q'},
  /* file name arg */
  {"times-file",		ARG_REQUIRED,	NULL, 'T'},
  {"plus-file",			ARG_REQUIRED,	NULL, 'P'},
  {"pre-join-file",		ARG_REQUIRED,	NULL, 'j'},
  {"post-join-file",		ARG_REQUIRED,	NULL, 'J'},
  /* floating point arg */
  {"times",			ARG_REQUIRED,	NULL, 't'},
  {"plus",			ARG_REQUIRED,	NULL, 'p'},
  /* integer arg */
  {"record-length",		ARG_REQUIRED,	NULL, 'R'},
  {"fields",			ARG_OPTIONAL,	NULL, 'f'}, /* 0,1,2, or ... */
  {"record-limits",		ARG_OPTIONAL,	NULL, 'r'}, /* 0,1,2,3 args*/
  {"dataset-limits",		ARG_OPTIONAL,	NULL, 'd'}, /* 0,1,2,3 args*/
  /* flags */
  {"version",			ARG_NONE,	NULL, 'V' << 8},
  {"help",			ARG_NONE,	NULL, 'h' << 8},
  {NULL, 0, 0, 0}
};

/* null-terminated list of options that we don't show to the user */
int hidden_options[] = { 0 };

/* forward references */
bool mung_dataset __P ((FILE *input, int record_length, bool *field_array, double scale, double baseline, FILE *add_fp, FILE *mult_fp, FILE *pre_join_fp, FILE *post_join_fp, int precision, bool suppress));
bool read_float __P ((FILE *input, double *dptr));
bool skip_whitespace __P ((FILE *stream));
bool write_float __P ((double data, int precision));
int get_record __P ((FILE *input, double *record, int record_length));
void maybe_emit_oob_warning __P ((void));
void open_file __P ((char *name, FILE **fpp));
void output_dataset_separator __P ((void));
void set_format_type __P ((char *s, data_type *typep));
/* from libcommon */
extern void display_usage __P((const char *progname, const int *omit_vals, bool files, bool fonts));
extern void display_version __P((const char *progname)); 
extern Voidptr xcalloc __P ((unsigned int nmemb, unsigned int size));
extern Voidptr xmalloc __P ((unsigned int size));
extern char *xstrdup __P ((const char *s));


int
#ifdef _HAVE_PROTOS
main (int argc, char **argv)
#else
main (argc, argv)
     int argc;
     char **argv;
#endif
{
  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered */
  bool show_version = false;	/* remember to show version message */
  bool show_usage = false;	/* remember to output usage message */
  char *add_file = NULL, *mult_file = NULL;
  char *pre_join_file = NULL, *post_join_file = NULL;
  FILE *add_fp = NULL, *mult_fp = NULL; 
  FILE *pre_join_fp = NULL, *post_join_fp = NULL;
  double scale = 1.0, baseline = 0.0; /* mult., additive constants */
  int record_length = 1;
  int record_min = 0, record_max = MAXINT, record_spacing = 1;
  int dataset_min = 0, dataset_max = MAXINT, dataset_spacing = 1;  
  int local_record_min, local_record_max, local_record_spacing;
  int local_dataset_min, local_dataset_max, local_dataset_spacing;  
  int field_array_len = 0;	/* initial size of field array */
  int dataset_index = 0;	/* running count */
  bool *field_array = NULL, *new_field_array;
  bool more_points, dataset_printed = false;

  while (true)
    {
      option = getopt_long (argc, argv, "I:O:q:T:P:j:J:t:p:R:r::f::d::", long_options, &opt_index);
      if (option == 0)
	option = long_options[opt_index].val;

      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 'V' << 8:		/* display version */
	  show_version = true;
	  break;
	case 'h' << 8:		/* help */
	  show_usage = true;
	  break;

	  /* ----------- options with a single argument --------------*/

	case 'I':
	  set_format_type (optarg, &input_type);
	  break;
	case 'O':
	  set_format_type (optarg, &output_type);
	  break;

	case 'T':		/* Times file name, ARG REQUIRED */
	  mult_file = xstrdup (optarg);
	  break;
	case 'P':		/* Plus file name, ARG REQUIRED	*/
	  add_file = xstrdup (optarg);
	  break;
	case 'j':		/* Pre-join file name, ARG REQUIRED */
	  pre_join_file = xstrdup (optarg);
	  break;
	case 'J':		/* Post-join file name, ARG REQUIRED */
	  post_join_file = xstrdup (optarg);
	  break;

	case 't':		/* Times (mult. constant), ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &scale) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: bad multiplicative constant `%s'\n", 
		       progname, optarg);
	      return 1;
	    }
	  break;
	case 'p':		/* Plus (add. constant), ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &baseline) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: bad additive constant `%s'\n", 
		       progname, optarg);
	      return 1;
	    }
	  break;
	case 'q':		/* Precision, ARG REQUIRED 	*/
	  if ((sscanf (optarg, "%d", &precision) <= 0)
	      || (precision < 1))
	    {
	      fprintf (stderr,
		       "%s: error: bad precision `%s', must be integer >= 1\n",
		       progname, optarg);
	      return 1;
	    }
	  break;

	case 'R':		/* Number of data per record, ARG REQUIRED */
	  if ((sscanf (optarg, "%d", &record_length) <= 0)
	      || (record_length < 1))
	    {
	      fprintf (stderr,
		       "%s: error: bad record length `%s', must be integer >= 1\n",
		       progname, optarg);
	      return 1;
	    }
	  break;

	  /* ----- Options with a variable number of arguments ----- */

	case 'r':		/* Record limits, ARG OPTIONAL [0,1,2,3] */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%d", &local_record_min) <= 0)
	    break;
	  record_min = local_record_min;
	  optind++;	/* tell getopt we recognized record_min */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%d", &local_record_max) <= 0)
	    break;
	  record_max = local_record_max;
	  optind++;	/* tell getopt we recognized record_max */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%d", &local_record_spacing) <= 0)
	    break;
	  record_spacing = local_record_spacing;
	  optind++;	/* tell getopt we recognized record_spacing */
	  break;

	case 'd':		/* Dataset limits, ARG OPTIONAL [0,1,2,3] */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%d", &local_dataset_min) <= 0)
	    break;
	  dataset_min = local_dataset_min;
	  optind++;	/* tell getopt we recognized dataset_min */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%d", &local_dataset_max) <= 0)
	    break;
	  dataset_max = local_dataset_max;
	  optind++;	/* tell getopt we recognized dataset_max */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%d", &local_dataset_spacing) <= 0)
	    break;
	  dataset_spacing = local_dataset_spacing;
	  optind++;	/* tell getopt we recognized dataset_spacing */
	  break;

	case 'f':
	  while (true)
	    {
	      int field_index;

	      if (optind >= argc)
		break;
	      if (sscanf (argv[optind], "%d", &field_index) <= 0)
		break;
	      if (field_index < 0)
		{
		  fprintf (stderr, "%s: error: bad field index `%d', must be >= 0\n",
			   progname, field_index);
		  return 1;
		}
	      if (field_index >= field_array_len)
		{
		  new_field_array 
		    = (bool *)xcalloc ((unsigned)(field_index + 1), sizeof(bool));
		  memcpy (new_field_array, field_array,
			  field_array_len * sizeof(bool));
		  if (field_array != NULL) /* NULL on entry */
		    free (field_array);
		  field_array = new_field_array;
		  field_array_len = field_index + 1;
		}

	      field_array[field_index] = true;
	      optind++;		/* tell getopt we recognized field index */
	    }
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
      fprintf (stderr, "Try `%s --help' for more information\n", progname);
      return 1;
    }
  if (show_version)
    {
      display_version (progname);
      return 0;
    }
  if (show_usage)
    {
      display_usage (progname, hidden_options, true, false);
      return 0;
    }

  if (dataset_spacing < 1)
    {
      fprintf (stderr, "%s: error: bad dataset spacing `%d' (must be > 0)\n",
	       progname, dataset_spacing);
      return 1;
    }

  if (record_spacing < 1)
    {
      fprintf (stderr, "%s: error: bad record spacing `%d' (must be > 0)\n",
	       progname, record_spacing);
      return 1;
    }

  /* Sanity checks on user-supplied options */

  if (field_array_len == 0)
    fprintf (stderr, "%s: no fields specified\n", progname);
  if (field_array_len > record_length)
    {
      fprintf (stderr, 
	       "%s: error: at least one field index out of bounds\n", progname);
      return 1;
    }
  new_field_array = (bool *)xcalloc ((unsigned)record_length, sizeof(bool));
  memcpy (new_field_array, field_array,
	  field_array_len * sizeof(bool));
  if (field_array != NULL) /* NULL originally */
    free (field_array);
  field_array = new_field_array;
  field_array_len = record_length;

  /*  open additive/multiplicative/join files. */
  if (add_file)
    open_file (add_file, &add_fp);
  if (mult_file)
    open_file (mult_file, &mult_fp);
  if (pre_join_file)
    open_file (pre_join_file, &pre_join_fp);
  if (post_join_file)
    open_file (post_join_file, &post_join_fp);
  
  if (optind < argc)
    {
      /* call mung_dataset() on all datasets contained in
	 each file specified on command line */
      for (; optind < argc; optind++)
	{
	  FILE *data_fp;
	  
	  /* open file, treat "-" as stdin */
	  if (strcmp (argv[optind], "-") == 0)
	    data_fp = stdin;
	  else
	    open_file (argv[optind], &data_fp);

	  /* loop through datasets in file (may be more than one) */
	  do
	    {
	      bool dataset_ok;
	      
	      dataset_ok = ((dataset_index >= dataset_min)
			    && (dataset_index <= dataset_max)
			    && ((dataset_index - dataset_min) 
				% dataset_spacing == 0)) ? true : false;

	      /* output a separator between successive datasets */
	      if (dataset_printed && dataset_ok)
		output_dataset_separator();

	      more_points = mung_dataset (data_fp,
					  record_length, field_array,
					  scale, baseline,
					  add_fp, mult_fp, 
					  pre_join_fp, post_join_fp,
					  precision, dataset_ok ? false : true);

	      if (dataset_ok)
		dataset_printed = true;
	      
	      dataset_index++;
	    } 
	  while (more_points);
	  
	  /* close file (but don't close stdin) */
	  if (data_fp != stdin)
	    fclose (data_fp);
	}
    }
  else			/* no files spec'd, read stdin instead */
    /* loop through datasets (may be more than one) */
    do
      {
	bool dataset_ok;
	
	dataset_ok = ((dataset_index >= dataset_min)
		      && (dataset_index <= dataset_max)
		      && ((dataset_index - dataset_min) 
			  % dataset_spacing == 0)) ? true : false;

	/* output a separator between successive datasets */
	if (dataset_printed && dataset_ok)
	  output_dataset_separator();

	more_points = mung_dataset (stdin,
				    record_length, field_array,
				    scale, baseline,
				    add_fp, mult_fp, 
				    pre_join_fp, post_join_fp,
				    precision, dataset_ok ? false : true);
	if (dataset_ok)
	  dataset_printed = true;

	dataset_index++;
      }
    while (more_points);	/* keep going if no EOF yet */

  return 0;
}

/* read_float reads a single floating point quantity from an input file
   (in either ascii or double format).  Return value indicates whether it
   was read successfully. */

bool 
#ifdef _HAVE_PROTOS
read_float (FILE *input, double *dptr)
#else
read_float (input, dptr)
     FILE *input;
     double *dptr;
#endif
{
  int num_read;
  double dval;
  float fval;
  int ival;

  switch (input_type)
    {
    case T_ASCII:
    default:
      num_read = fscanf (input, "%lf", &dval);
      break;
    case T_SINGLE:
      num_read = fread ((Voidptr) &fval, sizeof (fval), 1, input);
      dval = fval;
      break;
    case T_DOUBLE:
      num_read = fread ((Voidptr) &dval, sizeof (dval), 1, input);
      break;
    case T_INTEGER:
      num_read = fread ((Voidptr) &ival, sizeof (ival), 1, input);
      dval = ival;
      break;
    }
  if (num_read <= 0)
    return false;
  if (dval != dval)
    {
      fprintf (stderr, "%s: encountered a NaN (not-a-number) in binary input file\n",
	       progname);
      return false;
    }
  else
    {
      *dptr = dval;
      return true;
    }
}

/* get_record() attempts to read a record (a sequence of record_length
   data, i.e., floating-point quantities) from an input file.  Return
   value is 0 if a record was successfully read, 1 if no record could be
   read (i.e. EOF or garbage in stream).  A return value of 2 is special:
   it indicates that an explicit end-of-dataset indicator was seen in the
   input file.  For an ascii stream this is two newlines in succession;
   for a stream of doubles it is a MAXDOUBLE appearing at what would
   otherwise have been the beginning of the record, etc. */

int
#ifdef _HAVE_PROTOS
get_record (FILE *input, double *record, int record_length)
#else
get_record (input, record, record_length)
     FILE *input;
     double *record;
     int record_length;
#endif
{
  bool success;
  int i;

  if (input_type == T_ASCII)
    {
      bool two_newlines;

      /* skip whitespace, up to but not including 2nd newline */
      two_newlines = skip_whitespace (input);
      if (two_newlines)
	/* end-of-dataset indicator */
	return 2;
    }
  if (feof (input))
    return 1;			/* EOF */

  for (i = 0; i < record_length; i++)
    {
      double val;

      success = read_float (input, &val);
      if (i == 0 && 
	  ((input_type == T_DOUBLE && val == MAXDOUBLE)
	   || (input_type == T_SINGLE && val == (double)MAXFLOAT)
	   || (input_type == T_INTEGER && val == (double)MAXINT)))
	/* end-of-dataset indicator */
	return 2;
      if (!success)		/* EOF or garbage */
	{
	  if (i > 0)
	    fprintf (stderr, "%s: input file terminated prematurely\n",
		     progname);
	  return 1;
	}
      record[i] = val;
    }
  
  return 0;
}

/* Emit a double, in specified output representation.  Be sure to inform
   user if any of the emitted values were out-of-bounds for
   single-precision or integer format. */
bool
#ifdef _HAVE_PROTOS
write_float (double x, int precision)
#else
write_float (x, precision)
     double x;
     int precision;
#endif
{
  int num_written = 0;
  float fx;
  int ix;

  switch (output_type)
    {
    case T_ASCII:
    default:
      num_written = printf ("%.*g\n", precision, x);
      break;
    case T_SINGLE:
      fx = FROUND(x);
      if (fx == MAXFLOAT || fx == -(MAXFLOAT))
	{
	  maybe_emit_oob_warning();
	  if (fx == MAXFLOAT)
	    fx *= 0.99999;	/* kludge */
	}
      num_written = fwrite ((Voidptr) &fx, sizeof (fx), 1, stdout);
      break;
    case T_DOUBLE:
      num_written = fwrite ((Voidptr) &x, sizeof (x), 1, stdout);
      break;
    case T_INTEGER:
      ix = IROUND(x);
      if (ix == MAXINT || ix == -(MAXINT))
	{
	  maybe_emit_oob_warning();
	  if (ix == MAXINT)
	    ix--;
	}
      num_written = fwrite ((Voidptr) &ix, sizeof (ix), 1, stdout);
      break;
    }
  if (num_written < 0)
    return false;
  else
    return true;
}

void
#ifdef _HAVE_PROTOS
open_file (char *name, FILE **fpp)
#else
open_file (name, fpp)
     char *name;
     FILE **fpp;
#endif
{
  FILE *fp;

  fp = fopen (name, "r");
  if (fp == NULL)
    {
      fprintf (stderr, "%s: error: couldn't open file `%s'\n", progname, name);
      exit (1);
    }
  *fpp = fp;
}

void
#ifdef _HAVE_PROTOS
set_format_type (char *s, data_type *typep)
#else
set_format_type (s, typep)
     char *s;
     data_type *typep;
#endif
{
  switch (s[0])
    {
    case 'a':
    case 'A':
      *typep = T_ASCII;
      break;
    case 'f':
    case 'F':
      *typep = T_SINGLE;
      break;
    case 'd':
    case 'D':
      *typep = T_DOUBLE;
      break;
    case 'i':
    case 'I':
      *typep = T_INTEGER;
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

/* mung_dataset() is the main routine for extracting fields from records in
   a dataset, and munging them.  Its return value indicates whether the
   records in the input file ended with an explicit end-of-dataset
   indicator, i.e., whether another dataset is expected to follow.  An
   end-of-dataset indicator is two newlines in succession for an ascii
   stream, and a MAXDOUBLE for a stream of doubles, etc. */
bool
#ifdef _HAVE_PROTOS
mung_dataset (FILE *input, int record_length, bool *field_array, 
	      double scale, double baseline, FILE *add_fp, FILE *mult_fp, 
	      FILE *pre_join_fp, FILE *post_join_fp, int precision, 
	      bool suppress)
#else
mung_dataset (input, record_length, field_array, scale, baseline, add_fp, mult_fp, pre_join_fp, post_join_fp, precision, suppress)
     FILE *input;
     int record_length;		/* number of fields per record in dataset */
     bool *field_array;	/* fields we'll extract */
     double scale, baseline;
     FILE *add_fp, *mult_fp, *pre_join_fp, *post_join_fp;
     int precision;
     bool suppress;		/* suppress output for this dataset */
#endif
{
  double *record = (double *)xmalloc (record_length * sizeof(double));
  bool in_trouble = suppress; /* once in trouble, we never get out */
  
  if (!in_trouble)
    {
      /* rewind all fp's */
      if (add_fp)
	fseek(add_fp, 0L, 0);
      if (mult_fp)
	fseek(mult_fp, 0L, 0);
      if (pre_join_fp)
	fseek(pre_join_fp, 0L, 0);
      if (post_join_fp)
	fseek(post_join_fp, 0L, 0);
    }

  while (true)
    {
      int i;
      int success;
      double add_data, mult_data, pre_join_data, post_join_data;

      if (!in_trouble && add_fp && read_float (add_fp, &add_data) == false)
	in_trouble = true;
      if (!in_trouble && mult_fp && read_float (mult_fp, &mult_data) == false)
	in_trouble = true;
      if (!in_trouble && pre_join_fp 
	  && read_float (pre_join_fp, &pre_join_data) == false)
	in_trouble = true;
      if (!in_trouble && post_join_fp 
	  && read_float (post_join_fp, &post_join_data) == false)
	in_trouble = true;
  
      success = get_record (input, record, record_length);

      switch (success)
	{
	case 0:			/* good record */
	  if (in_trouble)	/* if in trouble, do nought till dataset end */
	    continue;

	  if (pre_join_fp)
	    write_float (pre_join_data, precision);

	  for (i = 0; i < record_length; i++)
	    if (field_array[i] == true)
	      {
		double datum;
		
		datum = record[i];
		if (mult_fp)
		  datum *= mult_data;
		if (add_fp)
		  datum += add_data;
		datum *= scale;
		datum += baseline;

		/* output the munged datum */
		write_float (datum, precision);
	      }
	      
	  if (post_join_fp)
	    write_float (post_join_data, precision);

	  break;
	case 1:			/* no more records, EOF seen */
	  return false;
	case 2:			/* end of dataset, but input continues */
	  return true;
	}
    }      
}	  

/* skip_whitespace() skips whitespace in an ascii-format input file,
   up to but not including a second newline.  Return value indicates
   whether or not two newlines were in fact seen.  (For ascii-format
   input files, two newlines signals an end-of-dataset.) */

bool
#ifdef _HAVE_PROTOS
skip_whitespace (FILE *stream)
#else
skip_whitespace (stream)
     FILE *stream;
#endif
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
    return false;
  
  ungetc (lookahead, stream);
  return (nlcount == 2 ? true : false);
}

/* Output a separator between datasets.  For ascii-format output streams
   this is an extra newline (after the one that the spline ended with,
   yielding two newlines in succession).  For double-format output streams
   this is a MAXDOUBLE, etc. */

void
#ifdef _HAVE_PROTOS
output_dataset_separator(void)
#else
output_dataset_separator()
#endif
{
  double ddummy;
  float fdummy;
  int idummy;

  switch (output_type)
    {
    case T_ASCII:
    default:
      printf ("\n");
      break;
    case T_DOUBLE:
      ddummy = MAXDOUBLE;
      fwrite ((Voidptr) &ddummy, sizeof(ddummy), 1, stdout);
      break;
    case T_SINGLE:
      fdummy = MAXFLOAT;
      fwrite ((Voidptr) &fdummy, sizeof(fdummy), 1, stdout);
      break;
    case T_INTEGER:
      idummy = MAXINT;
      fwrite ((Voidptr) &idummy, sizeof(idummy), 1, stdout);
      break;
    }
}

void
#ifdef _HAVE_PROTOS
maybe_emit_oob_warning (void)
#else
maybe_emit_oob_warning ()
#endif
{
  static bool warning_written = false;

  if (!warning_written)
    {
      fprintf (stderr, "%s: approximating one or more out-of-bounds output values\n", progname);
      warning_written = true;
    }
}
