/* This file contains a low-level routine for synchronizing the line style
   of a Tektronix device with libplottek's notion of what the line style
   ought to be. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

void
_tek_linemod ()
{
  if ((!(_tekstate.line_type_is_unknown))
      && (_tekstate.line_type == _drawstate->tek_line_type))
    return;

  switch (_drawstate->tek_line_type)
    {
    default:
    case L_SOLID:
      fputs ("\033`", _outstream); /* ASCII ESC `, i.e. ^[` */
      break;
    case L_DOTTED:
      fputs ("\033a", _outstream); /* ASCII ESC a, i.e. ^[a */
      break;
    /* following two are interchanged in kermit emulator */
    case L_DOTDASHED:
      if (_tek_display_type == D_KERMIT)
	fputs ("\033c", _outstream); /* ASCII ESC c, i.e. ^[c */
      else
	fputs ("\033b", _outstream); /* ASCII ESC b, i.e. ^[b */
      break;
    case L_SHORTDASHED:
      if (_tek_display_type == D_KERMIT)
	fputs ("\033b", _outstream); /* ASCII ESC b, i.e. ^[b */
      else
	fputs ("\033c", _outstream); /* ASCII ESC c, i.e. ^[c */
      break;
    case L_LONGDASHED:
      /* in kermit emulator, the following switches to "dotlongdashed"
	 rather than "longdashed", but we can live with that */
      fputs ("\033d", _outstream); /* ASCII ESC d, i.e. ^[d */
      break;
    }

  _tekstate.line_type = _drawstate->tek_line_type; /* Tek now agrees with us */
  _tekstate.line_type_is_unknown = FALSE;
}  
