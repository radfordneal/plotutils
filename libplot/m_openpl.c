#include "sys-defines.h"
#include "extern.h"

bool
#ifdef _HAVE_PROTOS
_m_begin_page (S___(Plotter *_plotter))
#else
_m_begin_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  if (_plotter->data->page_number == 1)
    /* emit metafile header, i.e. magic string */
    {
      _write_string (_plotter->data, PLOT_MAGIC);

      /* format type 1 = GNU binary, type 2 = GNU portable */
      if (_plotter->meta_portable_output)
	_write_string (_plotter->data, " 2\n");
      else
	_write_string (_plotter->data, " 1\n");
    }
  
  _m_emit_op_code (R___(_plotter) O_OPENPL);
  _m_emit_terminator (S___(_plotter));

  /* reset page-specific, i.e. picture-specific, dynamic variables */
  _plotter->meta_pos.x = 0.0;
  _plotter->meta_pos.y = 0.0;
  _plotter->meta_position_is_unknown = false;
  _plotter->meta_m_user_to_ndc[0] = 1.0;
  _plotter->meta_m_user_to_ndc[1] = 0.0;
  _plotter->meta_m_user_to_ndc[2] = 0.0;
  _plotter->meta_m_user_to_ndc[3] = 1.0;
  _plotter->meta_m_user_to_ndc[4] = 0.0;
  _plotter->meta_m_user_to_ndc[5] = 0.0;
  _plotter->meta_fill_rule_type = FILL_ODD_WINDING;
  _plotter->meta_line_type = L_SOLID;
  _plotter->meta_points_are_connected = true;  
  _plotter->meta_cap_type = CAP_BUTT;  
  _plotter->meta_join_type = JOIN_MITER;  
  _plotter->meta_miter_limit = DEFAULT_MITER_LIMIT;  
  _plotter->meta_line_width = 0.0;
  _plotter->meta_line_width_is_default = true;
  _plotter->meta_dash_array = (const double *)NULL;
  _plotter->meta_dash_array_len = 0;
  _plotter->meta_dash_offset = 0.0;  
  _plotter->meta_dash_array_in_effect = false;  
  _plotter->meta_pen_type = 1;  
  _plotter->meta_fill_type = 0;
  _plotter->meta_orientation = 1;  
  _plotter->meta_font_name = (const char *)NULL;
  _plotter->meta_font_size = 0.0;
  _plotter->meta_font_size_is_default = true;
  _plotter->meta_text_rotation = 0.0;  
  _plotter->meta_fgcolor.red = 0;
  _plotter->meta_fgcolor.green = 0;
  _plotter->meta_fgcolor.blue = 0;
  _plotter->meta_fillcolor_base.red = 0;
  _plotter->meta_fillcolor_base.green = 0;
  _plotter->meta_fillcolor_base.blue = 0;
  _plotter->meta_bgcolor.red = 65535;
  _plotter->meta_bgcolor.green = 65535;
  _plotter->meta_bgcolor.blue = 65535;

  return true;
}
