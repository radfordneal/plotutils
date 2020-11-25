/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

/* This version is for HPGL Plotters, which do not plot in real time, but
   emit a page of graphics when closepl() is called. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

int
#ifdef _HAVE_PROTOS
_h_openpl (void)
#else
_h_openpl ()
#endif
{
  if (_plotter->open)
    {
      _plotter->error ("openpl: invalid operation");
      return -1;
    }

  /* Prepare buffer in which we'll cache graphics code for this page. */
  _plotter->page = _new_outbuf ();
  
  /* flag device as open */
  _plotter->open = true;
  _plotter->opened = true;
  (_plotter->page_number)++;

  if (_plotter->type == PL_PCL)
    {
      if (_plotter->page_number > 1) /* not first page */
	/* eject previous page, by issuing PCL command */
	{	
	  strcpy (_plotter->page->point, "\014"); /* i.e. form feed */
	  _update_buffer (_plotter->page);
	}
      /* switch from PCL 5 to HP-GL/2 mode */
      strcpy (_plotter->page->point, "\033%0B\n");
      _update_buffer (_plotter->page);
    }

  /* output HP-GL prologue */
  if (_plotter->hpgl_version == 2)
    {
      sprintf (_plotter->page->point, "BP;IN;");
      _update_buffer (_plotter->page);
      sprintf (_plotter->page->point, "PS%d;",
	       IROUND(_plotter->plot_length));
      _update_buffer (_plotter->page);
    }
  else
    {
      sprintf (_plotter->page->point, "IN;");
      _update_buffer (_plotter->page);
    }
  
  /* rotate if user requested rotation */
  if (_plotter->rotation != 0)
    {
      sprintf (_plotter->page->point, "RO%d;", _plotter->rotation);
      _update_buffer (_plotter->page);
    }
  
  /* set scaling points P1, P2 at lower left and upper right corners of
     the region (square) that we call our ``graphics display'' */
  sprintf (_plotter->page->point, "IP%d,%d,%d,%d;",
	   IROUND(_plotter->p1x), IROUND(_plotter->p1y),
	   IROUND(_plotter->p2x), IROUND(_plotter->p2y));
  _update_buffer (_plotter->page);
  
  /* Set up `scaled device coordinates' within the graphics display.  All
     coordinates in our output file will be scaled device coordinates,
     not physical device coordinates. */
  sprintf (_plotter->page->point, "SC%d,%d,%d,%d;",
	   IROUND (_plotter->display_coors.left), 
	   IROUND (_plotter->display_coors.right), 
	   IROUND (_plotter->display_coors.bottom), 
	   IROUND (_plotter->display_coors.top));
  _update_buffer (_plotter->page);
  
  if (_plotter->hpgl_version == 2)
    {
      /* Begin to define a palette, by specifying a number of logical pens.
	 (All HP-GL/2 devices should support the `NP' instruction, even
	 though many support only a default palette.) */
      if (_plotter->palette)
	{
	  sprintf (_plotter->page->point, "NP%d;", MAX_NUM_PENS);
	  _update_buffer (_plotter->page);
	}
      /* use relative units for pen width */
      sprintf (_plotter->page->point, "WU1;");
      _update_buffer (_plotter->page);
    }
  
  /* select pen #1 (standard plotting convention) */
  sprintf (_plotter->page->point, "SP1;");
  _update_buffer (_plotter->page);
  
  /* For HP-GL/2 devices, set transparency mode to `opaque', if the user
     allows it.  It should always be opaque to agree with libplot
     conventions, but on some HP-GL/2 devices (mostly pen plotters) the
     `TR' command does not NOP gracefully. */
  if (_plotter->hpgl_version == 2 && _plotter->opaque_mode)
    {
      sprintf (_plotter->page->point, "TR0;");
      _update_buffer (_plotter->page);
    }

  /* freeze contents of output buffer, i.e. the initialization code we've
     just written to it, so that any later invocation of erase() won't
     remove it */
  _freeze_outbuf (_plotter->page);

  /* create drawing state, add it as the first member of the linked list */
  _plotter->savestate();			

  return 0;
}

