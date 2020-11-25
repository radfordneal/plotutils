#include "sys-defines.h"
#include "extern.h"

/* forward references */
static void _write_svg_transform ____P((plOutbuf *outbuf, const double m[6]));

bool
#ifdef _HAVE_PROTOS
_s_end_page (S___(Plotter *_plotter))
#else
_s_end_page (S___(_plotter))
     S___(Plotter *_plotter;)
#endif
{
  plOutbuf *svg_header, *svg_trailer;
      
  /* SVG files contain only one page of graphics so this is a sanity check */
  if (_plotter->data->page_number != 1)	
    return true;

  /* prepare SVG header (i.e. page header), write it to a plOutbuf */
  svg_header = _new_outbuf ();
      
  /* start with DTD */
  sprintf (svg_header->point, "\
<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"no\"?>\n\
<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20000303 Stylable//EN\"\n\
\"http://www.w3.org/TR/2000/03/WD-SVG-20000303/DTD/svg-20000303-stylable.dtd\">\n");
  _update_buffer (svg_header);

  /* Emit nominal physical size of the device-frame viewport (and specify
     that in the device-frame coordinates we use, it's a unit square).
     viewport_{x,y}size are set from the PAGESIZE Plotter parameter, and
     either or both may be negative.  If they are, we flipped the
     NDC_frame->device_frame map to compensate (see s_defplot.c).  Which is
     why we can take absolute values here. */

  if (_plotter->data->page_data->metric)
    sprintf (svg_header->point, 
	     "<svg width=\"%.5gcm\" height=\"%.5gcm\" %s %s>\n",
	     2.54 * FABS(_plotter->data->viewport_xsize),
	     2.54 * FABS(_plotter->data->viewport_ysize),
	     "viewBox=\"0 0 1 1\"",
	     "preserveAspectRatio=\"none\"");
  else
    sprintf (svg_header->point, 
	     "<svg width=\"%.5gin\" height=\"%.5gin\" %s %s>\n",
	     FABS(_plotter->data->viewport_xsize),
	     FABS(_plotter->data->viewport_ysize),
	     "viewBox=\"0 0 1 1\"",
	     "preserveAspectRatio=\"none\"");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "<title>SVG drawing</title>\n");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "<desc>This was produced by version %s of GNU libplot, a free library for exporting 2-D vector graphics.</desc>\n", 
	   PL_LIBPLOT_VER_STRING);
  _update_buffer (svg_header);

  if (_plotter->s_bgcolor_suppressed == false)
  /* place a background rectangle behind, covering entire viewport */
    {
      char color_buf[8];	/* enough room for "#ffffff", incl. NUL */

      sprintf (svg_header->point, 
	       "<rect x=\"0\" y=\"0\" width=\"1\" height=\"1\" style=\"stroke:none;fill:%s;\"/>\n",
	       _libplot_color_to_svg_color (_plotter->s_bgcolor, color_buf));
      _update_buffer (svg_header);
    }

  /* enclose everything else in a container */
  sprintf (svg_header->point, "<g ");
  _update_buffer (svg_header);
      
  if (_plotter->s_matrix_is_unknown == false
      && _plotter->s_matrix_is_bogus == false)
    /* Place a transform in the container: this page's default
       transformation matrix, which is simply the transformation matrix
       attribute of the very first graphical object plotted on the page.

       In libplot, `transformation matrix attribute' refers to the affine
       map from user space to NDC space.  So we're careful to multiply by
       `m_ndc_to_device', which transforms NDC space to device space.
       Because SVG uses a flipped-y convention, `m_ndc_to_device' flips the
       y coordinate.  There will be additional flipping if the
       user-specified xsize, ysize are negative; see s_defplot.c.  Also, if
       the user-specified ROTATION Plotter parameter is set, it may do a
       90, 180, or 270 degree rotation. */
    {
      double product[6];

      _matrix_product (_plotter->s_matrix, _plotter->data->m_ndc_to_device,
		       product);
      _write_svg_transform (svg_header, product);
    }

  /* turn off SVG's default [unfortunate] XML-inherited treatment of spaces */
  sprintf (svg_header->point, "xml:space=\"preserve\" ");
  _update_buffer (svg_header);

  /* specify style properties (all libplot defaults) */

  sprintf (svg_header->point, "style=\"");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke:%s;",
	   "black");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-linecap:%s;",
	   "butt");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-linejoin:%s;",
	   "miter");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-miterlimit:%.5g;",
	   DEFAULT_MITER_LIMIT);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-dasharray:%s;",
	   "none");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-dashoffset:%.5g;",
	   0.0);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-opacity:%.5g;",
	   1.0);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "fill:%s;",
	   "none");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "fill-rule:%s;",
	   "even-odd");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "fill-opacity:%.5g;",
	   1.0);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-style:%s;",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-variant:%s;",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-weight:%s;",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-stretch:%s;",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-size-adjust:%s;",
	   "none");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "letter-spacing:%s;",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "word-spacing:%s;",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "text-anchor:%s;",
	   "start");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "\"");
  _update_buffer (svg_header);

  sprintf (svg_header->point, ">\n");
  _update_buffer (svg_header);

  /* place SVG header in this page's plOutbuf */
  _plotter->data->page->header = svg_header;

  /* prepare SVG trailer too, write it to a plOutbuf */
  svg_trailer = _new_outbuf ();
  
  sprintf (svg_trailer->point, "</g>\n");
  _update_buffer (svg_trailer);

  sprintf (svg_trailer->point, "</svg>\n");
  _update_buffer (svg_trailer);
  
  /* place SVG trailer in this page's plOutbuf */
  _plotter->data->page->trailer = svg_trailer;

  return true;
}

