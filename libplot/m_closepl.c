/* Plotutils+ is copyright (C) 2020 Radford M. Neal.

   Based on the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, Free Software Foundation, Inc.

   The GNU plotutils package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU plotutils package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_m_end_page (S___(Plotter *_plotter))
{
  bool retval = true;

  retval = 0;

  _pl_m_emit_op_code (R___(_plotter) O_CLOSEPL);
  _pl_m_emit_terminator (S___(_plotter));

  if (_plotter->data->outfp)
    {
      if (fflush(_plotter->data->outfp) < 0)
	retval = false;
#ifdef MSDOS
      /* data can be caught in DOS buffers, so do an fsync() too */
      if (fsync (_plotter->data->outfp) < 0)
	retval = false;
#endif
    }
#ifdef LIBPLOTTER
  if (_plotter->data->outstream)
    {
      _plotter->data->outstream->flush ();
      if (!(*(_plotter->data->outstream)))
	retval = false;
    }
#endif

  /* clean up device-specific Plotter members that are heap-allocated */
  if (_plotter->meta_font_name != (const char *)NULL)
    free ((char *)_plotter->meta_font_name);

  if (_plotter->meta_dash_array != (const double *)NULL)
    free ((double *)_plotter->meta_dash_array);

  return true;
}

