/* This file defines GNU libplot's builtin line styles.  A line style is
   specified by invoking the linemod() operation.  The supported line
   styles are a superset of the line styles of traditional (Unix) libplot.

   Unix libplot originated at Bell Labs in the early 1970's, and the first
   supported display device was a Tektronix 611 storage scope.  The libplot
   API did not originally include linemod(), as the Unix Version 5 manual
   makes clear.  That is because the Tektronix 611 did not have any
   predefined set of line styles.  linemod() was added to the API slightly
   later, when it was extended to support the Tektronix 4010/4014 storage
   scope.  The 4010/4014 provided hardware support for the five line styles
   "solid" through "longdashed".

   GNU libplot supports the traditional five, and also two additional line
   styles, "dotdotdashed" and "dotdotdotdashed".  Each non-solid style is
   defined as a dash pattern, with the length of each dash (drawn or not
   drawn) being an integer multiple of the line width.  This `scaling by
   line width' applies for sufficiently wide lines, at least.

   GNU libplot also supports a special "disconnected" line style (if a path
   is disconnected, it's drawn as a sequence of filled circles, one at each
   of the path join points). */

#include "sys-defines.h"
#include "extern.h"

/* An array of dashes for each line style (dashes are cylically used,
   on/off/on/off...).  Styles must appear in a special order: it must agree
   with our internal numbering, i.e. must agree with the definitions of
   L_{SOLID,DOTTED,DOTDASHED,SHORTDASHED,LONGDASHED,DOTDOTDASHED etc.} in
   extern.h, which are 0,1,2,3,4,5 etc. respectively. */

const plLineStyle _line_styles[NUM_LINE_STYLES] =
/* Dash arrays for "dotted" through "longdashed" below are those used by
   the Tektronix emulator in xterm(1), except the emulator seems
   incorrectly to have on and off interchanged. */
{
  { "solid", 		L_SOLID, 		0, {0} 		}, /* dummy */
  { "dotted", 		L_DOTTED, 		2, {1, 3} 	},
  { "dotdashed", 	L_DOTDASHED, 		4, {4, 3, 1, 3} },
  { "shortdashed", 	L_SHORTDASHED, 		2, {4, 4} 	},
  { "longdashed", 	L_LONGDASHED, 		2, {7, 4} 	},
  { "dotdotdashed", 	L_DOTDOTDASHED,		6, {4, 3, 1, 3, 1, 3} },
  { "dotdotdotdashed", 	L_DOTDOTDOTDASHED,	8, {4, 3, 1, 3, 1, 3, 1, 3} }
};

/* N.B. `ps4014', the Tektronix->PS translator in Adobe's Transcript
   package, uses { 1, 2 }, { 8, 2, 1, 2 }, { 2, 2 }, { 12, 2 } for
   "dotted" through "longdashed", instead. */

/* N.B. a genuine Tektronix 4014 (with Enhanced Graphics Module) uses 
   { 1, 1 }, { 5, 1, 1, 1 }, { 3, 1 }, { 6, 2 } for "dotted"
   through "longdashed", instead.  See the Tektronix 4014 Service
   Instruction Manual (dated Aug. 1974) for the diode array that produces
   these patterns. */
