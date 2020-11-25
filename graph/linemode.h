/* This file is included by write_plot.c.  It defines the mapping from
   `virtual' linemodes (the linemodes reported by the point-reader) to
   physical linemodes.  On monochrome displays, a physical linemode is
   simply a linemode, in the traditional libplot sense.  On color displays,
   it involves a choice of color as well; see explanation at head of
   plotter.c.  NO_OF_LINEMODES is defined in extern.h. */

const char *linemodes[NO_OF_LINEMODES] =
{
  "solid", "dotted", "dotdashed", "shortdashed", "longdashed", 
};

/* following colors are the first five used by the gnuplot X11 driver */

const char *colorstyle[NO_OF_LINEMODES] =
{
  "red", "green", "blue", "magenta", "cyan", 
};
