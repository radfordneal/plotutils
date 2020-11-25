/* This C program, hershey-demo.c, when compiled and linked with GNU
   libplot, will emit to standard output a sample page illustrating many of
   the Hershey fonts.  The file will be in portable metafile format.  It
   may be translated to other formats, or viewed in an X window, with the
   `plot' utility.

   Compile and link with: 

   gcc -o hershey-demo hershey-demo.c -lplot -lXaw -lXmu -lXt -lXext -lX11 -lm

   (This assumes that libplot has been installed in a place where your
   linker will find it.  You may need to include such an option as
   -L/usr/local/lib to ensure this.  Also, if you are using Motif, the
   linking options may differ; and if compiling under SunOS 4.1.3, be sure
   to specify the `-static' option to work around problems with undefined
   symbols in the SunOS X libraries.  If you are using a C compiler other
   than gcc, consult your system administrator.  The -DNO_CONST_SUPPORT
   option may need to be used if the C compiler, like the SunOS 4.1.3 cc,
   is so old that it does not understand the `const' type qualifier.)

   To translate the sample page to Postscript, you would do 
   `hershey-demo | plot -T ps > page.ps'.  To display the page in an 
   X Window, you would do `hershey-demo | plot -T X', etc.  

   (If you have any trouble running the demo after compiling it, be sure
   your LD_LIBRARY_PATH environment variable includes the directory in
   which libplot is stored.  This applies on systems in which libplot is
   implemented as a DLL, i.e. a library that is linked at run time.)

   The sample page is taken from Allen Hershey's 1972 article in Computer
   Graphics and Image Processing (vol. 1, no. 4, pp. 373-385). */

#include <stdio.h> 
#include <plot.h>

#ifdef M_SQRT1_2
#undef M_SQRT1_2
#endif
#define M_SQRT1_2   (0.70710678118654752440) /* 1/sqrt(2) */

#define NUM_DEMO_WORDS 34
struct hershey_word 
{
  char *word;
  char *fontname;
  double m[6];
  char just;
};

#define CART (9.0/21.0)		/* `cartographic' size */
#define INDEXICAL (13.0/21.0)	/* `indexical' size */

#define LLX -3800.0
#define LLY -3450.0
#define URX 3800.0
#define URY 4150.0

#define BASE_FONTSIZE 220.0

struct hershey_word demo_word[NUM_DEMO_WORDS] = 
{
  {"Invitation", "HersheyScript-Bold", 
     { 1., 0., 0., 1., -3125., 3980. }, 'l' },
  {"ECONOMY", "HersheySans",           
     { 1., 0., 0., 1., -3125., 3340. }, 'l'},
  {"CARTOGRAPHY", "HersheySans",       
     { CART, 0., 0., CART, -3125., 2700. }, 'l'},
  {"Gramma", "HersheySerif-Symbol",    
     { 1., 0., 0., 1., -3125., 2060. }, 'l'},
  {"\347\322\301\306\311\313\301", "HersheyCyrillic",       
     { 1., 0., 0., 1., -3125., 1420. }, 'l'},

  {"COMMUNICATION", "HersheySans-Bold", 
     { 1., 0., 0., 1., 0., 3980. }, 'c'},
  {"VERSATILITY", "HersheySerif-Italic",
     { 1., 0., 0., 1., 0., 3340. }, 'c'},
  {"Standardization", "HersheySerif",   
     { 1., 0., 0., 1., 0., 2700. }, 'c'},
  {"Sumbolon", "HersheySerif-Symbol",  
     { INDEXICAL, 0., 0., INDEXICAL, 0., 2060. }, 'c'},
  {"\363\354\357\366\356\357\363\364\370", "HersheyCyrillic",      
     { 1., 0., 0., 1., 0., 1420. }, 'c'},

