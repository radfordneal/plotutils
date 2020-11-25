/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Brian V. Smith
 *
 * Modified 1997 by Robert Maier to support the full set of 35 PS fonts.
 *
 * The X Consortium, and any party obtaining a copy of these files from
 * the X Consortium, directly or indirectly, is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.  This license includes without
 * limitation a license to do the foregoing actions under any patents of
 * the party supplying this software to the X Consortium.
 */

#include "fig.h"
#include "resources.h"
#include "u_fonts.h"
#include "object.h"

/* printer font names for indicator window */

struct _xfstruct x_fontinfo[NUM_FONTS] = {
    {"-*-times-medium-r-*--", (struct xfont*) NULL},
    {"-*-times-medium-i-*--", (struct xfont*) NULL},
    {"-*-times-bold-r-*--", (struct xfont*) NULL},
    {"-*-times-bold-i-*--", (struct xfont*) NULL},

    {"-*-itc avant garde gothic-book-r-*--", (struct xfont*) NULL},
    {"-*-itc avant garde gothic-book-i-*--", (struct xfont*) NULL},
    {"-*-itc avant garde gothic-demi-r-*--", (struct xfont*) NULL},
    {"-*-itc avant garde gothic-demi-i-*--", (struct xfont*) NULL},

    {"-*-itc bookman-light-r-*--", (struct xfont*) NULL},
    {"-*-itc bookman-light-i-*--", (struct xfont*) NULL},
    {"-*-itc bookman-demi-r-*--", (struct xfont*) NULL},
    {"-*-itc bookman-demi-i-*--", (struct xfont*) NULL},

    {"-*-courier-medium-r-*--", (struct xfont*) NULL},
    {"-*-courier-medium-o-*--", (struct xfont*) NULL},
    {"-*-courier-bold-r-*--", (struct xfont*) NULL},
    {"-*-courier-bold-o-*--", (struct xfont*) NULL},

    {"-*-helvetica-medium-r-normal--", (struct xfont*) NULL},
    {"-*-helvetica-medium-o-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-r-normal--", (struct xfont*) NULL},
    {"-*-helvetica-bold-o-normal--", (struct xfont*) NULL},

    {"-*-helvetica-medium-r-narrow--", (struct xfont*) NULL},
    {"-*-helvetica-medium-o-narrow--", (struct xfont*) NULL},
    {"-*-helvetica-bold-r-narrow--", (struct xfont*) NULL},
    {"-*-helvetica-bold-o-narrow--", (struct xfont*) NULL},

    {"-*-new century schoolbook-medium-r-*--", (struct xfont*) NULL},
    {"-*-new century schoolbook-medium-i-*--", (struct xfont*) NULL},
    {"-*-new century schoolbook-bold-r-*--", (struct xfont*) NULL},
    {"-*-new century schoolbook-bold-i-*--", (struct xfont*) NULL},

    {"-*-palatino-medium-r-*--", (struct xfont*) NULL},
    {"-*-palatino-medium-i-*--", (struct xfont*) NULL},
    {"-*-palatino-bold-r-*--", (struct xfont*) NULL},
    {"-*-palatino-bold-i-*--", (struct xfont*) NULL},

    {"-*-symbol-medium-r-*--", (struct xfont*) NULL},
    {"-*-itc zapf chancery-medium-i-*--", (struct xfont*) NULL},
    {"-*-itc zapf dingbats-*-*-*--", (struct xfont*) NULL},
};

struct _fstruct ps_fontinfo[NUM_FONTS + 1] = {
    {"Default", -1},
    {"Times-Roman",			0},
    {"Times-Italic",			1},
    {"Times-Bold",			2},
    {"Times-BoldItalic",		3},
    {"AvantGarde-Book",			4},
    {"AvantGarde-BookOblique",		5},
    {"AvantGarde-Demi",			6},
    {"AvantGarde-DemiOblique",		7},
    {"Bookman-Light",			8},
    {"Bookman-LightItalic",		9},
    {"Bookman-Demi",			10},
    {"Bookman-DemiItalic",		11},
    {"Courier",				12},
    {"Courier-Oblique",			13},
    {"Courier-Bold",			14},
    {"Courier-BoldOblique",		15},
    {"Helvetica",			16},
    {"Helvetica-Oblique",		17},
    {"Helvetica-Bold",			18},
    {"Helvetica-BoldOblique",		19},
    {"Helvetica-Narrow",		20},
    {"Helvetica-Narrow-Oblique",	21},
    {"Helvetica-Narrow-Bold",		22},
    {"Helvetica-Narrow-BoldOblique",	23},
    {"NewCenturySchlbk-Roman",		24},
    {"NewCenturySchlbk-Italic",		25},
    {"NewCenturySchlbk-Bold",		26},
    {"NewCenturySchlbk-BoldItalic",	27},
    {"Palatino-Roman",			28},
    {"Palatino-Italic",			29},
    {"Palatino-Bold",			30},
    {"Palatino-BoldItalic",		31},
    {"Symbol",				32},
    {"ZapfChancery-MediumItalic",	33},
    {"ZapfDingbats",			34},
};

struct _fstruct latex_fontinfo[NUM_LATEX_FONTS] = {
    {"Default",		0},
    {"Roman",		0},
    {"Bold",		2},
    {"Italic",		1},
    {"Modern",		16},
    {"Typewriter",	12},
};

x_fontnum(psflag, fnum)
    int		    psflag, fnum;
{
    return (psflag ? ps_fontinfo[fnum + 1].xfontnum :
	    latex_fontinfo[fnum].xfontnum);
}

psfontnum(font)
char *font;
{
    int i;

    if (font == NULL)
	return(DEF_PS_FONT);
    for (i=0; i<NUM_FONTS; i++)
	if (strcasecmp(ps_fontinfo[i].name, font) == 0)
		return (i-1);
    return(DEF_PS_FONT);
}

latexfontnum(font)
char *font;
{
    int i;

    if (font == NULL)
	return(DEF_LATEX_FONT);
    for (i=0; i<NUM_LATEX_FONTS; i++)
	if (strcasecmp(latex_fontinfo[i].name, font) == 0)
		return (i);
    return(DEF_LATEX_FONT);
}
