/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for Tek Plotters.  We emit terminal initialization
   codes, depending on the Tektronix type. */

#include "sys-defines.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_t_openpl (void)
#else
_t_openpl ()
#endif
{
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* invoke generic method, to e.g. create drawing state */
  _g_openpl ();

  /* send graphics initialization commands to output stream if necessary */

  if (_plotter->tek_display_type == D_XTERM 
      || _plotter->tek_display_type == D_KERMIT)
    /* VT340 command, put xterm / kermit into Tek mode.*/
    _plotter->write_string ("\033[?38h");

  if (_plotter->tek_display_type == D_KERMIT)
    {
      const char *bg_color_name_s;

      /* if there's a user-specified background color, set it; it will take
	 effect only when erase() is invoked */
      bg_color_name_s = (const char *)_get_plot_param ("BG_COLOR");
      if (bg_color_name_s)
	_plotter->bgcolorname (bg_color_name_s);
    }

  return 0;
}
