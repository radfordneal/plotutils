// The common_output class: subclassed from the output class, 
// providing support for dotted/dashed arcs and circles, and for 
// rounded boxes (possible dotted/dashed).

#include "pic.h"
#include "output.h"
#include "common.h"

// output a dashed circle as a series of arcs
void
common_output::dashed_circle(const position &cent, double rad, const line_type &lt)
{
  assert (lt.type == line_type::dashed);
  line_type slt = lt;
  slt.type = line_type::solid;
  double dash_angle = lt.dash_width/rad; // dash angle in radians
  int ndashes;
  double gap_angle;

  if (dash_angle >= M_PI/4.0) 
    {
      if (dash_angle < M_PI/2.0) 
	{
	  gap_angle = M_PI/2.0 - dash_angle;
	  ndashes = 4;
	}
      else if (dash_angle < M_PI) 
	{
	  gap_angle = M_PI - dash_angle;
	  ndashes = 2;
	}
      else 
	{
	  circle (cent, rad, slt, -1.0);
	  return;
	}
    }
  else 
    {
      ndashes = 4 * int(ceil(M_PI/(4.0*dash_angle)));
      gap_angle = (M_PI*2.0)/ndashes - dash_angle;
    }
  for (int i = 0; i < ndashes; i++) 
    {
      double start_angle = i*(dash_angle+gap_angle) - 0.5 * dash_angle;
      solid_arc (cent, rad, start_angle, start_angle + dash_angle, lt);
    }
}

// output a dotted circle as a series of dots
void 
common_output::dotted_circle(const position &cent, double rad, const line_type &lt)
{
  double gap_angle = lt.dash_width/rad;	// gap angle in radians
  int ndots;
  double ang = 0.0;

  assert (lt.type == line_type::dotted);
  if (gap_angle >= M_PI/2.0) 
    {
      // always have at least 2 dots
      gap_angle = M_PI;
      ndots = 2;
    }
  else 
    {
      ndots = 4*int(M_PI/(2.0*gap_angle));
      gap_angle = (M_PI*2.0)/ndots;
    }
  for (int i = 0; i < ndots; i++, ang += gap_angle)
    dot (cent + position(cos(ang), sin(ang))*rad, lt);
}

// return non-zero iff we can compute a center
int
compute_arc_center(const position &start, const position &cent, const position &end, position *result)
{
  // This finds the point along the vector from start to cent that
  // is equidistant between start and end.
  distance c = cent - start;
  distance e = end - start;
  double n = c*e;
  if (n == 0.0)
    return 0;
  else
    {
      *result = start + c*((e*e)/(2.0*n));
      return 1;
    }
}

// output a dashed arc as a series of arcs
void
common_output::dashed_arc(const position &start, const position &cent, const position &end, const line_type &lt)
{
  assert (lt.type == line_type::dashed);
  position c;
  if (!compute_arc_center(start, cent, end, &c)) 
    {
      line(start, &end, 1, lt);
      return;
    }
  distance start_offset = start - c;
  distance end_offset = end - c;
  double start_angle = atan2(start_offset.y, start_offset.x);
  double end_angle = atan2(end_offset.y, end_offset.x);
  double rad = hypot(c - start);
  double dash_angle = lt.dash_width/rad;
  double total_angle = end_angle - start_angle;
  while (total_angle < 0)
    total_angle += M_PI + M_PI;
  if (total_angle <= dash_angle*2.0) 
    {
      solid_arc(cent, rad, start_angle, end_angle, lt);
      return;
    }
  int ndashes = int((total_angle - dash_angle)/(dash_angle*2.0) + 0.5);
  double dash_and_gap_angle = (total_angle - dash_angle)/ndashes;
  for (int i = 0; i <= ndashes; i++)
    solid_arc(cent, rad, start_angle + i*dash_and_gap_angle,
	      start_angle + i*dash_and_gap_angle + dash_angle, lt);
}

