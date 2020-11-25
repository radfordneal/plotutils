/* Initial values for all drawing state variables (see extern.h), when
   libplottek is initialized.  These variables include drawing attributes,
   and the state of any uncompleted polyline object (there is none).  This
   file is #include'd by savestate.c.
   
   Since libplottek supports only vector fonts, the default font below had
   better be a vector font. */

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
    TRUE,			/* raster device? */
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
  0,				/* fill level */
  "HersheySerif",		/* font name [must be valid pointer] */
  0.0,				/* font size in user coordinates (filled in in space.c) */
  F_STROKE,			/* font type */
  0,				/* typeface index (in fontdb.h typeface table; this is Hershey Serif)*/
  1,				/* font index (within typeface; this is Roman) */
  TRUE,				/* TRUE means an ISO-Latin-1 font */
  0.0,				/* degrees counterclockwise, for labels */
  {0, 0, 0},			/* foreground color */
  {0, 0, 0},			/* fill color */
  TRUE,				/* if not set, polyline reduces to points */
/* device-specific variables */
  MODE_PLOT,			/* must be one of MODE_PLOT or MODE_POINT */
  L_SOLID,			/* Tek's notion of line type, one of L_* */
  {0, 0},			/* Tek's notion of cursor position */
  ANSI_SYS_WHITE,		/* MS-DOS kermit's fg color */
  ANSI_SYS_BLACK,		/* MS-DOS kermit's bg color */
/* pointer to previous drawing state */
  (State *)NULL			/* pointer to previous state [must be null] */
};

