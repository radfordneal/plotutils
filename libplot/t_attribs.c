/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes of a Tektronix display (line type only, since
   cap type, join type, line width are not supported) to what they should be.*/

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_t_set_attributes (void)
#else
_t_set_attributes ()
#endif
{
  FILE *stream;
  
  stream = _plotter->outstream;
  if (stream == NULL)
    return;

  if ((!(_plotter->line_type_is_unknown))
      && (_plotter->line_type == _plotter->drawstate->line_type))
    return;

  switch (_plotter->drawstate->line_type)
    {
    default:
    case L_SOLID:
      fputs ("\033`", stream);	/* ASCII ESC `, i.e. ^[` */
      break;
    case L_DOTTED:
      fputs ("\033a", stream);	/* ASCII ESC a, i.e. ^[a */
      break;
      /* following two are interchanged in kermit emulator */
    case L_DOTDASHED:
      if (_plotter->display_type == D_KERMIT)
	fputs ("\033c", stream); /* ASCII ESC c, i.e. ^[c */
      else
	fputs ("\033b", stream); /* ASCII ESC b, i.e. ^[b */
      break;
    case L_SHORTDASHED:
      if (_plotter->display_type == D_KERMIT)
	fputs ("\033b", stream); /* ASCII ESC b, i.e. ^[b */
      else
	fputs ("\033c", stream); /* ASCII ESC c, i.e. ^[c */
      break;
    case L_LONGDASHED:
      /* in kermit emulator, the following switches to "dotlongdashed"
	 rather than "longdashed", but we can live with that */
      fputs ("\033d", stream);	/* ASCII ESC d, i.e. ^[d */
      break;
    }

  /* Tek now agrees with us on line type */
  _plotter->line_type = _plotter->drawstate->line_type;
  _plotter->line_type_is_unknown = false;
}  