// output a dotted arc as a series of dots
void
common_output::dotted_arc(const position &start, const position &cent, const position &end, const line_type &lt)
{
  assert (lt.type == line_type::dotted);
  position c;
  if (!compute_arc_center(start, cent, end, &c)) 
    {
      line(start, &end, 1, lt);
      return;
    }
  distance start_offset = start - c;
  distance end_offset = end - c;
  double start_angle = atan2(start_offset.y, start_offset.x);
  double total_angle = atan2(end_offset.y, end_offset.x) - start_angle;
  while (total_angle < 0)
    total_angle += M_PI + M_PI;
  double rad = hypot(c - start);
  int ndots = int(total_angle/(lt.dash_width/rad) + .5);
  if (ndots == 0)
    dot (start, lt);
  else 
    {
      for (int i = 0; i <= ndots; i++) 
	{
	  double a = start_angle + (total_angle*i)/ndots;
	  dot (cent + position(cos(a), sin(a))*rad, lt);
	}
    }
}

// output a solid arc, of the sort used in dashing
void
common_output::solid_arc(const position &cent, double rad, double start_angle, double end_angle, const line_type &lt)
{
  line_type slt = lt;
  slt.type = line_type::solid;
  arc (cent + position(cos(start_angle), sin(start_angle))*rad,
       cent,
       cent + position(cos(end_angle), sin(end_angle))*rad,
       slt);
}

// output a rounded box (of one of several line types)
void
common_output::rounded_box(const position &cent, const distance &dim, double rad, const line_type &lt, double fill)
{
  if (fill >= 0.0)
    filled_rounded_box(cent, dim, rad, fill);
  switch (lt.type) 
    {
    case line_type::invisible:
      break;
    case line_type::dashed:
      dashed_rounded_box(cent, dim, rad, lt);
      break;
    case line_type::dotted:
      dotted_rounded_box(cent, dim, rad, lt);
      break;
    case line_type::solid:
      solid_rounded_box(cent, dim, rad, lt);
      break;
    default:
      assert (0);
    }
}

// output a dashed rounded box as a series of arcs
void
common_output::dashed_rounded_box(const position &cent, const distance &dim, double rad, const line_type &lt)
{
  line_type slt = lt;
  slt.type = line_type::solid;

  double hor_length = dim.x + (M_PI/2.0 - 2.0)*rad;
  int n_hor_dashes = int(hor_length/(lt.dash_width*2.0) + 0.5);
  double hor_gap_width = (n_hor_dashes != 0
			  ? hor_length/n_hor_dashes - lt.dash_width
			  : 0.0);

  double vert_length = dim.y + (M_PI/2.0 - 2.0)*rad;
  int n_vert_dashes = int(vert_length/(lt.dash_width*2.0) + 0.5);
  double vert_gap_width = (n_vert_dashes != 0
			   ? vert_length/n_vert_dashes - lt.dash_width
			   : 0.0);
  // Each corner arc must be split into two for dashing,
  // with one part dashed using vert_gap_width, and the other
  // using hor_gap_width.
  double offset = lt.dash_width/2.0;
  dash_arc(cent + position(dim.x/2.0 - rad, -dim.y/2.0 + rad), rad,
	   -M_PI/4.0, 0, slt, lt.dash_width, vert_gap_width, &offset);
  dash_line(cent + position(dim.x/2.0, -dim.y/2.0 + rad),
	    cent + position(dim.x/2.0, dim.y/2.0 - rad),
	    slt, lt.dash_width, vert_gap_width, &offset);
  dash_arc(cent + position(dim.x/2.0 - rad, dim.y/2.0 - rad), rad,
	   0, M_PI/4.0, slt, lt.dash_width, vert_gap_width, &offset);

  offset = lt.dash_width/2.0;
  dash_arc(cent + position(dim.x/2.0 - rad, dim.y/2.0 - rad), rad,
	   M_PI/4.0, M_PI/2, slt, lt.dash_width, hor_gap_width, &offset);
  dash_line(cent + position(dim.x/2.0 - rad, dim.y/2.0),
	    cent + position(-dim.x/2.0 + rad, dim.y/2.0),
	    slt, lt.dash_width, hor_gap_width, &offset);
  dash_arc(cent + position(-dim.x/2.0 + rad, dim.y/2.0 - rad), rad,
	   M_PI/2, 3*M_PI/4.0, slt, lt.dash_width, hor_gap_width, &offset);

  offset = lt.dash_width/2.0;
  dash_arc(cent + position(-dim.x/2.0 + rad, dim.y/2.0 - rad), rad,
	   3.0*M_PI/4.0, M_PI, slt, lt.dash_width, vert_gap_width, &offset);
  dash_line(cent + position(-dim.x/2.0, dim.y/2.0 - rad),
	    cent + position(-dim.x/2.0, -dim.y/2.0 + rad),
	    slt, lt.dash_width, vert_gap_width, &offset);
  dash_arc(cent + position(-dim.x/2.0 + rad, -dim.y/2.0 + rad), rad,
	   M_PI, 5.0*M_PI/4.0, slt, lt.dash_width, vert_gap_width, &offset);

  offset = lt.dash_width/2.0;
  dash_arc(cent + position(-dim.x/2.0 + rad, -dim.y/2.0 + rad), rad,
	   5*M_PI/4.0, 3*M_PI/2.0, slt, lt.dash_width, hor_gap_width, &offset);
  dash_line(cent + position(-dim.x/2.0 + rad, -dim.y/2.0),
	    cent + position(dim.x/2.0 - rad, -dim.y/2.0),
	    slt, lt.dash_width, hor_gap_width, &offset);
  dash_arc(cent + position(dim.x/2.0 - rad, -dim.y/2.0 + rad), rad,
	   3*M_PI/2, 7*M_PI/4, slt, lt.dash_width, hor_gap_width, &offset);
}

