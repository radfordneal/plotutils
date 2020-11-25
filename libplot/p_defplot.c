/* This file defines the initialization for any PSPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

/* ctime_r() is currently not used, because there is apparently _no_
   universal way of ensuring that it is declared.  On some systems
   (e.g. Red Hat Linux), `#define _POSIX_SOURCE' will do it.  But on other
   systems, doing `#define _POSIX_SOURCE' **removes** the declaration! */
#ifdef HAVE_CTIME_R
#undef HAVE_CTIME_R
#endif

#ifdef MSDOS
#include <unistd.h>		/* for fsync() */
#endif

/* song and dance to define time_t, and declare both time() and ctime() */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* for time_t on some pre-ANSI Unix systems */
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>		/* for time() on some pre-ANSI Unix systems */
#include <time.h>		/* for ctime() */
#else  /* not TIME_WITH_SYS_TIME, include only one (prefer <sys/time.h>) */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else  /* not HAVE_SYS_TIME_H */
#include <time.h>
#endif /* not HAVE_SYS_TIME_H */
#endif /* not TIME_WITH_SYS_TIME */

#include "p_header.h"		/* idraw Postscript header (from InterViews) */

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a PSPlotter struct. */
const Plotter _p_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bezier2, _g_bezier2rel, _g_bezier3, _g_bezier3rel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _p_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _p_endpath, _g_endsubpath, _p_erase, _g_farc, _g_farcrel, _g_fbezier2, _g_fbezier2rel, _g_fbezier3, _g_fbezier3rel, _g_fbox, _g_fboxrel, _p_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _p_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_fillmod, _g_filltype, _g_flabelwidth, _g_fline, _g_flinedash, _g_flinerel, _g_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmiterlimit, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _p_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linedash, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _p_openpl, _g_orientation, _g_outfile, _g_pencolor, _g_pencolorname, _g_pentype, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* initialization (after creation) and termination (before deletion) */
  _p_initialize, _p_terminate,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _g_falabel_hershey, _p_falabel_ps, _p_falabel_pcl, _g_falabel_stick, _g_falabel_other,
  _g_flabelwidth_hershey, _g_flabelwidth_ps, _g_flabelwidth_pcl, _g_flabelwidth_stick, _g_flabelwidth_other,
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  _g_set_font,
  /* private low-level `sync line attributes' method */
  _g_set_attributes,
  /* private low-level `sync color' methods */
  _p_set_pen_color,
  _p_set_fill_color,
  _g_set_bg_color,
  /* private low-level `sync position' method */
  _g_set_position,
  /* error handlers */
  _g_warning,
  _g_error,
  /* low-level output routines */
  _g_write_byte,
  _g_write_bytes,
  _g_write_string
};
#endif /* not LIBPLOTTER */

/* The private `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  When this
   is invoked, _plotter points to the Plotter that has just been
   created. */

/* For PS Plotter objects, we initialize the used-font array(s).  We also
   determine the page size and the location on the page of the graphics
   display, so that we'll be able to work out the map from user coordinates
   to device coordinates in space.c. */

