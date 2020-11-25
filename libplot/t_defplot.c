/* This file defines the initialization for any TekPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

/* Note that Tektronix displays are rectangular, and wider than they are
   high: the aspect ratio is approximately 4:3.  In terms of integer
   Tektronix coordinates the Tektronix display is a [0..4095]x[0..3119]
   rectangle, and we choose our viewport to be the square
   [488..3607]x[0..3119].  I.e. we define it to be a square, occupying the
   entire height of the display, and centered on the display. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a TekPlotter struct. */
const Plotter _t_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _t_initialize, _t_terminate,
  /* page manipulation */
  _t_begin_page, _t_erase_page, _t_end_page,
  /* drawing state manipulation */
  _g_push_state, _g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _g_paint_path, _g_paint_paths, _t_path_is_flushable, _t_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _t_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_paint_text_string_with_escapes, _g_paint_text_string,
  _g_get_text_width,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* `flush output' method, called only if Plotter handles its own output */
  _g_flush_output,
  /* error handlers */
  _g_warning,
  _g_error,
};
#endif /* not LIBPLOTTER */

/* The private `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  When this
   is invoked, _plotter points to the Plotter that has just been
   created. */

void
#ifdef _HAVE_PROTOS
_t_initialize (S___(Plotter *_plotter))
#else
_t_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_TEK;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 0;
  _plotter->data->have_dash_array = 0;
  _plotter->data->have_solid_fill = 0;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 0;
  _plotter->data->have_escaped_string_support = 0;
  _plotter->data->have_ps_fonts = 0;
  _plotter->data->have_pcl_fonts = 0;
  _plotter->data->have_stick_fonts = 0;
  _plotter->data->have_extra_stick_fonts = 0;
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->data->default_font_type = F_HERSHEY;
  _plotter->data->pcl_before_ps = false;
  _plotter->data->have_horizontal_justification = false;
  _plotter->data->have_vertical_justification = false;
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal) */
  _plotter->data->max_unfilled_path_length = MAX_UNFILLED_PATH_LENGTH;
  _plotter->data->have_mixed_paths = false;
  _plotter->data->allowed_arc_scaling = AS_NONE;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;  
  _plotter->data->allowed_quad_scaling = AS_NONE;  
  _plotter->data->allowed_cubic_scaling = AS_NONE;  
  _plotter->data->allowed_box_scaling = AS_NONE;
  _plotter->data->allowed_circle_scaling = AS_NONE;
  _plotter->data->allowed_ellipse_scaling = AS_NONE;

  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->data->flipped_y = false;
  _plotter->data->imin = 488;
  _plotter->data->imax = 3607;  
  _plotter->data->jmin = 0;
  _plotter->data->jmax = 3119;  
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 0.0;  
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 0.0;  
  _plotter->data->page_data = (plPageData *)NULL;

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* initialize data members specific to this derived class */
  _plotter->tek_display_type = D_GENERIC;
  _plotter->tek_mode = MODE_ALPHA;
  _plotter->tek_line_type = L_SOLID;
  _plotter->tek_mode_is_unknown = true;
  _plotter->tek_line_type_is_unknown = true;
  _plotter->tek_kermit_fgcolor = -1; /* nonsensical value; means `unknown' */
  _plotter->tek_kermit_bgcolor = -1; /* same */
  _plotter->tek_position_is_unknown = true;
  _plotter->tek_pos.x = 0;
  _plotter->tek_pos.y = 0;

  /* initialize certain data members from device driver parameters */

  /* determine Tek display type (xterm(1) / kermit(1) / generic Tektronix);
     there are so many possible termcap/terminfo names out there that we
     key only on an initial substring */
  {
    const char* term_type;
    
    term_type = (const char *)_get_plot_param (_plotter->data, "TERM");
    if (term_type != NULL)
      {
	if (strncmp (term_type, "xterm", 5) == 0
	    || strncmp (term_type, "nxterm", 6) == 0
	    || strncmp (term_type, "kterm", 5) == 0)
	  _plotter->tek_display_type = D_XTERM;
	else if (strncmp (term_type, "ansi.sys", 8) == 0
		 || strncmp (term_type, "nansi.sys", 9) == 0
		 || strncmp (term_type, "ansisys", 7) == 0 /* undocumented */
		 || strncmp (term_type, "kermit", 6) == 0)
	  _plotter->tek_display_type = D_KERMIT;
	else
	  _plotter->tek_display_type = D_GENERIC;
      }
    else
      _plotter->tek_display_type = D_GENERIC; /* default value */
  }      
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  It may do such things as write
   to an output stream from internal storage, deallocate storage, etc.
   When this is invoked, _plotter points to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_t_terminate (S___(Plotter *_plotter))
#else
_t_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
TekPlotter::TekPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (ostream& out)
	: Plotter (out)
{
  _t_initialize ();
}

TekPlotter::TekPlotter ()
{
  _t_initialize ();
}

TekPlotter::TekPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _t_initialize ();
}

TekPlotter::TekPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _t_initialize ();
}

TekPlotter::~TekPlotter ()
{
  _t_terminate ();
}
#endif
