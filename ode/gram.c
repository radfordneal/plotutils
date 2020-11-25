
/*  A Bison parser, made from gram.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NUMBER	258
#define	IDENT	259
#define	SEP	260
#define	ABS	261
#define	SQRT	262
#define	EXP	263
#define	LOG	264
#define	LOG10	265
#define	SIN	266
#define	COS	267
#define	TAN	268
#define	ASIN	269
#define	ACOS	270
#define	ATAN	271
#define	SINH	272
#define	COSH	273
#define	TANH	274
#define	ASINH	275
#define	ACOSH	276
#define	ATANH	277
#define	FLOOR	278
#define	CEIL	279
#define	J0	280
#define	J1	281
#define	Y0	282
#define	Y1	283
#define	LGAMMA	284
#define	GAMMA	285
#define	ERF	286
#define	ERFC	287
#define	INVERF	288
#define	NORM	289
#define	INVNORM	290
#define	IGAMMA	291
#define	IBETA	292
#define	EVERY	293
#define	FROM	294
#define	PRINT	295
#define	STEP	296
#define	EXAM	297
#define	UMINUS	298

#line 1 "gram.y"

/*
 * Grammar for ode:
 * Most things are self-explanatory.
 * When you're done with a lexptr-type object
 * you should free it with lfree.  They are
 * used for passing constants around while parsing
 * (computing the value of) a cexpr.  The macros
 * for evaluating operators and functions are the
 * most important thing to be familiar with before
 * toying with the semantics.
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 * GNU enhancements copyright (C) 1996-1997 Free Software Foundation, Inc.
 */
#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * Value is true iff operands pass ONECON.
 */
#define TWOCON(x,y) (ONECON(x) && ONECON(y))
#define THREECON(x,y,z) (ONECON(x) && ONECON(y) && ONECON(z))
/*
 * Value must be a (struct expr *).  Returns true if its arg, when
 * evaluated, would put a constant on the stack.
 */
#define ONECON(x) (x->ex_oper == O_CONST && x->ex_next == NULL)
/*
 * Performs ordinary binary arithmetic, when there are two constants in an
 * expr (`op' is a C operator that includes an assignment, e.g., +=).
 */
#define COMBINE(x,y,r,op) {x->ex_value op y->ex_value; efree(y); r = x;}
/*
 * Generates stack code for a binary operation, as for a dyadic operator in
 * an expression.  E.g., op = O_PLUS.
 */
#define BINARY(x,y,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(y,ep);\
                concat(r=x,y);}
/*
 * Generates stack code for a ternary operation, as for a triadic operator in
 * an expression.  E.g., op = O_IBETA.
 */
#define TERNARY(x,y,z,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(z,ep);\
                concat(y,z);\
                concat(r=x,y);}
/*
 * Performs ordinary unary arithmetic, when there is a constant in an expr.
 * "-" seems to work as a monadic operator.
 */
#define CONFUNC(x,r,f) {x->ex_value = f(x->ex_value); r = x;}
/*
 * Generates stack code for a unary operation, as for a monadic operator in
 * an expression.
 */
#define UNARY(oprnd,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(r=oprnd,ep);}
/*
 * Performs binary arithmetic in a cexpr (`op' is a C operator that
 * includes an assignment, e.g. +=).
 */
#define CEXOP(x,y,r,op) {x->lx_u.lxu_value op y->lx_u.lxu_value;\
                lfree(y);\
                r = x;}
/*
 * Performs unary arithmetic in a cexpr.
 */
#define CEXFUNC(x,r,f) {x->lx_u.lxu_value = f(x->lx_u.lxu_value); r=x;}

/*
 * A hook for future upgrades in error reporting
 */
static  char    *errmess = NULL;

bool erritem;

#line 82 "gram.y"
typedef union {
        struct  lex     *lexptr;
        struct  expr    *exprptr;
        struct  prt     *prtptr;
        int     simple;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		346
#define	YYFLAG		-32768
#define	YYNTBASE	57

#define YYTRANSLATE(x) ((unsigned)(x) <= 298 ? yytranslate[x] : 66)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    53,     2,     2,     2,     2,     2,    50,    55,
    56,    46,    44,    51,    45,     2,    47,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    43,     2,    54,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    48,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,    52,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    49
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     7,    12,    15,    21,    27,    33,    41,
    45,    47,    51,    54,    55,    57,    59,    61,    63,    64,
    67,    68,    71,    75,    79,    83,    87,    91,    95,   100,
   105,   110,   115,   120,   125,   130,   135,   140,   145,   150,
   155,   160,   165,   170,   175,   180,   185,   190,   195,   200,
   205,   210,   215,   220,   225,   230,   235,   240,   245,   252,
   261,   264,   266,   270,   274,   278,   282,   286,   290,   295,
   300,   305,   310,   315,   320,   325,   330,   335,   340,   345,
   350,   355,   360,   365,   370,   375,   380,   385,   390,   395,
   400,   405,   410,   415,   420,   425,   430,   435,   440,   447,
   456,   459,   461
};