void
#ifdef _HAVE_PROTOS
_p_initialize (S___(Plotter *_plotter))
#else
_p_initialize (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double xoffset, yoffset;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _g_initialize (S___(_plotter));
#endif

  /* override generic initializations (which are appropriate to the base
     Plotter class), as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->type = PL_PS;
#endif

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->have_wide_lines = 1;
  _plotter->have_dash_array = 1;
  _plotter->have_solid_fill = 1;
  _plotter->have_odd_winding_fill = 1;
  _plotter->have_nonzero_winding_fill = 1;
  _plotter->have_settable_bg = 0;
  _plotter->have_hershey_fonts = 1;
  _plotter->have_ps_fonts = 1;
#ifdef USE_LJ_FONTS_IN_PS
  _plotter->have_pcl_fonts = 1;
#else
  _plotter->have_pcl_fonts = 0;
#endif
  _plotter->have_stick_fonts = 0;
  _plotter->have_extra_stick_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->default_font_type = F_POSTSCRIPT;
  _plotter->pcl_before_ps = false;
  _plotter->have_horizontal_justification = false;
  _plotter->have_vertical_justification = false;
  _plotter->issue_font_warning = true;

  /* path and polyline-related parameters (also internal); note that we
     don't set max_unfilled_polyline_length, because it was set by the
     superclass initialization */
  _plotter->have_mixed_paths = false;
  _plotter->allowed_arc_scaling = AS_NONE;
  _plotter->allowed_ellarc_scaling = AS_NONE;  
  _plotter->allowed_quad_scaling = AS_NONE;  
  _plotter->allowed_cubic_scaling = AS_NONE;  
  _plotter->flush_long_polylines = true;
  _plotter->hard_polyline_length_limit = INT_MAX;

  /* dimensions */
  _plotter->display_model_type = (int)DISP_MODEL_PHYSICAL;
  _plotter->display_coors_type = (int)DISP_DEVICE_COORS_REAL;
  _plotter->flipped_y = false;
  _plotter->imin = 0;
  _plotter->imax = 0;  
  _plotter->jmin = 0;
  _plotter->jmax = 0;  
  _plotter->xmin = 0.0;
  _plotter->xmax = 0.0;  
  _plotter->ymin = 0.0;
  _plotter->ymax = 0.0;  
  _plotter->page_data = (plPageData *)NULL;

  /* initialize certain data members from device driver parameters */
      
  /* determine page type, and user-specified viewport offset if any */
  _set_page_type (R___(_plotter) &xoffset, &yoffset);
  
  /* Determine range of device coordinates over which the viewport will
     extend (and hence the transformation from user to device coordinates;
     see g_space.c). */
  {
    double xmid, ymid, viewport_size;
    
    viewport_size = _plotter->page_data->viewport_size;
    xmid = 0.5 * _plotter->page_data->xsize + xoffset;
    ymid = 0.5 * _plotter->page_data->ysize + yoffset;

    _plotter->xmin = 72 * (xmid - 0.5 * viewport_size);
    _plotter->xmax = 72 * (xmid + 0.5 * viewport_size);    
    _plotter->ymin = 72 * (ymid - 0.5 * viewport_size);
    _plotter->ymax = 72 * (ymid + 0.5 * viewport_size);    
  }
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

/* This version is for Postscript Plotters.  It writes out the
   idraw-derived PS prologue to the output stream, and copies each stored
   page of graphics to it too, making sure to include the proper DSC
   [Document Structuring Convention] comment lines at the beginning and the
   end of the document, and at the beginning and end of each page.

   (PS Plotters differ from most other plotters that do not plot in real
   time in that they emit output only after all pages have pages have been
   drawn, rather than at the end of each page.  This is necessary for DSC
   compliance.)

   When this is called, the PS code for the body of each page is stored in
   a plOutbuf, and the page plOutbufs form a linked list.  In this function
   we write the document header, the document trailer, and the
   header/trailer for each page, all to separate plOutbufs.  We then copy
   the plOutbufs, one after another, to the output stream. */

void
#ifdef _HAVE_PROTOS
_p_terminate (S___(Plotter *_plotter))
#else
_p_terminate (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  double x_min, x_max, y_min, y_max;
  int i, n;
  time_t clock;
  plOutbuf *doc_header, *doc_trailer, *current_page;
  bool ps_font_used_in_doc[NUM_PS_FONTS];
#ifdef USE_LJ_FONTS_IN_PS
  bool pcl_font_used_in_doc[NUM_PCL_FONTS];	
#endif
  char *time_string, time_string_buffer[32];

  /* if specified plotter is open, close it */
  if (_plotter->open)
    _plotter->closepl (S___(_plotter));

#ifdef LIBPLOTTER
  if (_plotter->outfp || _plotter->outstream)
#else
  if (_plotter->outfp)
#endif
    /* have an output stream */
    {
      int num_pages = _plotter->page_number;

      /* First, prepare the document header (DSC lines, etc.), and write it
         to a plOutbuf.  The header is very long: most of it is simply the
         idraw header (see p_header.h). */
      doc_header = _new_outbuf ();

      if (num_pages == 1)
	/* will plot an EPS file, not just a PS file */
	sprintf (doc_header->point, "\
%%!PS-Adobe-3.0 EPSF-3.0\n");
      else
	sprintf (doc_header->point, "\
%%!PS-Adobe-3.0\n");
      _update_buffer (doc_header);

      /* Compute an ASCII representation of the current time, in a
	 reentrant way if we're supporting pthreads (i.e. by using ctime_r
	 if it's available). */
      time (&clock);
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
#ifdef HAVE_CTIME_R
      ctime_r (&clock, time_string_buffer);
      time_string = time_string_buffer;
#else
      time_string = ctime (&clock);
#endif
#else
      time_string = ctime (&clock);
#endif
#else
      time_string = ctime (&clock);
#endif

      sprintf (doc_header->point, "\
%%%%Creator: GNU libplot drawing library %s\n\
%%%%Title: PostScript plot\n\
%%%%CreationDate: %s\
%%%%DocumentData: Clean7Bit\n\
%%%%LanguageLevel: 1\n\
%%%%Pages: %d\n\
%%%%PageOrder: Ascend\n\
%%%%Orientation: Portrait\n",
	       LIBPLOT_VERSION, time_string, num_pages);
      _update_buffer (doc_header);
      
      /* emit the bounding box for the document */
      _bbox_of_outbufs (_plotter->first_page, &x_min, &x_max, &y_min, &y_max);
      if (x_min > x_max || y_min > y_max) 
	/* all pages empty */
	sprintf (doc_header->point, "\
%%%%BoundingBox: 0 0 0 0\n");
      else
	sprintf (doc_header->point, "\
%%%%BoundingBox: %d %d %d %d\n",
		 IROUND(x_min - 0.5), IROUND(y_min - 0.5),
		 IROUND(x_max + 0.5), IROUND(y_max + 0.5));
      _update_buffer (doc_header);
      
      /* determine fonts needed by document, by examining all pages */
      {
	current_page = _plotter->first_page;
	
	for (i = 0; i < NUM_PS_FONTS; i++)
	  ps_font_used_in_doc[i] = false;
#ifdef USE_LJ_FONTS_IN_PS	
	for (i = 0; i < NUM_PCL_FONTS; i++)
	  pcl_font_used_in_doc[i] = false;
#endif
	while (current_page)
	  {
	    for (i = 0; i < NUM_PS_FONTS; i++)
	      if (current_page->ps_font_used[i])
		ps_font_used_in_doc[i] = true;
#ifdef USE_LJ_FONTS_IN_PS
	    for (i = 0; i < NUM_PCL_FONTS; i++)
	      if (current_page->pcl_font_used[i])
		pcl_font_used_in_doc[i] = true;
#endif
	    current_page = current_page->next;
	  }
      }

      /* write out list of fonts needed by the document */
      {
	bool first_font = true;

	strcpy (doc_header->point, "\
%%DocumentNeededResources: ");
	_update_buffer (doc_header);

	for (i = 0; i < NUM_PS_FONTS; i++)
	  {
	    if (ps_font_used_in_doc[i])
	      {
		if (first_font == false)
		  {
		    strcpy (doc_header->point, "%%+ ");
		    _update_buffer (doc_header);
		  }
		strcpy (doc_header->point, "font ");
		_update_buffer (doc_header);
		strcpy (doc_header->point, _ps_font_info[i].ps_name);
		_update_buffer (doc_header);
		strcpy (doc_header->point, "\n");
		_update_buffer (doc_header);
		first_font = false;
	      }
	  }
#ifdef USE_LJ_FONTS_IN_PS
	for (i = 0; i < NUM_PCL_FONTS; i++)
	  {
	    if (pcl_font_used_in_doc[i])
	      {
		if (first_font == false)
		  {
		    strcpy (doc_header->point, "%%+ ");
		    _update_buffer (doc_header);
		  }
		strcpy (doc_header->point, "font ");
		_update_buffer (doc_header);
		/* use replacement font name if any (this is only to
                   support the Tidbits-is-Wingdings botch) */
		if (_pcl_font_info[i].substitute_ps_name)
		  strcpy (doc_header->point, _pcl_font_info[i].substitute_ps_name);
		else
		  strcpy (doc_header->point, _pcl_font_info[i].ps_name);
		_update_buffer (doc_header);
		strcpy (doc_header->point, "\n");
		_update_buffer (doc_header);
		first_font = false;
	      }
	  }
#endif
	if (first_font)		/* no fonts needed in document */
	  {
	    strcpy (doc_header->point, "\n");
	    _update_buffer (doc_header);	    
	  }
      }

      /* emit final DSC lines in header */
      if (num_pages > 0)
	{
	  sprintf (doc_header->point, "\
%%%%DocumentSuppliedResources: procset %s %s 0\n",
		   PS_PROCSET_NAME, PS_PROCSET_VERSION);
	  _update_buffer (doc_header);
	}
      strcpy (doc_header->point, "\
%%EndComments\n\n");
      _update_buffer (doc_header);

      /* write out list of fonts needed by the document, all over again;
	 this time it's interpreted as the default font list for each page */
      {
	bool first_font = true;

	strcpy (doc_header->point, "\
%%BeginDefaults\n");
	_update_buffer (doc_header);
	strcpy (doc_header->point, "\
%%PageResources: ");
	_update_buffer (doc_header);
	for (i = 0; i < NUM_PS_FONTS; i++)
	  {
	    if (ps_font_used_in_doc[i])
	      {
		if (first_font == false)
		  {
		    strcpy (doc_header->point, "%%+ ");
		    _update_buffer (doc_header);
		  }
		strcpy (doc_header->point, "font ");
		_update_buffer (doc_header);
		strcpy (doc_header->point, _ps_font_info[i].ps_name);
		_update_buffer (doc_header);
		strcpy (doc_header->point, "\n");
		_update_buffer (doc_header);
		first_font = false;
	      }
	  }
#ifdef USE_LJ_FONTS_IN_PS
	for (i = 0; i < NUM_PCL_FONTS; i++)
	  {
	    if (pcl_font_used_in_doc[i])
	      {
		if (first_font == false)
		  {
		    strcpy (doc_header->point, "%%+ ");
		    _update_buffer (doc_header);
		  }
		strcpy (doc_header->point, "font ");
		_update_buffer (doc_header);
		if (_pcl_font_info[i].substitute_ps_name)
		  /* this is to support the Tidbits-is-Wingdings botch */
		  strcpy (doc_header->point, _pcl_font_info[i].substitute_ps_name);
		else
		  strcpy (doc_header->point, _pcl_font_info[i].ps_name);
		_update_buffer (doc_header);
		strcpy (doc_header->point, "\n");
		_update_buffer (doc_header);
		first_font = false;
	      }
	  }
#endif
	if (first_font)		/* no fonts needed in document */
	  {
	    strcpy (doc_header->point, "\n");
	    _update_buffer (doc_header);
	  }

	strcpy (doc_header->point, "\
%%EndDefaults\n\n");
	_update_buffer (doc_header);
      }

      /* Document Prolog */
      strcpy (doc_header->point, "\
%%BeginProlog\n");
      _update_buffer (doc_header);
      if (num_pages > 1)
	/* PS [not EPS] file, include procset in document prolog */
	{
	  sprintf (doc_header->point, "\
%%%%BeginResource procset %s %s 0\n", 
		   PS_PROCSET_NAME, PS_PROCSET_VERSION);
	  _update_buffer (doc_header);
	  /* write out idraw-derived PS prologue (makes many definitions) */
	  for (i=0; *_ps_procset[i]; i++)
	    {
	      strcpy (doc_header->point, _ps_procset[i]);
	      _update_buffer (doc_header);
	    }
	  strcpy (doc_header->point, "\
%%EndResource\n");
	  _update_buffer (doc_header);
	}
      strcpy (doc_header->point, "\
%%EndProlog\n\n");
      _update_buffer (doc_header);

      /* Document Setup */
      strcpy (doc_header->point, "\
%%BeginSetup\n");
      _update_buffer (doc_header);

      /* tell driver to include any PS [or PCL] fonts that are needed */
      for (i = 0; i < NUM_PS_FONTS; i++)
	  if (ps_font_used_in_doc[i])
	    {
	      sprintf (doc_header->point, "\
%%%%IncludeResource: font %s\n", _ps_font_info[i].ps_name);
	      _update_buffer (doc_header);
	    }
#ifdef USE_LJ_FONTS_IN_PS
      for (i = 0; i < NUM_PCL_FONTS; i++)
	  if (pcl_font_used_in_doc[i])
	    {
	      /* this is to support the Tidbits-is-Wingdings botch */
	      if (_pcl_font_info[i].substitute_ps_name)
		sprintf (doc_header->point, "\
%%%%IncludeResource: font %s\n", _pcl_font_info[i].substitute_ps_name);
	      else
		sprintf (doc_header->point, "\
%%%%IncludeResource: font %s\n", _pcl_font_info[i].ps_name);
	      _update_buffer (doc_header);
	    }
#endif

      /* push private dictionary on stack */
      strcpy (doc_header->point, "\
/DrawDict 50 dict def\n\
DrawDict begin\n");
      _update_buffer (doc_header);

      /* do ISO-Latin-1 reencoding for any fonts that need it */
      {
	bool need_to_reencode = false;

	for (i = 0; i < NUM_PS_FONTS; i++)
	  if (ps_font_used_in_doc[i] && _ps_font_info[i].iso8859_1)
	    {
	      need_to_reencode = true;
	      break;
	    }
#ifdef USE_LJ_FONTS_IN_PS
	for (i = 0; i < NUM_PCL_FONTS; i++)
	  if (pcl_font_used_in_doc[i] && _pcl_font_info[i].iso8859_1)
	    {
	      need_to_reencode = true;
	      break;
	    }
#endif
	if (need_to_reencode)
	  {
	    strcpy (doc_header->point, _ps_fontproc);
	    _update_buffer (doc_header);
	    
	    for (i = 0; i < NUM_PS_FONTS; i++)
	      {
		if (ps_font_used_in_doc[i] && _ps_font_info[i].iso8859_1)
		  {
		    sprintf (doc_header->point, "\
/%s reencodeISO def\n",
			     _ps_font_info[i].ps_name);
		    _update_buffer (doc_header);
		  }
	      }
#ifdef USE_LJ_FONTS_IN_PS
	    for (i = 0; i < NUM_PCL_FONTS; i++)
	      {
		if (pcl_font_used_in_doc[i] && _pcl_font_info[i].iso8859_1)
		  {
		    sprintf (doc_header->point, "\
/%s reencodeISO def\n",
			     _pcl_font_info[i].ps_name);
		    _update_buffer (doc_header);
		  }
	      }
#endif
	  }
      }

      if (num_pages == 1)
	/* EPS [not just PS] file, include procset in setup section,
	   so that it will modify only the private dictionary */
	{
	  sprintf (doc_header->point, "\
%%%%BeginResource procset %s %s 0\n", 
		   PS_PROCSET_NAME, PS_PROCSET_VERSION);
	  _update_buffer (doc_header);

	  /* write out idraw-derived PS prologue in p_header.h (makes many
             definitions) */
	  for (i=0; *_ps_procset[i]; i++)
	    {
	      strcpy (doc_header->point, _ps_procset[i]);
	      _update_buffer (doc_header);
	    }
	  strcpy (doc_header->point, "\
%%EndResource\n");
	  _update_buffer (doc_header);
	}

      strcpy (doc_header->point, "\
%%EndSetup\n\n");
      _update_buffer (doc_header);
      
      /* Document header is now prepared, and stored in a plOutbuf.
	 Now do the same for the doc trailer (much shorter). */

      /* Document Trailer: just pop private dictionary off stack */
      doc_trailer = _new_outbuf ();
      strcpy (doc_trailer->point, "\
%%Trailer\n\
end\n\
%%EOF\n");
      _update_buffer (doc_trailer);

      /* WRITE DOCUMENT HEADER (and free its plOutbuf) */
      _plotter->write_string (R___(_plotter) doc_header->base); 
      _delete_outbuf (doc_header);

      /* now loop through pages, emitting each in turn */
      if (num_pages > 0)
	{
	  for (current_page = _plotter->first_page, n=1; 
	       current_page; 
	       current_page = current_page->next, n++)
	    {
	      plOutbuf *page_header, *page_trailer;

	      /* prepare page header, and store it in a plOutbuf */
	      page_header = _new_outbuf ();

	      sprintf (page_header->point, "\
%%%%Page: %d %d\n", n, n);
	      _update_buffer (page_header);

	      /* write out list of fonts needed by the page */
	      {
		bool first_font = true;

		strcpy (page_header->point, "\
%%PageResources: ");
		_update_buffer (page_header);
		for (i = 0; i < NUM_PS_FONTS; i++)
		  {
		    if (current_page->ps_font_used[i])
		      {
			if (first_font == false)
			  {
			    strcpy (page_header->point, "%%+ ");
			    _update_buffer (page_header);
			  }
			strcpy (page_header->point, "font ");
			_update_buffer (page_header);
			strcpy (page_header->point, _ps_font_info[i].ps_name);
			_update_buffer (page_header);
			strcpy (page_header->point, "\n");
			_update_buffer (page_header);
			first_font = false;
		      }
		  }
#ifdef USE_LJ_FONTS_IN_PS
		for (i = 0; i < NUM_PCL_FONTS; i++)
		  {
		    if (current_page->pcl_font_used[i])
		      {
			if (first_font == false)
			  {
			    strcpy (page_header->point, "%%+ ");
			    _update_buffer (page_header);
			  }
			strcpy (page_header->point, "font ");
			_update_buffer (page_header);
			if (_pcl_font_info[i].substitute_ps_name)
			  /* this is to support the Tidbits-is-Wingdings botch */
			  strcpy (page_header->point, _pcl_font_info[i].substitute_ps_name);
			else
			  strcpy (page_header->point, _pcl_font_info[i].ps_name);
			_update_buffer (page_header);
			strcpy (page_header->point, "\n");
			_update_buffer (page_header);
			first_font = false;
		      }
		  }
#endif
		if (first_font)	/* no fonts needed on page */
		  {
		    strcpy (page_header->point, "\n");
		    _update_buffer (page_header);
		  }
	      }

	      /* emit the bounding box for the page */
	      _bbox_of_outbuf (current_page, &x_min, &x_max, &y_min, &y_max);
	      if (x_min > x_max || y_min > y_max)
		/* empty page */
		sprintf (page_header->point, "\
%%%%PageBoundingBox: 0 0 0 0\n");
	      else
		sprintf (page_header->point, "\
%%%%PageBoundingBox: %d %d %d %d\n",
			 IROUND(x_min - 0.5), IROUND(y_min - 0.5),
			 IROUND(x_max + 0.5), IROUND(y_max + 0.5));
	      _update_buffer (page_header);
	      /* Page Setup */
	      strcpy (page_header->point, "\
%%BeginPageSetup\n");
	      _update_buffer (page_header);
	      /* emit initialization code (including idraw, PS directives) */
	      /* N.B. `8' below is the version number of the idraw PS format
		 we're producing; see <Unidraw/Components/psformat.h> */
	      strcpy (page_header->point, "\
%I Idraw 8\n\n\
Begin\n\
%I b u\n\
%I cfg u\n\
%I cbg u\n\
%I f u\n\
%I p u\n\
%I t\n\
[ 1 0 0 1 0 0 ] concat\n\
/originalCTM matrix currentmatrix def\n\
/trueoriginalCTM matrix currentmatrix def\n");
	      _update_buffer (page_header);
	      strcpy (page_header->point, "\
%%EndPageSetup\n\n");
	      _update_buffer (page_header);	  

	      /* Page header is now prepared, and stored in a plOutbuf.  
                 Do the same for the page trailer (much shorter). */

	      page_trailer = _new_outbuf ();
	      /* Page Trailer: includes `showpage' */
	      strcpy (page_trailer->point, "\
%%PageTrailer\n\
End %I eop\n\
showpage\n\n");
	      _update_buffer (page_trailer);
	      /* Page trailer is now ready */

	      /* WRITE PS CODE FOR THIS PAGE, including header, trailer */
	      _plotter->write_string (R___(_plotter) page_header->base); 
	      if (current_page->len > 0)
		_plotter->write_string (R___(_plotter) current_page->base);
	      _plotter->write_string (R___(_plotter) page_trailer->base);

	      /* free header, trailer plOutbufs */
	      _delete_outbuf (page_trailer);
	      _delete_outbuf (page_header);
	    }
	}
      
      /* WRITE DOCUMENT TRAILER (and free its plOutbuf) */
      _plotter->write_string (R___(_plotter) doc_trailer->base); 
      _delete_outbuf (doc_trailer);
    }
  
  /* delete all plOutbufs in which document pages are stored */
  current_page = _plotter->first_page;
  while (current_page)
    {
      plOutbuf *next_page;
	  
      next_page = current_page->next;
      _delete_outbuf (current_page);
      current_page = next_page;
    }
  
  /* flush output stream if any */
  if (_plotter->outfp)
    {
      if (fflush(_plotter->outfp) < 0
#ifdef MSDOS
	  /* data can be caught in DOS buffers, so do an fsync() too */
	  || fsync (_plotter->outfp) < 0
#endif
	  )
	_plotter->error (R___(_plotter) "output stream jammed");
    }
#ifdef LIBPLOTTER
  else if (_plotter->outstream)
    {
      _plotter->outstream->flush ();
      if (!(*(_plotter->outstream)))
	_plotter->error (R___(_plotter) "output stream jammed");
    }
#endif

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
PSPlotter::PSPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _p_initialize ();
}

PSPlotter::PSPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _p_initialize ();
}

PSPlotter::PSPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _p_initialize ();
}

PSPlotter::PSPlotter (ostream& out)
	: Plotter (out)
{
  _p_initialize ();
}

PSPlotter::PSPlotter ()
{
  _p_initialize ();
}

PSPlotter::PSPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _p_initialize ();
}

PSPlotter::PSPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _p_initialize ();
}

PSPlotter::PSPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _p_initialize ();
}

PSPlotter::PSPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _p_initialize ();
}

PSPlotter::PSPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _p_initialize ();
}

PSPlotter::~PSPlotter ()
{
  _p_terminate ();
}
#endif
