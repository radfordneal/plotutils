/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   For AIPlotter objects, we first output the Illustrator header.  We then
   output all objects, which we have saved in a resizable outbuf structure
   for the current page.  Finally we fflush the _plotter->outstream and
   reset all datastructures.

   All this applies only if this is page #1, since an Illustrator file may
   contain no more than a single page of graphics.  Later pages are simply
   deallocated. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

#ifndef HAVE_UNISTD_H
#include <sys/types.h>		/* if unistd.h found, included with it */
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>		/* for time() */
#include <time.h>		/* for ctime() */
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

static const char *_ai_symbol_reencoding = "32/space\n/exclam\n/universal\n/numbersign\n/existential\n/percent\n/ampersand\n/suchthat\n/parenleft\n/parenright\n/asteriskmath\n/plus\n/comma\n/minus\n/period\n/slash\n/zero\n/one\n/two\n/three\n/four\n/five\n/six\n/seven\n/eight\n/nine\n/colon\n/semicolon\n/less\n/equal\n/greater\n/question\n/congruent\n/Alpha\n/Beta\n/Chi\n/Delta\n/Epsilon\n/Phi\n/Gamma\n/Eta\n/Iota\n/theta1\n/Kappa\n/Lambda\n/Mu\n/Nu\n/Omicron\n/Pi\n/Theta\n/Rho\n/Sigma\n/Tau\n/Upsilon\n/sigma1\n/Omega\n/Xi\n/Psi\n/Zeta\n/bracketleft\n/therefore\n/bracketright\n/perpendicular\n/underscore\n/radicalex\n/alpha\n/beta\n/chi\n/delta\n/epsilon\n/phi\n/gamma\n/eta\n/iota\n/phi1\n/kappa\n/lambda\n/mu\n/nu\n/omicron\n/pi\n/theta\n/rho\n/sigma\n/tau\n/upsilon\n/omega1\n/omega\n/xi\n/psi\n/zeta\n/braceleft\n/bar\n/braceright\n/similar\n161/Upsilon1\n/minute\n/lessequal\n/fraction\n/infinity\n/florin\n/club\n/diamond\n/heart\n/spade\n/arrowboth\n/arrowleft\n/arrowup\n/arrowright\n/arrowdown\n/degree\n/plusminus\n/second\n/greaterequal\n/multiply\n/proportional\n/partialdiff\n/bullet\n/divide\n/notequal\n/equivalence\n/approxequal\n/ellipsis\n/arrowvertex\n/arrowhorizex\n/carriagereturn\n/aleph\n/Ifraktur\n/Rfraktur\n/weierstrass\n/circlemultiply\n/circleplus\n/emptyset\n/intersection\n/union\n/propersuperset\n/reflexsuperset\n/notsubset\n/propersubset\n/reflexsubset\n/element\n/notelement\n/angle\n/gradient\n/registerserif\n/copyrightserif\n/trademarkserif\n/product\n/radical\n/dotmath\n/logicalnot\n/logicaland\n/logicalor\n/arrowdblboth\n/arrowdblleft\n/arrowdblup\n/arrowdblright\n/arrowdbldown\n/lozenge\n/angleleft\n/registersans\n/copyrightsans\n/trademarksans\n/summation\n/parenlefttp\n/parenleftex\n/parenleftbt\n/bracketlefttp\n/bracketleftex\n/bracketleftbt\n/bracelefttp\n/braceleftmid\n/braceleftbt\n/braceex\n241/angleright\n/integral\n/integraltp\n/integralex\n/integralbt\n/parenrighttp\n/parenrightex\n/parenrightbt\n/bracketrighttp\n/bracketrightex\n/bracketrightbt\n/bracerighttp\n/bracerightmid\n/bracerightbt\n";

