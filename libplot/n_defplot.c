/* This file defines the initialization for any PNM Plotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a PNMPlotter struct.  It is the same as for a BitmapPlotter, except for
   the routines _n_initialize and _n_terminate. */
const Plotter _n_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _n_initialize, _n_terminate,
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
_n_initialize (S___(Plotter *_plotter))
#else
_n_initialize (S___(_plotter))
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
  _plotter->data->type = PL_PNM;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES;

  /* initialize data members specific to this derived class */
  _plotter->n_portable_output = false;

  /* initialize certain data members from device driver parameters */
      
  /* determine version of PBM/PGM/PPM format (binary or ascii) */
  {
    const char *portable_s;
    
    portable_s = (const char *)_get_plot_param (_plotter->data, "PNM_PORTABLE");
    if (strcasecmp (portable_s, "yes") == 0)
      _plotter->n_portable_output = true;
    else
      _plotter->n_portable_output = false; /* default value */
  }

}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
#ifdef _HAVE_PROTOS
_n_terminate (S___(Plotter *_plotter))
#else
_n_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _b_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
PNMPlotter::PNMPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	: BitmapPlotter (infile, outfile, errfile)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (FILE *outfile)
	: BitmapPlotter (outfile)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (istream& in, ostream& out, ostream& err)
	: BitmapPlotter (in, out, err)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (ostream& out)
	: BitmapPlotter (out)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter ()
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	: BitmapPlotter (infile, outfile, errfile, parameters)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (FILE *outfile, PlotterParams &parameters)
	: BitmapPlotter (outfile, parameters)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: BitmapPlotter (in, out, err, parameters)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (ostream& out, PlotterParams &parameters)
	: BitmapPlotter (out, parameters)
{
  _n_initialize ();
}

PNMPlotter::PNMPlotter (PlotterParams &parameters)
	: BitmapPlotter (parameters)
{
  _n_initialize ();
}

PNMPlotter::~PNMPlotter ()
{
  _n_terminate ();
}
#endif
