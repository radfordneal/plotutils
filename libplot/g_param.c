/* This file defines the PlotterParams class, which is a helper class.
   A PlotterParams object is used for specifying device driver parameters
   when a Plotter is instantiated.

   (In libplot, a PlotterParams struct is created with pl_newplparams() and
   deleted with pl_deleteplparams().  These are defined in apinewc.c; there
   is also a copy constructor, pl_copyplparams().)

   This file also includes the functions that are used for copying the
   parameters into the Plotter at instantiation time, and for accessing
   them later.  These are Plotter class members. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a PlotterParams struct. */
const PlotterParams _default_plotter_params = 
{
  /* methods */
  _setplparam
};
#endif /* not LIBPLOTTER */

#ifdef LIBPLOTTER
PlotterParams::PlotterParams ()
{
  int i;
  
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    plparams[i] = (voidptr_t)NULL;
}

PlotterParams::~PlotterParams ()
{
  int i;
  
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    if (_known_params[i].is_string && plparams[i] != NULL)
      free (plparams[i]);
}

PlotterParams::PlotterParams (const PlotterParams& oldPlotterParams)
{
  int i;
  
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    plparams[i] = oldPlotterParams.plparams[i];
}

PlotterParams& PlotterParams::operator= (const PlotterParams& oldPlotterParams)
{
  int i;
  
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    plparams[i] = oldPlotterParams.plparams[i];
  return (*this);
}
#endif

/* The parameter-setting method.  This is a PlotterParams method in
   libplotter (i.e. it is #defined to be PlotterParams::setplparam).  In
   libplot, a pointer to a PlotterParams struct must be passed to it as its
   first argument. */
int
#ifdef _HAVE_PROTOS
_setplparam (R___(PlotterParams *_plotter_params) const char *parameter, voidptr_t value)
#else
_setplparam (R___(_plotter_params) parameter, value)
     S___(PlotterParams *_plotter_params;)
     const char *parameter;
     voidptr_t value;
#endif
{
  int j;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    {
      if (strcmp (_known_params[j].parameter, parameter) == 0)
	{
	  if (_known_params[j].is_string)
	    /* parameter value is a string, so treat specially: copy the
	       string, byte by byte */
	    {
	      if (_plotter_params->plparams[j])
		free (_plotter_params->plparams[j]);
	      if (value != NULL)
		{
		  _plotter_params->plparams[j] = 
		    (char *)_plot_xmalloc (strlen ((char *)value) + 1);
		  strcpy ((char *)_plotter_params->plparams[j], (char *)value);
		}
	      else
		_plotter_params->plparams[j] = NULL;
	    }
	  else
	    /* parameter value is a (void *), so just copy the
               user-specified pointer */
	    _plotter_params->plparams[j] = value;
	  
	  /* matched, so return happily */
	  return 0;
	}
    }

  /* silently ignore requests to set unknown parameters */
  return 0;
}

/**********************************************************************/ 

/* This function is called when a Plotter is instantiated.  It copies
   parameters from a PlotterParams object into the Plotter.  We adopt the
   following convention: if the PlotterParams object does not include a
   value for a parameter, a default value (retrieved in the global
   _known_params[] array) is used.  Unless, that is, an environment
   variable of the same name has been set, in which case its value is used
   instead.

   We support both parameters whose values are strings (which must be
   copied byte-by-byte) and those whose values are void pointers (which may
   simply be copied. */
void 
#ifdef _HAVE_PROTOS
_copy_params_to_plotter (R___(Plotter *_plotter) const PlotterParams *plotter_params)
#else
_copy_params_to_plotter (R___(_plotter) plotter_params)
     S___(Plotter *_plotter;) 
     const PlotterParams *plotter_params;
#endif
{
  int j;
  char *envs;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    {
      if (!_known_params[j].is_string)
	/* not a string, just copy the void pointer into the plotter */
	_plotter->params[j] = plotter_params->plparams[j];

      else
	/* parameter value is a string, so use malloc and strcpy */
	{
	  if (plotter_params->plparams[j])
	    /* have user-specified value */
	    {
	      _plotter->params[j] = 
		(char *)_plot_xmalloc (strlen ((char *)plotter_params->plparams[j]) + 1);
	      strcpy ((char *)_plotter->params[j], 
		      (char *)plotter_params->plparams[j]);
	    }
	  else if ((envs = getenv (_known_params[j].parameter)) != NULL)
	    /* have value of environment variable */
	    {
	      _plotter->params[j] = 
		(char *)_plot_xmalloc (strlen (envs) + 1);
	      strcpy ((char *)_plotter->params[j], envs);
	    }
	  else if (_known_params[j].default_value)
	    /* have default libplot value */
	    {
	      _plotter->params[j] = 
		(char *)_plot_xmalloc (strlen ((char *)_known_params[j].default_value) + 1);
	      strcpy ((char *)_plotter->params[j], 
		      (char *)_known_params[j].default_value);
	    }
	  else			/* punt */
	    _plotter->params[j] = NULL;
	}
    }
}

/* This retrieves the value of any specified Plotter parameter,
   as stored in a Plotter instance. */
voidptr_t
#ifdef _HAVE_PROTOS
_get_plot_param (R___(Plotter *_plotter) const char *parameter_name)
#else
_get_plot_param (R___(_plotter) parameter_name)
     S___(Plotter *_plotter;)
     const char *parameter_name;
#endif
{
  int j;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    if (strcmp (_known_params[j].parameter, parameter_name) == 0)
      return _plotter->params[j];

  return (voidptr_t)NULL;		/* name not matched */
}

/* This function is called when a Plotter is deleted, to delete the
   instance-specific copies of Plotter parameters.  */
void 
#ifdef _HAVE_PROTOS
_free_params_in_plotter (S___(Plotter *_plotter))
#else
_free_params_in_plotter (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  int j;

  /* deallocate stored values of class variables */
  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    if (_known_params[j].is_string && _plotter->params[j] != NULL)
      /* stored parameter is a previously malloc'd string, so free it */
      free (_plotter->params[j]);

}

/* This retrieves the default value of any specified Plotter parameter.
   Default values for each parameter are stored in the _known_params[]
   array, which is read-only global data.  So unlike the preceding
   functions, this is not a Plotter method. */
voidptr_t
#ifdef _HAVE_PROTOS
_get_default_plot_param (const char *parameter_name)
#else
_get_default_plot_param (parameter_name)
     const char *parameter_name;
#endif
{
  int j;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    if (strcmp (_known_params[j].parameter, parameter_name) == 0)
      return _known_params[j].default_value;

  return (voidptr_t)NULL;		/* name not matched */
}