  {"Publication", "HersheyScript-Bold", 
     { 1., 0., 0., 1., 3125., 3980. }, 'r'},
  {"Quality", "HersheyGothic-English",  
     { 1., 0., 0., 1., 3125., 3340. }, 'r'},
  {"TYPOGRAPHY", "HersheySans",         
     { CART, 0., 0., CART, 3125., 2700. }, 'r'},
  {"AriJmo\\s-", "HersheySerif-Symbol",    
     { 1., 0., 0., 1., 3125., 2060. }, 'r'},
  {"\346\317\316\305\324\311\313\301", "HersheyCyrillic",       
     { 1., 0., 0., 1., 3125., 1420. }, 'r'},

  {"EXTENSION", "HersheySans", 
     { 17./7., 0., 0., 2./7., 0., 780. }, 'c'},
  {"CONDENSATION", "HersheySans", 
     { 5./7., 0., 0., 17./7., 0., -20. }, 'c'},
  {"Rotation", "HersheySans", 
     { M_SQRT1_2, M_SQRT1_2, -M_SQRT1_2, M_SQRT1_2, -2880., -20. }, 'l'},
  {"ROTATION", "HersheySans", 
     { M_SQRT1_2, -M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, 2880., -20. }, 'r'},

  {"Syllabary", "HersheySerif",           
     { 1., 0., 0., 1., -3125., -780. }, 'l'},
  {"Art", "HersheyGothic-English",        
     { 1., 0., 0., 1., -3125., -1420. }, 'l'},
  {"Meteorology", "HersheySerif-Italic",  
     { INDEXICAL, 0., 0., INDEXICAL, -3125., -2060.}, 'l'},
  {"CHEMISTRY", "HersheySerif",           
     { 1., 0., 0., 1., -3125., -2700.}, 'l'},
  {"Analysis", "HersheySerif-BoldItalic", 
     { 1., 0., 0., 1., -3125., -3340.}, 'l'},

  {"LEXIKON", "HersheySerif-Symbol",      
     { 1., 0., 0., 1., 0., -780.}, 'c'},
  {"\\#N3719\\#N4724", "HersheySerif",      
     { 1./.7, 0., 0., 1./.7, 0., -1420.}, 'c'},
  {"Wissenschaft", "HersheyGothic-German",
     { 1., 0., 0., 1., 0., -2060.}, 'c'},
  {"Electronics", "HersheySerif-Italic",  
     { 1., 0., 0., 1., 0., -2700.}, 'c'},
  {"COMPUTATION", "HersheySerif-Bold",    
     { 1., 0., 0., 1., 0., -3340.}, 'c'},

  {"Alphabet", "HersheySerif",            
     { 1., 0., 0., 1., 3125., -780.}, 'r'},
  {"Music", "HersheyGothic-Italian",      
     { 1., 0., 0., 1., 3125., -1420.}, 'r'},
  {"Astronomy", "HersheySerif",           
     { INDEXICAL, 0., 0., INDEXICAL, 3125., -2060.}, 'r'},
  {"MATHEMATICS", "HersheySerif",         
     { 1., 0., 0., 1., 3125., -2700.}, 'r'},
  {"Program", "HersheySerif-BoldItalic",  
     { 1., 0., 0., 1., 3125., -3340.}, 'r'},
};

int
main()
{
  int handle, i;

  parampl ("META_PORTABLE", "yes");

  handle = newpl ("meta", stdin, stdout, stderr);
  selectpl (handle);
  openpl();
  erase();
  fspace (LLX, LLY, URX, URY);

  for (i = 0; i < NUM_DEMO_WORDS; i++)
    {
      savestate();
      fontname (demo_word[i].fontname);
      fconcat (demo_word[i].m[0], demo_word[i].m[1], 
	       demo_word[i].m[2], demo_word[i].m[3],
	       demo_word[i].m[4], demo_word[i].m[5]);
      ffontsize (BASE_FONTSIZE);
      fmove (0.0, 0.0);
      alabel (demo_word[i].just, 'c', demo_word[i].word);
      restorestate();
    }

  closepl();
  return 0;
}