static const char *_ai_zapf_dingbats_reencoding = "32/space\n/a1\n/a2\n/a202\n/a3\n/a4\n/a5\n/a119\n/a118\n/a117\n/a11\n/a12\n/a13\n/a14\n/a15\n/a16\n/a105\n/a17\n/a18\n/a19\n/a20\n/a21\n/a22\n/a23\n/a24\n/a25\n/a26\n/a27\n/a28\n/a6\n/a7\n/a8\n/a9\n/a10\n/a29\n/a30\n/a31\n/a32\n/a33\n/a34\n/a35\n/a36\n/a37\n/a38\n/a39\n/a40\n/a41\n/a42\n/a43\n/a44\n/a45\n/a46\n/a47\n/a48\n/a49\n/a50\n/a51\n/a52\n/a53\n/a54\n/a55\n/a56\n/a57\n/a58\n/a59\n/a60\n/a61\n/a62\n/a63\n/a64\n/a65\n/a66\n/a67\n/a68\n/a69\n/a70\n/a71\n/a72\n/a73\n/a74\n/a203\n/a75\n/a204\n/a76\n/a77\n/a78\n/a79\n/a81\n/a82\n/a83\n/a84\n/a97\n/a98\n/a99\n/a100\n160/space\n/a101\n/a102\n/a103\n/a104\n/a106\n/a107\n/a108\n/a112\n/a111\n/a110\n/a109\n/a120\n/a121\n/a122\n/a123\n/a124\n/a125\n/a126\n/a127\n/a128\n/a129\n/a130\n/a131\n/a132\n/a133\n/a134\n/a135\n/a136\n/a137\n/a138\n/a139\n/a140\n/a141\n/a142\n/a143\n/a144\n/a145\n/a146\n/a147\n/a148\n/a149\n/a150\n/a151\n/a152\n/a153\n/a154\n/a155\n/a156\n/a157\n/a158\n/a159\n/a160\n/a161\n/a163\n/a164\n/a196\n/a165\n/a192\n/a166\n/a167\n/a168\n/a169\n/a170\n/a171\n/a172\n/a173\n/a162\n/a174\n/a175\n/a176\n/a177\n/a178\n/a179\n/a193\n/a180\n/a199\n/a181\n/a200\n/a182\n241/a201\n/a183\n/a184\n/a197\n/a185\n/a194\n/a198\n/a186\n/a195\n/a187\n/a188\n/a189\n/a190\n/a191\n";

