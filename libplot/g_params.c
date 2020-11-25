/* This file contains the user-callable parampl function, which is a GNU
   extension to libplot, and some related internal functions of the Plotter
   class. */

#include "sys-defines.h"
#include "extern.h"
#include "g_params.h"

/* User-callable, affects only the _plot_params[] array, which implements
   `class variables' (see g_params.h). */

int
#ifdef _HAVE_PROTOS
#ifdef NO_VOID_SUPPORT
#ifdef LIBPLOTTER
parampl (const char *parameter, char *value)
#else  /* not LIBPLOTTER */
pl_parampl (const char *parameter, char *value)
#endif /* not LIBPLOTTER */
#else
#ifdef LIBPLOTTER
parampl (const char *parameter, void *value)
#else  /* not LIBPLOTTER */
pl_parampl (const char *parameter, void *value)
#endif /* not LIBPLOTTER */
#endif
#else  /* not _HAVE_PROTOS, possible only if this is libplot, not libplotter */
#ifdef NO_VOID_SUPPORT
pl_parampl (parameter, value)
     const char *parameter;
     char *value;
#else
pl_parampl (parameter, value)
     const char *parameter;
     void *value;
#endif /* not NO_VOID_SUPPORT */
#endif
{
  int j;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    {
      if (strcmp (_plot_params[j].parameter, parameter) == 0)
	{
	  if (_plot_params[j].is_string)
	    /* parameter value is a string, so treat specially: copy the
	       string, byte by byte */
	    {
	      if (_plot_params[j].value)
		free (_plot_params[j].value);
	      if (value != NULL)
		{
		  _plot_params[j].value = 
		    (char *)_plot_xmalloc (strlen ((char *)value) + 1);
		  strcpy ((char *)_plot_params[j].value, (char *)value);
		}
	      else
		_plot_params[j].value = NULL;
	    }
	  else
	    /* parameter value is a (void *), so just copy the
               user-specified pointer */
	    _plot_params[j].value = value;
	  
	  /* matched, so return happily */
	  return 0;
	}
    }

  /* silently ignore requests to set unknown parameters */
  return 0;
}

/* This is used when a Plotter is initialized, to make instance-specific
   copies of class variables, i.e., Plotter parameters.  When this invoked,
   _plotter points (temporarily) to the newly created Plotter. */

void 
#ifdef _HAVE_PROTOS
_copy_params_to_plotter (void)
#else
_copy_params_to_plotter ()
#endif
{
  int j;
  char *envs;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    {
      if (!_plot_params[j].is_string)
	/* not a string, just copy the void pointer into the plotter */
	_plotter->params[j] = _plot_params[j].value;

      else
	/* parameter value is a string, so use malloc and strcpy */
	{
	  if (_plot_params[j].value)
	    /* have user-specified value */
	    {
	      _plotter->params[j] = 
		(char *)_plot_xmalloc (strlen ((char *)_plot_params[j].value) + 1);
	      strcpy ((char *)_plotter->params[j], 
		      (char *)_plot_params[j].value);
	    }
	  else if ((envs = getenv (_plot_params[j].parameter)) != NULL)
	    /* have value of environment variable */
	    {
	      _plotter->params[j] = 
		(char *)_plot_xmalloc (strlen (envs) + 1);
	      strcpy ((char *)_plotter->params[j], envs);
	    }
	  else if (_plot_params[j].default_value)
	    /* have default libplot value */
	    {
	      _plotter->params[j] = 
		(char *)_plot_xmalloc (strlen ((char *)_plot_params[j].default_value) + 1);
	      strcpy ((char *)_plotter->params[j], 
		      (char *)_plot_params[j].default_value);
	    }
	  else			/* punt */
	    _plotter->params[j] = NULL;
	}
    }
}

/* This is used when a Plotter is deleted, to delete the instance-specific
   copies of class variables, i.e. Plotter parameters.  When this invoked,
   _plotter points (temporarily) to the Plotter about to be deleted. */

void 
#ifdef _HAVE_PROTOS
_free_params_in_plotter (void)
#else
_free_params_in_plotter ()
#endif
{
  int j;

  /* deallocate stored values of class variables */
  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    if (_plot_params[j].is_string && _plotter->params[j] != NULL)
      /* stored parameter is a previously malloc'd string, so free it */
      free (_plotter->params[j]);

}

/* This is used when a Plotter is initialized, to retrieve values of stored
   class variables, i.e. Plotter parameters.  [And also, if necessary, by
   the public openpl() method.]  Class variables are copied to a Plotter
   instance by _copy_params_to_plotter() above, at initialization time. */

Voidptr
#ifdef _HAVE_PROTOS
_get_plot_param (const char *parameter_name)
#else
_get_plot_param (parameter_name)
     const char *parameter_name;
#endif
{
  int j;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    if (strcmp (_plot_params[j].parameter, parameter_name) == 0)
      return _plotter->params[j];

  return (Voidptr)NULL;		/* name not matched */
}

/* This retrieves the default value of any specified Plotter parameter. */

Voidptr
#ifdef _HAVE_PROTOS
_get_default_plot_param (const char *parameter_name)
#else
_get_default_plot_param (parameter_name)
     const char *parameter_name;
#endif
{
  int j;

  for (j = 0; j < NUM_PLOTTER_PARAMETERS; j++)
    if (strcmp (_plot_params[j].parameter, parameter_name) == 0)
      return _plot_params[j].default_value;

  return (Voidptr)NULL;		/* name not matched */
}
