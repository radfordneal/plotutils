/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes of a ReGIS display to what they should be. */

#include "sys-defines.h"
#include "extern.h"

/* ReGIS 8-bit `patterns', i.e., line types, indexed into by our internal
   line style number (L_SOLID/L_DOTTED/
   L_DOTDASHED/L_SHORTDASHED/L_LONGDASHED/L_DOTDOTDASHED/L_DOTDOTDOTDASHED).

   ReGIS supports standard patterns P0..P9, and user-specified patterns
   made up of 2 to 8 bits.  If fewer than 8 bits are supplied, ReGIS
   repeats as much of the pattern as possible in what remains of the 8-bit
   segment.  Standard pattern P1 is solid. */

static const char *_regis_line_types[] =
{ "P1", "P1000", 
  "P11100100", "P11110000", "P11111100", "P11101010", "P10" };

void
#ifdef _HAVE_PROTOS
_r_set_attributes (S___(Plotter *_plotter))
#else
_r_set_attributes (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (_plotter->regis_line_type_is_unknown
      || _plotter->regis_line_type != _plotter->drawstate->line_type)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "W(%s)\n",
	       _regis_line_types[_plotter->drawstate->line_type]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_line_type_is_unknown = false;
      _plotter->regis_line_type = _plotter->drawstate->line_type;
    }
}
