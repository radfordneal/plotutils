#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* _handle_x_events() handles any pending X events.  We call this at the
   end of most of the libplotX routines, since control isn't transferred to
   XtAppMainLoop() until closepl() is called.  A kludge of course, but what
   else can we do? */
void
_handle_x_events()
{
  while (XtAppPending (_xdata.app_con))
    {
      XEvent event;
      
      XtAppNextEvent (_xdata.app_con, &event);
      XtDispatchEvent (&event);
    }
}
