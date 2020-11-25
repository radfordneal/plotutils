/* A Spans structure is effectively a list of ranges [xmin,xmax], sorted
   by the corresponding values of y.  There may be more than one span
   at a given y. */

typedef struct 
{
  int         count;		/* number of spans		    */
  miIntPoint  *points;		/* pointer to list of start points  */
  unsigned int *widths;		/* pointer to list of widths	    */
} Spans;

/* A SpanGroup is an unsorted list of Spans's. */

typedef struct 
{
    int		size;		/* total number of *Spans allocated	*/
    int		count;		/* Number of *Spans actually in group   */
    Spans       *group;		/* List of Spans			*/
    int		ymin, ymax;	/* Min, max y values over all Spans's	*/
} SpanGroup;

/* internal Spans manipulation routines */

extern void miInitSpanGroup ____P((SpanGroup *spanGroup));
extern void miFreeSpanGroup ____P((SpanGroup *spanGroup));
extern void miAppendSpans ____P((SpanGroup *spanGroup, SpanGroup *otherGroup, const Spans *spans));
extern void miFillUniqueSpanGroup ____P((miGC *pGC, SpanGroup *spanGroup));
extern void miFillSpanGroup ____P((miGC *pGC, SpanGroup *spanGroup));