static const short yyrhs[] = {    58,
     0,    57,    58,     0,     5,     0,     4,    43,    65,     5,
     0,     1,     5,     0,     4,    50,    43,    65,     5,     0,
    40,    59,    62,    63,     5,     0,    41,    64,    51,    64,
     5,     0,    41,    64,    51,    64,    51,    64,     5,     0,
    42,     4,     5,     0,    60,     0,    59,    51,    60,     0,
     4,    61,     0,     0,    50,     0,    52,     0,    53,     0,
    54,     0,     0,    38,    64,     0,     0,    39,    64,     0,
    55,    64,    56,     0,    64,    44,    64,     0,    64,    45,
    64,     0,    64,    46,    64,     0,    64,    47,    64,     0,
    64,    48,    64,     0,     7,    55,    64,    56,     0,     6,
    55,    64,    56,     0,     8,    55,    64,    56,     0,     9,
    55,    64,    56,     0,    10,    55,    64,    56,     0,    11,
    55,    64,    56,     0,    12,    55,    64,    56,     0,    13,
    55,    64,    56,     0,    20,    55,    64,    56,     0,    21,
    55,    64,    56,     0,    22,    55,    64,    56,     0,    14,
    55,    64,    56,     0,    15,    55,    64,    56,     0,    16,
    55,    64,    56,     0,    17,    55,    64,    56,     0,    18,
    55,    64,    56,     0,    19,    55,    64,    56,     0,    23,
    55,    64,    56,     0,    24,    55,    64,    56,     0,    25,
    55,    64,    56,     0,    26,    55,    64,    56,     0,    27,
    55,    64,    56,     0,    28,    55,    64,    56,     0,    32,
    55,    64,    56,     0,    31,    55,    64,    56,     0,    33,
    55,    64,    56,     0,    29,    55,    64,    56,     0,    30,
    55,    64,    56,     0,    34,    55,    64,    56,     0,    35,
    55,    64,    56,     0,    36,    55,    64,    51,    64,    56,
     0,    37,    55,    64,    51,    64,    51,    64,    56,     0,
    45,    64,     0,     3,     0,    55,    65,    56,     0,    65,
    44,    65,     0,    65,    45,    65,     0,    65,    46,    65,
     0,    65,    47,    65,     0,    65,    48,    65,     0,     7,
    55,    65,    56,     0,     6,    55,    65,    56,     0,     8,
    55,    65,    56,     0,     9,    55,    65,    56,     0,    10,
    55,    65,    56,     0,    11,    55,    65,    56,     0,    12,
    55,    65,    56,     0,    13,    55,    65,    56,     0,    20,
    55,    65,    56,     0,    21,    55,    65,    56,     0,    22,
    55,    65,    56,     0,    14,    55,    65,    56,     0,    15,
    55,    65,    56,     0,    16,    55,    65,    56,     0,    17,
    55,    65,    56,     0,    18,    55,    65,    56,     0,    19,
    55,    65,    56,     0,    23,    55,    65,    56,     0,    24,
    55,    65,    56,     0,    25,    55,    65,    56,     0,    26,
    55,    65,    56,     0,    27,    55,    65,    56,     0,    28,
    55,    65,    56,     0,    29,    55,    65,    56,     0,    30,
    55,    65,    56,     0,    32,    55,    65,    56,     0,    31,
    55,    65,    56,     0,    33,    55,    65,    56,     0,    34,
    55,    65,    56,     0,    35,    55,    65,    56,     0,    36,
    55,    65,    51,    65,    56,     0,    37,    55,    65,    51,
    65,    51,    65,    56,     0,    45,    65,     0,     3,     0,
     4,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   107,   108,   111,   113,   124,   136,   163,   170,   182,   202,
   241,   246,   256,   268,   270,   272,   277,   282,   289,   291,
   299,   301,   309,   313,   317,   321,   325,   329,   336,   340,
   346,   350,   354,   358,   362,   366,   370,   374,   378,   382,
   386,   390,   394,   398,   402,   406,   410,   414,   418,   422,
   426,   430,   434,   438,   442,   446,   450,   454,   458,   465,
   473,   477,   481,   483,   490,   497,   504,   515,   581,   588,
   597,   604,   611,   618,   625,   632,   639,   646,   653,   660,
   667,   674,   681,   688,   695,   702,   709,   716,   723,   730,
   737,   744,   751,   758,   765,   772,   779,   786,   793,   804,
   816,   823,   830
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NUMBER",
"IDENT","SEP","ABS","SQRT","EXP","LOG","LOG10","SIN","COS","TAN","ASIN","ACOS",
"ATAN","SINH","COSH","TANH","ASINH","ACOSH","ATANH","FLOOR","CEIL","J0","J1",
"Y0","Y1","LGAMMA","GAMMA","ERF","ERFC","INVERF","NORM","INVNORM","IGAMMA","IBETA",
"EVERY","FROM","PRINT","STEP","EXAM","'='","'+'","'-'","'*'","'/'","'^'","UMINUS",
"'''","','","'~'","'!'","'?'","'('","')'","program","stat","prtlist","prtitem",
"prttag","optevery","optfrom","cexpr","expr", NULL
};
#endif

static const short yyr1[] = {     0,
    57,    57,    58,    58,    58,    58,    58,    58,    58,    58,
    59,    59,    60,    61,    61,    61,    61,    61,    62,    62,
    63,    63,    64,    64,    64,    64,    64,    64,    64,    64,
    64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
    64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
    64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
    64,    64,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65,    65
};

static const short yyr2[] = {     0,
     1,     2,     1,     4,     2,     5,     5,     5,     7,     3,
     1,     3,     2,     0,     1,     1,     1,     1,     0,     2,
     0,     2,     3,     3,     3,     3,     3,     3,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     6,     8,
     2,     1,     3,     3,     3,     3,     3,     3,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     6,     8,
     2,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     3,     0,     0,     0,     0,     1,     5,     0,
     0,    14,    19,    11,    62,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     2,   102,   103,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    15,    16,    17,    18,    13,     0,     0,    21,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    61,     0,     0,     0,     0,     0,     0,     0,    10,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   101,     0,     4,     0,     0,     0,     0,     0,     0,
    20,    12,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
    27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    63,    64,    65,    66,    67,
    68,     6,    22,     7,    30,    29,    31,    32,    33,    34,
    35,    36,    40,    41,    42,    43,    44,    45,    37,    38,
    39,    46,    47,    48,    49,    50,    51,    55,    56,    53,
    52,    54,    57,    58,     0,     0,     8,     0,    70,    69,
    71,    72,    73,    74,    75,    76,    80,    81,    82,    83,
    84,    85,    77,    78,    79,    86,    87,    88,    89,    90,
    91,    92,    93,    95,    94,    96,    97,    98,     0,     0,
     0,     0,     0,     0,     0,    59,     0,     9,    99,     0,
     0,     0,    60,   100,     0,     0
};

static const short yydefgoto[] = {     7,
     8,    13,    14,    95,    98,   184,    50,    89
};