/* This function is invoked while writing any graphical object on a page to
   the page's output buffer.  It emits the string "transform=\"...\" ",
   where the "\"...\"" is computed from the transformation matrix attribute
   of the object, which is passed.

   The transformation matrix attribute of the first object to be written on
   the page sets `s_matrix', the global transformation matrix for the page,
   which will later be written at the head of the SVG code for the page
   when closepl() is invoked (see above).  Because we may need to compute
   the inverse of the global transformation matrix, we flag `s_matrix' as
   bogus if it's singular.  If it's bogus, it won't be written out when
   closepl() is invoked, and the global transformation matrix of the page
   will effectively be the identity.

   This function supports passing the transformation matrix attribute of an
   object via two arguments: a base piece and a local piece, only the first
   of which may get stored as `s_matrix'.  This two-piece approach is
   useful when plotting rotated text, for example (the local piece would
   contains the rotation, which really shouldn't affect `s_matrix'). */

void
#ifdef _HAVE_PROTOS
_s_set_matrix (R___(Plotter *_plotter) const double m_base[6], const double m_local[6])
#else
_s_set_matrix (R___(_plotter) m_base, m_local)
     S___(Plotter *_plotter;)
     const double m_base[6], m_local[6];
#endif
{
  double m[6];
  const double *m_emitted = (const double *)NULL; /* keep compiler happy */
  bool need_transform_attribute = false;
  int i;
  
  /* if this is the first time this function is invoked on a page, store
     base matrix for later use as global page transformation matrix */
  if (_plotter->s_matrix_is_unknown)
    {
      for (i = 0; i < 6; i++)
	_plotter->s_matrix[i] = m_base[i];

      _plotter->s_matrix_is_unknown = false;

      if (m_base[0] * m_base[3] - m_base[1] * m_base[2] == 0.0) 
	/* singular, won't be used even though stored */
	_plotter->s_matrix_is_bogus = true;
    }

  /* compute product: current transformation matrix */
  _matrix_product (m_local, m_base, m);

  /* determine whether current matrix is different from the global one that
     will be wrapped around the entire page (if there is one) */

  if (_plotter->s_matrix_is_bogus == false)
    /* have a global page-specific transformation matrix that will be
       applied, so object's transform attribute may need to compensate */
    {
      for (i = 0; i < 6; i++)
	{
	  if (m[i] != _plotter->s_matrix[i])
	    /* different, so need to compensate */
	    {
	      need_transform_attribute = true;
	      break;
	    }
	}

      if (need_transform_attribute)
	{
	  double inverse[6], product[6];

	  _matrix_inverse (_plotter->s_matrix, inverse);
	  _matrix_product (m, inverse, product);
	  m_emitted = product;
	}
    }
  else
    /* no global transformation matrix for this page (no doubt because of
       the abovementioned non-invertibility problem), so object's transform
       attribute will simply be the current matrix */
    {
      need_transform_attribute = true;
      m_emitted = m;
    }
  
  /* emit object's transform attribute if it's not the identity */
  if (need_transform_attribute)
    _write_svg_transform (_plotter->data->page, m_emitted);
}

