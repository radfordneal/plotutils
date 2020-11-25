/* _clean_iso_string() removes all characters not in the ISO-8859-?
   character sets from a string.  I.e. it removes control characters
   (characters in the range 0x01 to 0x1F, including LF and CR).  We take
   characters in the range 0x80 to 0x9F to be control characters also,
   since they are undefined in the ISO character sets.  

   Actually, in PS fonts (with ISO encoding vector) they encode accents;
   and in the encoding used in fig files, they encode a few special
   characters not found elsewhere.  But the interpretation of the
   0x80--0x9F range is device dependent, and our goal is device
   independence, so away the range goes. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#define GOOD_ISO(c) (((c >= 0x20) && (c <= 0x7E)) || ((c >= 0xA0) && (c <= 0xFF)))

Boolean
_clean_iso_string (s)
     unsigned char *s;
{
  Boolean was_clean = TRUE;
  unsigned char *t;
  
  for (t = s; *s; s++)
    {
      if (GOOD_ISO(*s))
	{
	  *t = *s;
	  t++;
	}
      else
	was_clean = FALSE;
    }
  *t = (unsigned char)'\0';
  
  return was_clean;
}
