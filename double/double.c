/* This program, double, is a filter for converting, scaling, cutting and
   joining data sets.  The data sets may be unformatted (i.e., in binary
   double precision format), or in ascii format.  Copyright (C) 1989-1997
   Free Software Foundation, Inc. */

#include "sys-defines.h"
#include "getopt.h"

typedef enum
{
  T_DOUBLE, T_ASCII
}
data_type;

data_type input_type = T_ASCII;
data_type output_type = T_ASCII;

char *progname = "double";

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

/* forward references */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif
Boolean mung_dataset P__ ((FILE *input, int record_length, Boolean *field_array, double scale, double baseline, FILE *add_fp, FILE *mult_fp, FILE *pre_join_fp, FILE *post_join_fp, int precision, Boolean suppress));
Boolean read_datum P__ ((FILE *input, double *dptr));
Boolean skip_whitespace P__ ((FILE *stream));
Boolean write_datum P__ ((double data, int precision));
Voidptr xcalloc P__ ((unsigned int nmemb, unsigned int size));
Voidptr xmalloc P__ ((unsigned int size));
char *xstrdup P__ ((const char *s));
int get_record P__ ((FILE *input, double *record, int record_length));
void display_usage P__ ((void));
void display_version P__ ((void));
void open_file P__ ((char *name, FILE **fpp));
void output_dataset_separator P__ ((void));
void set_format_type P__ ((char *s, data_type *typep));
#undef P__


int
main (argc, argv)
     int argc;
     char **argv;
{
  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered */
  Boolean show_version = FALSE;	/* remember to show version message */
  Boolean show_usage = FALSE;	/* remember to output usage message */
  char *add_file = NULL, *mult_file = NULL;
  char *pre_join_file = NULL, *post_join_file = NULL;
  FILE *add_fp = NULL, *mult_fp = NULL; 
  FILE *pre_join_fp = NULL, *post_join_fp = NULL;
  double scale = 1.0, baseline = 0.0; /* mult., additive constants */
  int record_length;
  int record_min = 0, record_max = MAXINT, record_spacing = 1;
  int dataset_min = 0, dataset_max = MAXINT, dataset_spacing = 1;  
  int local_record_min, local_record_max, local_record_spacing;
  int local_dataset_min, local_dataset_max, local_dataset_spacing;  
  int field_array_len = 0;	/* initial size of field array */
  int dataset_index = 0;	/* running count */
  Boolean *field_array = NULL, *new_field_array;
  Boolean more_points, dataset_printed = FALSE;

  while (TRUE)
    {
      option = getopt_long (argc, argv, "I:O:q:T:P:j:J:t:p:R:r::f::d::", long_options, &opt_index);
      if (option == 0)
	option = long_options[opt_index].val;

      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 'V' << 8:		/* display version */
	  show_version = TRUE;
	  break;
	case 'h' << 8:		/* help */
	  show_usage = TRUE;
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
		       "%s: bad record length `%s', must be integer >= 1\n",
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
	  while (TRUE)
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
		    = xcalloc ((unsigned)(field_index + 1), sizeof(Boolean));
		  memcpy (new_field_array, field_array,
			  field_array_len * sizeof(Boolean));
		  if (field_array != NULL) /* NULL on entry */
		    free (field_array);
		  field_array = new_field_array;
		  field_array_len = field_index + 1;
		}

	      field_array[field_index] = TRUE;
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
    fprintf (stderr, "%s: warning: no fields specified\n", progname);
  if (field_array_len > record_length)
    {
      fprintf (stderr, 
	       "%s: error: at least one field index out of bounds\n", progname);
      return 1;
    }
  new_field_array = xcalloc ((unsigned)record_length, sizeof(Boolean));
  memcpy (new_field_array, field_array,
	  field_array_len * sizeof(Boolean));
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
	  
	  open_file (argv[optind], &data_fp);

	  /* loop through datasets in file (may be more than one) */
	  do
	    {
	      Boolean dataset_ok;
	      
	      dataset_ok = ((dataset_index >= dataset_min)
			    && (dataset_index <= dataset_max)
			    && ((dataset_index - dataset_min) 
				% dataset_spacing == 0));

	      /* output a separator between successive datasets */
	      if (dataset_printed && dataset_ok)
		output_dataset_separator();

	      more_points = mung_dataset (data_fp,
					  record_length, field_array,
					  scale, baseline,
					  add_fp, mult_fp, 
					  pre_join_fp, post_join_fp,
					  precision, !dataset_ok);

	      if (dataset_ok)
		dataset_printed = TRUE;
	      
	      dataset_index++;
	    } 
	  while (more_points);
	  
	  fclose (data_fp);
	}
    }
  else			/* no files spec'd, read stdin instead */
    /* loop through datasets (may be more than one) */
    do
      {
	Boolean dataset_ok;
	
	dataset_ok = ((dataset_index >= dataset_min)
		      && (dataset_index <= dataset_max)
		      && ((dataset_index - dataset_min) 
			  % dataset_spacing == 0));

	/* output a separator between successive datasets */
	if (dataset_printed && dataset_ok)
	  output_dataset_separator();

	more_points = mung_dataset (stdin,
				    record_length, field_array,
				    scale, baseline,
				    add_fp, mult_fp, 
				    pre_join_fp, post_join_fp,
				    precision, !dataset_ok);
	if (dataset_ok)
	  dataset_printed = TRUE;

	dataset_index++;
      }
    while (more_points);	/* keep going if no EOF yet */

  return 0;
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
  fprintf (stderr, "Copyright (C) 1997 Free Software Foundation, Inc.\n");
  fprintf (stderr, 
	   "The GNU plotutils come with NO WARRANTY, to the extent permitted by law.\n");
  fprintf (stderr, "You may redistribute copies of the GNU plotutils\n");
  fprintf (stderr, "under the terms of the GNU General Public License.\n");
}

