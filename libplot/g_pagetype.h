/* This header file is #include'd by g_pagetype.c.  It is a database rather
   than a true header file: it includes known page sizes and their
   interpretation (used by various FigPlotter methods, PSPlotter methods,
   and HPGLPlotter methods).

   For each page size, the coordinates, in the device frame, of the left,
   right, bottom, and top of the "graphics display" (by convention a
   square) are specified here in terms of inches.  Margin sizes, for each
   page size, are the same as used by AutoCAD.

   There are different coordinates for each of the three output devices
   (PS, FIG, HP-GL[/2]).  For the PS output device, we choose the graphics
   display to be a square centered on the printed page and occupying the
   full width of the page (exclusive of margins). The origin for the PS
   coordinate system, in terms of which the coordinates below are
   expressed, is the lower left corner of the printed page.  For the FIG
   output device, we choose the graphics display to be a square of the same
   size, but positioned flush against the upper left corner of the xfig
   display (with margins taken into account).  The origin for the
   coordinate system is the upper left corner of the xfig display, and the
   flipped-y convention is used (y increases downward).

   For the HP-GL[/2] output device, we choose the width of the graphics
   display to be the same as for the PS output device.  That is, we take
   the size of the graphics display to be slightly smaller than the width
   of the page.  The origin for the HP-GL[/2] coordinate system is not the
   lower left corner of the printed page.  It is the lower left corner of
   the `hard-clip region', a proper subrectangle of the printed page.  The
   size and orientation of the hard-clip region differ from device to
   device.

   (There is a subtle difference between generic HP-GL and HP-GL/2: for
   HP-GL/2 devices the lower left corner of the hard-clip region is by
   default the same as the default location of the so-called `scaling
   point' P1, but for HP-GL devices it is not.)

   The fifth element below is significant only in the HP-GL[/2] and PS
   structures.  In the HP-GL/2 case the plot length (emitted in an HP-GL/2
   `PS' instruction, and of importance mostly for roll plotters) is stored
   there.  In the PS case the paper height is stored there. */

#define NUM_PAGESIZES 19

