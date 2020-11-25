/* This version is for both HPGLPlotters and PCLPlotters.  

   For HPGL Plotter objects, we output all plotted objects, which we have
   saved in a resizable outbuf structure for the current page.  An HP-GL or
   HP-GL/2 prologue and epilogue are included.  We then flush the output
   stream, and reset all datastructures. */

#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_h_end_page (S___(Plotter *_plotter))
#else
_h_end_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* output HP-GL epilogue to page buffer */

  if (_plotter->hpgl_pendown == true)
    /* lift pen */
    {
      sprintf (_plotter->data->page->point, "PU;");
      _update_buffer (_plotter->data->page);
    }
  /* move to lower left hand corner */
  sprintf (_plotter->data->page->point, "PA0,0;");
  _update_buffer (_plotter->data->page);

  /* select pen zero, i.e. return pen to carousel */
  if (_plotter->hpgl_pen != 0)
    {
      sprintf (_plotter->data->page->point, "SP0;");
      _update_buffer (_plotter->data->page);
    }

  if (_plotter->hpgl_version >= 1)
    /* have a `page advance' command, so use it */
    {
      sprintf (_plotter->data->page->point, "PG0;");
      _update_buffer (_plotter->data->page);
    }

  /* add newline at end */
  sprintf (_plotter->data->page->point, "\n");
  _update_buffer (_plotter->data->page);

  /* if a PCL Plotter, switch back from HP-GL/2 mode to PCL mode */
  _maybe_switch_from_hpgl (S___(_plotter));
  
  /* set this, so that no drawing on the next page will take place without
     a pen advance */
  _plotter->hpgl_position_is_unknown = true;

  _plotter->hpgl_pendown = false; /* be on the safe side */

  return true;
}

void
#ifdef _HAVE_PROTOS
_h_maybe_switch_from_hpgl (S___(Plotter *_plotter))
#else
_h_maybe_switch_from_hpgl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
}

void
#ifdef _HAVE_PROTOS
_q_maybe_switch_from_hpgl (S___(Plotter *_plotter))
#else
_q_maybe_switch_from_hpgl (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  /* switch back from HP-GL/2 to PCL 5 mode */
  strcpy (_plotter->data->page->point, "\033%0A");
  _update_buffer (_plotter->data->page);
}
