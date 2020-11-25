/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes of a Tektronix display (line type only, since
   cap type, join type, line width are not supported) to what they should
   be.

   Our canonical line types are the same as those of a Tektronix, except
   that we also support "dotdotdashed" lines.  So setting the line type is
   straightforward.  kermit's assignment of line types is different from
   that of a genuine Tektronix, though. */

#include "sys-defines.h"
#include "extern.h"

void
#ifdef _HAVE_PROTOS
_t_set_attributes (void)
#else
_t_set_attributes ()
#endif
{
  if ((!(_plotter->tek_line_type_is_unknown))
      && (_plotter->tek_line_type == _plotter->drawstate->line_type))
    return;

  switch (_plotter->drawstate->line_type)
    {
    default:
    case L_SOLID:
      _plotter->write_string ("\033`");	/* ASCII ESC `, i.e. ^[` */
      break;
    case L_DOTTED:
      _plotter->write_string ("\033a");	/* ASCII ESC a, i.e. ^[a */
      break;
      /* following two are interchanged in kermit emulator */
    case L_DOTDASHED:
      if (_plotter->tek_display_type == D_KERMIT)
	_plotter->write_string ("\033c"); /* ASCII ESC c, i.e. ^[c */
      else
	_plotter->write_string ("\033b"); /* ASCII ESC b, i.e. ^[b */
      break;
    case L_SHORTDASHED:
      if (_plotter->tek_display_type == D_KERMIT)
	_plotter->write_string ("\033b"); /* ASCII ESC b, i.e. ^[b */
      else
	_plotter->write_string ("\033c"); /* ASCII ESC c, i.e. ^[c */
      break;
    case L_LONGDASHED:
      /* in kermit emulator, the following switches to "dotlongdashed"
	 rather than "longdashed", but we can live with that */
      _plotter->write_string ("\033d");	/* ASCII ESC d, i.e. ^[d */
      break;
    case L_DOTDOTDASHED:
      if (_plotter->tek_display_type == D_KERMIT)
	_plotter->write_string ("\033e"); /* ASCII ESC e, i.e. ^[e */
      else
	/* not supported on a genuine Tektronix, so punt */
	_plotter->write_string ("\033b"); /* ASCII ESC b, i.e. ^[b */
      break;
    case L_DOTDOTDOTDASHED:
      /* not supported, so punt */
      _plotter->write_string ("\033b");/* ASCII ESC b, i.e. ^[b */
      break;
    }

  /* Tek now agrees with us on line type */
  _plotter->tek_line_type = _plotter->drawstate->line_type;
  _plotter->tek_line_type_is_unknown = false;
}  

/* The reason for the kermit-specific modifications above is that according
   to kermit documentation, the MS-DOS kermit Tektronix emulator has a
   different ordering for line types:

        ` = solid		11111111 11111111
        a = dotted      	10101010 10101010 
        b = shortdashed		11110000 11110000
        c = dotdashed           11111010 11111010
        d = dotlongdashed       11111111 11001100
        e = dotdotdashed        11111100 10010010
        x = user defined (by ESC / Pn a)
        y = user defined (by ESC / Pn b)
        z = user defined (by ESC / Pn c)
	
   Incidentally, line type characters recognized by VT-type terminals in
   Tektronix emulator mode also allegedly differ.  According to an old doc
   file,

        ` = solid
        a = dotted
	b = shortdashed
	c = dotdashed
	d = dotlongdashed
	h = solid (bold) 
        i = dotted (bold) 
        j = shortdashed (bold)
        k = dotdashed (bold)
        l = dotlongdashed (bold)

   Interestingly, BSD atoplot(1) recognizes "dotlongdashed",
   "dotshortdashed", and "dotdotdashed" (with final "ed" omitted), besides
   the five canonical Tektronix line types.  So when atoplot(1) was
   written, there must have been plot(1) filters for output devices that
   supported those additional types.  Presumably on VT-type terminals? */