static const Pagedata _pagedata[NUM_PAGESIZES] =
{
  /* ANSI A, 8.5in x 11.0in ["letter"] */
  /* AutoCAD uses 8.0x10.5 */
  { "A", false,
      {0.25, 8.25, 8.25, 0.25, 0.0}, /* fig */
      {0.25, 8.25, 1.5, 9.5, 11.0}, /* ps */
      {0.0, 8.0, 0.0, 8.0, 10.5}, /* hp-gl */
  },
  /* ANSI B, 11.0in x 17.0in ["tabloid"] */
  /* AutoCAD uses 10.0x16.0 */
  { "B", false,
      {0.5, 10.5, 10.5, 0.5, 0.0}, /* fig */
      {0.5, 10.5, 3.5, 13.0, 17.0}, /* ps */
      {0.0, 10.0, 0.0, 10.0, 16.0}, /* hp-gl */
  },
  /* ANSI C, 17.0in x 22.0in */
  /* AutoCAD uses 16.0x21.0 */
  { "C", false,
      {0.5, 16.5, 16.5, 0.5, 0.0}, /* fig */
      {0.5, 16.5, 3.0, 19.0, 22.0}, /* ps */
      {0.0, 16.0, 0.0, 16.0, 21.0}, /* hp-gl */
  },
  /* ANSI D, 22.0in x 34.0in */
  /* AutoCAD uses 21.0x33.0 */
  { "D", false,
      {0.5, 21.5, 21.5, 0.5, 0.0}, /* fig */
      {0.5, 21.5, 6.5, 27.5, 34.0}, /* ps */
      {0.0, 21.0, 0.0, 21.0, 33.0}, /* hp-gl */
  },
  /* ANSI E, 34.0in x 44.0in */
  /* AutoCAD uses 33.0x43.0 */
  { "E", false,
      {0.5, 33.5, 33.5, 0.5, 0.0}, /* fig */
      {0.5, 33.5, 6.5, 38.5, 44.0}, /* ps */
      {0.0, 33.0, 0.0, 33.0, 43.0}, /* hp-gl */
  },

  /* aliases for some of the preceding ANSI sizes */
  /* letter = ANSI A */
  { "Letter", false,
      {0.25, 8.25, 8.25, 0.25, 0.0},	/* fig */
      {0.25, 8.25, 1.5, 9.5, 11.0},	/* ps */
      {0.0, 8.0, 0.0, 8.0, 10.5},	/* hp-gl */
  },
  { "USLetter", false,
      {0.25, 8.25, 8.25, 0.25, 0.0},	/* fig */
      {0.25, 8.25, 1.5, 9.5, 11.0},	/* ps */
      {0.0, 8.0, 0.0, 8.0, 10.5},	/* hp-gl */
  },
  /* legal, 8.5in x 14in; not an ANSI size ? */
  /* we use square size = 8.0in */
  { "Legal", false,
      {0.25, 8.25, 8.25, 0.25, 0.0}, /* fig */
      {0.25, 8.25, 3.0, 11.0, 14.0}, /* ps */
      {0.0, 8.0, 0.0, 8.0, 16.0}, /* hp-gl */
  },
  { "USLegal", false,
      {0.25, 8.25, 8.25, 0.25, 0.0}, /* fig */
      {0.25, 8.25, 3.0, 11.0, 14.0}, /* ps */
      {0.0, 8.0, 0.0, 8.0, 16.0}, /* hp-gl */
  },
  /* ledger, 17in x 11in (rotated ANSI B); not an ANSI size ? */
  /* we use square size = 10.0in */
  { "Ledger", false,
      {3.5, 13.5, 10.5, 0.5, 0.0},	/* fig */
      {3.5, 13.5, 0.5, 10.5, 11.0},	/* ps */
      {0.0, 10.0, 0.0, 10.0, 10.0},	/* hp-gl */
  },
  /* usledger = same as ledger */
  { "USLedger", false,
      {3.5, 13.5, 10.5, 0.5, 0.0},	/* fig */
      {3.5, 13.5, 0.5, 10.5, 11.0},	/* ps */
      {0.0, 10.0, 0.0, 10.0, 10.0},	/* hp-gl */
  },
  /* tabloid = ANSI B */
  { "Tabloid", false,
      {0.5, 10.5, 10.5, 0.5, 0.0}, /* fig */
      {0.5, 10.5, 3.5, 13.0, 17.0}, /* ps */
      {0.0, 10.0, 0.0, 10.0, 16.0}, /* hp-gl */
  },
  /* ustabloid = same as tabloid */
  { "USTabloid", false,
      {0.5, 10.5, 10.5, 0.5, 0.0}, /* fig */
      {0.5, 10.5, 3.5, 13.0, 17.0}, /* ps */
      {0.0, 10.0, 0.0, 10.0, 16.0}, /* hp-gl */
  },

  /* ISO page sizes */
  /* ISO A4, 21.0cm x 29.7 cm = 8.27 x 11.69 */
  /* AutoCAD uses 7.8x11.2 */
  { "A4", true,
      {0.235, 8.035, 8.035, 0.235, 0.0}, /* fig */
      {0.235, 8.035, 1.945, 9.745, 11.69}, /* ps */
      {0.0, 7.8, 0.0, 7.8, 11.2}, /* hp-gl */ 
  },
  /* ISO A3, 29.7cm x 42.0 cm  = 11.69 x 16.54 */
  /* AutoCAD uses 10.7x15.6 */
  { "A3", true,
      {0.495, 11.195, 11.195, 0.495, 0.0}, /* fig */
      {0.495, 11.195, 2.92, 13.62, 16.54}, /* ps */
      {0.0, 10.7, 0.0, 10.7, 15.6}, /* hp-gl */
  },
  /* ISO A2, 42.0cm x 59.4 cm = 16.54 x 23.39 */
  /* AutoCAD uses 15.6x22.4 */
  { "A2", true,
      {0.47, 16.07, 16.07, 0.47, 0.0}, /* fig */
      {0.47, 16.07, 3.895, 19.495, 23.39}, /* ps */
      {0.0, 15.6, 0.0, 15.6, 22.4}, /* hp-gl */
  },
  /* ISO A1, 59.4cm x 84.1 cm = 23.39 x 33.11 */
  /* AutoCAD uses 22.4x32.2 */
  { "A1", true,
      {0.495, 22.895, 22.895, 0.495, 0.0}, /* fig */
      {0.495, 22.895, 5.335, 27.735, 33.11}, /* ps */
      {0.0, 22.4, 0.0, 22.4, 32.2}, /* hp-gl */
  },
  /* ISO A0, 84.1cm x 118.9 cm = 33.11 x 46.81 */
  /* AutoCAD uses 32.2x45.9 */
  { "A0", true,
      {0.435, 32.635, 32.635, 0.435, 0.0}, /* fig */
      {0.435, 32.635, 7.285, 39.485, 46.81}, /* ps */
      {0.0, 32.2, 0.0, 32.2, 45.9}, /* hp-gl */ 
  },

  /* Japanese page sizes */
  /* ISO B5, 18.2cm x 25.7 cm = 7.17 x 10.12 */
  /* AutoCAD presumably uses 6.67x9.62 */
  { "B5", true,
      {0.25, 6.92, 6.92, 0.25, 0.0}, /* fig */
      {0.25, 6.92, 1.725, 8.395, 10.12}, /* ps */
      {0.0, 6.67, 0.0, 6.67, 9.62}, /* hp-gl */ 
  }
};
