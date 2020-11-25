/* This file defines libplot's builtin line styles, which may be selected
   by invoking the linemod() operation.  Five of the styles defined here
   ("solid" through "longdashed") originated with the Tektronix 4010/4014
   in the early '70s.  According to the Unix Version 5 manual, they were
   not present on the Tektronix 611, which was the first display device
   that Unix libplot supported.  But they were added to Unix libplot later.
   The final two were recently added, in GNU plotutils-2.2.

   We define each non-solid style as a dash pattern, with the length of
   each dash (drawn or not drawn) being an integer multiple of the line
   width.  This `scaling by line width' applies for sufficiently wide
   lines, at least.

   There is also a special "disconnected" line style (if a path is
   disconnected, it's drawn as a sequence of filled circles, one at each of
   the path join points). */

#include "sys-defines.h"
#include "extern.h"

/* An array of dashes for each line style (dashes are cylically used,
   on/off/on/off...).  Styles must appear in a special order: it must agree
   with our internal numbering, i.e. must agree with the definitions of
   L_{SOLID,DOTTED,DOTDASHED,SHORTDASHED,LONGDASHED,DOTDOTDASHED etc.} in
   extern.h, which are 0,1,2,3,4,5 etc. respectively. */

const LineStyle _line_styles[NUM_LINE_STYLES] =
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
