/* This file contains the internal paint_path() and paint_paths() methods,
   which the public method endpath() is a wrapper around. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_m_paint_path (S___(Plotter *_plotter))
#else
_m_paint_path (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  const plPath *path;
  bool explicit_endpath, sync_miter_limit = false;

  /* sync basic path attributes */
  _m_set_attributes (R___(_plotter) 
		     PL_ATTR_TRANSFORMATION_MATRIX 
		     | PL_ATTR_PEN_COLOR  | PL_ATTR_PEN_TYPE
		     | PL_ATTR_LINE_STYLE | PL_ATTR_LINE_WIDTH
		     | PL_ATTR_JOIN_STYLE | PL_ATTR_CAP_STYLE
		     | PL_ATTR_FILL_COLOR | PL_ATTR_FILL_TYPE
		     | PL_ATTR_FILL_RULE);

  /* our one and only simple path to paint */
  path = _plotter->drawstate->path;

  if (_plotter->drawstate->join_type == JOIN_MITER
      && (path->type == PATH_SEGMENT_LIST || path->type == PATH_BOX))
    /* path may have mitered juncture points */
    sync_miter_limit = true;
  if (sync_miter_limit)
    _m_set_attributes (R___(_plotter) PL_ATTR_MITER_LIMIT);

  if (path->type == PATH_SEGMENT_LIST)
    explicit_endpath = true;
  else
    explicit_endpath = false;

  /* emit metafile object-drawing instructions to draw the path; include a
     preliminary syncing of the `orientation' attribute if relevant */
  _m_paint_path_internal (R___(_plotter) path);

  if (explicit_endpath)
    {
      _m_emit_op_code (R___(_plotter) O_ENDPATH);
      _m_emit_terminator (S___(_plotter));
    }
}

bool
#ifdef _HAVE_PROTOS
_m_paint_paths (S___(Plotter *_plotter))
#else
_m_paint_paths (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  const plPath *path;
  bool sync_miter_limit = false;
  int i;

  /* sanity check */
  if (_plotter->drawstate->num_paths == 0)
    return true;

  /* sync basic path attributes */
  _m_set_attributes (R___(_plotter) 
		     PL_ATTR_TRANSFORMATION_MATRIX 
		     | PL_ATTR_PEN_COLOR  | PL_ATTR_PEN_TYPE
		     | PL_ATTR_LINE_STYLE | PL_ATTR_LINE_WIDTH
		     | PL_ATTR_JOIN_STYLE | PL_ATTR_CAP_STYLE
		     | PL_ATTR_FILL_COLOR | PL_ATTR_FILL_TYPE
		     | PL_ATTR_FILL_RULE);

  if (_plotter->drawstate->join_type == JOIN_MITER)
    {
      for (i = 0; i < _plotter->drawstate->num_paths; i++)
	{
	  path = _plotter->drawstate->paths[i];
	  if (path->type == PATH_SEGMENT_LIST || path->type == PATH_BOX)
	    /* compound path may have mitered juncture points */
	    {
	      sync_miter_limit = true;
	      break;
	    }
	}
    }
  if (sync_miter_limit)
    _m_set_attributes (R___(_plotter) PL_ATTR_MITER_LIMIT);
  
  /* loop over simple paths in compound path */
  for (i = 0; i < _plotter->drawstate->num_paths; i++)
    {
      path = _plotter->drawstate->paths[i];

      /* emit metafile object-drawing instructions to draw the path; first
	 sync `orientation' attribute, if relevant */
      _m_paint_path_internal (R___(_plotter) path);

      if (i < _plotter->drawstate->num_paths - 1)
	{
	  _m_emit_op_code (R___(_plotter) O_ENDSUBPATH);
	  _m_emit_terminator (S___(_plotter));
	}
    }

  if (_plotter->drawstate->paths[_plotter->drawstate->num_paths - 1]->type == PATH_SEGMENT_LIST)
    /* append explicit (as opposed to implicit) endpath; if we didn't wish
       to be clever, we'd append one even if the final simple path isn't a
       segment list */
    {
      _m_emit_op_code (R___(_plotter) O_ENDPATH);
      _m_emit_terminator (S___(_plotter));
    }

  /* succesfully painted compound path */
  return true;
}

/* Internal routine, called by the MetaPlotter-specific versions of
   paint_path() and paint_paths().  Besides emitting metafile instructions
   to draw a path, it may emit an instruction to update the `orientation'
   attribute, relevant to paths that are boxes/circles/ellipses. */

void
#ifdef _HAVE_PROTOS
_m_paint_path_internal (R___(Plotter *_plotter) const plPath *path)
#else
_m_paint_path_internal (R___(_plotter) path)
     S___(Plotter *_plotter;)
     const plPath *path;