/* Internal function for writing out a PS-style affine transformation as a
   SVG-style affine transformation.  If matrix is the identity, nothing is
   written. */

static void
#ifdef _HAVE_PROTOS
_write_svg_transform (plOutbuf *outbuf, const double m[6])
#else
_write_svg_transform (outbuf, m)
     plOutbuf *outbuf;
     const double m[6];
#endif
{
  double mm[6];
  double max_value = 0.0;
  int i;
  int type = 0;			/* default */
      
  /* compensate for possible roundoff error: treat very small elements of
     linear transformation (if any) as zero */
#define VERY_SMALL_FACTOR 1e-10
  
  for (i = 0; i < 4; i++)
    max_value = DMAX(max_value, FABS(m[i]));
  for (i = 0; i < 6; i++)
    if (i < 4 && FABS(m[i]) < VERY_SMALL_FACTOR * max_value)
      mm[i] = 0;
    else
      mm[i] = m[i];

  if (mm[0] == 1.0 && mm[1] == 0.0 && mm[2] == 0.0 && mm[3] == 1.0
      && mm[4] == 0.0 && mm[5] == 0.0)
    /* identity matrix, unnecessary to write it */
    return;

  /* treat several types of affine transformation specially */

  if (mm[1] == 0.0 && mm[2] == 0.0)
    type = 1;			/* scale + translation */

  else if (mm[0] == 0.0 && mm[1] == 1.0 && mm[2] == -1.0 && mm[3] == 0.0)
    type = 2;			/* rotation by 90 + translation */
  else if (mm[0] == 0.0 && mm[1] == -1.0 && mm[2] == 1.0 && mm[3] == 0.0)
    type = 3;			/* rotation by 270 + translation */
  else if (mm[0] == 0.0 && mm[1] == 1.0 && mm[2] == 1.0 && mm[3] == 0.0)
    type = 4;			/* y-flip + rotation by 90 + translation */
  else if (mm[0] == 0.0 && mm[1] == -1.0 && mm[2] == -1.0 && mm[3] == 0.0)
    type = 5;			/* y-flip + rotation by 270 + translation */
  
  sprintf (outbuf->point, "transform=\"");
  _update_buffer (outbuf);
      
  if (type != 0)
    {
      /* emit translation if any (SVG will perform it last, since SVG uses
	 opposite order from PS for multiplying matrices) */
      if (mm[4] != 0.0 || mm[5] != 0.0)
	{
	  if (mm[5] == 0.0)
	    sprintf (outbuf->point, "translate(%.5g) ",
		     mm[4]);
	  else
	    sprintf (outbuf->point, "translate(%.5g,%.5g) ",
		     mm[4], mm[5]);
	  _update_buffer (outbuf);	      
	}

      switch (type)
	{
	case 1:
	  if (mm[0] != 1.0 || mm[3] != 1.0)
	    {
	      if (mm[3] == mm[0])
		sprintf (outbuf->point, "scale(%.5g) ",
			 mm[0]);
	      else if (mm[3] == -mm[0])
		{
		  if (mm[0] != 1.0)
		    sprintf (outbuf->point, "scale(1,-1) scale(%.5g) ",
			     mm[0]);
		  else
		    sprintf (outbuf->point, "scale(1,-1) ");
		}
	      else
		sprintf (outbuf->point, "scale(%.5g,%.5g) ",
			 mm[0], mm[3]);
	      _update_buffer (outbuf);	      
	    }
	  break;

	case 2:
	  sprintf (outbuf->point, "rotate(90) ");
	  _update_buffer (outbuf);	      
	  break;

	case 3:
	  sprintf (outbuf->point, "rotate(270) ");
	  _update_buffer (outbuf);	      
	  break;

	case 4:
	  sprintf (outbuf->point, "rotate(90) scale(1,-1) ");
	  _update_buffer (outbuf);	      
	  break;

	case 5:
	  sprintf (outbuf->point, "rotate(270) scale(1,-1) ");
	  _update_buffer (outbuf);	      
	  break;

	default:		/* shouldn't happen */
	  break;
	}
    }
  else
    /* general affine transformation */
    {
      sprintf (outbuf->point, "matrix(%.5g %.5g %.5g %.5g %.5g %.5g) ",
	       mm[0], mm[1], mm[2], mm[3], mm[4], mm[5]);
      _update_buffer (outbuf);
    }

  sprintf (outbuf->point, "\" ");
  _update_buffer (outbuf);
}
