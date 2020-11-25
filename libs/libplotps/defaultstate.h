/* Initial values for all drawing state variables (see extern.h), when
   libplotps is initialized.  These variables include drawing attributes,
   and the state of any uncompleted polyline object (there is none).  This
   file is #include'd by savestate.c. */

/* Two state variables (font size and line width in user coordinates) play
   an important role at later times, e.g. an invalid font size resets the
   font size to the default.  For that reason, those variables are filled
   in, in space.c, when space() is called.  They are computed from the two
   quantities DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_WIDTH and
   DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_WIDTH (defined in
   extern.h). */

State _default_drawstate = {
/* affine transformation from user coordinates to device coordinates */
  {
    {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, /* PS-style transformation matrix */
    TRUE,			/* transformation scaling is uniform? */
    TRUE,			/* transf. preserves axis directions? */
    FALSE,			/* raster device? */
  },
/* graphics cursor position */
  {0.0, 0.0},			/* graphics cursor position */
/* the state of any uncompleted polyline object */
  (Point *)NULL,		/* array of accumulated points [here NULL]*/
  0,				/* number of accumulated points */
  0,				/* length of point storage buffer (bytes) */
/* modal drawing attributes */
  "solid",			/* line mode [must be valid] */
  "miter",			/* join mode [must be valid] */
  "butt",			/* cap mode [must be valid] */
  0.0,				/* line width in user coors (filled in in space.c) */
  0,				/* fill level (0 = transparent, no fill) */
  "helvetica",			/* font name [must be valid pointer] */
  0.0,				/* font size in user coordinates (filled in in space.c) */
  F_STANDARD,			/* font type */
  0,				/* typeface index (in fontdb.h typeface table; this is Helvetica)*/
  1,				/* font index (within typeface; this is the Helvetica font)*/
  TRUE,				/* TRUE means an ISO-Latin-1 font */
  0.0,				/* degrees counterclockwise, for labels */
  {0, 0, 0},			/* foreground color (= black) */
  {0, 0, 0},			/* fill color (= black) */
  TRUE,				/* if not set, polyline reduces to points */
/* device-specific variables */
  L_SOLID,			/* PS line type (one of L_*) */
  0.0,				/* RGB for PS fg color (floats) */
  0.0,
  0.0,
  1.0,				/* RGB for PS fill color (floats) */
  1.0,
  1.0,
  TRUE,				/* whether PS fg color has been computed */
  TRUE,				/* whether PS fill color has been computed */
  0,				/* idraw fg color (0=black, 9=white) */
  9,				/* idraw bg color (0=black, 9=white) */
  0.0,				/* shading (0.0=fg, 1.0=bg), if not transparent */
/* pointer to previous drawing state */
  (State *)NULL			/* pointer to previous state [must be null] */
};