static const short yypact[] = {   232,
     9,   -23,-32768,    43,   180,    46,   227,-32768,-32768,   130,
    40,   -17,   -36,-32768,-32768,    29,   113,   114,   121,   122,
   123,   125,   127,   129,   169,   171,   175,   179,   183,   189,
   190,   191,   194,   202,   203,   205,   206,   215,   216,   233,
   235,   237,   250,   253,   254,   255,   268,   180,   180,   208,
   176,-32768,-32768,-32768,   271,   272,   273,   286,   289,   290,
   291,   304,   307,   308,   309,   322,   325,   326,   327,   340,
   343,   344,   345,   358,   361,   362,   363,   376,   379,   380,
   381,   394,   397,   398,   399,   412,   130,   130,   126,   130,
-32768,-32768,-32768,-32768,-32768,   180,    43,    93,   180,   180,
   180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
   180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
   180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
-32768,   -40,   180,   180,   180,   180,   180,   180,-32768,   130,
   130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
   130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
   130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
   130,-32768,   -35,-32768,   130,   130,   130,   130,   130,   174,
   827,-32768,   180,   224,   -22,   -16,   195,   231,   238,   251,
   256,   269,   274,   287,   292,   305,   310,   323,   328,   341,
   346,   359,   364,   377,   382,   395,   400,   413,   418,   431,
   436,   449,   454,   467,   779,   787,-32768,   -29,   -29,   245,
   245,   245,    -2,   472,   485,   490,   503,   508,   521,   526,
   539,   544,   557,   562,   575,   580,   593,   598,   611,   616,
   629,   634,   647,   652,   665,   670,   683,   688,   701,   706,
   719,   724,   737,   795,   803,-32768,    77,    77,   258,   258,
   258,-32768,   827,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   180,   180,-32768,   180,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   130,   130,
   742,   811,   218,   755,   819,-32768,   180,-32768,-32768,   130,
   760,   773,-32768,-32768,   311,-32768
};

static const short yypgoto[] = {-32768,
   317,-32768,   263,-32768,-32768,-32768,   -48,   -49
};


#define	YYLAST		875


static const short yytable[] = {   131,
   132,    96,   297,   133,   134,   135,   136,   137,   175,   176,
   177,   178,   179,     9,    97,   217,   135,   136,   137,    10,
   256,   133,   134,   135,   136,   137,    11,   133,   134,   135,
   136,   137,    91,   265,    92,    93,    94,   172,   173,   266,
   180,   133,   134,   135,   136,   137,    12,   181,   298,    51,
   185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
   195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
   205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
   215,   216,    90,    99,   218,   219,   220,   221,   222,   223,
   224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
   234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
   244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
   254,   255,   177,   178,   179,   257,   258,   259,   260,   261,
   174,   183,    53,    54,   263,    55,    56,    57,    58,    59,
    60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
    70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
    80,    81,    82,    83,    84,    85,    86,   100,   101,   175,
   176,   177,   178,   179,    87,   102,   103,   104,   262,   105,
   139,   106,    15,   107,    88,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
    31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    44,    45,    46,    47,   175,   176,   177,
   178,   179,   338,   108,    48,   109,   345,     1,   264,   110,
     2,     3,     1,   111,    49,     2,     3,   112,   133,   134,
   135,   136,   137,   113,   114,   115,   331,   332,   116,   333,
   267,   133,   134,   135,   136,   137,   117,   118,   138,   119,
   120,   133,   134,   135,   136,   137,     4,     5,     6,   121,
   122,     4,     5,     6,   133,   134,   135,   136,   137,   334,
   335,   133,   134,   135,   136,   137,   268,   123,   341,   124,
   342,   125,   137,   269,   133,   134,   135,   136,   137,   133,
   134,   135,   136,   137,   126,   179,   270,   127,   128,   129,
   346,   271,   133,   134,   135,   136,   137,   133,   134,   135,
   136,   137,   130,    52,   272,   140,   141,   142,     0,   273,
   133,   134,   135,   136,   137,   133,   134,   135,   136,   137,
   143,     0,   274,   144,   145,   146,     0,   275,   133,   134,
   135,   136,   137,   133,   134,   135,   136,   137,   147,   182,
   276,   148,   149,   150,     0,   277,   133,   134,   135,   136,
   137,   133,   134,   135,   136,   137,   151,     0,   278,   152,
   153,   154,     0,   279,   133,   134,   135,   136,   137,   133,
   134,   135,   136,   137,   155,     0,   280,   156,   157,   158,
     0,   281,   133,   134,   135,   136,   137,   133,   134,   135,
   136,   137,   159,     0,   282,   160,   161,   162,     0,   283,
   133,   134,   135,   136,   137,   133,   134,   135,   136,   137,
   163,     0,   284,   164,   165,   166,     0,   285,   133,   134,
   135,   136,   137,   133,   134,   135,   136,   137,   167,     0,
   286,   168,   169,   170,     0,   287,   133,   134,   135,   136,
   137,   133,   134,   135,   136,   137,   171,     0,   288,     0,
     0,     0,     0,   289,   133,   134,   135,   136,   137,   133,
   134,   135,   136,   137,     0,     0,   290,     0,     0,     0,
     0,   291,   133,   134,   135,   136,   137,   133,   134,   135,
   136,   137,     0,     0,   292,     0,     0,     0,     0,   293,
   133,   134,   135,   136,   137,   175,   176,   177,   178,   179,
     0,     0,   294,     0,     0,     0,     0,   299,   175,   176,
   177,   178,   179,   175,   176,   177,   178,   179,     0,     0,
   300,     0,     0,     0,     0,   301,   175,   176,   177,   178,
   179,   175,   176,   177,   178,   179,     0,     0,   302,     0,
     0,     0,     0,   303,   175,   176,   177,   178,   179,   175,
   176,   177,   178,   179,     0,     0,   304,     0,     0,     0,
     0,   305,   175,   176,   177,   178,   179,   175,   176,   177,
   178,   179,     0,     0,   306,     0,     0,     0,     0,   307,
   175,   176,   177,   178,   179,   175,   176,   177,   178,   179,
     0,     0,   308,     0,     0,     0,     0,   309,   175,   176,
   177,   178,   179,   175,   176,   177,   178,   179,     0,     0,
   310,     0,     0,     0,     0,   311,   175,   176,   177,   178,
   179,   175,   176,   177,   178,   179,     0,     0,   312,     0,
     0,     0,     0,   313,   175,   176,   177,   178,   179,   175,
   176,   177,   178,   179,     0,     0,   314,     0,     0,     0,
     0,   315,   175,   176,   177,   178,   179,   175,   176,   177,
   178,   179,     0,     0,   316,     0,     0,     0,     0,   317,
   175,   176,   177,   178,   179,   175,   176,   177,   178,   179,
     0,     0,   318,     0,     0,     0,     0,   319,   175,   176,
   177,   178,   179,   175,   176,   177,   178,   179,     0,     0,
   320,     0,     0,     0,     0,   321,   175,   176,   177,   178,
   179,   175,   176,   177,   178,   179,     0,     0,   322,     0,
     0,     0,     0,   323,   175,   176,   177,   178,   179,   175,
   176,   177,   178,   179,     0,     0,   324,     0,     0,     0,
     0,   325,   175,   176,   177,   178,   179,   175,   176,   177,
   178,   179,     0,     0,   326,     0,     0,     0,     0,   327,
   175,   176,   177,   178,   179,   133,   134,   135,   136,   137,
     0,     0,   328,     0,     0,     0,     0,   336,   175,   176,
   177,   178,   179,   133,   134,   135,   136,   137,     0,     0,
   339,     0,     0,     0,     0,   343,   175,   176,   177,   178,
   179,     0,   133,   134,   135,   136,   137,     0,   344,   295,
   133,   134,   135,   136,   137,     0,     0,   296,   175,   176,
   177,   178,   179,     0,     0,   329,   175,   176,   177,   178,
   179,     0,     0,   330,   133,   134,   135,   136,   137,     0,
     0,   337,   175,   176,   177,   178,   179,     0,     0,   340,
   133,   134,   135,   136,   137
};

