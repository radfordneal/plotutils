/* This file contains the space method, which is a standard part of the
   libplot library.  It sets the mapping from user coordinates to display
   coordinates.  On the display device, the drawing region is a rectangle
   (usually, a square).  The arguments to the space routine are the lower
   left and upper right vertices of a `window' (a drawing rectangle), in
   user coordinates.  This window, whose axes are aligned with the
   coordinate axes, will be mapped affinely onto the drawing region on the
   display device.

   This file also contains the space2 routine, which is a GNU extension to
   libplot.  The arguments to the space2 method are the vertices of an
   `affine window' (a drawing parallelogram), in user coordinates.  (The
   specified vertices are the lower left, the lower right, and the upper
   left.)  This window will be mapped affinely onto the drawing region on
   the display device. */

/* In this MetaPlotter version, we don't maintain an affine map from user
   coordinates to device coordinates (we have no idea what device
   coordinates will be, since the emitted graphics metafile will be
   displayed by an invocation of `plot').  We do however perform some
   computations to initialize the font size and line width, in terms of the
   display size, to default values.  The font size in terms of user
   coordinates needs to be kept track of by a MetaPlotter, so that the
   labelwidth() operation will work. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_m_space (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
#else
_m_space (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "space: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_SPACE);
  _meta_emit_integer (R___(_plotter) x0);
  _meta_emit_integer (R___(_plotter) y0);
  _meta_emit_integer (R___(_plotter) x1);
  _meta_emit_integer (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method */
  return _g_fspace2 (R___(_plotter) 
		     (double)x0, (double)y0, 
		     (double)x1, (double)y0, 
		     (double)x0, (double)y1);
}

int
#ifdef _HAVE_PROTOS
_m_fspace (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
#else
_m_fspace (R___(_plotter) x0, y0, x1, y1)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fspace: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_SPACE : (int)O_FSPACE);
  _meta_emit_float (R___(_plotter) x0);
  _meta_emit_float (R___(_plotter) y0);
  _meta_emit_float (R___(_plotter) x1);
  _meta_emit_float (R___(_plotter) y1);
  _meta_emit_terminator (S___(_plotter));

  /* invoke generic method */
  return _g_fspace2 (R___(_plotter) x0, y0, x1, y0, x0, y1);
}

int
#ifdef _HAVE_PROTOS
_m_space2 (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2)
#else
_m_space2 (R___(_plotter) x0, y0, x1, y1, x2, y2)
     S___(Plotter *_plotter;) 
     int x0, y0, x1, y1, x2, y2;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "space2: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) (int)O_SPACE2);
  _meta_emit_integer (R___(_plotter) x0);
  _meta_emit_integer (R___(_plotter) y0);
  _meta_emit_integer (R___(_plotter) x1);
  _meta_emit_integer (R___(_plotter) y1);
  _meta_emit_integer (R___(_plotter) x2);
  _meta_emit_integer (R___(_plotter) y2);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method */
  return _g_fspace2 (R___(_plotter) 
		     (double)x0, (double)y0, 
		     (double)x1, (double)y1, 
		     (double)x2, (double)y2);
}

int
#ifdef _HAVE_PROTOS
_m_fspace2 (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2)
#else
_m_fspace2 (R___(_plotter) x0, y0, x1, y1, x2, y2)
     S___(Plotter *_plotter;) 
     double x0, y0, x1, y1, x2, y2;
#endif
{
  if (!_plotter->open)
    {
      _plotter->error (R___(_plotter) 
		       "fspace2: invalid operation");
      return -1;
    }

  _meta_emit_byte (R___(_plotter) _plotter->meta_portable_output ? (int)O_SPACE2 : (int)O_FSPACE2);
  _meta_emit_float (R___(_plotter) x0);
  _meta_emit_float (R___(_plotter) y0);
  _meta_emit_float (R___(_plotter) x1);
  _meta_emit_float (R___(_plotter) y1);
  _meta_emit_float (R___(_plotter) x2);
  _meta_emit_float (R___(_plotter) y2);
  _meta_emit_terminator (S___(_plotter));
  
  /* invoke generic method */
  return _g_fspace2 (R___(_plotter) x0, y0, x1, y1, x2, y2);
}
