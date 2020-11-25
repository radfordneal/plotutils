/* Special definitions for GNU plotutils, processed by autoheader.
   Copyright (C) 1997-2000 Free Software Foundation, Inc.
   Robert S. Maier <rsm@math.arizona.edu>, 1997-2000.
*/

/* Define to be the name of the distribution. */
#undef PACKAGE

/* Define to be the version of the distribution.  */
#undef VERSION

/* Define to signal compiler support for void and void * types.  */
#define HAVE_VOID 1

/* Define (if compiling as C++) to signal compiler support for bool type.  */
#undef HAVE_BOOL

/* Define under HP/UX to get matherr-related things from math.h. */
#undef _HPUX_SOURCE

/* Define to signal that fflush(NULL) works: it flushes all outstreams. */
#undef HAVE_NULL_FLUSH

/* Redundant and unused: Define to signal that fflush(NULL) doesn't work. */
#undef DONT_HAVE_NULL_FLUSH

/* Define to signal that libpng is available.  */
#undef HAVE_LIBPNG

/* Define to request PNG support (requires both HAVE_PNG_H and HAVE_LIBPNG). */
#undef INCLUDE_PNG_SUPPORT

/* Define to signal that Motif should be used, if X11 is used.  */
#undef USE_MOTIF

/* Define to signal support in libc for the pthread threads library.  */
#undef PTHREAD_SUPPORT

/* Define if we have the thread-safe library function ctime_r().  */
#undef HAVE_CTIME_R

/* Define if we have the thread-safe library function localtime_r().  */
#undef HAVE_LOCALTIME_R

/* Define to signal support in libXt/libX11 for multithreading.  */
#undef X_THREAD_SUPPORT

/* Define to signal support in -lXext for the DBE X11 protocol extension.  */
#undef HAVE_DBE_SUPPORT

/* Define to signal support in -lXext for the MBX X11 protocol extension.  */
#undef HAVE_MBX_SUPPORT

/* Define to enable support for the 45 LaserJet fonts in X output.  */
#undef USE_LJ_FONTS_IN_X

/* Define to enable support for the 45 LaserJet fonts in PS output.  */
#undef USE_LJ_FONTS_IN_PS

/* Define to enable support for the 35 PS fonts in PCL output.  */
#undef USE_PS_FONTS_IN_PCL