static const short yycheck[] = {    48,
    49,    38,     5,    44,    45,    46,    47,    48,    44,    45,
    46,    47,    48,     5,    51,    56,    46,    47,    48,    43,
    56,    44,    45,    46,    47,    48,    50,    44,    45,    46,
    47,    48,    50,    56,    52,    53,    54,    87,    88,    56,
    90,    44,    45,    46,    47,    48,     4,    96,    51,     4,
    99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
   109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
   119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
   129,   130,    43,    55,   133,   134,   135,   136,   137,   138,
   140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
   150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
   160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
   170,   171,    46,    47,    48,   175,   176,   177,   178,   179,
     5,    39,     3,     4,   183,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
    31,    32,    33,    34,    35,    36,    37,    55,    55,    44,
    45,    46,    47,    48,    45,    55,    55,    55,     5,    55,
     5,    55,     3,    55,    55,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
    31,    32,    33,    34,    35,    36,    37,    44,    45,    46,
    47,    48,     5,    55,    45,    55,     0,     1,     5,    55,
     4,     5,     1,    55,    55,     4,     5,    55,    44,    45,
    46,    47,    48,    55,    55,    55,   295,   296,    55,   298,
    56,    44,    45,    46,    47,    48,    55,    55,    51,    55,
    55,    44,    45,    46,    47,    48,    40,    41,    42,    55,
    55,    40,    41,    42,    44,    45,    46,    47,    48,   329,
   330,    44,    45,    46,    47,    48,    56,    55,   337,    55,
   340,    55,    48,    56,    44,    45,    46,    47,    48,    44,
    45,    46,    47,    48,    55,    48,    56,    55,    55,    55,
     0,    56,    44,    45,    46,    47,    48,    44,    45,    46,
    47,    48,    55,     7,    56,    55,    55,    55,    -1,    56,
    44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
    55,    -1,    56,    55,    55,    55,    -1,    56,    44,    45,
    46,    47,    48,    44,    45,    46,    47,    48,    55,    97,
    56,    55,    55,    55,    -1,    56,    44,    45,    46,    47,
    48,    44,    45,    46,    47,    48,    55,    -1,    56,    55,
    55,    55,    -1,    56,    44,    45,    46,    47,    48,    44,
    45,    46,    47,    48,    55,    -1,    56,    55,    55,    55,
    -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
    47,    48,    55,    -1,    56,    55,    55,    55,    -1,    56,
    44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
    55,    -1,    56,    55,    55,    55,    -1,    56,    44,    45,
    46,    47,    48,    44,    45,    46,    47,    48,    55,    -1,
    56,    55,    55,    55,    -1,    56,    44,    45,    46,    47,
    48,    44,    45,    46,    47,    48,    55,    -1,    56,    -1,
    -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
    45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
    -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
    47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
    44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
    46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
    56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
    48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
    -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
    45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
    -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
    47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
    44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
    46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
    56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
    48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
    -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
    45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
    -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
    47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
    44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
    46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
    56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
    48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
    -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
    45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
    -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
    47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
    44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
    46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
    56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
    48,    -1,    44,    45,    46,    47,    48,    -1,    56,    51,
    44,    45,    46,    47,    48,    -1,    -1,    51,    44,    45,
    46,    47,    48,    -1,    -1,    51,    44,    45,    46,    47,
    48,    -1,    -1,    51,    44,    45,    46,    47,    48,    -1,
    -1,    51,    44,    45,    46,    47,    48,    -1,    -1,    51,
    44,    45,    46,    47,    48
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/share/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/local/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
#line 112 "gram.y"
{ lfree(yyvsp[0].lexptr); ;
    break;}
case 4:
#line 114 "gram.y"
{
                        struct sym *sp;

                        sp = lookup(yyvsp[-3].lexptr->lx_u.lxu_name);
                        sp->sy_value = eval(yyvsp[-1].exprptr);
                        sp->sy_flags |= SF_INIT;
                        lfree(yyvsp[-3].lexptr);
                        efree(yyvsp[-1].exprptr);
                        lfree(yyvsp[0].lexptr);
                        ;
    break;}
case 5:
#line 125 "gram.y"
{
                        if (errmess == NULL)
                                errmess = "syntax error";
			fprintf (stderr, "%s:%s:%d: %s\n", 
				 progname, filename,
				 (yyvsp[0].lexptr->lx_lino), errmess);
                        errmess = NULL;
                        lfree(yyvsp[0].lexptr);
                        yyerrok;
                        yyclearin;
                        ;
    break;}
case 6:
#line 137 "gram.y"
{
                        struct sym *sp;
                        struct prt *pp, *qp;

                        sp = lookup(yyvsp[-4].lexptr->lx_u.lxu_name);
                        efree(sp->sy_expr);
                        sp->sy_expr = yyvsp[-1].exprptr;
                        sp->sy_flags |= SF_ISEQN;
                        if (!sawprint) {
                                for (pp=pqueue; pp!=NULL; pp=pp->pr_link)
                                        if (pp->pr_sym == sp)
                                                goto found;
                                pp = palloc();
                                pp->pr_sym = sp;
                                if (pqueue == NULL)
                                        pqueue = pp;
                                else {
                                        for (qp=pqueue; qp->pr_link!=NULL; )
                                                qp = qp->pr_link;
                                        qp->pr_link = pp;
                                }
                        }
                found:
                        lfree(yyvsp[-4].lexptr);
                        lfree(yyvsp[0].lexptr);
                        ;
    break;}
case 7:
#line 164 "gram.y"
{
                        sawprint = true;
                        prerr = erritem;
                        erritem = false;
                        lfree(yyvsp[0].lexptr);
                        ;
    break;}
case 8:
#line 171 "gram.y"
{
                        lfree(yyvsp[0].lexptr);
                        tstart = yyvsp[-3].lexptr->lx_u.lxu_value;
                        lfree(yyvsp[-3].lexptr);
                        tstop = yyvsp[-1].lexptr->lx_u.lxu_value;
                        lfree(yyvsp[-1].lexptr);
                        if (!conflag)
                                startstep();
                        solve();
                        sawstep = true;
                        ;
    break;}
case 9:
#line 183 "gram.y"
{
			double savstep;
			bool savconflag;

                        lfree(yyvsp[0].lexptr);
                        tstart = yyvsp[-5].lexptr->lx_u.lxu_value;
                        lfree(yyvsp[-5].lexptr);
                        tstop = yyvsp[-3].lexptr->lx_u.lxu_value;
                        lfree(yyvsp[-3].lexptr);
                        savstep = tstep;
                        tstep = yyvsp[-1].lexptr->lx_u.lxu_value;
                        lfree(yyvsp[-1].lexptr);
                        savconflag = conflag;
                        conflag = true;
                        solve();
                        tstep = savstep;
                        conflag = savconflag;
                        sawstep = true;
                        ;
    break;}
case 10:
#line 203 "gram.y"
{
                        struct sym *sp;

                        lfree(yyvsp[0].lexptr);
                        sp = lookup(yyvsp[-1].lexptr->lx_u.lxu_name);
                        lfree(yyvsp[-1].lexptr);
                        printf ("\"%.*s\" is ",NAMMAX,sp->sy_name);
                        switch (sp->sy_flags & SF_DEPV) {
                        case SF_DEPV:
                        case SF_ISEQN:
                                printf ("a dynamic variable\n");
                                break;
                        case SF_INIT:
                                printf ("an initialized constant\n");
                                break;
                        case 0:
                                printf ("an uninitialized constant\n");
                                break;
                        default:
                                panicn ("impossible (%d) in EXAM action",
					sp->sy_flags);
                        }
                        printf ("value:");
                        prval (sp->sy_value);
                        printf ("\nprime:");
                        prval (sp->sy_prime);
                        printf ("\nsserr:");
                        prval (sp->sy_sserr);
                        printf ("\naberr:");
                        prval (sp->sy_aberr);
                        printf ("\nacerr:");
                        prval (sp->sy_acerr);
                        putchar ('\n');
                        prexq(sp->sy_expr);
                        fflush(stdout);
                        ;
    break;}
case 11:
#line 242 "gram.y"
{
                        pfree(pqueue);
                        pqueue = yyvsp[0].prtptr;
                        ;
    break;}
case 12:
#line 247 "gram.y"
{
                        struct prt *pp;

                        for (pp=pqueue; pp->pr_link!=NULL; pp=pp->pr_link)
                                ;
                        pp->pr_link = yyvsp[0].prtptr;
                        ;
    break;}
case 13:
#line 257 "gram.y"
{
                        struct prt *pp;

                        pp = palloc();
                        pp->pr_sym = lookup(yyvsp[-1].lexptr->lx_u.lxu_name);
                        pp->pr_which = (ent_cell)(yyvsp[0].simple);
                        lfree(yyvsp[-1].lexptr);
                        yyval.prtptr = pp;
                        ;
    break;}
case 14:
#line 269 "gram.y"
{ yyval.simple = P_VALUE; ;
    break;}
case 15:
#line 271 "gram.y"
{ yyval.simple = P_PRIME; ;
    break;}
case 16:
#line 273 "gram.y"
{
                        yyval.simple = P_ACERR;
                        erritem = true;
                        ;
    break;}
case 17:
#line 278 "gram.y"
{
                        yyval.simple = P_ABERR;
                        erritem = true;
                        ;
    break;}
case 18:
#line 283 "gram.y"
{
                        yyval.simple = P_SSERR;
                        erritem = true;
                        ;
    break;}
case 19:
#line 290 "gram.y"
{ sawevery = false; ;
    break;}
case 20:
#line 292 "gram.y"
{
                        sawevery = true;
                        tevery = IROUND(yyvsp[0].lexptr->lx_u.lxu_value);
                        lfree(yyvsp[0].lexptr);
                        ;
    break;}
case 21:
#line 300 "gram.y"
{ sawfrom = false; ;
    break;}
case 22:
#line 302 "gram.y"
{
                        sawfrom = true;
                        tfrom = yyvsp[0].lexptr->lx_u.lxu_value;
                        lfree(yyvsp[0].lexptr);
                        ;
    break;}
case 23:
#line 310 "gram.y"
{
                        yyval.lexptr = yyvsp[-1].lexptr;
                        ;
    break;}
case 24:
#line 314 "gram.y"
{
                        CEXOP(yyvsp[-2].lexptr,yyvsp[0].lexptr,yyval.lexptr,+=)
                        ;
    break;}
case 25:
#line 318 "gram.y"
{
                        CEXOP(yyvsp[-2].lexptr,yyvsp[0].lexptr,yyval.lexptr,-=)
                        ;
    break;}
case 26:
#line 322 "gram.y"
{
                        CEXOP(yyvsp[-2].lexptr,yyvsp[0].lexptr,yyval.lexptr,*=)
                        ;
    break;}
case 27:
#line 326 "gram.y"
{
                        CEXOP(yyvsp[-2].lexptr,yyvsp[0].lexptr,yyval.lexptr,/=)
                        ;
    break;}
case 28:
#line 330 "gram.y"
{
                        yyvsp[-2].lexptr->lx_u.lxu_value =
                                pow(yyvsp[-2].lexptr->lx_u.lxu_value,yyvsp[0].lexptr->lx_u.lxu_value);
                        lfree(yyvsp[0].lexptr);
                        yyval.lexptr = yyvsp[-2].lexptr;
                        ;
    break;}
case 29:
#line 337 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,sqrt)
                        ;
    break;}
