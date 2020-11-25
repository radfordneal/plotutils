/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for HPGL Plotters, which do not plot in real time, but
   emit a page of graphics when closepl() is called. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_openpl (void)
#else
_h_openpl ()
#endif
{
  int i;

  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* Prepare buffer in which we'll cache graphics code for this page. */
  _plotter->page = _new_outbuf ();
  
  /* With each call to openpl(), we reset our knowledge of the HP-GL
     internal state, i.e. the dynamic derived-class-specific data members
     of the HPGL or PCL Plotter.  The values are the same as are used in
     initializing the Plotter (see h_defplot.c). */
     
  /* reset any soft-defined colors in the pen color array */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    if (_plotter->pen_defined[i] == 1) /* i.e. soft-defined */
      _plotter->pen_defined[i] = 0; /* i.e. undefined */

  /* reset current pen */
  _plotter->pen = 1;  

  /* if we can soft-define pen colors, reset free_pen data member by
     determining what the next free pen is */
  {
    
    bool undefined_pen_seen = false;
    
    if (_plotter->palette)	/* can soft-define pen colors */
      for (i = 2; i < HPGL2_MAX_NUM_PENS; i++)
	{
	  if (_plotter->pen_defined[i] == 0)
	    /* at least one pen with number > 1 is not yet defined */
	    {
	      /* record which such was encountered first */
	      _plotter->free_pen = i;
	      undefined_pen_seen = true;
	      break;
	    }
	}
    if (!undefined_pen_seen)	
      /* too many pens specified, can't soft-define colors */
      _plotter->palette = false;
  }
  
  /* reset additional data members of Plotter */
  _plotter->bad_pen = false;  
  _plotter->pendown = false;  
  _plotter->pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
  _plotter->hpgl_miter_limit = 5.0; /* default HP-GL/2 value */
  _plotter->fill_type = HPGL_FILL_SOLID_BI;
  _plotter->shading_level = 0.0;
  _plotter->pcl_symbol_set = PCL_ROMAN_8;  
  _plotter->pcl_spacing = 0;  
  _plotter->pcl_posture = 0;  
  _plotter->pcl_stroke_weight = 0;  
  _plotter->pcl_typeface = STICK_TYPEFACE;  
  _plotter->hpgl_charset_lower = HP_ASCII;
  _plotter->hpgl_charset_upper = HP_ASCII;
  _plotter->relative_char_height = 0.0;
  _plotter->relative_char_width = 0.0;  
  _plotter->relative_label_rise = 0.0;    
  _plotter->relative_label_run = 0.0;      
  _plotter->char_slant_tangent = 0.0;      
  _plotter->hpgl_position_is_unknown = true;
  _plotter->hpgl_pos.x = 0;
  _plotter->hpgl_pos.y = 0;

  /* if a PCL Plotter, switch from PCL mode to HP-GL/2 mode */
  _maybe_switch_to_hpgl ();

  /* output HP-GL prologue */
  if (_plotter->hpgl_version == 2)
    {
      sprintf (_plotter->page->point, "BP;IN;");
      _update_buffer (_plotter->page);
      sprintf (_plotter->page->point, "PS%d;",
	       IROUND(_plotter->plot_length));
      _update_buffer (_plotter->page);
    }
  else
    {
      sprintf (_plotter->page->point, "IN;");
      _update_buffer (_plotter->page);
    }
  
  /* rotate if user requested rotation */
  if (_plotter->rotation != 0)
    {
      sprintf (_plotter->page->point, "RO%d;", _plotter->rotation);
      _update_buffer (_plotter->page);
    }
  
  /* set scaling points P1, P2 at lower left and upper right corners of
     the region (square) that we call our ``graphics display'' */
  sprintf (_plotter->page->point, "IP%d,%d,%d,%d;",
	   IROUND(_plotter->p1.x), IROUND(_plotter->p1.y),
	   IROUND(_plotter->p2.x), IROUND(_plotter->p2.y));
  _update_buffer (_plotter->page);
  
  /* Set up `scaled device coordinates' within the graphics display.  All
     coordinates in our output file will be scaled device coordinates,
     not physical device coordinates. */
  sprintf (_plotter->page->point, "SC%d,%d,%d,%d;",
	   IROUND (_plotter->display_coors.left), 
	   IROUND (_plotter->display_coors.right), 
	   IROUND (_plotter->display_coors.bottom), 
	   IROUND (_plotter->display_coors.top));
  _update_buffer (_plotter->page);
  
  if (_plotter->hpgl_version == 2)
    {
      /* Begin to define a palette, by specifying a number of logical pens.
	 (All HP-GL/2 devices should support the `NP' instruction, even
	 though many support only a default palette.) */
      if (_plotter->palette)
	{
	  sprintf (_plotter->page->point, "NP%d;", HPGL2_MAX_NUM_PENS);
	  _update_buffer (_plotter->page);
	}
      /* use relative units for pen width */
      sprintf (_plotter->page->point, "WU1;");
      _update_buffer (_plotter->page);
    }
  
  /* select pen #1 (standard plotting convention) */
  sprintf (_plotter->page->point, "SP1;");
  _update_buffer (_plotter->page);
  
  /* For HP-GL/2 devices, set transparency mode to `opaque', if the user
     allows it.  It should always be opaque to agree with libplot
     conventions, but on some HP-GL/2 devices (mostly pen plotters) the
     `TR' command does not NOP gracefully. */
  if (_plotter->hpgl_version == 2 && _plotter->opaque_mode)
    {
      sprintf (_plotter->page->point, "TR0;");
      _update_buffer (_plotter->page);
    }

  /* freeze contents of output buffer, i.e. the initialization code we've
     just written to it, so that any later invocation of erase() won't
     remove it */
  _freeze_outbuf (_plotter->page);

  /* invoke generic method, to e.g. create drawing state */
  _g_openpl ();

  return 0;
}

void
#ifdef _HAVE_PROTOS
_h_maybe_switch_to_hpgl (void)
#else
_h_maybe_switch_to_hpgl ()
#endif
{
}

void
#ifdef _HAVE_PROTOS
_q_maybe_switch_to_hpgl (void)
#else
_q_maybe_switch_to_hpgl ()
#endif
{
  if (_plotter->page_number > 0) /* not first page */
    /* eject previous page, by issuing PCL command */
    {	
      strcpy (_plotter->page->point, "\f"); /* i.e. form feed */
      _update_buffer (_plotter->page);
    }
  /* switch from PCL 5 to HP-GL/2 mode */
  strcpy (_plotter->page->point, "\033%0B\n");
  _update_buffer (_plotter->page);
}
