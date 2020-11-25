typedef struct 
{
  int x;
  int y;
  int mask;
} miZeroArcPt;

typedef struct 
{
  int x, y, k1, k3, a, b, d, dx, dy;
  int alpha, beta;
  int xorg, yorg;		/* upper left corner */
  int xorgo, yorgo;
  unsigned int w, h;
  int initialMask;
  miZeroArcPt start, altstart, end, altend;
  int firstx, firsty;
  int startAngle, endAngle;	/* in 1/64 degrees */
} miZeroArc;

/* miZeroPolyArc() draws an arc only if it satisfies the following
   constraint (why?).  If it doesn't, miZeroPolyArc() hands it off to
   miPolyArc(). */

#define miCanZeroArc(arc) (((arc)->width == (arc)->height) || \
			   (((arc)->width <= 800) && ((arc)->height <= 800)))

#define MIARCSETUP() \
x = info.x; \
y = info.y; \
k1 = info.k1; \
k3 = info.k3; \
a = info.a; \
b = info.b; \
d = info.d; \
dx = info.dx; \
dy = info.dy

#define MIARCOCTANTSHIFT(clause) \
if (a < 0) \
{ \
    if (y == (int)info.h) \
      { \
	d = -1; \
	  a = b = k1 = 0; \
} \
  else \
    { \
      dx = (k1 << 1) - k3; \
	k1 = dx - k1; \
	  k3 = -k3; \
	    b = b + a - (k1 >> 1); \
	      d = b + ((-a) >> 1) - d + (k3 >> 3); \
		if (dx < 0) \
		  a = -((-dx) >> 1) - a; \
		    else \
		      a = (dx >> 1) - a; \
			dx = 0; \
			  dy = 1; \
			    clause \
} \
}

#define MIARCSTEP(move1,move2) \
b -= k1; \
if (d < 0) \
{ \
    x += dx; \
      y += dy; \
	a += k1; \
	  d += b; \
	    move1 \
} \
else \
{ \
    x++; \
      y++; \
	a += k3; \
	  d -= a; \
	    move2 \
}

#define MIARCCIRCLESTEP(clause) \
b -= k1; \
x++; \
if (d < 0) \
{ \
    a += k1; \
      d += b; \
} \
else \
{ \
    y++; \
      a += k3; \
	d -= a; \
	  clause \
}