/* read_datum reads a single floating point quantity from an input stream
   (in either ascii or double format).  Return value indicates whether it
   was read successfully. */

Boolean 
read_datum (input, dptr)
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

/* get_record() attempts to read a record (a sequence of record_length
   data, i.e., floating-point quantities) from an input stream.  Return
   value is 0 if a record was successfully read, 1 if no record could be
   read (i.e. EOF or garbage in stream).  A return value of 2 is special:
   it indicates that an explicit end-of-dataset indicator was seen in the
   input stream.  For an ascii stream this is two newlines in succession;
   for a stream of doubles it is a MAXDOUBLE appearing at what would
   otherwise have been the beginning of the record. */

int
get_record (input, record, record_length)
     FILE *input;
     double *record;
     int record_length;
{
  Boolean success;
  int i;

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
    return 1;			/* EOF */

  for (i = 0; i < record_length; i++)
    {
      double val;

      success = read_datum (input, &val);
      if (i == 0 && input_type == T_DOUBLE && val == MAXDOUBLE)
	/* end-of-dataset indicator */
	return 2;
      if (!success)		/* EOF or garbage */
	{
	  if (i > 0)
	    fprintf (stderr, "%s: warning: premature EOF in input stream\n",
		     progname);
	  return 1;
	}
      record[i] = val;
    }
  
  return 0;
}

Boolean
write_datum (datum, precision)
     double datum;
     int precision;
{
  int num_written = 0;

  switch (output_type)
    {
    case T_DOUBLE:
      num_written = fwrite ((Voidptr) &datum, sizeof (datum), 1, stdout);
      break;
    case T_ASCII:
    default:
      num_written = printf ("%.*g\n", precision, datum);
      break;
    }
  if (num_written < 0)
    return FALSE;
  else
    return TRUE;
}

void
open_file (name, fpp)
     char *name;
     FILE **fpp;
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
xmalloc (size)
     unsigned int size;
{
  Voidptr p;
  p = (Voidptr) malloc (size);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "%s: ", progname);
      perror ("malloc failed");
      exit (1);
    }
  return p;
}

Voidptr 
xcalloc (nmemb, size)
     unsigned int nmemb, size;
{
  Voidptr p;
  p = (Voidptr) calloc (nmemb, size);

  if (p == (Voidptr) NULL)
    {
      fprintf (stderr, "%s: ", progname);
      perror ("calloc failed");
      exit (1);
    }
  return p;
}

char *
xstrdup (s)
     const char *s;
{
  char *t = (char *)xmalloc (strlen (s) + 1);

  strcpy (t, s);
  return t;
}

/* mung_dataset() is the main routine for extracting fields from records in
   a dataset, and munging them.  Its return value indicates whether the
   records in the input stream ended with an explicit end-of-dataset
   indicator, i.e., whether another dataset is expected to follow.  An
   end-of-dataset indicator is two newlines in succession for an ascii
   stream, and a MAXDOUBLE for a stream of doubles. */

Boolean
mung_dataset (input, record_length, field_array, scale, baseline, add_fp, mult_fp, pre_join_fp, post_join_fp, precision, suppress)
     FILE *input;
     int record_length;		/* number of fields per record in dataset */
     Boolean *field_array;	/* fields we'll extract */
     double scale, baseline;
     FILE *add_fp, *mult_fp, *pre_join_fp, *post_join_fp;
     int precision;
     Boolean suppress;		/* suppress output for this dataset */
{
  double *record = (double *)xmalloc (record_length * sizeof(double));
  Boolean in_trouble = suppress; /* once in trouble, we never get out */
  
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

  while (TRUE)
    {
      int i;
      int success;
      double add_data, mult_data, pre_join_data, post_join_data;

      if (!in_trouble && add_fp && read_datum (add_fp, &add_data) == FALSE)
	in_trouble = TRUE;
      if (!in_trouble && mult_fp && read_datum (mult_fp, &mult_data) == FALSE)
	in_trouble = TRUE;
      if (!in_trouble && pre_join_fp 
	  && read_datum (pre_join_fp, &pre_join_data) == FALSE)
	in_trouble = TRUE;
      if (!in_trouble && post_join_fp 
	  && read_datum (post_join_fp, &post_join_data) == FALSE)
	in_trouble = TRUE;
  
      success = get_record (input, record, record_length);

      switch (success)
	{
	case 0:			/* good record */
	  if (in_trouble)	/* if in trouble, do nought till dataset end */
	    continue;

	  if (pre_join_fp)
	    write_datum (pre_join_data, precision);

	  for (i = 0; i < record_length; i++)
	    if (field_array[i] == TRUE)
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
		write_datum (datum, precision);
	      }
	      
	  if (post_join_fp)
	    write_datum (post_join_data, precision);

	  break;
	case 1:			/* no more records, EOF seen */
	  return FALSE;
	case 2:			/* end of dataset, but input continues */
	  return TRUE;
	}
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