// Private, used by dashed_rounded_box.
void
common_output::dash_arc(const position &cent, double rad, double start_angle, double end_angle, const line_type &lt, double dash_width, double gap_width, double *offsetp)
{
  double length = (end_angle - start_angle)*rad;
  double pos = 0.0;
  for (;;) 
    {
      if (*offsetp >= dash_width) 
	{
	  double rem = dash_width + gap_width - *offsetp;
	  if (pos + rem > length) 
	    {
	      *offsetp += length - pos;
	      break;
	    }
	  else 
	    {
	      pos += rem;
	      *offsetp = 0.0;
	    }
	}
      else 
	{
	  double rem = dash_width  - *offsetp;
	  if (pos + rem > length) 
	    {
	      solid_arc(cent, rad, start_angle + pos/rad, end_angle, lt);
	      *offsetp += length - pos;
	      break;
	    }
	  else 
	    {
	      solid_arc(cent, rad, start_angle + pos/rad,
			start_angle + (pos + rem)/rad, lt);
	      pos += rem;
	      *offsetp = dash_width;
	    }
	}
    }
}

// Private, used by dashed_rounded_box.
void
common_output::dash_line(const position &start, const position &end, const line_type &lt, double dash_width, double gap_width, double *offsetp)
{
  distance dist = end - start;
  double length = hypot(dist);
  if (length == 0.0)
    return;
  double pos = 0.0;
  for (;;) 
    {
      if (*offsetp >= dash_width) 
	{
	  double rem = dash_width + gap_width - *offsetp;
	  if (pos + rem > length) 
	    {
	      *offsetp += length - pos;
	      break;
	    }
	  else 
	    {
	      pos += rem;
	      *offsetp = 0.0;
	    }
	}
      else 
	{
	  double rem = dash_width  - *offsetp;
	  if (pos + rem > length) 
	    {
	      line(start + dist*(pos/length), &end, 1, lt);
	      *offsetp += length - pos;
	      break;
	    }
	  else 
	    {
	      position p(start + dist*((pos + rem)/length));
	      line(start + dist*(pos/length), &p, 1, lt);
	      pos += rem;
	      *offsetp = dash_width;
	    }
	}
    }
}

