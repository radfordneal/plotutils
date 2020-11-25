/* This internal method is invoked by a CGMPlotter before drawing any path.
   It sets the relevant CGM attributes (line type, cap type, join type,
   line width) to what they should be. */

#include "sys-defines.h"
#include "extern.h"

/* CGM join styles, indexed by internal number (miter/rd./bevel/triangular) */
const int _cgm_join_style[] =
{ CGM_JOIN_MITER, CGM_JOIN_ROUND, CGM_JOIN_BEVEL, CGM_JOIN_ROUND };

/* CGM cap styles, indexed by internal number (butt/rd./project/triangular) */
const int _cgm_cap_style[] =
{ CGM_CAP_BUTT, CGM_CAP_ROUND, CGM_CAP_PROJECTING, CGM_CAP_TRIANGULAR };

void
#ifdef _HAVE_PROTOS
_c_set_attributes (S___(Plotter *_plotter))
#else
_c_set_attributes (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* just before this function is called, a hint is passed, specifying
     whether a closed path, an open path, or a marker etc. is to be drawn */
  int object_type = _plotter->drawstate->cgm_object_type;
  int desired_width = _plotter->drawstate->quantized_device_line_width;
  int desired_line_type = CGM_L_SOLID; /* keep compiler happy */
  double desired_dash_offset = 0.0;

  /* alter CGM line width if necessary */

  switch (object_type)
    {
    case CGM_OBJECT_OPEN:
      if (_plotter->cgm_line_width != desired_width)
	/* emit "LINE WIDTH" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = CGM_BINARY_BYTES_PER_INTEGER;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 3,
				    data_len, &byte_count,
				    "LINEWIDTH");
	  _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			     desired_width,
			     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update line width */
	  _plotter->cgm_line_width = desired_width;
	}
      break;
    case CGM_OBJECT_CLOSED:
      if (_plotter->cgm_edge_width != desired_width)
	/* emit "EDGE WIDTH" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = CGM_BINARY_BYTES_PER_INTEGER;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 28,
				    data_len, &byte_count,
				    "EDGEWIDTH");
	  _cgm_emit_integer (_plotter->page, false, _plotter->cgm_encoding,
			     desired_width,
			     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update edge width */
	  _plotter->cgm_edge_width = desired_width;
	}
      break;
    default:			/* shouldn't happen */
      break;
    }
  
  /* determine line type */

  if (_plotter->have_dash_array && _plotter->drawstate->dash_array_in_effect)
    /* user specified a dash array, and this version of CGM supports them;
       so compute a CGM-style dash array, and maybe add a new line type to
       page-specific line type table */
    {
      int num_dashes = _plotter->drawstate->dash_array_len;
      int our_num_dashes = num_dashes; /* will double if array length is odd */

      if (num_dashes > 0)
	/* non-solid line type */
	{
	  double min_sing_val, max_sing_val;
	  int i, *dashbuf;
	  plCGMCustomLineType *line_type_ptr, *old_line_type_ptr;
	  int line_type;
	  bool odd_length, matched_line_type;
	  
	  /* compute minimum singular value of user->device coordinate
	     map, which we use as a multiplicative factor to convert
	     line widths (cf. g_linewidth.c), dash lengths, etc. */
	  _matrix_sing_vals (_plotter->drawstate->transform.m, 
			     &min_sing_val, &max_sing_val);
	  

	  /* double array length if odd (we don't trust CGM interpreters to
             handle odd-length dash arrays in the way that PS does) */
	  odd_length = (num_dashes % 2 != 0 ? true : false);
	  if (odd_length)
	    our_num_dashes *= 2;

	  dashbuf = (int *)_plot_xmalloc (our_num_dashes * sizeof(int));
	  for (i = 0; i < num_dashes; i++)
	    {
	      double dashlen;
	      int i_dashlen;
	      
	      dashlen =
		min_sing_val * _plotter->drawstate->dash_array[i];
	      i_dashlen = IROUND(dashlen);
	      if (i_dashlen == 0 && dashlen > 0.0)
		i_dashlen = 1;	/* don't use 0 if user specified non-0 */
	      dashbuf[i] = i_dashlen;
	      if (odd_length)
		dashbuf[i + num_dashes] = i_dashlen;
	    }
	  desired_dash_offset = 
	    min_sing_val * _plotter->drawstate->dash_offset;

	  /* search table of user-defined, page-specific CGM line types;
	     they're numbered -1, -2, -3, ... */
	  line_type_ptr = (plCGMCustomLineType *)_plotter->page->extra;
	  old_line_type_ptr = (plCGMCustomLineType *)NULL;
	  line_type = 0;
	  matched_line_type = false;
	  while (line_type_ptr != (plCGMCustomLineType *)NULL)
	    {
	      line_type--;

	      if (line_type_ptr->dash_array_len == our_num_dashes)
		{
		  bool foundit = true;

		  for (i = 0; i < our_num_dashes; i++)
		    {
		      if (dashbuf[i] != line_type_ptr->dashes[i])
			{
			  foundit = false;
			  break; /* break out of for loop */
			}
		    }
		  if (foundit)
		    {
		      matched_line_type = true;
		      break;	/* break out of while loop */
		    }
		}

	      /* on to next entry in line type table */
	      old_line_type_ptr = line_type_ptr;
	      line_type_ptr = line_type_ptr->next;
	    }
	  /* on exit from while(), either matched_line_type = true (with
	     line_type set correctly), or old_line_type_ptr points to tail
	     of line type list, and `line_type' is the last valid type */

	  if (matched_line_type)
	    {
	      desired_line_type = line_type;
	      free (dashbuf);
	    }
	  else
	    {
	      /* construct new record from `dashbuf', add to tail of list;
		 `dashbuf' and the record will be freed when the page is
		 written out */
	      plCGMCustomLineType *newguy;

	      newguy = (plCGMCustomLineType *)_plot_xmalloc (sizeof(plCGMCustomLineType));
	      newguy->dashes = dashbuf;
	      newguy->dash_array_len = our_num_dashes;
	      newguy->next = (plCGMCustomLineType *)NULL;
	      if (old_line_type_ptr != (plCGMCustomLineType *)NULL)
		old_line_type_ptr->next = newguy;
	      else
		_plotter->page->extra = newguy;

	      /* new line type index is one less than most negative
		 previously defined index */
	      desired_line_type = line_type - 1;
	    }
	}
      else
	/* zero-length dash array, i.e. solid line type */
	{
	  desired_line_type = CGM_L_SOLID;
	  desired_dash_offset = 0.0;
	}
    }
  else
    /* dash array not in effect or cannot be used, use one of CGM's
       canonical line types instead */
    {
      switch (_plotter->drawstate->line_type)
	{
	case L_SOLID:
	default:
	  desired_line_type = CGM_L_SOLID;
	  break;
	case L_DOTTED:
	  desired_line_type = CGM_L_DOTTED;
	  break;
	case L_DOTDASHED:
	  desired_line_type = CGM_L_DOTDASHED;
	  break;
	case L_SHORTDASHED:
	  desired_line_type = CGM_L_DASHED;
	  break;
	case L_LONGDASHED:
	  /* can't distinguish from shortdashed */
	  desired_line_type = CGM_L_DASHED;
	  break;
	case L_DOTDOTDASHED:
	  desired_line_type = CGM_L_DOTDOTDASHED;
	  break;
	case L_DOTDOTDOTDASHED:
	  /* map to "dotdotdashed" */
	  desired_line_type = CGM_L_DOTDOTDASHED;
	  break;
	}

      desired_dash_offset = 0.0;
    }
      
  switch (object_type)
    {
    case CGM_OBJECT_OPEN:
      if (_plotter->cgm_line_type != desired_line_type)
	/* emit "LINE TYPE" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = 2;		/* 2 bytes per index */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 2,
				    data_len, &byte_count,
				    "LINETYPE");
	  _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			   desired_line_type,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update line type */
	  _plotter->cgm_line_type = desired_line_type;
	}

      if (_plotter->cgm_max_version >= 3
	  && _plotter->cgm_dash_offset != desired_dash_offset)
	/* emit "LINE TYPE INITIAL OFFSET" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = 4;		/* 4 bytes per fixed-pt. real */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 40,
				    data_len, &byte_count,
				    "LINETYPEINITOFFSET");
	  _cgm_emit_real_fixed_point (_plotter->page, false, _plotter->cgm_encoding,
				      desired_dash_offset,
				      data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update dash offset, and CGM version needed for this page */
	  _plotter->cgm_dash_offset = desired_dash_offset;
	  _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	}
      break;

    case CGM_OBJECT_CLOSED:
      if (_plotter->cgm_edge_type != desired_line_type)
	/* emit "EDGE TYPE" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = 2;		/* 2 bytes per index */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 27,
				    data_len, &byte_count,
				    "EDGETYPE");
	  _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			   desired_line_type,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update edge type */
	  _plotter->cgm_edge_type = desired_line_type;
	}

      if (_plotter->cgm_max_version >= 3
	  && _plotter->cgm_edge_dash_offset != desired_dash_offset)
	/* emit "EDGE TYPE INITIAL OFFSET" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = 4;		/* 4 bytes per fixed-pt. real */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 47,
				    data_len, &byte_count,
				    "EDGETYPEINITOFFSET");
	  _cgm_emit_real_fixed_point (_plotter->page, false, _plotter->cgm_encoding,
				      desired_dash_offset,
				      data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update dash offset, and CGM version needed for this page */
	  _plotter->cgm_edge_dash_offset = desired_dash_offset;
	  _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	}
      break;

    default:
      break;
    }

  if (_plotter->cgm_max_version >= 3)
    /* have line/edge cap/join style, and miter limit commands */
    {
      int desired_join_style = _cgm_join_style[_plotter->drawstate->join_type];
      int desired_cap_style = _cgm_cap_style[_plotter->drawstate->cap_type];
      double desired_miter_limit = _plotter->drawstate->miter_limit;
      
      switch (object_type)
	{
	case CGM_OBJECT_OPEN:
	  if (_plotter->cgm_cap_style != desired_cap_style)
	    /* emit "LINE CAP" command */
	    {
	      int byte_count, data_byte_count, data_len;
	      
	      /* set line cap style */
	      data_len = 2 * 2;	/* 2 bytes per index */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 37,
					data_len, &byte_count,
					"LINECAP");
	      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			       desired_cap_style,
			       data_len, &data_byte_count, &byte_count);
	      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			       CGM_DASH_CAP_MATCH,
			       data_len, &data_byte_count, &byte_count);
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update cap style, and CGM version needed for this page */
	      _plotter->cgm_cap_style = desired_cap_style;
	      _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	    }
	  break;
	case CGM_OBJECT_CLOSED:
	  if (_plotter->cgm_edge_cap_style != desired_cap_style)
	    /* emit "EDGE CAP" command */
	    {
	      int byte_count, data_byte_count, data_len;
	      
	      data_len = 2 * 2;	/* 2 bytes per index */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 44,
					data_len, &byte_count,
					"EDGECAP");
	      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			       desired_cap_style,
			       data_len, &data_byte_count, &byte_count);
	      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			       CGM_DASH_CAP_MATCH,
			       data_len, &data_byte_count, &byte_count);
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update edge cap style, and CGM version needed for this page */
	      _plotter->cgm_edge_cap_style = desired_cap_style;
	      _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	    }
	  break;
	default:
	  break;
	}
  
      switch (object_type)
	{
	case CGM_OBJECT_OPEN:
	  if (_plotter->cgm_join_style != desired_join_style)
	    /* emit "LINE JOIN" command */
	    {
	      int byte_count, data_byte_count, data_len;
	      
	      /* set line join style */
	      data_len = 2;	/* 2 bytes per index */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 38,
					data_len, &byte_count,
					"LINEJOIN");
	      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			       desired_join_style,
			       data_len, &data_byte_count, &byte_count);
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update join style, and CGM version needed for this page */
	      _plotter->cgm_join_style = desired_join_style;
	      _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	    }
	  break;
	case CGM_OBJECT_CLOSED:
	  if (_plotter->cgm_edge_join_style != desired_join_style)
	    /* emit "EDGE JOIN" command */
	    {
	      int byte_count, data_byte_count, data_len;
	      
	      /* do it over again, this time for edge join style */
	      data_len = 2;	/* 2 bytes per index */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 45,
					data_len, &byte_count,
					"EDGEJOIN");
	      _cgm_emit_index (_plotter->page, false, _plotter->cgm_encoding,
			       desired_join_style,
			       data_len, &data_byte_count, &byte_count);
	      _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					    &byte_count);
	      /* update edge join style, and CGM version needed for this page*/
	      _plotter->cgm_edge_join_style = desired_join_style;
	      _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	    }
	  break;
	default:
	  break;
	}

      if (_plotter->cgm_miter_limit != desired_miter_limit)
	/* emit "MITRE LIMIT" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = 4;	/* 4 bytes per fixed-point real */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->page, _plotter->cgm_encoding,
				    CGM_CONTROL_ELEMENT, 19,
				    data_len, &byte_count,
				    "MITRELIMIT");
	  _cgm_emit_real_fixed_point (_plotter->page, false, _plotter->cgm_encoding,
				      desired_miter_limit,
				      data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update miter limit, and CGM version needed for this page */
	  _plotter->cgm_miter_limit = desired_miter_limit;
	  _plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	}
    }
}


