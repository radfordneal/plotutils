/* This file defines the initialization for any PSPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#ifndef HAVE_UNISTD_H
#include <sys/types.h>		/* if unistd.h found, included with it */
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>		/* for time() */
#include <time.h>		/* for ctime() */
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include "p_header.h"		/* Idraw Postscript header */

static void _get_document_bbox __P ((Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax));
static void _get_page_bbox __P ((Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax));

/* Note that we define the graphics display to be a square, centered on the
   printed page and occupying the full width of the page. */

/* The size of the graphics display is determined by the PAGESIZE
   environment variable ("usletter", "a4", etc.)  The table of known
   pagetypes is in pagetype.h.  The default is "usletter", for which the
   graphics display is an 8.5" by 8.5" square, centered on an 8.5" by 11"
   page. */

const Plotter _ps_default_plotter = 
{
  /* methods */
  _g_alabel, _g_arc, _g_arcrel, _g_bgcolor, _g_bgcolorname, _g_box, _g_boxrel, _g_capmod, _g_circle, _g_circlerel, _p_closepl, _g_color, _g_colorname, _g_cont, _g_contrel, _g_ellarc, _g_ellarcrel, _g_ellipse, _g_ellipserel, _p_endpath, _p_erase, _g_farc, _g_farcrel, _g_fbox, _g_fboxrel, _p_fcircle, _g_fcirclerel, _g_fconcat, _g_fcont, _g_fcontrel, _g_fellarc, _g_fellarcrel, _p_fellipse, _g_fellipserel, _g_ffontname, _g_ffontsize, _g_fillcolor, _g_fillcolorname, _g_filltype, _g_flabelwidth, _g_fline, _g_flinerel, _g_flinewidth, _g_flushpl, _g_fmarker, _g_fmarkerrel, _g_fmove, _g_fmoverel, _g_fontname, _g_fontsize, _p_fpoint, _g_fpointrel, _g_frotate, _g_fscale, _g_fspace, _g_fspace2, _g_ftextangle, _g_ftranslate, _g_havecap, _g_joinmod, _g_label, _g_labelwidth, _g_line, _g_linemod, _g_linerel, _g_linewidth, _g_marker, _g_markerrel, _g_move, _g_moverel, _p_openpl, _g_outfile, _g_pencolor, _g_pencolorname, _g_point, _g_pointrel, _g_restorestate, _g_savestate, _g_space, _g_space2, _g_textangle,
  /* internal methods that plot strings in non-Hershey fonts */
#ifdef USE_LJ_FONTS
  _p_falabel_ps, _p_falabel_ps, NULL, NULL,
  _g_flabelwidth_ps, _g_flabelwidth_pcl, NULL, NULL,
#else
  _p_falabel_ps, NULL, NULL, NULL,
  _g_flabelwidth_ps, NULL, NULL, NULL,
#endif
  /* private low-level `retrieve font' method */
  _g_retrieve_font,
  /* private low-level `sync font' method */
  NULL,
  /* private low-level `sync line attributes' method */
  NULL,
  /* private low-level `sync color' methods */
  _p_set_pen_color,
  _p_set_fill_color,
  NULL,
  /* private low-level `sync position' method */
  NULL,
  /* error handlers */
  _g_warning,
  _g_error,
  /* basic plotter parameters */
  PL_PS,			/* plotter type */
  false,			/* open? */
  false,			/* opened? */
  0,				/* number of times opened */
  (FILE *)NULL,			/* input stream [not used] */
  (FILE *)NULL,			/* output stream (if any) */
  (FILE *)NULL,			/* error stream (if any) */
  /* NUM_DEVICE_DRIVER_PARAMETERS Plotter parameters (see g_params.h) */
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  /* capabilities */
#ifdef USE_LJ_FONTS
  1, 1, 0, 1, 1, 1, 0, 0, 0,	/* capability flags (see extern.h) */
#else
  1, 1, 0, 1, 1, 0, 0, 0, 0,	/* capability flags (see extern.h) */
#endif
  INT_MAX,			/* hard polyline length limit */
  /* output buffers */
  NULL,				/* pointer to output buffer for current page */
  NULL,				/* pointer to output buffer for first page */
  /* associated process id's */
  NULL,				/* list of pids of forked-off processes */
  0,				/* number of pids in list */
  /* drawing state(s) */
  (State *)NULL,		/* pointer to top of drawing state stack */
  &_ps_default_drawstate,	/* for initialization and resetting */
  /* dimensions */  
  false,			/* bitmap display device? */
  0, 0, 0, 0,			/* range of coordinates (for a bitmap device)*/
  {0.25, 8.25, 1.5, 9.5, 0.0},	/* same, for a physical device (in inches) */
  72.0,				/* units/inch for a physical device */
  false,			/* y increases downward? */
  /* elements used by more than one device */
  MAX_UNFILLED_POLYLINE_LENGTH,	/* user-settable, for unfilled polylines */
  true,				/* position is unknown? */
  {0, 0},			/* cursor position (for a bitmap device) */
  false,			/* issued warning on font substitution? */
  false,			/* issued warning on colorname substitution? */
  false,			/* issued warning on colorname substitution? */
  false,			/* issued warning on colorname substitution? */
  /* elements specific to the metafile device driver */
  false,			/* portable, not binary output format? */
  /* elements specific to the Tektronix device driver */
  D_GENERIC,			/* which sort of Tektronix? */
  MODE_ALPHA,			/* one of MODE_* */
  L_SOLID,			/* one of L_* */
  true,				/* mode is unknown? */
  true,				/* line type is unknown? */
  ANSI_SYS_GRAY30,		/* MS-DOS kermit's fg color */
  ANSI_SYS_WHITE,		/* MS-DOS kermit's bg color */
  /* elements specific to the HP-GL device driver */
  2,				/* version, 0=HP-GL, 1=HP7550A, 2=HP-GL/2 */
  0,				/* HP-GL rotation angle */
  0.0, 8128.0,			/* scaling point P1 in native HP-GL coors */
  0.0, 8128.0,			/* scaling point P2 in native HP-GL coors */
  10668.0,			/* plot length (for HP-GL/2 roll plotters) */
  1,				/* current pen (initted in h_closepl.c) */
  false,			/* bad pen? (advisory, see h_color.c) */
  false,			/* pen down rather than up? */
  0.001,			/* pen width (frac of diag dist betw P1,P2) */
  HPGL_L_SOLID,			/* line type */
  HPGL_CAP_BUTT,		/* cap style for lines */
  HPGL_JOIN_MITER,		/* join style for lines */
  HPGL_FILL_SOLID_BI,		/* fill type */
  0.0,				/* percent shading (used if FILL_SHADING) */
  2,				/* pen to be assigned a color next */
  false,			/* can construct a palette? (HP-GL/2 only) */
  true,				/* pen marks sh'd be opaque? (HP-GL/2 only) */
  PCL_ROMAN_8,			/* encoding, 14=ISO-Latin-1,.. (HP-GL/2 only)*/
  0,				/* font spacing, 0=fixed, 1=not(HP-GL/2 only)*/
  0,				/* posture, 0=upright, 1=italic(HP-GL/2 only)*/
  0,				/* weight,0=normal,3=bold, etc.(HP-GL/2 only)*/
  STICK_TYPEFACE,		/* typeface, as in g_fontdb.c (HP-GL/2 only) */
  HP_ASCII,			/* old HP character set number (lower half) */
  HP_ASCII,			/* old HP character set number (upper half) */
  0,				/* char. ht., % of p2y-p1y (HP-GL/2 only) */
  0,				/* char. width, % of p2x-p1x (HP-GL/2 only) */
  0,				/* label rise, % of p2y-p1y (HP-GL/2 only) */
  0,				/* label run, % of p2x-p1x (HP-GL/2 only) */
  0,				/* tangent of character slant (HP-GL/2 only)*/
  (unsigned char)3,		/* label terminator char (^C) */
  /* elements specific to the fig device driver */
  false,			/* whether xfig display should be in metric */
  FIG_INITIAL_DEPTH,		/* fig's current value for `depth' attribute */
  0,				/* drawing priority for last-drawn object */
  0,				/* number of colors currently defined */
  /* elements specific to the Postscript/idraw device driver */
#ifndef X_DISPLAY_MISSING
  /* elements specific to the X11 and X11 Drawable device drivers */
  (Drawable)0,			/* an X drawable (e.g. a window) */
  (Drawable)0,			/* an X drawable (e.g. a pixmap) */
  (Drawable)0,			/* graphics buffer, if double buffering */
  (Fontrecord *)NULL,		/* head of list of retrieved X fonts */
  (Colorrecord *)NULL,		/* head of list of retrieved color cells */
  (Display *)NULL,		/* display */
  (Colormap)0,			/* colormap */
  DBL_NONE,			/* double buffering type (if any) */
  0,				/* number of frame in page */
  NULL,				/* label (hint to font retrieval routine) */
  /* elements specific to the X11 device driver */
  (XtAppContext)NULL,		/* application context */
  (Widget)NULL,			/* toplevel widget */
  (Widget)NULL,			/* Label widget */
  (Drawable)0,			/* used for server-side double buffering */
  false,			/* using private colormap? */
  false,			/* window(s) disappear on Plotter deletion? */
  false,			/* issued warning on color cell exhaustion? */
#endif /* X_DISPLAY_MISSING */

  /* Long arrays are positioned at the end, and are not initialized */
  /* HP-GL driver: pen_color[] and pen_defined[] arrays */
  /* FIG: fig_usercolors[] array */
  /* PS: ps_font_used[] array (and possible pcl_font_used[] array also) */
};