// output a dotted rounded box as a series of dots
void
common_output::dotted_rounded_box(const position &cent, const distance &dim, double rad, const line_type &lt)
{
  line_type slt = lt;
  slt.type = line_type::solid;

  double hor_length = dim.x + (M_PI/2.0 - 2.0)*rad;
  int n_hor_dots = int(hor_length/lt.dash_width + .5);
  double hor_gap_width = (n_hor_dots != 0
			  ? hor_length/n_hor_dots
			  : lt.dash_width);

  double vert_length = dim.y + (M_PI/2.0 - 2.0)*rad;
  int n_vert_dots = int(vert_length/lt.dash_width + .5);
  double vert_gap_width = (n_vert_dots != 0
			   ? vert_length/n_vert_dots
			   : lt.dash_width);
  double epsilon = lt.dash_width/(rad*100.0);

  double offset = 0.0;
  dot_arc(cent + position(dim.x/2.0 - rad, -dim.y/2.0 + rad), rad,
	  -M_PI/4.0, 0, slt, vert_gap_width, &offset);
  dot_line(cent + position(dim.x/2.0, -dim.y/2.0 + rad),
	   cent + position(dim.x/2.0, dim.y/2.0 - rad),
	   slt, vert_gap_width, &offset);
  dot_arc(cent + position(dim.x/2.0 - rad, dim.y/2.0 - rad), rad,
	  0, M_PI/4.0 - epsilon, slt, vert_gap_width, &offset);

  offset = 0.0;
  dot_arc(cent + position(dim.x/2.0 - rad, dim.y/2.0 - rad), rad,
	  M_PI/4.0, M_PI/2, slt, hor_gap_width, &offset);
  dot_line(cent + position(dim.x/2.0 - rad, dim.y/2.0),
	   cent + position(-dim.x/2.0 + rad, dim.y/2.0),
	   slt, hor_gap_width, &offset);
  dot_arc(cent + position(-dim.x/2.0 + rad, dim.y/2.0 - rad), rad,
	  M_PI/2, 3*M_PI/4.0 - epsilon, slt, hor_gap_width, &offset);

  offset = 0.0;
  dot_arc(cent + position(-dim.x/2.0 + rad, dim.y/2.0 - rad), rad,
	  3.0*M_PI/4.0, M_PI, slt, vert_gap_width, &offset);
  dot_line(cent + position(-dim.x/2.0, dim.y/2.0 - rad),
	   cent + position(-dim.x/2.0, -dim.y/2.0 + rad),
	   slt, vert_gap_width, &offset);
  dot_arc(cent + position(-dim.x/2.0 + rad, -dim.y/2.0 + rad), rad,
	  M_PI, 5.0*M_PI/4.0 - epsilon, slt, vert_gap_width, &offset);

  offset = 0.0;
  dot_arc(cent + position(-dim.x/2.0 + rad, -dim.y/2.0 + rad), rad,
	  5*M_PI/4.0, 3*M_PI/2.0, slt, hor_gap_width, &offset);
  dot_line(cent + position(-dim.x/2.0 + rad, -dim.y/2.0),
	   cent + position(dim.x/2.0 - rad, -dim.y/2.0),
	   slt, hor_gap_width, &offset);
  dot_arc(cent + position(dim.x/2.0 - rad, -dim.y/2.0 + rad), rad,
	  3*M_PI/2, 7*M_PI/4 - epsilon, slt, hor_gap_width, &offset);
}

// Private, used by dotted_rounded_box.
void
common_output::dot_arc(const position &cent, double rad, double start_angle, double end_angle, const line_type &lt, double gap_width, double *offsetp)
{
  double length = (end_angle - start_angle)*rad;
  double pos = 0.0;
  for (;;) 
    {
      if (*offsetp == 0.0) 
	{
	  double ang = start_angle + pos/rad;
	  dot (cent + position(cos(ang), sin(ang))*rad, lt);
	}
      double rem = gap_width - *offsetp;
      if (pos + rem > length) 
	{
	  *offsetp += length - pos;
	  break;
	}
      else 
	{
	  pos += rem;
	  *offsetp = 0.0;
	}
    }
}