int
#ifdef _HAVE_PROTOS
_a_closepl (void)
#else
_a_closepl ()
#endif
{
  bool fonts_used = false;
  int i;

  if (!_plotter->open)
    {
      _plotter->error ("closepl: invalid operation");
      return -1;
    }

  _plotter->endpath (); /* flush polyline if any */

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_plotter->restorestate();
    }
  
  /* Output the page, but only if it's page #1 (currently AI format
     supports only one page of graphics output per file). */
  if (_plotter->page_number == 1 && _plotter->outstream)
    {
      char *time_s;
      double xmin, xmax, ymin, ymax;
      double xmid, ymid;
      int ixmid, iymid;
      time_t clock;

      /* compute center of graphics display in device coors (i.e. points) */
      xmid = 72 * (0.5 * (_plotter->display_coors.left 
			  + _plotter->display_coors.right));
      ymid = 72 * (0.5 * (_plotter->display_coors.bottom
			  + _plotter->display_coors.top));
      ixmid = IROUND(xmid);
      iymid = IROUND(ymid);      

      /* emit first few comment lines */
      fprintf (_plotter->outstream, "\
%%!PS-Adobe-3.0\n\
%%%%Creator: GNU libplot drawing library %s\n\
%%%%For: (Unknown) (Unknown)\n\
%%%%Title: (Untitled)\n", 
	       LIBPLOT_VERSION);

      /* emit creation date and time, if possible */
      time(&clock);
      time_s = ctime(&clock);
      if (time_s != NULL)
	{
	  char weekday[32], month[32], day[32], hour_min_sec[32], year[32];
	  int retval;

	  retval = sscanf (time_s, "%s %s %s %s %s",
			   weekday, month, day, hour_min_sec, year);
	  if (retval == 5)
	    fprintf (_plotter->outstream, "\
%%%%CreationDate: (%s %s %s) (%s)\n",
		     day, month, year, hour_min_sec);
	}

      /* emit bounding box for the page */
      _bbox_of_outbuf (_plotter->page, &xmin, &xmax, &ymin, &ymax);
      if (xmin > xmax || ymin > ymax) /* no objects */
	/* place degenerate box at center of page */
	fprintf (_plotter->outstream, "\
%%%%BoundingBox: %d %d %d %d\n",
		 ixmid, iymid, ixmid, iymid);
      else
	/* emit true bounding box */
	fprintf (_plotter->outstream, "\
%%%%BoundingBox: %d %d %d %d\n",
		 IROUND(xmin - 0.5), IROUND(ymin - 0.5),
		 IROUND(xmax + 0.5), IROUND(ymax + 0.5));
      if (_plotter->ai_version >= AI_VERSION_5)
	/* emit hi-res bounding box too */
	{
	  if (xmin > xmax || ymin > ymax) /* empty page */
	    /* place degenerate box at center of page */
	    fprintf (_plotter->outstream, "\
%%%%HiResBoundingBox: %.4f %.4f %.4f %.4f\n",
		     xmid, ymid, xmid, ymid);
	  else
	    /* emit true bounding box */
	    fprintf (_plotter->outstream, "\
%%%%HiResBoundingBox: %.4f %.4f %.4f %.4f\n",
		     xmin, ymin, xmax, ymax);
	}

      /* emit process colors used */
      fprintf (_plotter->outstream, "\
%%%%DocumentProcessColors:");
      if (_plotter->ai_cyan_used)
	fprintf (_plotter->outstream, " Cyan");
      if (_plotter->ai_magenta_used)
	fprintf (_plotter->outstream, " Magenta");
      if (_plotter->ai_yellow_used)
	fprintf (_plotter->outstream, " Yellow");
      if (_plotter->ai_black_used)
	fprintf (_plotter->outstream, " Black");
      fprintf (_plotter->outstream, "\n");
      
      /* tell AI to include any PS [or PCL] fonts that are needed */
      fprintf (_plotter->outstream, "\
%%%%DocumentFonts: ");
      for (i = 0; i < NUM_PS_FONTS; i++)
	if (_plotter->page->ps_font_used[i])
	  {
	    if (fonts_used)	/* not first font */
	      fprintf (_plotter->outstream, 
		       "%%%%+ %s\n", _ps_font_info[i].ps_name);
	    else		/* first font */
	      fprintf (_plotter->outstream, 
		       "%s\n", _ps_font_info[i].ps_name);
	    fonts_used = true;
	  }
      for (i = 0; i < NUM_PCL_FONTS; i++)
	if (_plotter->page->pcl_font_used[i])
	  {
	    if (fonts_used)	/* not first font */
	      fprintf (_plotter->outstream, 
		       "%%%%+ %s\n", _pcl_font_info[i].ps_name);
	    else		/* first font */
	      fprintf (_plotter->outstream, 
		       "%s\n", _pcl_font_info[i].ps_name);
	    fonts_used = true;
	  }
      if (!fonts_used)
	fprintf (_plotter->outstream, "\n");

      /* tell AI or print spooler that we need procsets */
      if (_plotter->ai_version == AI_VERSION_5)
	{
	  fprintf (_plotter->outstream, "\
%%%%DocumentNeededResources: procset Adobe_level2_AI5 1.0 0\n\
%%%%+ procset Adobe_typography_AI5 1.0 0\n\
%%%%+ procset Adobe_Illustrator_AI6_vars Adobe_Illustrator_AI6\n\
%%%%+ procset Adobe_Illustrator_AI5 1.0 0\n");
	  fprintf (_plotter->outstream, "\
%%AI5_FileFormat 3\n");
	}
      else			/* AI_VERSION_3 */
	{
	  fprintf (_plotter->outstream, "\
%%%%DocumentNeededResources: procset Adobe_packedarray 2.0 0\n\
%%%%+ procset Adobe_cmykcolor 1.1 0\n\
%%%%+ procset Adobe_cshow 1.1 0\n\
%%%%+ procset Adobe_customcolor 1.0 0\n\
%%%%+ procset Adobe_typography_AI3 1.0 1\n\
%%%%+ procset Adobe_pattern_AI3 1.0 0\n\
%%%%+ procset Adobe_Illustrator_AI3 1.0 1\n");
	}

      /* tell AI whether or not we're monochrome */
      fprintf (_plotter->outstream, "\
%%AI3_ColorUsage: ");
      if (_plotter->ai_cyan_used || _plotter->ai_magenta_used || _plotter->ai_yellow_used)
	fprintf (_plotter->outstream, "Color\n");
      else
	fprintf (_plotter->outstream, "Black&White\n");

      /* place degenerate template box at center of graphics display, for
	 centering */
      fprintf (_plotter->outstream, "\
%%AI3_TemplateBox: %d %d %d %d\n",
	       ixmid, iymid, ixmid, iymid);

      /* specify nominal imageable area of the page */
      /* (we use our horizontal range, and the full page height) */
      fprintf (_plotter->outstream, "\
%%AI3_TileBox: %d %d %d %d\n",
	       IROUND(72 * _plotter->display_coors.left),
	       0,
	       IROUND(72 * _plotter->display_coors.right),
	       IROUND(72 * _plotter->display_coors.extra));
	       
      /* misc comment lines, probably default values in value fields */
      fprintf (_plotter->outstream, "\
%%AI3_DocumentPreview: None\n");
      if (_plotter->ai_version >= AI_VERSION_5)
	{
	  fprintf (_plotter->outstream, "\
%%AI5_ArtSize: %d %d\n\
%%AI5_RulerUnits: %d\n\
%%no_good_AI5_ArtFlags: 1 0 0 1 0 0 1 1 0\n\
%%AI5_TargetResolution: 800\n\
%%AI5_NumLayers: 1\n\
%%AI5_OpenViewLayers: 7\n",
		   /* For `ArtSize' we specify entire physical page.
		      We assume libplot graphics display is horizontally
		      centered on physical page, so can get page width by
		      adding left and right display edges. */
		   IROUND(72 * (_plotter->display_coors.left
				+ _plotter->display_coors.right)),
		   /* page height */
		   IROUND(72 * _plotter->display_coors.extra),
		   /* label AI's rulers with centimeters or inches */
		   _plotter->use_metric ? 4 : 0);
	}
      
      /* following three may be used only by old Macintosh versions of AI? */
      fprintf (_plotter->outstream, "\
%%%%PageOrigin:%d %d\n",
	       IROUND(72 * _plotter->display_coors.left), 0);
      fprintf (_plotter->outstream, "\
%%%%AI3_PaperRect:%d %d %d %d\n",
	       -IROUND(72 * _plotter->display_coors.left),
	       IROUND(72 * _plotter->display_coors.extra),
	       IROUND(72 * _plotter->display_coors.right),
	       0);
      fprintf (_plotter->outstream, "\
%%%%AI3_Margin:%d %d %d %d\n",
	       IROUND(72 * _plotter->display_coors.left),
	       0,
	       -IROUND(72 * _plotter->display_coors.left),
	       0);

      if (_plotter->use_metric)
	/* visible grid spacing = 1 cm, 3 subdivisions / division */
	fprintf (_plotter->outstream, "\
%%AI7_GridSettings: %.4f 3 %.4f 3 1 0 0.8 0.8 0.8 0.9 0.9 0.9\n",
		 72.0/2.54, 72.0/2.54);
      else
	/* visible grid spacing = 1 in, 8 subdivisions / division */
	fprintf (_plotter->outstream, "\
%%AI7_GridSettings: 72 8 72 8 1 0 0.8 0.8 0.8 0.9 0.9 0.9\n");
      
      fprintf (_plotter->outstream, "\
%%%%EndComments\n");

      /* header emitted, so fflush it */
      fflush(_plotter->outstream);
      
      /* Prolog section: include the procsets */
      if (_plotter->ai_version == AI_VERSION_5)
	{
	  fprintf (_plotter->outstream, "\
%%%%BeginProlog\n\
%%%%IncludeResource: procset Adobe_level2_AI5 1.0 0\n\
%%%%IncludeResource: procset Adobe_typography_AI5 1.0 0\n\
%%%%IncludeResource: procset Adobe_Illustrator_AI6_vars Adobe_Illustrator_AI6\n\
%%%%IncludeResource: procset Adobe_Illustrator_AI5 1.0 0\n\
%%%%EndProlog\n");
	}
      else			/* AI_VERSION_3 */
	{
	  fprintf (_plotter->outstream, "\
%%%%BeginProlog\n\
%%%%IncludeResource: procset Adobe_packedarray 2.0 0\n\
Adobe_packedarray /initialize get exec\n\
%%%%IncludeResource: procset Adobe_cmykcolor 1.1 0\n\
%%%%IncludeResource: procset Adobe_cshow 1.1 0\n\
%%%%IncludeResource: procset Adobe_customcolor 1.0 0\n\
%%%%IncludeResource: procset Adobe_typography_AI3 1.0 1\n\
%%%%IncludeResource: procset Adobe_pattern_AI3 1.0 0\n\
%%%%IncludeResource: procset Adobe_Illustrator_AI3 1.0 1\n\
%%%%EndProlog\n");
	}
      
      /* beginning of Setup section */
      fprintf (_plotter->outstream, "\
%%%%BeginSetup\n");

      /* include fonts if any */
      if (fonts_used)
	{
	  for (i = 0; i < NUM_PS_FONTS; i++)
	    if (_plotter->page->ps_font_used[i])
	      fprintf (_plotter->outstream, "\
%%%%IncludeFont: %s\n", 
		       _ps_font_info[i].ps_name);
	  for (i = 0; i < NUM_PCL_FONTS; i++)
	    if (_plotter->page->pcl_font_used[i])
	      fprintf (_plotter->outstream, "\
%%%%IncludeFont: %s\n", 
		       _pcl_font_info[i].ps_name);
	}

      /* do setup of procsets */
      if (_plotter->ai_version == AI_VERSION_5)
	{
	  fprintf (_plotter->outstream, "\
Adobe_level2_AI5 /initialize get exec\n\
Adobe_Illustrator_AI5_vars Adobe_Illustrator_AI5 Adobe_typography_AI5 /initialize get exec\n\
Adobe_ColorImage_AI6 /initialize get exec\n\
Adobe_Illustrator_AI5 /initialize get exec\n");
	}
      else			/* AI_VERSION_3 */
	{
	  fprintf (_plotter->outstream, "\
Adobe_cmykcolor /initialize get exec\n\
Adobe_cshow /initialize get exec\n\
Adobe_customcolor /initialize get exec\n\
Adobe_typography_AI3 /initialize get exec\n\
Adobe_pattern_AI3 /initialize get exec\n\
Adobe_Illustrator_AI3 /initialize get exec\n");
	}

      if (fonts_used)
	/* do whatever font reencodings are needed */
	{
	  /* don't modify StandardEncoding */
	  fprintf (_plotter->outstream, "\
[\n\
TE\n");
          /* reencode each used font */
	  for (i = 0; i < NUM_PS_FONTS; i++)
	    if (_plotter->page->ps_font_used[i])
              {
                const char *reencoding;

                if (_ps_font_info[i].iso8859_1)	/* ISO-Latin-1 font */
                  reencoding = "";
                else if (strcmp (_ps_font_info[i].ps_name, "ZapfDingbats")== 0)
                  reencoding = _ai_zapf_dingbats_reencoding;
                else if (strcmp (_ps_font_info[i].ps_name, "Symbol") == 0)
                  reencoding = _ai_symbol_reencoding;
                else		/* don't know what to do */
                  reencoding = "";
 	        fprintf (_plotter->outstream, "\
%%AI3_BeginEncoding: _%s %s\n\
[%s/_%s/%s 0 0 0 TZ\n\
%%AI3_EndEncoding AdobeType\n",
		       _ps_font_info[i].ps_name, _ps_font_info[i].ps_name,
                       reencoding,
		       _ps_font_info[i].ps_name, _ps_font_info[i].ps_name);
               }
	  for (i = 0; i < NUM_PCL_FONTS; i++)
	    if (_plotter->page->pcl_font_used[i])
	        fprintf (_plotter->outstream, "\
%%AI3_BeginEncoding: _%s %s\n\
[/_%s/%s 0 0 0 TZ\n\
%%AI3_EndEncoding TrueType\n",
		       _pcl_font_info[i].ps_name, _pcl_font_info[i].ps_name,
		       _pcl_font_info[i].ps_name, _pcl_font_info[i].ps_name);
	}
      /* end of Setup section */
      fprintf (_plotter->outstream, "\
%%%%EndSetup\n");

      /* objects will belong to layer #1 (if layers are supported) */
      if (_plotter->ai_version >= AI_VERSION_5)
	fprintf (_plotter->outstream, "\
%%AI5_BeginLayer\n\
1 1 1 1 0 0 0 79 128 255 Lb\n\
(Layer 1) Ln\n");

      if (_plotter->ai_version >= AI_VERSION_5)
        /* switch to even-odd filling from nonzero-winding-number filling */
	fprintf (_plotter->outstream, "\
1 XR\n");

      /* OUTPUT ALL CACHED OBJECTS */
      if (_plotter->page->len > 0)
	fputs (_plotter->page->base, _plotter->outstream); 

      /* after outputing objects, end layer */
      if (_plotter->ai_version >= AI_VERSION_5)
        fprintf (_plotter->outstream, "\
LB\n\
%%AI5_EndLayer--\n");

      /* emit page trailer */
      fprintf (_plotter->outstream, "\
%%%%PageTrailer\n\
gsave annotatepage grestore showpage\n");

      /* trailer: terminate procsets */
      if (_plotter->ai_version == AI_VERSION_5)
        {
          fprintf (_plotter->outstream, "\
%%%%Trailer\n\
Adobe_Illustrator_AI5 /terminate get exec\n\
Adobe_ColorImage_AI6 /terminate get exec\n\
Adobe_typography_AI5 /terminate get exec\n\
Adobe_level2_AI5 /terminate get exec\n\
%%%%EOF\n");
        }
      else			/* AI_VERSION_3 */
        {
          fprintf (_plotter->outstream, "\
%%%%Trailer\n\
Adobe_Illustrator_AI3 /terminate get exec\n\
Adobe_pattern_AI3 /terminate get exec\n\
Adobe_typography_AI3 /terminate get exec\n\
Adobe_customcolor /terminate get exec\n\
Adobe_cshow /terminate get exec\n\
Adobe_cmykcolor /terminate get exec\n\
Adobe_packedarray /terminate get exec\n\
%%%%EOF\n");
        }
    }

  /* Delete the page buffer, since AI Plotters don't need to maintain a
     linked list of pages. */
  _delete_outbuf (_plotter->page);
  _plotter->page = NULL;

  /* remove zeroth drawing state too, so we can start afresh */

  /* elements of state that are strings are freed separately */
  free (_plotter->drawstate->line_mode);
  free (_plotter->drawstate->join_mode);
  free (_plotter->drawstate->cap_mode);
  free (_plotter->drawstate->font_name);
  
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;

  /* reset our knowledge of Illustrator's internal state (necessary only if
     user calls outfile() to reuse the Plotter) */
  _plotter->ai_pen_cyan = _ai_default_plotter.ai_pen_cyan;
  _plotter->ai_pen_magenta = _ai_default_plotter.ai_pen_magenta;
  _plotter->ai_pen_yellow = _ai_default_plotter.ai_pen_yellow;
  _plotter->ai_pen_black = _ai_default_plotter.ai_pen_black;
  _plotter->ai_fill_cyan = _ai_default_plotter.ai_fill_cyan;
  _plotter->ai_fill_magenta = _ai_default_plotter.ai_fill_magenta;
  _plotter->ai_fill_yellow = _ai_default_plotter.ai_fill_yellow;
  _plotter->ai_fill_black = _ai_default_plotter.ai_fill_black;
  _plotter->ai_cyan_used = _ai_default_plotter.ai_cyan_used;
  _plotter->ai_magenta_used = _ai_default_plotter.ai_magenta_used;
  _plotter->ai_yellow_used = _ai_default_plotter.ai_yellow_used;
  _plotter->ai_black_used = _ai_default_plotter.ai_black_used;
  _plotter->ai_cap_style = _ai_default_plotter.ai_cap_style;
  _plotter->ai_join_style = _ai_default_plotter.ai_join_style;
  _plotter->ai_line_width = _ai_default_plotter.ai_line_width;

  _plotter->open = false;	/* flag device as closed */

  if (_plotter->outstream && fflush(_plotter->outstream) < 0)
	{
	  _plotter->error ("output stream jammed");
	  return -1;
	}
  else
    return 0;
}