case 30:
#line 341 "gram.y"
{
                        if (yyvsp[-1].lexptr->lx_u.lxu_value < 0)
                                yyvsp[-1].lexptr->lx_u.lxu_value = -(yyvsp[-1].lexptr->lx_u.lxu_value);
                        yyval.lexptr = yyvsp[-1].lexptr;
                        ;
    break;}
case 31:
#line 347 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,exp)
                        ;
    break;}
case 32:
#line 351 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,log)
                        ;
    break;}
case 33:
#line 355 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,log10)
                        ;
    break;}
case 34:
#line 359 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,sin)
                        ;
    break;}
case 35:
#line 363 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,cos)
                        ;
    break;}
case 36:
#line 367 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,tan)
                        ;
    break;}
case 37:
#line 371 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,asinh)
                        ;
    break;}
case 38:
#line 375 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,acosh)
                        ;
    break;}
case 39:
#line 379 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,atanh)
                        ;
    break;}
case 40:
#line 383 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,asin)
                        ;
    break;}
case 41:
#line 387 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,acos)
                        ;
    break;}
case 42:
#line 391 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,atan)
                        ;
    break;}
case 43:
#line 395 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,sinh)
                        ;
    break;}
case 44:
#line 399 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,cosh)
                        ;
    break;}
case 45:
#line 403 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,tanh)
                        ;
    break;}
