/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_GRAM_H_INCLUDED
# define YY_YY_GRAM_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LABEL = 258,
    VARIABLE = 259,
    NUMBER = 260,
    TEXT = 261,
    COMMAND_LINE = 262,
    DELIMITED = 263,
    ORDINAL = 264,
    TH = 265,
    LEFT_ARROW_HEAD = 266,
    RIGHT_ARROW_HEAD = 267,
    DOUBLE_ARROW_HEAD = 268,
    LAST = 269,
    UP = 270,
    DOWN = 271,
    LEFT = 272,
    RIGHT = 273,
    BOX = 274,
    CIRCLE = 275,
    ELLIPSE = 276,
    ARC = 277,
    LINE = 278,
    ARROW = 279,
    MOVE = 280,
    SPLINE = 281,
    HEIGHT = 282,
    RADIUS = 283,
    WIDTH = 284,
    DIAMETER = 285,
    FROM = 286,
    TO = 287,
    AT = 288,
    WITH = 289,
    BY = 290,
    THEN = 291,
    DOTTED = 292,
    DASHED = 293,
    CHOP = 294,
    SAME = 295,
    INVISIBLE = 296,
    LJUST = 297,
    RJUST = 298,
    ABOVE = 299,
    BELOW = 300,
    OF = 301,
    THE = 302,
    WAY = 303,
    BETWEEN = 304,
    AND = 305,
    HERE = 306,
    DOT_N = 307,
    DOT_E = 308,
    DOT_W = 309,
    DOT_S = 310,
    DOT_NE = 311,
    DOT_SE = 312,
    DOT_NW = 313,
    DOT_SW = 314,
    DOT_C = 315,
    DOT_START = 316,
    DOT_END = 317,
    DOT_X = 318,
    DOT_Y = 319,
    DOT_HT = 320,
    DOT_WID = 321,
    DOT_RAD = 322,
    SIN = 323,
    COS = 324,
    ATAN2 = 325,
    LOG = 326,
    EXP = 327,
    SQRT = 328,
    K_MAX = 329,
    K_MIN = 330,
    INT = 331,
    RAND = 332,
    SRAND = 333,
    COPY = 334,
    THRU = 335,
    TOP = 336,
    BOTTOM = 337,
    UPPER = 338,
    LOWER = 339,
    SH = 340,
    PRINT = 341,
    CW = 342,
    CCW = 343,
    FOR = 344,
    DO = 345,
    IF = 346,
    ELSE = 347,
    ANDAND = 348,
    OROR = 349,
    NOTEQUAL = 350,
    EQUALEQUAL = 351,
    LESSEQUAL = 352,
    GREATEREQUAL = 353,
    LEFT_CORNER = 354,
    RIGHT_CORNER = 355,
    CENTER = 356,
    END = 357,
    START = 358,
    RESET = 359,
    UNTIL = 360,
    PLOT = 361,
    THICKNESS = 362,
    FILL = 363,
    ALIGNED = 364,
    SPRINTF = 365,
    COMMAND = 366,
    DEFINE = 367,
    UNDEF = 368
  };
#endif
/* Tokens.  */
#define LABEL 258
#define VARIABLE 259
#define NUMBER 260
#define TEXT 261
#define COMMAND_LINE 262
#define DELIMITED 263
#define ORDINAL 264
#define TH 265
#define LEFT_ARROW_HEAD 266
#define RIGHT_ARROW_HEAD 267
#define DOUBLE_ARROW_HEAD 268
#define LAST 269
#define UP 270
#define DOWN 271
#define LEFT 272
#define RIGHT 273
#define BOX 274
#define CIRCLE 275
#define ELLIPSE 276
#define ARC 277
#define LINE 278
#define ARROW 279
#define MOVE 280
#define SPLINE 281
#define HEIGHT 282
#define RADIUS 283
#define WIDTH 284
#define DIAMETER 285
#define FROM 286
#define TO 287
#define AT 288
#define WITH 289
#define BY 290
#define THEN 291
#define DOTTED 292
#define DASHED 293
#define CHOP 294
#define SAME 295
#define INVISIBLE 296
#define LJUST 297
#define RJUST 298
#define ABOVE 299
#define BELOW 300
#define OF 301
#define THE 302
#define WAY 303
#define BETWEEN 304
#define AND 305
#define HERE 306
#define DOT_N 307
#define DOT_E 308
#define DOT_W 309
#define DOT_S 310
#define DOT_NE 311
#define DOT_SE 312
#define DOT_NW 313
#define DOT_SW 314
#define DOT_C 315
#define DOT_START 316
#define DOT_END 317
#define DOT_X 318
#define DOT_Y 319
#define DOT_HT 320
#define DOT_WID 321
#define DOT_RAD 322
#define SIN 323
#define COS 324
#define ATAN2 325
#define LOG 326
#define EXP 327
#define SQRT 328
#define K_MAX 329
#define K_MIN 330
#define INT 331
#define RAND 332
#define SRAND 333
#define COPY 334
#define THRU 335
#define TOP 336
#define BOTTOM 337
#define UPPER 338
#define LOWER 339
#define SH 340
#define PRINT 341
#define CW 342
#define CCW 343
#define FOR 344
#define DO 345
#define IF 346
#define ELSE 347
#define ANDAND 348
#define OROR 349
#define NOTEQUAL 350
#define EQUALEQUAL 351
#define LESSEQUAL 352
#define GREATEREQUAL 353
#define LEFT_CORNER 354
#define RIGHT_CORNER 355
#define CENTER 356
#define END 357
#define START 358
#define RESET 359
#define UNTIL 360
#define PLOT 361
#define THICKNESS 362
#define FILL 363
#define ALIGNED 364
#define SPRINTF 365
#define COMMAND 366
#define DEFINE 367
#define UNDEF 368

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 70 "gram.yy"

	char *str;
	int n;
	double x;
	struct { double x, y; } pair;
	struct { double x; char *body; } if_data;
	struct { char *str; const char *filename; int lineno; } lstr;
	struct { double *v; int nv; int maxv; } dv;
	struct { double val; int is_multiplicative; } by;
	place pl;
	object *obj;
	corner crn;
	path *pth;
	object_spec *spec;
	saved_state *pstate;
	graphics_state state;
	object_type obtype;

#line 302 "gram.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAM_H_INCLUDED  */