// Private, used by dotted_rounded_box.
void
common_output::dot_line(const position &start, const position &end, const line_type &lt, double gap_width, double *offsetp)
{
  distance dist = end - start;
  double length = hypot(dist);
  if (length == 0.0)
    return;
  double pos = 0.0;
  for (;;) 
    {
      if (*offsetp == 0.0)
	dot (start + dist*(pos/length), lt);
      double rem = gap_width - *offsetp;
      if (pos + rem > length) 
	{
	  *offsetp += length - pos;
	  break;
	}
      else 
	{
	  pos += rem;
	  *offsetp = 0.0;
	}
    }
}

// draw a solid rounded box, from arcs and lines
void
common_output::solid_rounded_box(const position &cent, const distance &dim, double rad, const line_type &lt)
{
  position tem = cent - dim/2.0;
  arc(tem + position(0.0, rad),
      tem + position(rad, rad),
      tem + position(rad, 0.0),
      lt);
  tem = cent + position(-dim.x/2.0, dim.y/2.0);
  arc(tem + position(rad, 0.0),
      tem + position(rad, -rad),
      tem + position(0.0, -rad),
      lt);
  tem = cent + dim/2.0;
  arc(tem + position(0.0, -rad),
      tem + position(-rad, -rad),
      tem + position(-rad, 0.0),
      lt);
  tem = cent + position(dim.x/2.0, -dim.y/2.0);
  arc(tem + position(-rad, 0.0),
      tem + position(-rad, rad),
      tem + position(0.0, rad),
      lt);
  position end;
  end = cent + position(-dim.x/2.0, dim.y/2.0 - rad);
  line(cent - dim/2.0 + position(0.0, rad), &end, 1, lt);
  end = cent + position(dim.x/2.0 - rad, dim.y/2.0);
  line(cent + position(-dim.x/2.0 + rad, dim.y/2.0), &end, 1, lt);
  end = cent + position(dim.x/2.0, -dim.y/2.0 + rad);
  line(cent + position(dim.x/2.0, dim.y/2.0 - rad), &end, 1, lt);
  end = cent + position(-dim.x/2.0 + rad, -dim.y/2.0);
  line(cent + position(dim.x/2.0 - rad, -dim.y/2.0), &end, 1, lt);
}

// Draw a filled rounded box, by drawing filled circles etc.
// Not used by libplot driver.
void
common_output::filled_rounded_box(const position &cent, const distance &dim, double rad, double fill)
{
  line_type ilt;
  ilt.type = line_type::invisible;
  circle(cent + position(dim.x/2.0 - rad, dim.y/2.0 - rad), rad, ilt, fill);
  circle(cent + position(-dim.x/2.0 + rad, dim.y/2.0 - rad), rad, ilt, fill);
  circle(cent + position(-dim.x/2.0 + rad, -dim.y/2.0 + rad), rad, ilt, fill);
  circle(cent + position(dim.x/2.0 - rad, -dim.y/2.0 + rad), rad, ilt, fill);
  position vec[4];
  vec[0] = cent + position(dim.x/2.0, dim.y/2.0 - rad);
  vec[1] = cent + position(-dim.x/2.0, dim.y/2.0 - rad);
  vec[2] = cent + position(-dim.x/2.0, -dim.y/2.0 + rad);
  vec[3] = cent + position(dim.x/2.0, -dim.y/2.0 + rad);
  polygon(vec, 4, ilt, fill);
  vec[0] = cent + position(dim.x/2.0 - rad, dim.y/2.0);
  vec[1] = cent + position(-dim.x/2.0 + rad, dim.y/2.0);
  vec[2] = cent + position(-dim.x/2.0 + rad, -dim.y/2.0);
  vec[3] = cent + position(dim.x/2.0 - rad, -dim.y/2.0);
  polygon(vec, 4, ilt, fill);
}
