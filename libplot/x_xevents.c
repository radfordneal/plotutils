#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* _handle_x_events() handles any pending X events.  We call this at the
   end of most of the XPlotter methods.  (Since the same methods are used
   by XDrawablePlotters, this gets called when invoking drawing operations
   on an XDrawablePlotter too, but is a no-op.)

   Once closepl() has been invoked on an XPlotter, the window popped up by
   openpl() is managed by a forked-off process via XtAppMainLoop().  But
   until that time, it is useful to call this function occasionally.  A
   kludge of course, but what else can we do?

   More than one XPlotter may be open at a time, in which case
   _handle_x_events() will update the windows associated with each open
   connection (since libplot uses only a single X application context, for
   all XPlotters). */

/* external function in api.c, at least for the C binding */
extern void _process_other_plotter_events __P ((Plotter *plotter));

void
#ifdef _HAVE_PROTOS
_handle_x_events(void)
#else
_handle_x_events()
#endif
{
  if (_plotter->type == PL_X11)
    /* an XPlotter, not an XDrawablePlotter, so need to handle events */
    {
      _process_other_plotter_events (_plotter);

      while (XtAppPending (_plotter->app_con))
	XtAppProcessEvent (_plotter->app_con, XtIMAll);
    }
}