case 46:
#line 407 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,floor)
                        ;
    break;}
case 47:
#line 411 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,ceil)
                        ;
    break;}
case 48:
#line 415 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,j0)
                        ;
    break;}
case 49:
#line 419 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,j1)
                        ;
    break;}
case 50:
#line 423 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,y0)
                        ;
    break;}
case 51:
#line 427 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,y1)
                        ;
    break;}
case 52:
#line 431 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,erfc)
                        ;
    break;}
case 53:
#line 435 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,erf)
                        ;
    break;}
case 54:
#line 439 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,inverf)
                        ;
    break;}
case 55:
#line 443 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,F_LGAMMA)
                        ;
    break;}
case 56:
#line 447 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,f_gamma)
                        ;
    break;}
case 57:
#line 451 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,norm)
                        ;
    break;}
case 58:
#line 455 "gram.y"
{
                        CEXFUNC(yyvsp[-1].lexptr,yyval.lexptr,invnorm)
                        ;
    break;}
case 59:
#line 459 "gram.y"
{
                        yyvsp[-3].lexptr->lx_u.lxu_value =
                                igamma(yyvsp[-3].lexptr->lx_u.lxu_value,yyvsp[-1].lexptr->lx_u.lxu_value);
                        lfree(yyvsp[-1].lexptr);
                        yyval.lexptr = yyvsp[-3].lexptr;
                        ;
    break;}
case 60:
#line 466 "gram.y"
{
                        yyvsp[-5].lexptr->lx_u.lxu_value =
                                ibeta(yyvsp[-5].lexptr->lx_u.lxu_value,yyvsp[-3].lexptr->lx_u.lxu_value,yyvsp[-1].lexptr->lx_u.lxu_value);
                        lfree(yyvsp[-3].lexptr);
                        lfree(yyvsp[-1].lexptr);
                        yyval.lexptr = yyvsp[-5].lexptr;
                        ;
    break;}
case 61:
#line 474 "gram.y"
{
                        CEXFUNC(yyvsp[0].lexptr,yyval.lexptr,-)
                        ;
    break;}
case 62:
#line 478 "gram.y"
{ yyval.lexptr = yyvsp[0].lexptr; ;
    break;}
case 63:
#line 482 "gram.y"
{ yyval.exprptr = yyvsp[-1].exprptr; ;
    break;}
case 64:
#line 484 "gram.y"
{
                        if (TWOCON(yyvsp[-2].exprptr,yyvsp[0].exprptr))
                                COMBINE(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,+=)
                        else
                                BINARY(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,O_PLUS);
                        ;
    break;}
case 65:
#line 491 "gram.y"
{
                        if (TWOCON(yyvsp[-2].exprptr,yyvsp[0].exprptr))
                                COMBINE(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,-=)
                        else
                                BINARY(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,O_MINUS);
                        ;
    break;}
case 66:
#line 498 "gram.y"
{
                        if (TWOCON(yyvsp[-2].exprptr,yyvsp[0].exprptr))
                                COMBINE(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,*=)
                        else
                                BINARY(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,O_MULT);
                        ;
    break;}
case 67:
#line 505 "gram.y"
{
                        if (TWOCON(yyvsp[-2].exprptr,yyvsp[0].exprptr))
                                COMBINE(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,/=)
                        else if (ONECON(yyvsp[0].exprptr) && yyvsp[0].exprptr->ex_value!=0.) {
                                /* division by constant */
                                yyvsp[0].exprptr->ex_value = 1./yyvsp[0].exprptr->ex_value;
                                BINARY(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,O_MULT);
                        } else
                                BINARY(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,O_DIV);
                        ;
    break;}