#endif
{
  if (path->type == PATH_BOX 
      || path->type == PATH_CIRCLE || path->type == PATH_ELLIPSE)
    /* sync orientation; orientation is stored in the path itself, not in
       the drawing state */
    {
      int orientation = (path->clockwise ? -1 : 1);

      if (_plotter->meta_orientation != orientation)
	{
	  _m_emit_op_code (R___(_plotter) O_ORIENTATION);
	  _m_emit_integer (R___(_plotter) orientation);
	  _m_emit_terminator (S___(_plotter));
	  _plotter->meta_orientation = orientation;
	}
    }

  switch ((int)path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	plPathSegment segment;
	int i;

	/* last-minute sanity check */
	if (path->num_segments == 0)/* nothing to do */
	  break;
	if (path->num_segments == 1) /* shouldn't happen */
	  break;

	segment = path->segments[0]; /* initial moveto */
	if (_plotter->meta_pos.x != segment.p.x
	    || _plotter->meta_pos.y != segment.p.y)
	  {
	    _m_emit_op_code (R___(_plotter) O_FMOVE);
	    _m_emit_float (R___(_plotter) segment.p.x);
	    _m_emit_float (R___(_plotter) segment.p.y);
	    _m_emit_terminator (S___(_plotter));
	    _plotter->meta_pos = segment.p;
	  }

	for (i = 1; i < path->num_segments; i++)
	  {
	    plPathSegment prev_segment;

	    prev_segment = segment;
	    segment = path->segments[i];
	    switch ((int)segment.type)
	      {
	      case (int)S_LINE:
		_m_emit_op_code (R___(_plotter) O_FCONT);
		_m_emit_float (R___(_plotter) segment.p.x);
		_m_emit_float (R___(_plotter) segment.p.y);
		_m_emit_terminator (S___(_plotter));
		_plotter->meta_pos = segment.p;
		break;
		
	      case (int)S_ARC:
		_m_emit_op_code (R___(_plotter) O_FARC);
		_m_emit_float (R___(_plotter) segment.pc.x);
		_m_emit_float (R___(_plotter) segment.pc.y);
		_m_emit_float (R___(_plotter) prev_segment.p.x);
		_m_emit_float (R___(_plotter) prev_segment.p.y);
		_m_emit_float (R___(_plotter) segment.p.x);
		_m_emit_float (R___(_plotter) segment.p.y);
		_m_emit_terminator (S___(_plotter));
		_plotter->meta_pos = segment.p;
		break;
		
	      case (int)S_ELLARC:
		_m_emit_op_code (R___(_plotter) O_FELLARC);
		_m_emit_float (R___(_plotter) segment.pc.x);
		_m_emit_float (R___(_plotter) segment.pc.y);
		_m_emit_float (R___(_plotter) prev_segment.p.x);
		_m_emit_float (R___(_plotter) prev_segment.p.y);
		_m_emit_float (R___(_plotter) segment.p.x);
		_m_emit_float (R___(_plotter) segment.p.y);
		_m_emit_terminator (S___(_plotter));
		_plotter->meta_pos = segment.p;
		break;
		
	      case (int)S_QUAD:
		_m_emit_op_code (R___(_plotter) O_FBEZIER2);
		_m_emit_float (R___(_plotter) prev_segment.p.x);
		_m_emit_float (R___(_plotter) prev_segment.p.y);
		_m_emit_float (R___(_plotter) segment.pc.x);
		_m_emit_float (R___(_plotter) segment.pc.y);
		_m_emit_float (R___(_plotter) segment.p.x);
		_m_emit_float (R___(_plotter) segment.p.y);
		_m_emit_terminator (S___(_plotter));
		_plotter->meta_pos = segment.p;
		break;
		
	      case (int)S_CUBIC:
		_m_emit_op_code (R___(_plotter) O_FBEZIER3);
		_m_emit_float (R___(_plotter) prev_segment.p.x);
		_m_emit_float (R___(_plotter) prev_segment.p.y);
		_m_emit_float (R___(_plotter) segment.pc.x);
		_m_emit_float (R___(_plotter) segment.pc.y);
		_m_emit_float (R___(_plotter) segment.pd.x);
		_m_emit_float (R___(_plotter) segment.pd.y);
		_m_emit_float (R___(_plotter) segment.p.x);
		_m_emit_float (R___(_plotter) segment.p.y);
		_m_emit_terminator (S___(_plotter));
		_plotter->meta_pos = segment.p;
		break;
		
	      default:		/* shouldn't happen */
		break;
	      }
	  }
      }
      break;
      
    case (int)PATH_BOX:
      {
	_m_emit_op_code (R___(_plotter) O_FBOX);
	_m_emit_float (R___(_plotter) path->p0.x);
	_m_emit_float (R___(_plotter) path->p0.y);
	_m_emit_float (R___(_plotter) path->p1.x);
	_m_emit_float (R___(_plotter) path->p1.y);
	_m_emit_terminator (S___(_plotter));
	
	_plotter->meta_pos.x = 0.5 * (path->p0.x + path->p1.x);
	_plotter->meta_pos.y = 0.5 * (path->p0.y + path->p1.y);
      }
      break;
      
    case (int)PATH_CIRCLE:
      {
	_m_emit_op_code (R___(_plotter) O_FCIRCLE);
	_m_emit_float (R___(_plotter) path->pc.x);
	_m_emit_float (R___(_plotter) path->pc.y);
	_m_emit_float (R___(_plotter) path->radius);
	_m_emit_terminator (S___(_plotter));
	
	_plotter->meta_pos = path->pc;
      }
      break;
      
    case (int)PATH_ELLIPSE:
      {
	_m_emit_op_code (R___(_plotter) O_FELLIPSE);
	_m_emit_float (R___(_plotter) path->pc.x);
	_m_emit_float (R___(_plotter) path->pc.y);
	_m_emit_float (R___(_plotter) path->rx);
	_m_emit_float (R___(_plotter) path->ry);
	_m_emit_float (R___(_plotter) path->angle);
	_m_emit_terminator (S___(_plotter));
	
	_plotter->meta_pos = path->pc;
      }
      break;

    default:			/* shouldn't happen */
      break;
    }
}

bool
#ifdef _HAVE_PROTOS
_m_path_is_flushable (S___(Plotter *_plotter))
#else
_m_path_is_flushable (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  return true;
}

void
#ifdef _HAVE_PROTOS
_m_maybe_prepaint_segments (R___(Plotter *_plotter) int prev_num_segments)
#else
_m_maybe_prepaint_segments (R___(_plotter) prev_num_segments)
     S___(Plotter *_plotter;)
     int prev_num_segments;
#endif
{
  return;
}

