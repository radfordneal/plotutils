/* This file contains the linedash method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the dash array used when
   subsequently drawing paths. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linedash (int n, const int *dashes, int offset)
#else
_m_linedash (n, dashes, offset)
     int n;
     const int *dashes;
     int offset;
#endif
{
  int i;

  if (!_plotter->open)
    {
      _plotter->error ("linedash: invalid operation");
      return -1;
    }

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0.0)
      return -1;

  _meta_emit_byte ((int)O_LINEDASH);
  _meta_emit_integer (n);
  for (i = 0; i < n; i++)
    _meta_emit_integer (dashes[i]);
  _meta_emit_integer (offset);
  _meta_emit_terminator ();
  
  /* invoke generic method */
  return _g_linedash (n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
_m_flinedash (int n, const double *dashes, double offset)
#else
_m_flinedash (n, dashes, offset)
     int n;
     const double *dashes;
     double offset;
#endif
{
  int i;
  
  if (!_plotter->open)
    {
      _plotter->error ("flinedash: invalid operation");
      return -1;
    }

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0.0)
      return -1;

  _meta_emit_byte (_plotter->meta_portable_output ? (int)O_LINEDASH : (int)O_LINEDASH);
  _meta_emit_integer (n);
  for (i = 0; i < n; i++)
    _meta_emit_float (dashes[i]);
  _meta_emit_float (offset);
  _meta_emit_terminator ();
  
  /* invoke generic method */
  return _g_flinedash (n, dashes, offset);
}