case 68:
#line 516 "gram.y"
{
                        double f;
			bool invert = false;

                        if (TWOCON(yyvsp[-2].exprptr,yyvsp[0].exprptr)) {
                                /* case const ^ const */
                                yyvsp[-2].exprptr->ex_value = pow(yyvsp[-2].exprptr->ex_value,yyvsp[0].exprptr->ex_value);
                                efree(yyvsp[0].exprptr);
                        } else if (ONECON(yyvsp[-2].exprptr)) {
                                if (yyvsp[-2].exprptr->ex_value == 1.) {
                                        /* case 1 ^ x */
                                        efree(yyvsp[0].exprptr);
                                        yyval.exprptr = yyvsp[-2].exprptr;
                                } else
                                        goto other;
                        } else if (!ONECON(yyvsp[0].exprptr))
                                goto other;
                        else {
                                f = yyvsp[0].exprptr->ex_value;
                                if (f < 0.) {
                                        /*
                                         * negative exponent means
                                         * to append an invert cmd
                                         */
                                        f = -f;
                                        invert = true;
                                }
                                if (f == 2.) {
                                        /* case x ^ 2 */
                                        yyvsp[0].exprptr->ex_oper = O_SQAR;
                                        concat(yyvsp[-2].exprptr,yyvsp[0].exprptr);
                                        yyval.exprptr = yyvsp[-2].exprptr;
                                } else if (f == 3.) {
                                        /* case x ^ 3 */
                                        yyvsp[0].exprptr->ex_oper = O_CUBE;
                                        concat(yyvsp[-2].exprptr,yyvsp[0].exprptr);
                                        yyval.exprptr = yyvsp[-2].exprptr;
                                } else if (f == 0.5) {
                                        /* case x ^ .5 */
                                        yyvsp[0].exprptr->ex_oper = O_SQRT;
                                        concat(yyvsp[-2].exprptr,yyvsp[0].exprptr);
                                        yyval.exprptr = yyvsp[-2].exprptr;
                                } else if (f == 1.5) {
                                        /* case x ^ 1.5 */
                                        yyvsp[0].exprptr->ex_oper = O_CUBE;
                                        BINARY(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,O_SQRT);
                                } else if (f == 1.) {
                                        /* case x ^ 1 */
                                        efree(yyvsp[0].exprptr);
                                        yyval.exprptr = yyvsp[-2].exprptr;
                                } else if (f == 0.) {
                                        /* case x ^ 0 */
                                        efree(yyvsp[-2].exprptr);
                                        yyvsp[0].exprptr->ex_value = 1.;
                                        yyval.exprptr = yyvsp[0].exprptr;
                                } else {
                        other:
                                        /* default */
                                        invert = false;
                                        BINARY(yyvsp[-2].exprptr,yyvsp[0].exprptr,yyval.exprptr,O_POWER);
                                }
                                if (invert)
                                        UNARY(yyval.exprptr,yyval.exprptr,O_INV)
                        }
                        ;
    break;}
case 69:
#line 582 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,sqrt)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_SQRT);
                        ;
    break;}
case 70:
#line 589 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr)) {
                                if (yyvsp[-1].exprptr->ex_value < 0)
                                        yyvsp[-1].exprptr->ex_value = -(yyvsp[-1].exprptr->ex_value);
                                yyval.exprptr = yyvsp[-1].exprptr;
                        } else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ABS);
                        ;
    break;}
case 71:
#line 598 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,exp)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_EXP);
                        ;
    break;}
case 72:
#line 605 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,log)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_LOG);
                        ;
    break;}
case 73:
#line 612 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,log10)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_LOG10);
                        ;
    break;}
case 74:
#line 619 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,sin)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_SIN);
                        ;
    break;}
case 75:
#line 626 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,cos)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_COS);
                        ;
    break;}
case 76:
#line 633 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,tan)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_TAN);
                        ;
    break;}
case 77:
#line 640 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,asinh)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ASINH);
                        ;
    break;}
case 78:
#line 647 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,acosh)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ACOSH);
                        ;
    break;}
case 79:
#line 654 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,atanh)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ATANH);
                        ;
    break;}
case 80:
#line 661 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,asin)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ASIN);
                        ;
    break;}
case 81:
#line 668 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,acos)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ACOS);
                        ;
    break;}
case 82:
#line 675 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,atan)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ATAN);
                        ;
    break;}
case 83:
#line 682 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,sinh)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_SINH);
                        ;
    break;}
case 84:
#line 689 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,cosh)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_COSH);
                        ;
    break;}
case 85:
#line 696 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,tanh)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_TANH);
                        ;
    break;}
case 86:
#line 703 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,floor)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_FLOOR);
                        ;
    break;}
case 87:
#line 710 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,ceil)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_CEIL);
                        ;
    break;}
case 88:
#line 717 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,j0)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_J0);
                        ;
    break;}
case 89:
#line 724 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,j1)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_J1);
                        ;
    break;}
case 90:
#line 731 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,y0)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_Y0);
                        ;
    break;}
case 91:
#line 738 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,y1)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_Y1);
                        ;
    break;}
case 92:
#line 745 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,F_LGAMMA)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_LGAMMA);
                        ;
    break;}
case 93:
#line 752 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,f_gamma)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_GAMMA);
                        ;
    break;}
case 94:
#line 759 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,erfc)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ERFC);
                        ;
    break;}
case 95:
#line 766 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,erf)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_ERF);
                        ;
    break;}
case 96:
#line 773 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,inverf)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_INVERF);
                        ;
    break;}
case 97:
#line 780 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,norm)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_NORM);
                        ;
    break;}
case 98:
#line 787 "gram.y"
{
                        if (ONECON(yyvsp[-1].exprptr))
                                CONFUNC(yyvsp[-1].exprptr,yyval.exprptr,invnorm)
                        else
                                UNARY(yyvsp[-1].exprptr,yyval.exprptr,O_INVNORM);
                        ;
    break;}
