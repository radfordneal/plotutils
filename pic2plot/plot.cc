// The `plot_output' class, subclassed from the `common_output' class.
// In this class we invoke GNU libplot operations to draw objects.

// If the `precision_dashing' flag is set, we draw some types of object
// (arcs, polygons, circles, rounded boxes) in a special way.  The object
// boundary is drawn as a sequence of line segments (if it's to be
// "dashed") or a sequence of filled circles (if it's to be "dotted").
// This is done by invoking e.g. the dashed_arc, dotted_arc, dashed_circle,
// dotted_circle, and rounded_box operations in the `common_output'
// superclass.

// This is the only reason why we subclass from `common_output', rather
// than directly from `output'.

#include "pic.h"
#include "plot.h"
#include "common.h"

// size of graphics display in `virtual inches'
#define DISPLAY_SIZE_IN_INCHES 8.0

#define POINTS_PER_INCH 72.0

// color name array in libplot; undocumented but accessible to programmers

typedef struct
{
  const char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Colornameinfo;

extern const Colornameinfo _colornames[];

// our libplot driver

class plot_output : public common_output
{
public:
  // ctor, dtor
  plot_output();
  ~plot_output();
  // basic interface
  void start_picture (double sc, const position &ll, const position &ur);
  void finish_picture (void);
  // draw objects
  void arc (const position &start, const position &cent, const position &end,
	    const line_type &lt);
  void circle (const position &cent, double rad, const line_type &lt, 
	       double fill);
  void ellipse (const position &cent, const distance &dim,
		const line_type &lt, double fill);
  void line (const position &start, const position *v, int n,
	     const line_type &lt);
  void polygon (const position *v, int n,
		const line_type &lt, double fill);
  void spline (const position &start, const position *v, int n,
	       const line_type &lt);
  void text (const position &center, text_piece *v, int n, double angle);
  void rounded_box (const position &cent, const distance &dim,
		    double rad, const line_type &lt, double fill);
  // attribute-querying function
  int supports_filled_polygons (void);
private:
  // parameters
  int plotter_handle; // libplot Plotter handle
  double default_plotter_line_thickness; // in virtual points
  int pen_red, pen_green, pen_blue;	 // 48-bit pen color
  // dynamic variables, keep track of Plotter drawing state
  int plotter_line_type; // one of line_type::solid etc.
  int plotter_fill_fraction; // libplot fill fraction
  double plotter_line_thickness; // in virtual points
  bool plotter_path_in_progress; // need to break?
  // internal functions, modify Plotter drawing state
  void set_line_type_and_thickness (const line_type &lt);
  void set_fill (double fill);
  // invoked by common_output dotting methods
  void dot(const position &pos, const line_type &lt);
};

output *
make_plot_output()
{
  return new plot_output;
}

plot_output::plot_output()
{
  if ((plotter_handle = pl_newpl (display_type, NULL, stdout, stderr)) < 0)
    {
      fprintf (stderr, "%s: error: could not open plot device\n", 
	       program_name);
      exit (EXIT_FAILURE);
    }
  else
    pl_selectpl (plotter_handle);
}

plot_output::~plot_output()
{
  pl_selectpl (0);
  pl_deletepl (plotter_handle);
}

void 
plot_output::start_picture(double sc, const position &ll,
			   const position &ur)
{
  double xcen, ycen, xmin, xmax, ymin, ymax;
  double scale;

  // open Plotter; record Plotter drawing state defaults
  pl_openpl ();
  plotter_line_type = line_type::solid;
  plotter_fill_fraction = 0;	// i.e. unfilled
  plotter_path_in_progress = false;

  // Compute scale factor via compute_scale() method of output
  // class; see object.cc.  .PS line may contain desired width/height
  // in virtual inches; if so, scale to it.  If .PS line doesn't contain
  // desired width/height, scale according to the global `scale' variable
  // (normally set at top of pic file.  But on no account violate
  // the bounds maxpswid/maxpsht.
  scale = compute_scale(sc, ll, ur);

  /* Initialize map from user space to device space, by specifying
     rectangle in user space that will be mapped to graphics display in
     device space.  Possibly choose rectangle so that plot will be
     centered on the display. */

  if (no_centering_flag)
    {
      xmin = 0.0;
      ymin = 0.0;
    }
  else				// center
    {
      xcen = 0.5 * (ll.x + ur.x);
      ycen = 0.5 * (ll.y + ur.y);
      xmin = xcen - 0.5 * DISPLAY_SIZE_IN_INCHES * scale;
      ymin = ycen - 0.5 * DISPLAY_SIZE_IN_INCHES * scale;
    }
  xmax = xmin + DISPLAY_SIZE_IN_INCHES * scale;
  ymax = ymin + DISPLAY_SIZE_IN_INCHES * scale;

  switch ((int)(rotation_angle))
    {
    case (int)ROT_0:
    default:
      pl_fspace (xmin, ymin, xmax, ymax);
      break;
    case (int)ROT_90:
      pl_fspace2 (xmin, ymax, xmin, ymin, xmax, ymax);
      break;
    case (int)ROT_180:
      pl_fspace2 (xmax, ymax, xmin, ymax, xmax, ymin);
      break;
    case (int)ROT_270:
      pl_fspace2 (xmax, ymin, xmax, ymax, xmin, ymin);
      break;
    }

  // clear Plotter of objects; initialize font name
  pl_erase();
  if (font_name)
    pl_fontname (font_name);
  
  // determine and set color(s)
  if (!precision_dashing)
    // set pen/fill color (will modify only by invoking pl_filltype)
    {
      if (pen_color_name)
	pl_colorname (pen_color_name);
    }
  else
    // set pen color; will invoke pl_pencolor and pl_fillcolor when drawing
    // filled objects, to work around the `zero-width edge' problem
    {
      bool pen_color_found = false;
      const Colornameinfo *color_p = _colornames;

      if (pen_color_name)
	{
	  while (color_p->name)
	    {
	      if (strcmp (pen_color_name, color_p->name) == 0)
		{
		  pen_color_found = true;
		  break;
		}
	      color_p++;
	    }
	}
      
      if (pen_color_found)
	{
	  // convert from 8 to 16 bit intensities
	  pen_red = 0x101 * color_p->red;
	  pen_green = 0x101 * color_p->green;
	  pen_blue = 0x101 * color_p->blue;
	}
      else
	// black pen
	pen_red = pen_green = pen_blue = 0;

      pl_pencolor (pen_red, pen_green, pen_blue);
    }

  // initialize font size and line thickness (latter is dynamic, can
  // be altered in pic file)

  font_size *= scale;
  line_width *= scale;

  if (font_size > 0.0)
    // font size is set on command line in terms of width of display,
    // but libplot uses virtual inches
    pl_ffontsize (DISPLAY_SIZE_IN_INCHES * font_size);

  if (line_width >= 0.0)
    {
      // line_width is set on command line in terms of width of display,
      // but libplot uses virtual inches, and pic2plot uses virtual points
      pl_flinewidth (DISPLAY_SIZE_IN_INCHES * line_width);
      default_plotter_line_thickness 
	= DISPLAY_SIZE_IN_INCHES * POINTS_PER_INCH * line_width;
    }
  else
    // use Plotter default, represented internally by pic2plot as -1
    default_plotter_line_thickness = -1.0;

  /* store initial line thickness as a default, for later use */
  plotter_line_thickness = default_plotter_line_thickness;
}

void 
plot_output::finish_picture()
{
  pl_closepl ();
}

//////////////////////////////////////////////////////////////////////
// SET PLOTTER DRAWING ATTRIBUTES
//////////////////////////////////////////////////////////////////////

// Manipulate fill color (idempotent, so may not actually do anything,
// i.e. may not break the path in progress, if any).

// Two possibilities: (1) If we're not doing precision dashing, we invoke
// pl_filltype.  (2) If we're doing precision dashing, we set the fill
// color by computing it as an RGB, and set it as both our fill color and
// pen color.  This is to avoid the zero edge thickness problem.

void
plot_output::set_fill (double fill)
{
  int fill_fraction;

  if (fill < 0.0)
    fill_fraction = 0;		// unfilled
  else
    {
      if (fill > 1.0)
	fill = 1.0;
      /* fill=0.0 is white, fraction=0xffff; 
	 fill=1.0 is solid color, fraction = 1 */
      fill_fraction = 0xffff - IROUND(0xfffe * fill);
    }

  if (fill_fraction != plotter_fill_fraction)
    {
      if (!precision_dashing)
	// manipulate fill color by setting the fill fraction
	pl_filltype (fill_fraction);
      else
	// doing precision dashing
	{
	  if (fill_fraction == 0)
	    // not filling; reset pen color to default
	    {
	      pl_pencolor (pen_red, pen_green, pen_blue);
	      pl_filltype (0);
	    }
	  else
	    // filling; set fill color, and pen color to be the same
	    {
	      double d_fill_red, d_fill_green, d_fill_blue;
	      int fill_red, fill_green, fill_blue;

	      d_fill_red = 0xffff - fill * (0xffff - pen_red);
	      d_fill_green = 0xffff - fill * (0xffff - pen_green);
	      d_fill_blue = 0xffff - fill * (0xffff - pen_blue);
	      fill_red = IROUND(d_fill_red);
	      fill_green = IROUND(d_fill_green);
	      fill_blue = IROUND(d_fill_blue);
	      pl_color (fill_red, fill_green, fill_blue);
	      pl_filltype (1);
	    }
	}
      
      plotter_fill_fraction = fill_fraction;
      plotter_path_in_progress = false;
    }
}

// Set line type (solid/dashed/dotted) and thickness.  May not invoke a
// libplot operation if neither needs to be changed, so may not break the
// path in progress (if any).
void
plot_output::set_line_type_and_thickness (const line_type &lt)
{
  switch(lt.type)
    {
    case line_type::solid:
    default:
      if (plotter_line_type != line_type::solid)
	{
	  pl_linemod ("solid");
	  plotter_line_type = line_type::solid;
	  plotter_path_in_progress = false;
	}
      break;
    case line_type::dotted:
      if (plotter_line_type != line_type::dotted)
	{
	  double dashbuf[2];

	  pl_linemod ("dotted");
	  dashbuf[0] = 0.25 * lt.dash_width;
	  dashbuf[1] = 0.75 * lt.dash_width;
	  pl_flinedash (2, dashbuf, 0.0);
	  plotter_line_type = line_type::dotted;
	  plotter_path_in_progress = false;
	}
      break;
    case line_type::dashed:
      if (plotter_line_type != line_type::dashed)
	{
	  double dashbuf[2];

	  pl_linemod ("shortdashed");
	  dashbuf[0] = dashbuf[1] = lt.dash_width;
	  pl_flinedash (2, dashbuf, 0.0);
	  plotter_line_type = line_type::dashed;
	  plotter_path_in_progress = false;
	}
      break;
    }
  if (!(lt.thickness < 0.0 && plotter_line_thickness < 0.0)
      && lt.thickness != plotter_line_thickness)
    {
      if (lt.thickness < 0)
	pl_flinewidth (default_plotter_line_thickness / POINTS_PER_INCH);
      else
	pl_flinewidth (lt.thickness / POINTS_PER_INCH);
      plotter_line_thickness = lt.thickness;
      plotter_path_in_progress = false;
    }
}

//////////////////////////////////////////////////////////////////////
// TEXT
//////////////////////////////////////////////////////////////////////

// Draw a text object.
void
plot_output::text(const position &center, text_piece *v, int n, double angle)
{
  int horizontal_adj, vertical_adj;
  double line_spacing;

  // convert from fraction of width of display, to virtual inches
  // also multiply by 1.2 (cf. 10pt with 12pt leading)
  line_spacing = 1.2 * (DISPLAY_SIZE_IN_INCHES * font_size);

  if (n > 0)
    {
      pl_ftextangle (180 * angle / M_PI);
      plotter_path_in_progress = false;

      set_fill (-1.0);		// resets pen color to default
    }
  for (int i = 0; i < n; i++)
    {
      pl_fmove (center.x - (0.5*(n-1) - i) * line_spacing * sin(angle), 
		center.y + (0.5*(n-1) - i) * line_spacing * cos(angle));
      plotter_path_in_progress = false;

      switch ((int)(v[i].adj.h))
	{
	case (int)CENTER_ADJUST:
	default:
	  horizontal_adj = 'c';
	  break;
	case (int)LEFT_ADJUST:
	  horizontal_adj = 'l';
	  break;
	case (int)RIGHT_ADJUST:
	  horizontal_adj = 'r';
	  break;
	}
      switch ((int)(v[i].adj.v))
	{
	case (int)NONE_ADJUST:
	default:
	  vertical_adj = 'c';
	  break;
	case (int)ABOVE_ADJUST:
	  vertical_adj = 'b';
	  break;
	case (int)BELOW_ADJUST:
	  vertical_adj = 't';
	  break;
	}
      pl_alabel (horizontal_adj, vertical_adj, v[i].text);
      plotter_path_in_progress = false;
    }
}

//////////////////////////////////////////////////////////////////////
// OPEN PIC OBJECTS
//////////////////////////////////////////////////////////////////////

// Draw a polyline ("open" in pic's sense, i.e., unfilled, may be part of a
// continuing path).
void
plot_output::line(const position &start, const position *v, int n,
		  const line_type &lt)
{
  if (n == 0)
    return;
  if (lt.type == line_type::invisible)
    {
      pl_fmove (v[n-1].x, v[n-1].y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (-1.0);		// unfilled, pic convention

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_line_type_and_thickness (lt);
      pl_fline (start.x, start.y, v[0].x, v[0].y);
      for (int i = 1; i < n; i++)
	pl_fcont (v[i].x, v[i].y);
      plotter_path_in_progress = true;
    }
  else
    {
      switch (lt.type) 
	{
	case line_type::dashed:
	  {
	    // edge polyline, with dashes
	    line_type slt = lt;
	    slt.type = line_type::solid;
	    set_line_type_and_thickness (slt);
	    position from_point = start, to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		if (dist <= lt.dash_width*2.0)
		  pl_fline(from_point.x, from_point.y, to_point.x, to_point.y);
		else 
		  {
		    // round number of dashes to integer, along each segment
		    int ndashes = int((dist - lt.dash_width)/(lt.dash_width*2.0) + .5);
		    distance dash_vec = vec*(lt.dash_width/dist);
		    double dash_gap = (dist - lt.dash_width)/ndashes;
		    distance dash_gap_vec = vec*(dash_gap/dist);
		    for (int j = 0; j <= ndashes; j++) 
		      {
			position s(from_point + dash_gap_vec*j);
			pl_fline(s.x, s.y, s.x + dash_vec.x, s.y + dash_vec.y);
		      }
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	    pl_endpath ();
	    plotter_path_in_progress = false;
	  }
	  break;
	case line_type::dotted:
	  {
	    // edge polyline, with dots
	    position from_point = start, to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		// round dot spacings to integer, along line segment
		int ndots = IROUND(dist/lt.dash_width);
		if (ndots == 0)
		  dot(from_point, lt);
		else 
		  {
		    vec /= double(ndots);
		    for (int j = 0; j <= ndots; j++)
		      dot(from_point + vec*j, lt);
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	  }
	  break;
	default:
	  break;
	}
    }
}

// Draw a spline ("open" in pic's sense, i.e. unfilled, may be part 
// of a continuing path).
void
plot_output::spline(const position &start, const position *v, int n,
		    const line_type &lt)
{
  if (n == 0)
    return;
  if (lt.type == line_type::invisible)
    {
      pl_fmove (v[n-1].x, v[n-1].y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (-1.0);		// unfilled, pic convention
  set_line_type_and_thickness (lt);

  if (n == 1)
    pl_fline (start.x, start.y, v[0].x, v[0].y);    
  else if (n == 2)
    pl_fbezier2 (start.x, start.y, v[0].x, v[0].y, v[1].x, v[1].y);
  else
    {
      pl_fbezier2 (start.x, start.y, 
		   v[0].x, v[0].y,
		   0.5 * (v[0].x + v[1].x), 0.5 * (v[0].y + v[1].y));
      for (int i = 0; i < n - 3; i++)
	pl_fbezier2 (0.5 * (v[i].x + v[i+1].x), 0.5 * (v[i].y + v[i+1].y),
		     v[i+1].x, v[i+1].y,
		     0.5 * (v[i+1].x + v[i+2].x), 0.5 * (v[i+1].y + v[i+2].y));
      pl_fbezier2 (0.5 * (v[n-3].x + v[n-2].x), 0.5 * (v[n-3].y + v[n-2].y),
		   v[n-2].x, v[n-2].y,
		   v[n-1].x, v[n-1].y);
    }
  plotter_path_in_progress = true;
}

// Draw an arc object ("open" in pic's sense, i.e., unfilled, may
// be part of a continuing path).
void
plot_output::arc (const position &start, const position &cent,
		  const position &end, const line_type &lt)
     // in libplot, arcs don't subtend >= 180 degrees, but that's OK
     // because they don't subtend >=180 degrees in pic either
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove (end.x, end.y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (-1.0);		// unfilled (pic convention)

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_line_type_and_thickness (lt);
      pl_farc (cent.x, cent.y, start.x, start.y, end.x, end.y);
      plotter_path_in_progress = true;
    }
  else
    {
      line_type slt;

      slt = lt;
      slt.type = line_type::solid;
      set_line_type_and_thickness (slt);
      switch (lt.type) 
	{
	case line_type::dashed:
	  // edge arc, with dashes
	  if (plotter_path_in_progress)
	    pl_endpath ();
	  dashed_arc(start, cent, end, lt);
	  pl_endpath ();
	  plotter_path_in_progress = false;
	  break;
	case line_type::dotted:
	  // edge arc, with dots
	  dotted_arc(start, cent, end, lt);
	  plotter_path_in_progress = false;
	  break;
	default:
	  break;
	}
    }
}

//////////////////////////////////////////////////////////////////////
// CLOSED PIC OBJECTS
// (some drawn differently if we do `precision dashing')
//////////////////////////////////////////////////////////////////////

// Draw a polyline object ("closed" in pic's sense).
void
plot_output::polygon(const position *v, int n,
		     const line_type &lt, double fill)
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove (v[n-1].x, v[n-1].y);
      plotter_path_in_progress = false;
      return;
    }

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_fill (fill);
      set_line_type_and_thickness (lt);
      if (n == 4 
	  && v[0].x == v[1].x && v[2].x == v[3].x
	  && v[0].y == v[3].y && v[1].y == v[2].y)
	{
	  pl_fbox (v[3].x, v[3].y, v[1].x, v[1].y);
	  plotter_path_in_progress = false;
	}
      else
	{
	  pl_fmove (v[n-1].x, v[n-1].y);
	  for (int i = 0; i < n; i++)
	    pl_fcont (v[i].x, v[i].y);
	  pl_endpath ();
	  plotter_path_in_progress = false;
	}
    }
  else
    // precision dashing (or dotting)
    {
      line_type slt;

      if (fill >= 0.0)
	// fill polygon, by drawing/filling with zero edge width
	{
	  set_fill (fill);
	  slt.type = line_type::solid;
	  slt.thickness = 0.0;
	  set_line_type_and_thickness (slt);
	  pl_fmove (v[n-1].x, v[n-1].y);
	  for (int i = 0; i < n; i++)
	    pl_fcont (v[i].x, v[i].y);
	  pl_endpath ();
	  plotter_path_in_progress = false;
	}

      // draw polygon boundary (unfilled) 
      set_fill (-1.0);
      switch (lt.type) 
	{
	case line_type::dashed:
	  {
	    // edge polygon, with dashes
	    slt = lt;
	    slt.type = line_type::solid;
	    set_line_type_and_thickness (slt);
	    position from_point = v[n-1], to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		if (dist <= lt.dash_width*2.0)
		  pl_fline(from_point.x, from_point.y, to_point.x, to_point.y);
		else 
		  {
		    // round number of dashes to integer, along each segment
		    int ndashes = int((dist - lt.dash_width)/(lt.dash_width*2.0) + .5);
		    distance dash_vec = vec*(lt.dash_width/dist);
		    double dash_gap = (dist - lt.dash_width)/ndashes;
		    distance dash_gap_vec = vec*(dash_gap/dist);
		    for (int j = 0; j <= ndashes; j++) 
		      {
			position s(from_point + dash_gap_vec*j);
			pl_fline(s.x, s.y, s.x + dash_vec.x, s.y + dash_vec.y);
		      }
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	    pl_endpath ();
	    plotter_path_in_progress = false;
	  }
	  break;
	case line_type::dotted:
	  {
	    // edge polygon, with dots
	    position from_point = v[n-1], to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		// round dot spacings to integer, along line segment
		int ndots = IROUND(dist/lt.dash_width);
		if (ndots == 0)
		  dot(from_point, lt);
		else 
		  {
		    vec /= double(ndots);
		    for (int j = 0; j <= ndots; j++)
		      dot(from_point + vec*j, lt);
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	  }
	  break;
	default:		// shouldn't happen
	  break;
	}
    }
}

// Draw a circle object ("closed" in pic's sense).
void
plot_output::circle (const position &cent, double rad,
		     const line_type &lt, double fill)
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove (cent.x, cent.y);
      plotter_path_in_progress = false;
      return;
    }

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_fill (fill);
      set_line_type_and_thickness (lt);
      pl_fcircle (cent.x, cent.y, rad);
      plotter_path_in_progress = false;
    }
  else
    // precision dashing (or dotting)
    {
      line_type slt;

      if (fill >= 0.0)
	// fill circle, by drawing/filling with zero edge width
	{
	  set_fill (fill);
	  slt = lt;
	  slt.type = line_type::solid;
	  slt.thickness = 0.0;
	  set_line_type_and_thickness (slt);
	  pl_fcircle (cent.x, cent.y, rad);
	  plotter_path_in_progress = false;
	}

      // draw circle boundary (unfilled)
      set_fill (-1.0);
      slt = lt;
      slt.type = line_type::solid;
      set_line_type_and_thickness (slt);
      switch (lt.type) 
	{
	case line_type::dashed:
	  // edge circle, with dashes
	  if (plotter_path_in_progress)
	    pl_endpath ();
	  dashed_circle(cent, rad, lt);
	  pl_endpath ();
	  plotter_path_in_progress = false;
	  break;
	case line_type::dotted:
	  // edge circle, with dots
	  dotted_circle(cent, rad, lt);
	  break;
	default:		// shouldn't happen
	  break;
	}
    }
}

// Draw a rounded box object ("closed" in pic's sense).
void
plot_output::rounded_box(const position &cent, const distance &dim, double rad, const line_type &lt, double fill)
{
  position tem, arc_start, arc_cent, arc_end;
  position line_start, line_end;

  if (lt.type == line_type::invisible)
    {
      pl_fmove (cent.x, cent.y);      
      plotter_path_in_progress = false;
      return;
    }

  if (plotter_path_in_progress)
    {
      pl_endpath ();
      plotter_path_in_progress = false;      
    }

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_fill (fill);
      set_line_type_and_thickness (lt);

      tem = cent - dim/2.0;
      arc_start = tem + position(0.0, rad);
      arc_cent = tem + position(rad, rad);
      arc_end = tem + position(rad, 0.0);
      pl_farc (arc_cent.x, arc_cent.y, 
	       arc_start.x, arc_start.y, arc_end.x, arc_end.y);

      line_start = cent + position(-dim.x/2.0 + rad, -dim.y/2.0);
      line_end = cent + position(dim.x/2.0 - rad, -dim.y/2.0);
      pl_fline (arc_end.x, arc_end.y, line_end.x, line_end.y);

      tem = cent + position(dim.x/2.0, -dim.y/2.0);
      arc_start = tem + position(-rad, 0.0);
      arc_cent = tem + position(-rad, rad);
      arc_end = tem + position(0.0, rad);
      pl_farc (arc_cent.x, arc_cent.y, 
	       line_end.x, line_end.y, arc_end.x, arc_end.y);

      line_start = cent + position(dim.x/2.0, -dim.y/2.0 + rad);
      line_end = cent + position(dim.x/2.0, dim.y/2.0 - rad);
      pl_fline (arc_end.x, arc_end.y, line_end.x, line_end.y);

      tem = cent + dim/2.0;
      arc_start = tem + position(0.0, -rad);
      arc_cent = tem + position(-rad, -rad);
      arc_end = tem + position(-rad, 0.0);
      pl_farc (arc_cent.x, arc_cent.y, 
	       line_end.x, line_end.y, arc_end.x, arc_end.y);

      line_start = cent + position(dim.x/2.0 - rad, dim.y/2.0);
      line_end = cent + position(-dim.x/2.0 + rad, dim.y/2.0);
      pl_fline (arc_end.x, arc_end.y, line_end.x, line_end.y);

      tem = cent + position(-dim.x/2.0, dim.y/2.0);
      arc_start  = tem + position(rad, 0.0);
      arc_cent =  tem + position(rad, -rad);
      arc_end =  tem + position(0.0, -rad);
      pl_farc (arc_cent.x, arc_cent.y, 
	       line_end.x, line_end.y, arc_end.x, arc_end.y);

      line_start = cent + position(-dim.x/2.0, dim.y/2.0 - rad);
      line_end = cent + position(-dim.x/2.0, -dim.y/2.0 + rad);
      pl_fline (arc_end.x, arc_end.y, line_end.x, line_end.y);

      pl_endpath ();
      plotter_path_in_progress = false;
    }
  else
    // precision dashing (or dotting)
    {
      if (fill >= 0.0)
	{
	  // fill rounded box (boundary solid, thickness 0) via recursive call
	  set_fill (fill);
	  line_type slt = lt;
	  slt.type = line_type::solid;
	  slt.thickness = 0.0;
	  rounded_box(cent, dim, rad, slt, fill);
	  plotter_path_in_progress = false;
	}

      // draw rounded box boundary, unfilled
      set_line_type_and_thickness (lt);	// only thickness is relevant
      common_output::rounded_box(cent, dim, rad, lt, -1.0); //-1 means unfilled
      if (plotter_path_in_progress)
	{
	  pl_endpath ();
	  plotter_path_in_progress = false;      
	}
    }
}

// Draw an ellipse object ("closed" in pic's sense).
void
plot_output::ellipse(const position &cent, const distance &dim,
		     const line_type &lt, double fill)
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove (cent.x, cent.y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (fill);
  set_line_type_and_thickness (lt);
  pl_fellipse (cent.x, cent.y, 0.5 * dim.x, 0.5 * dim.y, 0.0);
  plotter_path_in_progress = false;
}

//////////////////////////////////////////////////////////////////////
// MISC.
//////////////////////////////////////////////////////////////////////

// Internal function, used for precision dotting; also invoked by
// precision dotting methods in the common_output superclass.
void
plot_output::dot(const position &cent, const line_type &lt)
// lt arg determines diameter of dot
{
  line_type slt;
  
  set_fill (1.0);
  slt.type = line_type::solid;
  slt.thickness = 0.0;
  set_line_type_and_thickness (slt);
  pl_fcircle (cent.x, cent.y, 0.5 * lt.thickness / POINTS_PER_INCH);
  plotter_path_in_progress = false;
}

int
plot_output::supports_filled_polygons()
{
  return 1;
}
