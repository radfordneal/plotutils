/* This file defines the initialization for any PNG Plotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a PNGPlotter struct.  It is the same as for a BitmapPlotter, except for
   the routines _z_initialize and _z_terminate. */
const Plotter _z_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _z_initialize, _z_terminate,
  /* page manipulation */
  _b_begin_page, _b_erase_page, _b_end_page,
  /* drawing state manipulation */
  _g_push_state, _g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _b_paint_path, _b_paint_paths, _g_path_is_flushable, _g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _g_paint_marker, _b_paint_point,
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
_z_initialize (S___(Plotter *_plotter))
#else
_z_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _b_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_PNG;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES;

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->z_interlace = false;
  _plotter->z_transparent = false;  
  _plotter->z_transparent_color.red = 255; /* dummy */
  _plotter->z_transparent_color.green = 255; /* dummy */
  _plotter->z_transparent_color.blue = 255; /* dummy */

  /* initialize certain data members from device driver parameters */
      
  /* produce an interlaced PNG? */
  {
    const char *interlace_s;

    interlace_s = (const char *)_get_plot_param (_plotter->data, "INTERLACE" );
    if (strcasecmp (interlace_s, "yes") == 0)
      _plotter->z_interlace = true;
  }

  /* is there a user-specified transparent color? */
  {
    const char *transparent_name_s;
    plColor color;

    transparent_name_s = (const char *)_get_plot_param (_plotter->data, "TRANSPARENT_COLOR");
    if (transparent_name_s 
	&& _string_to_color (transparent_name_s, &color, _plotter->data->color_name_cache))
      /* have 24-bit RGB */
      {
	_plotter->z_transparent = true;
	_plotter->z_transparent_color = color;
      }
  }
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_z_terminate (S___(Plotter *_plotter))
#else
_z_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _b_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
PNGPlotter::PNGPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	: BitmapPlotter (infile, outfile, errfile)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (FILE *outfile)
	: BitmapPlotter (outfile)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (istream& in, ostream& out, ostream& err)
	: BitmapPlotter (in, out, err)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (ostream& out)
	: BitmapPlotter (out)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter ()
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	: BitmapPlotter (infile, outfile, errfile, parameters)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (FILE *outfile, PlotterParams &parameters)
	: BitmapPlotter (outfile, parameters)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: BitmapPlotter (in, out, err, parameters)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (ostream& out, PlotterParams &parameters)
	: BitmapPlotter (out, parameters)
{
  _z_initialize ();
}

PNGPlotter::PNGPlotter (PlotterParams &parameters)
	: BitmapPlotter (parameters)
{
  _z_initialize ();
}

PNGPlotter::~PNGPlotter ()
{
  _z_terminate ();
}
#endif
