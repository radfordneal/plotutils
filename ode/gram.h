/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     IDENT = 259,
     SEP = 260,
     ABS = 261,
     SQRT = 262,
     EXP = 263,
     LOG = 264,
     LOG10 = 265,
     SIN = 266,
     COS = 267,
     TAN = 268,
     ASIN = 269,
     ACOS = 270,
     ATAN = 271,
     SINH = 272,
     COSH = 273,
     TANH = 274,
     ASINH = 275,
     ACOSH = 276,
     ATANH = 277,
     FLOOR = 278,
     CEIL = 279,
     J0 = 280,
     J1 = 281,
     Y0 = 282,
     Y1 = 283,
     LGAMMA = 284,
     GAMMA = 285,
     ERF = 286,
     ERFC = 287,
     INVERF = 288,
     NORM = 289,
     INVNORM = 290,
     IGAMMA = 291,
     IBETA = 292,
     EVERY = 293,
     FROM = 294,
     PRINT = 295,
     STEP = 296,
     EXAM = 297,
     UMINUS = 298
   };
#endif
#define NUMBER 258
#define IDENT 259
#define SEP 260
#define ABS 261
#define SQRT 262
#define EXP 263
#define LOG 264
#define LOG10 265
#define SIN 266
#define COS 267
#define TAN 268
#define ASIN 269
#define ACOS 270
#define ATAN 271
#define SINH 272
#define COSH 273
#define TANH 274
#define ASINH 275
#define ACOSH 276
#define ATANH 277
#define FLOOR 278
#define CEIL 279
#define J0 280
#define J1 281
#define Y0 282
#define Y1 283
#define LGAMMA 284
#define GAMMA 285
#define ERF 286
#define ERFC 287
#define INVERF 288
#define NORM 289
#define INVNORM 290
#define IGAMMA 291
#define IBETA 292
#define EVERY 293
#define FROM 294
#define PRINT 295
#define STEP 296
#define EXAM 297
#define UMINUS 298




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 85 "gram.y"
typedef union YYSTYPE {
        struct  lex     *lexptr;
        struct  expr    *exprptr;
        struct  prt     *prtptr;
        int     simple;
} YYSTYPE;
/* Line 1285 of yacc.c.  */
#line 130 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