/* The internal `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  Return
   value indicates whether everything proceeded smoothly. */

/* For PS Plotter objects, we initialize the used-font array(s).  We also
   determine the page size and the location on the page of the graphics
   display, so that we'll be able to work out the map from user coordinates
   to device coordinates in space.c. */

bool
#ifdef _HAVE_PROTOS
_ps_init_plotter (Plotter *plotter)
#else
_ps_init_plotter (plotter)
     Plotter *plotter;
#endif
{
  const char *length_s, *pagesize;
  const Pagedata *pagedata;
  bool retval = true;
  int i;

  /* initialize certain data members from values of relevant class
     variables */
  length_s = (const char *)_get_plot_param (plotter, "MAX_LINE_LENGTH");
  {
    int local_length;
	
    if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
      {
	plotter->error ("bad MAX_LINE_LENGTH parameter, can't initialize");
	retval = false;
      }
    else
      plotter->max_unfilled_polyline_length = local_length;
  }
      
  /* determine page type i.e. determine the range of device coordinates
     over which the graphics display will extend (and hence the
     transformation from user to device coordinates). */
  pagesize = (const char *)_get_plot_param (plotter, "PAGESIZE");
  pagedata = _pagetype(pagesize);
  if (pagedata == NULL)
    {
      plotter->error ("bad PAGESIZE variable, can't initialize");
      retval = false;
    }
  plotter->display_coors = pagedata->ps;
      
  /* initialize `font used' array(s) for the document */
  for (i = 0; i < NUM_PS_FONTS; i++)
    plotter->ps_font_used[i] = false;
#ifdef USE_LJ_FONTS
  for (i = 0; i < NUM_PCL_FONTS; i++)
    plotter->pcl_font_used[i] = false;
#endif

  return retval;
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  (See api.c.)  It may do such
   things as write to an output stream from internal storage, deallocate
   storage, etc.  Return value indicates whether everything went
   smoothly. */

/* This version is for Postscript Plotters.  It writes out the
   idraw-derived PS prologue to the output stream, and copies each stored
   page of graphics to it too, making sure to include the proper DSC
   [Document Structuring Convention] comment lines.  

   PS Plotters differ from other plotters that do not plot in real time in
   that they emit output only after all pages have pages have been drawn,
   rather than at the end of each page.  This is necessary for DSC
   compliance. */

bool
#ifdef _HAVE_PROTOS
_ps_terminate_plotter (Plotter *plotter)
#else
_ps_terminate_plotter (plotter)
     Plotter *plotter;
#endif
{
  double xmin, xmax, ymin, ymax;
  int i, n;
  time_t clock;
  Outbuf *page;

  if (plotter->outstream)
    {
      int num_pages = plotter->page_number;

      if (num_pages == 1)
	/* will plot an EPS file, not just a PS file */
	fprintf (plotter->outstream, "\
%%!PS-Adobe-3.0 EPSF-3.0\n");
      else
	fprintf (plotter->outstream, "\
%%!PS-Adobe-3.0\n");

      fprintf (plotter->outstream, "\
%%%%Creator: GNU libplot drawing library\n\
%%%%Title: PostScript plot file\n\
%%%%CreationDate: %s\
%%%%DocumentData: Clean7Bit\n\
%%%%LanguageLevel: 1\n\
%%%%Pages: %d\n\
%%%%PageOrder: Ascend\n\
%%%%Orientation: Portrait\n",
	       (time(&clock), ctime(&clock)), num_pages);
      
      /* emit the bounding box for the document */
      _get_document_bbox (plotter->first_page, &xmin, &xmax, &ymin, &ymax);
      fprintf (plotter->outstream, "\
%%%%BoundingBox: %d %d %d %d\n",
	       IROUND(xmin - 0.5), IROUND(ymin - 0.5),
	       IROUND(xmax + 0.5), IROUND(ymax + 0.5));
      
      /* write out list of fonts needed by the document */
      {
	bool first_font = true;

	fputs ("\
%%DocumentNeededResources: ", plotter->outstream);
	for (i = 0; i < NUM_PS_FONTS; i++)
	  {
	    if (plotter->ps_font_used[i])
	      {
		if (first_font == false)
		  fputs ("%%+ ", plotter->outstream);
		fputs ("font ", plotter->outstream);
		fputs (_ps_font_info[i].ps_name, plotter->outstream);
		fputs ("\n", plotter->outstream);
		first_font = false;
	      }
	  }
#ifdef USE_LJ_FONTS
	for (i = 0; i < NUM_PCL_FONTS; i++)
	  {
	    if (plotter->pcl_font_used[i])
	      {
		if (first_font == false)
		  fputs ("%%+ ", plotter->outstream);
		fputs ("font ", plotter->outstream);
		if (_pcl_font_info[i].substitute_ps_name)
		  /* this is to support the Tidbits-is-Wingdings botch */
		  fputs (_pcl_font_info[i].substitute_ps_name, plotter->outstream);
		else
		  fputs (_pcl_font_info[i].ps_name, plotter->outstream);
		fputs ("\n", plotter->outstream);
		first_font = false;
	      }
	  }
#endif
	if (first_font)		/* no fonts needed */
	  fputs ("\n", plotter->outstream);
      }

      /* emit final DSC lines in header */
      if (num_pages > 0)
	fprintf (plotter->outstream, "\
%%%%DocumentSuppliedResources: procset %s %s 0\n",
		 PS_PROCSET_NAME, PS_PROCSET_VERSION);
      fputs ("\
%%EndComments\n\n", plotter->outstream);

      /* Document Prolog */
      fputs ("\
%%BeginProlog\n", plotter->outstream);
      if (num_pages > 1)
	/* PS [not EPS] file, include procset in document prolog */
	{
	  fprintf (plotter->outstream, "\
%%%%BeginResource procset %s %s 0\n", 
		   PS_PROCSET_NAME, PS_PROCSET_VERSION);
	  /* write out idraw-derived PS prologue (makes many definitions) */
	  for (i=0; *_ps_procset[i]; i++)
	    fputs (_ps_procset[i], plotter->outstream);
	  fputs ("\
%%EndResource\n", plotter->outstream);
	}
      fputs ("\
%%EndProlog\n\n", plotter->outstream);

      /* Document Setup */
      fputs ("\
%%BeginSetup\n",
	     plotter->outstream);

      /* tell driver to include any PS [or PCL] fonts that are needed */
      for (i = 0; i < NUM_PS_FONTS; i++)
	  if (plotter->ps_font_used[i])
	    fprintf (plotter->outstream, "\
%%%%IncludeResource: font %s\n", _ps_font_info[i].ps_name);
#ifdef USE_LJ_FONTS
      for (i = 0; i < NUM_PCL_FONTS; i++)
	  if (plotter->pcl_font_used[i])
	    {
	      /* this is to support the Tidbits-is-Wingdings botch */
	      if (_pcl_font_info[i].substitute_ps_name)
		fprintf (plotter->outstream, "\
%%%%IncludeResource: font %s\n", _pcl_font_info[i].substitute_ps_name);
	      else
		fprintf (plotter->outstream, "\
%%%%IncludeResource: font %s\n", _pcl_font_info[i].ps_name);
	    }
#endif

      /* push private dictionary on stack */
      fprintf (plotter->outstream, "\
/DrawDict 50 dict def\n\
DrawDict begin\n");

      /* do ISO-Latin-1 reencoding for any fonts that need it */
      {
	bool need_to_reencode = false;

	for (i = 0; i < NUM_PS_FONTS; i++)
	  if (plotter->ps_font_used[i] && _ps_font_info[i].iso8859_1)
	    {
	      need_to_reencode = true;
	      break;
	    }
#ifdef USE_LJ_FONTS
	for (i = 0; i < NUM_PCL_FONTS; i++)
	  if (plotter->pcl_font_used[i] && _pcl_font_info[i].iso8859_1)
	    {
	      need_to_reencode = true;
	      break;
	    }
#endif
	if (need_to_reencode)
	  {
	    fputs (_ps_fontproc, plotter->outstream);
	    
	    for (i = 0; i < NUM_PS_FONTS; i++)
	      {
		if (plotter->ps_font_used[i] && _ps_font_info[i].iso8859_1)
		  fprintf (plotter->outstream, "\
/%s reencodeISO def\n",
			   _ps_font_info[i].ps_name);
	      }
#ifdef USE_LJ_FONTS
	    for (i = 0; i < NUM_PCL_FONTS; i++)
	      {
		if (plotter->pcl_font_used[i] && _pcl_font_info[i].iso8859_1)
		  fprintf (plotter->outstream, "\
/%s reencodeISO def\n",
			   _pcl_font_info[i].ps_name);
	      }
#endif
	  }
      }

      if (num_pages == 1)
	/* EPS [not just PS] file, include procset in setup section,
	   so that it will modify only the private dictionary */
	{
	  fprintf (plotter->outstream, "\
%%%%BeginResource procset %s %s 0\n", 
		   PS_PROCSET_NAME, PS_PROCSET_VERSION);
	  /* write out idraw-derived PS prologue (makes many definitions) */
	  for (i=0; *_ps_procset[i]; i++)
	    fputs (_ps_procset[i], plotter->outstream);
	  fputs ("\
%%EndResource\n", plotter->outstream);
	}

      fputs ("\
%%EndSetup\n\n",
	     plotter->outstream);
      
      /* loop through pages, emitting each in turn */
      if (num_pages > 0)
	for (page = plotter->first_page, n=1; page; page = page->next, n++)
	  {
	    fprintf (plotter->outstream, "\
%%%%Page: %d %d\n",
		     n, n);
	    /* emit the bounding box for the page */
	    _get_page_bbox (page, &xmin, &xmax, &ymin, &ymax);
	    fprintf (plotter->outstream, "\
%%%%PageBoundingBox: %d %d %d %d\n",
		     IROUND(xmin - 0.5), IROUND(ymin - 0.5),
		     IROUND(xmax + 0.5), IROUND(ymax + 0.5));
	    /* Page Setup */
	    fputs ("\
%%BeginPageSetup\n", plotter->outstream);
	    /* emit initialization code (including idraw, PS directives) */
	    /* N.B. `8' below is the version number of the idraw PS format
	       we're producing; see <Unidraw/Components/psformat.h> */
	    fputs ("\
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
/trueoriginalCTM matrix currentmatrix def\n", plotter->outstream);
	    fputs ("\
%%EndPageSetup\n\n", plotter->outstream);
	  
	    /* OUTPUT POSTSCRIPT FOR THIS PAGE */
	    if (page->len > 0)
              fputs (page->base, plotter->outstream); 
	  
            /* Page Trailer: includes idraw, PS directives */
            fputs ("\
%%PageTrailer\n\
End %I eop\n\
showpage\n\n",
		   plotter->outstream);

          /* flush output at end of page (useful in debugging) */
          fflush (plotter->outstream);
          }
      
        /* Document Trailer: pop private dictionary off stack */
        fputs ("\
%%Trailer\n\
end\n\
%%EOF\n",
	       plotter->outstream);
      }
  
    /* delete all pages in document */
  page = plotter->first_page;
  while (page)
    {
      Outbuf *next_page;
	  
      next_page = page->next;
      _delete_outbuf (page);
      page = next_page;
    }
  
    /* flush output stream if any */
  if (plotter->outstream && fflush(plotter->outstream) < 0)
    {
      plotter->warning ("output stream jammed");
      return false;
    }
  else
    return true;
}

/* compute bounding box information for an entire document, starting
   with the page pointed to by BUFP */
static void 
#ifdef _HAVE_PROTOS
_get_document_bbox (Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax)
#else
_get_document_bbox (bufp, xmin, xmax, ymin, ymax)
     Outbuf *bufp;
     double *xmin, *xmax, *ymin, *ymax;
#endif
{
  double doc_x_min = DBL_MAX;
  double doc_y_min = DBL_MAX;  
  double doc_x_max = -(DBL_MAX);
  double doc_y_max = -(DBL_MAX);  
  double page_x_min, page_x_max, page_y_min, page_y_max;
  Outbuf *page = bufp;

  while (page)
    {
      page_x_max = page->xrange_max;
      page_x_min = page->xrange_min;
      page_y_max = page->yrange_max;
      page_y_min = page->yrange_min;

      if (!((page_x_max < page_x_min || page_y_max < page_y_min)))
	/* nonempty page */
	{
	  if (page_x_max > doc_x_max) doc_x_max = page_x_max;
	  if (page_y_max > doc_y_max) doc_y_max = page_y_max;
	  if (page_x_min < doc_x_min) doc_x_min = page_x_min;
	  if (page_y_min < doc_y_min) doc_y_min = page_y_min;
	}
      page = page->next;
    }

  if (doc_x_max < doc_x_min || doc_y_max < doc_y_min)
    /* empty document or all pages empty, return silly values */
    {
      *xmin = 0.5;
      *ymin = 0.5;
      *xmax = -0.5;
      *ymax = -0.5;
    }
  else
    {
      *xmin = doc_x_min;
      *ymin = doc_y_min;
      *xmax = doc_x_max;
      *ymax = doc_y_max;
    }
}

/* compute bounding box information for the page pointed to by BUFP */
static void 
#ifdef _HAVE_PROTOS
_get_page_bbox (Outbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax)
#else
_get_page_bbox (bufp, xmin, xmax, ymin, ymax)
     Outbuf *bufp;
     double *xmin, *xmax, *ymin, *ymax;
#endif
{
  double page_x_min = DBL_MAX;
  double page_y_min = DBL_MAX;  
  double page_x_max = -(DBL_MAX);
  double page_y_max = -(DBL_MAX);  

  if (bufp)
    {
      page_x_max = bufp->xrange_max;
      page_x_min = bufp->xrange_min;
      page_y_max = bufp->yrange_max;
      page_y_min = bufp->yrange_min;
    }

  if (page_x_max < page_x_min || page_y_max < page_y_min)
    /* empty page, return silly values */
    {
      *xmin = 0.5;
      *ymin = 0.5;
      *xmax = -0.5;
      *ymax = -0.5;
    }
  else
    {
      *xmin = page_x_min;
      *ymin = page_y_min;
      *xmax = page_x_max;
      *ymax = page_y_max;
    }
}
