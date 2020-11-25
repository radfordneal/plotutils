/* This file contains four rather silly Plotter methods that are an
   undocumented part of the libplot/libplotter API.  Each returns a pointer
   to the head of a font database in g_fontdb.c, so that an application
   program that is too nosy for its own good can pry out font information.

   These should be replaced by a properly crafted API for querying font
   names, font metrics, etc. */

#include "sys-defines.h"
#include "extern.h"

voidptr_t
#ifdef _HAVE_PROTOS
get_hershey_font_info (S___(Plotter *_plotter))
#else
get_hershey_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_hershey_font_info;
}

voidptr_t
#ifdef _HAVE_PROTOS
get_ps_font_info (S___(Plotter *_plotter))
#else
get_ps_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_ps_font_info;
}

voidptr_t
#ifdef _HAVE_PROTOS
get_pcl_font_info (S___(Plotter *_plotter))
#else
get_pcl_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_pcl_font_info;
}

voidptr_t
#ifdef _HAVE_PROTOS
get_stick_font_info (S___(Plotter *_plotter))
#else
get_stick_font_info (S___(_plotter))
     S___(Plotter *_plotter;) 
#endif
{
  return (voidptr_t)_stick_font_info;
}

