/* This file contains the generic warning and error methods.  They simply
   write the specified message to the plotter error stream, if it has one.
   There is provision for user-specifiable warning/error message handlers
   (not yet documented). */

/* All libplot warnings and error messages go through these functions, with
   the exception of libpng error messages produced by PNG Plotters (see
   z_write.c; they're different because they need to be produced by
   callbacks). */

#include "sys-defines.h"
#include "extern.h"

/* mutex for locking the warning/error message subsystem */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
pthread_mutex_t _message_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

/* user-settable handlers, defined in g_defplot.c to be NULL */
extern int (*libplot_warning_handler) ____P((const char *msg));
extern int (*libplot_error_handler) ____P((const char *msg));

void
#ifdef _HAVE_PROTOS
_g_warning (R___(Plotter *_plotter) const char *msg)
#else
_g_warning (R___(_plotter) msg)
     S___(Plotter *_plotter;) 
     const char *msg;
#endif
{
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the message subsystem */
  pthread_mutex_lock (&_message_mutex);
#endif
#endif

  if (libplot_warning_handler != NULL)
    (*libplot_warning_handler)(msg);
  else if (_plotter->data->errfp)
    fprintf (_plotter->data->errfp, "libplot: %s\n", msg);
#ifdef LIBPLOTTER
  else if (_plotter->data->errstream)
    (*(_plotter->data->errstream)) << "libplot: " << msg << '\n';
#endif

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the message subsystem */
  pthread_mutex_unlock (&_message_mutex);
#endif
#endif
}

void
#ifdef _HAVE_PROTOS
_g_error (R___(Plotter *_plotter) const char *msg)
#else
_g_error (R___(_plotter) msg)
     S___(Plotter *_plotter;) 
     const char *msg;
#endif
{
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the message subsystem */
  pthread_mutex_lock (&_message_mutex);
#endif
#endif

  if (libplot_error_handler != NULL)
    (*libplot_error_handler)(msg);
  else if (_plotter->data->errfp)
    fprintf (_plotter->data->errfp, "libplot error: %s\n", msg);
#ifdef LIBPLOTTER
  else if (_plotter->data->errstream)
    (*(_plotter->data->errstream)) << "libplot error: " << msg << '\n';
#endif

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the message subsystem */
  pthread_mutex_unlock (&_message_mutex);
#endif
#endif
}