case 99:
#line 794 "gram.y"
{
                        if (TWOCON(yyvsp[-3].exprptr,yyvsp[-1].exprptr)) {
                                yyvsp[-3].exprptr->ex_value = 
				  igamma(yyvsp[-3].exprptr->ex_value,yyvsp[-1].exprptr->ex_value);
				efree(yyvsp[-1].exprptr);
				yyval.exprptr = yyvsp[-3].exprptr;
			       }
			else 
			  BINARY(yyvsp[-3].exprptr,yyvsp[-1].exprptr,yyval.exprptr,O_IGAMMA);
		        ;
    break;}
case 100:
#line 805 "gram.y"
{
                        if (THREECON(yyvsp[-5].exprptr,yyvsp[-3].exprptr,yyvsp[-1].exprptr)) {
                                yyvsp[-5].exprptr->ex_value = 
				  ibeta(yyvsp[-5].exprptr->ex_value,yyvsp[-3].exprptr->ex_value,yyvsp[-1].exprptr->ex_value);
				efree(yyvsp[-3].exprptr);
				efree(yyvsp[-1].exprptr);
				yyval.exprptr = yyvsp[-5].exprptr;
			       }
			else 
			  TERNARY(yyvsp[-5].exprptr,yyvsp[-3].exprptr,yyvsp[-1].exprptr,yyval.exprptr,O_IBETA);
		        ;
    break;}
case 101:
#line 817 "gram.y"
{
                        if (ONECON(yyvsp[0].exprptr))
                                CONFUNC(yyvsp[0].exprptr,yyval.exprptr,-)
                        else
                                UNARY(yyvsp[0].exprptr,yyval.exprptr,O_NEG);
                        ;
    break;}
case 102:
#line 824 "gram.y"
{
                        yyval.exprptr = ealloc();
                        yyval.exprptr->ex_oper = O_CONST;
                        yyval.exprptr->ex_value = yyvsp[0].lexptr->lx_u.lxu_value;
                        lfree(yyvsp[0].lexptr);
                        ;
    break;}
case 103:
#line 831 "gram.y"
{
                        yyval.exprptr = ealloc();
                        yyval.exprptr->ex_oper = O_IDENT;
                        yyval.exprptr->ex_sym = lookup(yyvsp[0].lexptr->lx_u.lxu_name);
                        lfree(yyvsp[0].lexptr);
                        ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/local/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 838 "gram.y"


int
#ifdef _HAVE_PROTOS
yyerror (const char *s)
#else
yyerror (s)
     const char *s;
#endif
{
  return 0;
}

/*
 * tack two queues of stack code together
 * e1 is connected on the tail of e0
 * There is no good way to test for circular
 * lists, hence the silly count.
 */
void
#ifdef _HAVE_PROTOS
concat (struct expr *e0, struct expr *e1)
#else
concat (e0, e1)
     struct expr *e0, *e1;
#endif
{
  int count;
  
  if (e0 == NULL || e1 == NULL) 
    panic ("NULL expression queue");

  for (count = 0; e0->ex_next != NULL; e0 = e0->ex_next)
    if (++count > 10000) 
      panic ("circular expression queue");

  e0->ex_next = e1;
}

/*
 * print an expression queue
 * called when EXAMINE is invoked on a variable (see above)
 */
void
#ifdef _HAVE_PROTOS
prexq (struct expr *ep)
#else
prexq (ep)
     struct expr *ep;
#endif
{
  const char *s;
  
  printf (" code:");
  if (ep == NULL)
    putchar ('\n');

  for (; ep != NULL; ep = ep->ex_next) 
    {
      switch (ep->ex_oper) 
	{
	case O_PLUS: s = "add"; break;
	case O_MINUS: s = "subtract"; break;
	case O_MULT: s = "multiply"; break;
	case O_DIV: s = "divide"; break;
	case O_POWER: s = "power"; break;
	case O_SQRT: s = "sqrt"; break;
	case O_EXP: s = "exp"; break;
	case O_LOG: s = "log"; break;
	case O_LOG10: s = "log10"; break;
	case O_SIN: s = "sin"; break;
	case O_COS: s = "cos"; break;
	case O_TAN: s = "cos"; break;
	case O_ASIN: s = "sin"; break;
	case O_ACOS: s = "cos"; break;
	case O_ATAN: s = "cos"; break;
	case O_NEG: s = "negate"; break;
	case O_ABS: s = "abs"; break;
	case O_SINH: s = "sinh"; break;
	case O_COSH: s = "cosh"; break;
	case O_TANH: s = "tanh"; break;
	case O_ASINH: s = "asinh"; break;
	case O_ACOSH: s = "acosh"; break;
	case O_ATANH: s = "atanh"; break;
	case O_SQAR: s = "square"; break;
	case O_CUBE: s = "cube"; break;
	case O_INV: s = "invert"; break;
	case O_FLOOR: s = "floor"; break;
	case O_CEIL: s = "ceil"; break;
	case O_J0: s = "besj0"; break;
	case O_J1: s = "besj1"; break;
	case O_Y0: s = "besy0"; break;
	case O_Y1: s = "besy1"; break;
	case O_ERF: s = "erf"; break;
	case O_ERFC: s = "erfc"; break;
	case O_INVERF: s = "inverf"; break;
	case O_LGAMMA: s = "lgamma"; break;
	case O_GAMMA: s = "gamma"; break;
	case O_NORM: s = "norm"; break;
	case O_INVNORM: s = "invnorm"; break;
	case O_IGAMMA: s = "igamma"; break;
	case O_IBETA: s = "ibeta"; break;
	case O_CONST:
	  printf ("\tpush ");
	  prval (ep->ex_value);
	  putchar ('\n');
	  s = NULL;
	  break;
	case O_IDENT:
	  printf ("\tpush \"%.*s\"\n",
		 NAMMAX, ep->ex_sym->sy_name);
	  s = NULL;
	  break;
	default: s = "unknown!";
	}

      if (s != NULL)
	printf ("\t%s\n",s);
    }
}
