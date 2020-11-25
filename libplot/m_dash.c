/* This file contains the linedash method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the dash array used when
   subsequently drawing paths. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_linedash (R___(Plotter *_plotter) int n, const int *dashes, int offset)
#else
_m_linedash (R___(_plotter) n, dashes, offset)
     S___(Plotter *_plotter;) 
     int n;
     const int *dashes;
     int offset;
#endif
{
  int i;

  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "linedash: invalid operation");
      return -1;
    }

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0.0)
      return -1;

  _meta_emit_byte (R___(_plotter) (int)O_LINEDASH);
  _meta_emit_integer (R___(_plotter) n);
  for (i = 0; i < n; i++)
    _meta_emit_integer (R___(_plotter) dashes[i]);
  _meta_emit_integer (R___(_plotter) offset);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method */
  return _g_linedash (R___(_plotter) n, dashes, offset);
}

int
#ifdef _HAVE_PROTOS
_m_flinedash (R___(Plotter *_plotter) int n, const double *dashes, double offset)
#else
_m_flinedash (R___(_plotter) n, dashes, offset)
     S___(Plotter *_plotter;) 
     int n;
     const double *dashes;
     double offset;
#endif
{
  int i;
  
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "flinedash: invalid operation");
      return -1;
    }

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0.0)
      return -1;

  _meta_emit_byte (R___(_plotter) 
		   _plotter->meta_portable_output ? (int)O_LINEDASH : (int)O_FLINEDASH);
  _meta_emit_integer (R___(_plotter) n);
  for (i = 0; i < n; i++)
    _meta_emit_float (R___(_plotter) dashes[i]);
  _meta_emit_float (R___(_plotter) offset);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method */
  return _g_flinedash (R___(_plotter) n, dashes, offset);
}
