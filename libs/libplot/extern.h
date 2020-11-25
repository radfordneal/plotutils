/* Besides definitions, this file contains external declarations for the
   private variables and low-level functions of libplot.  We attempt to
   keep namespace clean by beginning the name of each variable appearing in
   an external declaration with an underscore. */

/* whether or not we're inside an openpl()...closepl() pair */

extern Boolean _grdevice_open;

/* the output stream for all plot commands */

extern FILE *_outstream;

/* structure used in colorname.c for storing names of colors we recognize */

typedef struct
{
  char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Colornameinfo;

/* This elides the argument prototypes if the compiler does not
   support them. The name protarg is chosen in hopes that it will not
   collide with any others. */
#if __STDC__
#define P__(a)	a
#else
#define P__(a)	()
#endif

extern Voidptr _plot_xcalloc P__ ((unsigned int nmemb, unsigned int size));
extern Voidptr _plot_xmalloc P__ ((unsigned int size));
extern Voidptr _plot_xrealloc P__ ((Voidptr p, unsigned int size));
extern void _putshort P__ ((int x, FILE *_stream)); 
extern void _putfloat P__ ((double x, FILE *_stream)); 

#undef P__
