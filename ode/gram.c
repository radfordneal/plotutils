/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "gram.y"

/* Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1999, 2005, 2008, Free Software 
 * Foundation, Inc.
 */
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

#line 155 "gram.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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
/* Tokens.  */
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

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 85 "gram.y"

        struct  lex     *lexptr;
        struct  expr    *exprptr;
        struct  prt     *prtptr;
        int     simple;

#line 300 "gram.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAM_H_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  52
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   876

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  10
/* YYNRULES -- Number of rules.  */
#define YYNRULES  104
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  346

#define YYUNDEFTOK  2
#define YYMAXUTOK   298


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    53,     2,     2,     2,     2,     2,    50,
      55,    56,    46,    44,    51,    45,     2,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    43,     2,    54,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    48,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    52,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    49
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   110,   110,   111,   114,   116,   127,   139,   168,   175,
     187,   207,   247,   252,   262,   275,   276,   278,   283,   288,
     296,   297,   306,   307,   315,   319,   323,   327,   331,   335,
     342,   346,   352,   356,   360,   364,   368,   372,   376,   380,
     384,   388,   392,   396,   400,   404,   408,   412,   416,   420,
     424,   428,   432,   436,   440,   444,   448,   452,   456,   460,
     464,   471,   479,   483,   487,   489,   496,   503,   510,   523,
     610,   617,   628,   635,   642,   649,   656,   663,   670,   677,
     684,   691,   698,   705,   712,   719,   726,   733,   740,   747,
     754,   761,   768,   775,   782,   789,   796,   803,   810,   817,
     824,   836,   849,   856,   863
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NUMBER", "IDENT", "SEP", "ABS", "SQRT",
  "EXP", "LOG", "LOG10", "SIN", "COS", "TAN", "ASIN", "ACOS", "ATAN",
  "SINH", "COSH", "TANH", "ASINH", "ACOSH", "ATANH", "FLOOR", "CEIL", "J0",
  "J1", "Y0", "Y1", "LGAMMA", "GAMMA", "ERF", "ERFC", "INVERF", "NORM",
  "INVNORM", "IGAMMA", "IBETA", "EVERY", "FROM", "PRINT", "STEP", "EXAM",
  "'='", "'+'", "'-'", "'*'", "'/'", "'^'", "UMINUS", "'\\''", "','",
  "'~'", "'!'", "'?'", "'('", "')'", "$accept", "program", "stat",
  "prtlist", "prtitem", "prttag", "optevery", "optfrom", "cexpr", "expr", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,    61,    43,    45,    42,    47,    94,   298,
      39,    44,   126,    33,    63,    40,    41
};
# endif

#define YYPACT_NINF (-50)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     233,    -2,   -22,   -50,    12,   181,    44,   228,   -50,   -50,
     131,     8,   -16,   -36,   -50,   -50,    29,    30,   114,   115,
     122,   123,   124,   126,   128,   130,   170,   172,   176,   180,
     184,   190,   191,   192,   195,   203,   204,   206,   207,   216,
     217,   234,   236,   238,   251,   254,   255,   256,   181,   181,
     209,   177,   -50,   -50,   -50,   -50,   269,   272,   273,   274,
     287,   290,   291,   292,   305,   308,   309,   310,   323,   326,
     327,   328,   341,   344,   345,   346,   359,   362,   363,   364,
     377,   380,   381,   382,   395,   398,   399,   400,   131,   131,
     127,   131,   -50,   -50,   -50,   -50,   -50,   181,    12,    94,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   -50,   -39,   181,   181,   181,   181,   181,   181,
     -50,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   -50,   -34,   -50,   131,   131,   131,   131,
     131,   175,   828,   -50,   181,   225,   -21,   -15,   196,   232,
     239,   252,   257,   270,   275,   288,   293,   306,   311,   324,
     329,   342,   347,   360,   365,   378,   383,   396,   401,   414,
     419,   432,   437,   450,   455,   468,   780,   788,   -50,   -28,
     -28,   246,   246,   246,    -1,   473,   486,   491,   504,   509,
     522,   527,   540,   545,   558,   563,   576,   581,   594,   599,
     612,   617,   630,   635,   648,   653,   666,   671,   684,   689,
     702,   707,   720,   725,   738,   796,   804,   -50,    78,    78,
     259,   259,   259,   -50,   828,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   181,   181,   -50,   181,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     131,   131,   743,   812,   219,   756,   820,   -50,   181,   -50,
     -50,   131,   761,   774,   -50,   -50
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     4,     0,     0,     0,     0,     2,     6,
       0,     0,    15,    20,    12,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     3,   103,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    16,    17,    18,    19,    14,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    62,     0,     0,     0,     0,     0,     0,     0,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   102,     0,     5,     0,     0,     0,     0,
       0,     0,    21,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    64,    65,    66,
      67,    68,    69,     7,    23,     8,    31,    30,    32,    33,
      34,    35,    36,    37,    41,    42,    43,    44,    45,    46,
      38,    39,    40,    47,    48,    49,    50,    51,    52,    56,
      57,    54,    53,    55,    58,    59,     0,     0,     9,     0,
      71,    70,    72,    73,    74,    75,    76,    77,    81,    82,
      83,    84,    85,    86,    78,    79,    80,    87,    88,    89,
      90,    91,    92,    93,    94,    96,    95,    97,    98,    99,
       0,     0,     0,     0,     0,     0,     0,    60,     0,    10,
     100,     0,     0,     0,    61,   101
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -50,   -50,   318,   -50,   214,   -50,   -50,   -50,   -48,   -49
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,    13,    14,    96,    99,   185,    50,    90
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     132,   133,    97,     9,   298,   134,   135,   136,   137,   138,
     176,   177,   178,   179,   180,    98,    12,   218,   136,   137,
     138,    10,   257,   134,   135,   136,   137,   138,    11,   134,
     135,   136,   137,   138,    92,   266,    93,    94,    95,   173,
     174,   267,   181,   134,   135,   136,   137,   138,    51,   182,
     299,    91,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   100,   101,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   178,   179,   180,   258,   259,   260,
     261,   262,   175,   184,    54,    55,   264,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,   102,
     103,   176,   177,   178,   179,   180,    88,   104,   105,   106,
     263,   107,   140,   108,    15,   109,    89,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,   176,
     177,   178,   179,   180,   339,   110,    48,   111,    52,     1,
     265,   112,     2,     3,     1,   113,    49,     2,     3,   114,
     134,   135,   136,   137,   138,   115,   116,   117,   332,   333,
     118,   334,   268,   134,   135,   136,   137,   138,   119,   120,
     139,   121,   122,   134,   135,   136,   137,   138,     4,     5,
       6,   123,   124,     4,     5,     6,   134,   135,   136,   137,
     138,   335,   336,   134,   135,   136,   137,   138,   269,   125,
     342,   126,   343,   127,   138,   270,   134,   135,   136,   137,
     138,   134,   135,   136,   137,   138,   128,   180,   271,   129,
     130,   131,   183,   272,   134,   135,   136,   137,   138,   134,
     135,   136,   137,   138,   141,    53,   273,   142,   143,   144,
       0,   274,   134,   135,   136,   137,   138,   134,   135,   136,
     137,   138,   145,     0,   275,   146,   147,   148,     0,   276,
     134,   135,   136,   137,   138,   134,   135,   136,   137,   138,
     149,     0,   277,   150,   151,   152,     0,   278,   134,   135,
     136,   137,   138,   134,   135,   136,   137,   138,   153,     0,
     279,   154,   155,   156,     0,   280,   134,   135,   136,   137,
     138,   134,   135,   136,   137,   138,   157,     0,   281,   158,
     159,   160,     0,   282,   134,   135,   136,   137,   138,   134,
     135,   136,   137,   138,   161,     0,   283,   162,   163,   164,
       0,   284,   134,   135,   136,   137,   138,   134,   135,   136,
     137,   138,   165,     0,   285,   166,   167,   168,     0,   286,
     134,   135,   136,   137,   138,   134,   135,   136,   137,   138,
     169,     0,   287,   170,   171,   172,     0,   288,   134,   135,
     136,   137,   138,   134,   135,   136,   137,   138,     0,     0,
     289,     0,     0,     0,     0,   290,   134,   135,   136,   137,
     138,   134,   135,   136,   137,   138,     0,     0,   291,     0,
       0,     0,     0,   292,   134,   135,   136,   137,   138,   134,
     135,   136,   137,   138,     0,     0,   293,     0,     0,     0,
       0,   294,   134,   135,   136,   137,   138,   176,   177,   178,
     179,   180,     0,     0,   295,     0,     0,     0,     0,   300,
     176,   177,   178,   179,   180,   176,   177,   178,   179,   180,
       0,     0,   301,     0,     0,     0,     0,   302,   176,   177,
     178,   179,   180,   176,   177,   178,   179,   180,     0,     0,
     303,     0,     0,     0,     0,   304,   176,   177,   178,   179,
     180,   176,   177,   178,   179,   180,     0,     0,   305,     0,
       0,     0,     0,   306,   176,   177,   178,   179,   180,   176,
     177,   178,   179,   180,     0,     0,   307,     0,     0,     0,
       0,   308,   176,   177,   178,   179,   180,   176,   177,   178,
     179,   180,     0,     0,   309,     0,     0,     0,     0,   310,
     176,   177,   178,   179,   180,   176,   177,   178,   179,   180,
       0,     0,   311,     0,     0,     0,     0,   312,   176,   177,
     178,   179,   180,   176,   177,   178,   179,   180,     0,     0,
     313,     0,     0,     0,     0,   314,   176,   177,   178,   179,
     180,   176,   177,   178,   179,   180,     0,     0,   315,     0,
       0,     0,     0,   316,   176,   177,   178,   179,   180,   176,
     177,   178,   179,   180,     0,     0,   317,     0,     0,     0,
       0,   318,   176,   177,   178,   179,   180,   176,   177,   178,
     179,   180,     0,     0,   319,     0,     0,     0,     0,   320,
     176,   177,   178,   179,   180,   176,   177,   178,   179,   180,
       0,     0,   321,     0,     0,     0,     0,   322,   176,   177,
     178,   179,   180,   176,   177,   178,   179,   180,     0,     0,
     323,     0,     0,     0,     0,   324,   176,   177,   178,   179,
     180,   176,   177,   178,   179,   180,     0,     0,   325,     0,
       0,     0,     0,   326,   176,   177,   178,   179,   180,   176,
     177,   178,   179,   180,     0,     0,   327,     0,     0,     0,
       0,   328,   176,   177,   178,   179,   180,   134,   135,   136,
     137,   138,     0,     0,   329,     0,     0,     0,     0,   337,
     176,   177,   178,   179,   180,   134,   135,   136,   137,   138,
       0,     0,   340,     0,     0,     0,     0,   344,   176,   177,
     178,   179,   180,     0,   134,   135,   136,   137,   138,     0,
     345,   296,   134,   135,   136,   137,   138,     0,     0,   297,
     176,   177,   178,   179,   180,     0,     0,   330,   176,   177,
     178,   179,   180,     0,     0,   331,   134,   135,   136,   137,
     138,     0,     0,   338,   176,   177,   178,   179,   180,     0,
       0,   341,   134,   135,   136,   137,   138
};

static const yytype_int16 yycheck[] =
{
      48,    49,    38,     5,     5,    44,    45,    46,    47,    48,
      44,    45,    46,    47,    48,    51,     4,    56,    46,    47,
      48,    43,    56,    44,    45,    46,    47,    48,    50,    44,
      45,    46,    47,    48,    50,    56,    52,    53,    54,    88,
      89,    56,    91,    44,    45,    46,    47,    48,     4,    97,
      51,    43,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,    55,    55,   134,   135,   136,   137,
     138,   139,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,    46,    47,    48,   176,   177,   178,
     179,   180,     5,    39,     3,     4,   184,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    55,
      55,    44,    45,    46,    47,    48,    45,    55,    55,    55,
       5,    55,     5,    55,     3,    55,    55,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    44,
      45,    46,    47,    48,     5,    55,    45,    55,     0,     1,
       5,    55,     4,     5,     1,    55,    55,     4,     5,    55,
      44,    45,    46,    47,    48,    55,    55,    55,   296,   297,
      55,   299,    56,    44,    45,    46,    47,    48,    55,    55,
      51,    55,    55,    44,    45,    46,    47,    48,    40,    41,
      42,    55,    55,    40,    41,    42,    44,    45,    46,    47,
      48,   330,   331,    44,    45,    46,    47,    48,    56,    55,
     338,    55,   341,    55,    48,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    55,    48,    56,    55,
      55,    55,    98,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    55,     7,    56,    55,    55,    55,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    55,    -1,    56,    55,    55,    55,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      55,    -1,    56,    55,    55,    55,    -1,    56,    44,    45,
      46,    47,    48,    44,    45,    46,    47,    48,    55,    -1,
      56,    55,    55,    55,    -1,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    55,    -1,    56,    55,
      55,    55,    -1,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    55,    -1,    56,    55,    55,    55,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    55,    -1,    56,    55,    55,    55,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      55,    -1,    56,    55,    55,    55,    -1,    56,    44,    45,
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
      48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
      46,    47,    48,    -1,    44,    45,    46,    47,    48,    -1,
      56,    51,    44,    45,    46,    47,    48,    -1,    -1,    51,
      44,    45,    46,    47,    48,    -1,    -1,    51,    44,    45,
      46,    47,    48,    -1,    -1,    51,    44,    45,    46,    47,
      48,    -1,    -1,    51,    44,    45,    46,    47,    48,    -1,
      -1,    51,    44,    45,    46,    47,    48
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     4,     5,    40,    41,    42,    58,    59,     5,
      43,    50,     4,    60,    61,     3,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    45,    55,
      65,     4,     0,    59,     3,     4,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    45,    55,
      66,    43,    50,    52,    53,    54,    62,    38,    51,    63,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    65,    65,    44,    45,    46,    47,    48,    51,
       5,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    66,    66,     5,    44,    45,    46,    47,
      48,    66,    65,    61,    39,    64,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    56,    65,
      65,    65,    65,    65,    65,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    56,    66,    66,
      66,    66,    66,     5,    65,     5,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    51,    51,     5,    51,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      51,    51,    65,    65,    65,    66,    66,    56,    51,     5,
      56,    51,    65,    66,    56,    56
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    57,    58,    58,    59,    59,    59,    59,    59,    59,
      59,    59,    60,    60,    61,    62,    62,    62,    62,    62,
      63,    63,    64,    64,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     4,     2,     5,     5,     5,
       7,     3,     1,     3,     2,     0,     1,     1,     1,     1,
       0,     2,     0,     2,     3,     3,     3,     3,     3,     3,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       6,     8,     2,     1,     3,     3,     3,     3,     3,     3,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       6,     8,     2,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 4:
#line 115 "gram.y"
                        { lfree((yyvsp[0].lexptr)); }
#line 1783 "gram.c"
    break;

  case 5:
#line 117 "gram.y"
                        {
			  struct sym *sp;
			  
			  sp = lookup((yyvsp[-3].lexptr)->lx_u.lxu_name);
			  sp->sy_value = eval((yyvsp[-1].exprptr));
			  sp->sy_flags |= SF_INIT;
			  lfree((yyvsp[-3].lexptr));
			  efree((yyvsp[-1].exprptr));
			  lfree((yyvsp[0].lexptr));
                        }
#line 1798 "gram.c"
    break;

  case 6:
#line 128 "gram.y"
                        {
			  if (errmess == NULL)
			    errmess = "syntax error";
			  fprintf (stderr, "%s:%s:%d: %s\n", 
				   progname, filename,
				   ((yyvsp[0].lexptr)->lx_lino), errmess);
			  errmess = NULL;
			  lfree((yyvsp[0].lexptr));
			  yyerrok;
			  yyclearin;
                        }
#line 1814 "gram.c"
    break;

  case 7:
#line 140 "gram.y"
                        {
			  struct sym *sp;
			  struct prt *pp, *qp;
			  
			  sp = lookup((yyvsp[-4].lexptr)->lx_u.lxu_name);
			  efree(sp->sy_expr);
			  sp->sy_expr = (yyvsp[-1].exprptr);
			  sp->sy_flags |= SF_ISEQN;
			  if (!sawprint) 
			    {
			      for (pp=pqueue; pp!=NULL; pp=pp->pr_link)
				if (pp->pr_sym == sp)
				  goto found;
			      pp = palloc();
			      pp->pr_sym = sp;
			      if (pqueue == NULL)
				pqueue = pp;
			      else 
				{
				  for (qp=pqueue; qp->pr_link!=NULL; )
				    qp = qp->pr_link;
				  qp->pr_link = pp;
                                }
			    }
			found:
			  lfree((yyvsp[-4].lexptr));
			  lfree((yyvsp[0].lexptr));
                        }
#line 1847 "gram.c"
    break;

  case 8:
#line 169 "gram.y"
                        {
			  sawprint = true;
			  prerr = erritem;
			  erritem = false;
			  lfree((yyvsp[0].lexptr));
                        }
#line 1858 "gram.c"
    break;

  case 9:
#line 176 "gram.y"
                        {
			  lfree((yyvsp[0].lexptr));
			  tstart = (yyvsp[-3].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[-3].lexptr));
			  tstop = (yyvsp[-1].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[-1].lexptr));
			  if (!conflag)
			    startstep();
			  solve();
			  sawstep = true;
                        }
#line 1874 "gram.c"
    break;

  case 10:
#line 188 "gram.y"
                        {
			  double savstep;
			  bool savconflag;
			  
			  lfree((yyvsp[0].lexptr));
			  tstart = (yyvsp[-5].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[-5].lexptr));
			  tstop = (yyvsp[-3].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[-3].lexptr));
			  savstep = tstep;
			  tstep = (yyvsp[-1].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[-1].lexptr));
			  savconflag = conflag;
			  conflag = true;
			  solve();
			  tstep = savstep;
			  conflag = savconflag;
			  sawstep = true;
                        }
#line 1898 "gram.c"
    break;

  case 11:
#line 208 "gram.y"
                        {
			  struct sym *sp;
			  
			  lfree((yyvsp[0].lexptr));
			  sp = lookup((yyvsp[-1].lexptr)->lx_u.lxu_name);
			  lfree((yyvsp[-1].lexptr));
			  printf ("\"%.*s\" is ",NAMMAX,sp->sy_name);
			  switch (sp->sy_flags & SF_DEPV)
			    {
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
                        }
#line 1940 "gram.c"
    break;

  case 12:
#line 248 "gram.y"
                        {
			  pfree(pqueue);
			  pqueue = (yyvsp[0].prtptr);
                        }
#line 1949 "gram.c"
    break;

  case 13:
#line 253 "gram.y"
                        {
			  struct prt *pp;
			  
			  for (pp=pqueue; pp->pr_link!=NULL; pp=pp->pr_link)
			    ;
			  pp->pr_link = (yyvsp[0].prtptr);
                        }
#line 1961 "gram.c"
    break;

  case 14:
#line 263 "gram.y"
                        {
			  struct prt *pp;
			  
			  pp = palloc();
			  pp->pr_sym = lookup((yyvsp[-1].lexptr)->lx_u.lxu_name);
			  pp->pr_which = (ent_cell)((yyvsp[0].simple));
			  lfree((yyvsp[-1].lexptr));
			  (yyval.prtptr) = pp;
                        }
#line 1975 "gram.c"
    break;

  case 15:
#line 275 "gram.y"
                        { (yyval.simple) = P_VALUE; }
#line 1981 "gram.c"
    break;

  case 16:
#line 277 "gram.y"
                        { (yyval.simple) = P_PRIME; }
#line 1987 "gram.c"
    break;

  case 17:
#line 279 "gram.y"
                        {
			  (yyval.simple) = P_ACERR;
			  erritem = true;
                        }
#line 1996 "gram.c"
    break;

  case 18:
#line 284 "gram.y"
                        {
			  (yyval.simple) = P_ABERR;
			  erritem = true;
                        }
#line 2005 "gram.c"
    break;

  case 19:
#line 289 "gram.y"
                        {
			  (yyval.simple) = P_SSERR;
			  erritem = true;
                        }
#line 2014 "gram.c"
    break;

  case 20:
#line 296 "gram.y"
                        { sawevery = false; }
#line 2020 "gram.c"
    break;

  case 21:
#line 298 "gram.y"
                        {
                        sawevery = true;
                        tevery = IROUND((yyvsp[0].lexptr)->lx_u.lxu_value);
                        lfree((yyvsp[0].lexptr));
                        }
#line 2030 "gram.c"
    break;

  case 22:
#line 306 "gram.y"
                        { sawfrom = false; }
#line 2036 "gram.c"
    break;

  case 23:
#line 308 "gram.y"
                        {
			  sawfrom = true;
			  tfrom = (yyvsp[0].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[0].lexptr));
                        }
#line 2046 "gram.c"
    break;

  case 24:
#line 316 "gram.y"
                        {
			  (yyval.lexptr) = (yyvsp[-1].lexptr);
                        }
#line 2054 "gram.c"
    break;

  case 25:
#line 320 "gram.y"
                        {
			  CEXOP((yyvsp[-2].lexptr),(yyvsp[0].lexptr),(yyval.lexptr),+=)
                        }
#line 2062 "gram.c"
    break;

  case 26:
#line 324 "gram.y"
                        {
			  CEXOP((yyvsp[-2].lexptr),(yyvsp[0].lexptr),(yyval.lexptr),-=)
                        }
#line 2070 "gram.c"
    break;

  case 27:
#line 328 "gram.y"
                        {
			  CEXOP((yyvsp[-2].lexptr),(yyvsp[0].lexptr),(yyval.lexptr),*=)
                        }
#line 2078 "gram.c"
    break;

  case 28:
#line 332 "gram.y"
                        {
			  CEXOP((yyvsp[-2].lexptr),(yyvsp[0].lexptr),(yyval.lexptr),/=)
                        }
#line 2086 "gram.c"
    break;

  case 29:
#line 336 "gram.y"
                        {
			  (yyvsp[-2].lexptr)->lx_u.lxu_value =
			    pow((yyvsp[-2].lexptr)->lx_u.lxu_value,(yyvsp[0].lexptr)->lx_u.lxu_value);
			  lfree((yyvsp[0].lexptr));
			  (yyval.lexptr) = (yyvsp[-2].lexptr);
                        }
#line 2097 "gram.c"
    break;

  case 30:
#line 343 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),sqrt)
                        }
#line 2105 "gram.c"
    break;

  case 31:
#line 347 "gram.y"
                        {
			  if ((yyvsp[-1].lexptr)->lx_u.lxu_value < 0)
			    (yyvsp[-1].lexptr)->lx_u.lxu_value = -((yyvsp[-1].lexptr)->lx_u.lxu_value);
			  (yyval.lexptr) = (yyvsp[-1].lexptr);
                        }
#line 2115 "gram.c"
    break;

  case 32:
#line 353 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),exp)
                        }
#line 2123 "gram.c"
    break;

  case 33:
#line 357 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),log)
                        }
#line 2131 "gram.c"
    break;

  case 34:
#line 361 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),log10)
                        }
#line 2139 "gram.c"
    break;

  case 35:
#line 365 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),sin)
                        }
#line 2147 "gram.c"
    break;

  case 36:
#line 369 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),cos)
                        }
#line 2155 "gram.c"
    break;

  case 37:
#line 373 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),tan)
                        }
#line 2163 "gram.c"
    break;

  case 38:
#line 377 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),asinh)
                        }
#line 2171 "gram.c"
    break;

  case 39:
#line 381 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),acosh)
                        }
#line 2179 "gram.c"
    break;

  case 40:
#line 385 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),atanh)
                        }
#line 2187 "gram.c"
    break;

  case 41:
#line 389 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),asin)
                        }
#line 2195 "gram.c"
    break;

  case 42:
#line 393 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),acos)
                        }
#line 2203 "gram.c"
    break;

  case 43:
#line 397 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),atan)
                        }
#line 2211 "gram.c"
    break;

  case 44:
#line 401 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),sinh)
                        }
#line 2219 "gram.c"
    break;

  case 45:
#line 405 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),cosh)
                        }
#line 2227 "gram.c"
    break;

  case 46:
#line 409 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),tanh)
                        }
#line 2235 "gram.c"
    break;

  case 47:
#line 413 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),floor)
                        }
#line 2243 "gram.c"
    break;

  case 48:
#line 417 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),ceil)
                        }
#line 2251 "gram.c"
    break;

  case 49:
#line 421 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),j0)
                        }
#line 2259 "gram.c"
    break;

  case 50:
#line 425 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),j1)
                        }
#line 2267 "gram.c"
    break;

  case 51:
#line 429 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),y0)
                        }
#line 2275 "gram.c"
    break;

  case 52:
#line 433 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),y1)
                        }
#line 2283 "gram.c"
    break;

  case 53:
#line 437 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),erfc)
                        }
#line 2291 "gram.c"
    break;

  case 54:
#line 441 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),erf)
                        }
#line 2299 "gram.c"
    break;

  case 55:
#line 445 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),inverf)
                        }
#line 2307 "gram.c"
    break;

  case 56:
#line 449 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),F_LGAMMA)
                        }
#line 2315 "gram.c"
    break;

  case 57:
#line 453 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),f_gamma)
                        }
#line 2323 "gram.c"
    break;

  case 58:
#line 457 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),norm)
                        }
#line 2331 "gram.c"
    break;

  case 59:
#line 461 "gram.y"
                        {
			  CEXFUNC((yyvsp[-1].lexptr),(yyval.lexptr),invnorm)
                        }
#line 2339 "gram.c"
    break;

  case 60:
#line 465 "gram.y"
                        {
			  (yyvsp[-3].lexptr)->lx_u.lxu_value =
			    igamma((yyvsp[-3].lexptr)->lx_u.lxu_value,(yyvsp[-1].lexptr)->lx_u.lxu_value);
			  lfree((yyvsp[-1].lexptr));
			  (yyval.lexptr) = (yyvsp[-3].lexptr);
                        }
#line 2350 "gram.c"
    break;

  case 61:
#line 472 "gram.y"
                        {
			  (yyvsp[-5].lexptr)->lx_u.lxu_value =
			    ibeta((yyvsp[-5].lexptr)->lx_u.lxu_value,(yyvsp[-3].lexptr)->lx_u.lxu_value,(yyvsp[-1].lexptr)->lx_u.lxu_value);
			  lfree((yyvsp[-3].lexptr));
			  lfree((yyvsp[-1].lexptr));
			  (yyval.lexptr) = (yyvsp[-5].lexptr);
                        }
#line 2362 "gram.c"
    break;

  case 62:
#line 480 "gram.y"
                        {
			  CEXFUNC((yyvsp[0].lexptr),(yyval.lexptr),-)
                        }
#line 2370 "gram.c"
    break;

  case 63:
#line 484 "gram.y"
                        { (yyval.lexptr) = (yyvsp[0].lexptr); }
#line 2376 "gram.c"
    break;

  case 64:
#line 488 "gram.y"
                        { (yyval.exprptr) = (yyvsp[-1].exprptr); }
#line 2382 "gram.c"
    break;

  case 65:
#line 490 "gram.y"
                        {
			  if (TWOCON((yyvsp[-2].exprptr),(yyvsp[0].exprptr)))
			    COMBINE((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),+=)
			  else
			    BINARY((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),O_PLUS);
                        }
#line 2393 "gram.c"
    break;

  case 66:
#line 497 "gram.y"
                        {
			  if (TWOCON((yyvsp[-2].exprptr),(yyvsp[0].exprptr)))
			    COMBINE((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),-=)
			  else
			    BINARY((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),O_MINUS);
                        }
#line 2404 "gram.c"
    break;

  case 67:
#line 504 "gram.y"
                        {
			  if (TWOCON((yyvsp[-2].exprptr),(yyvsp[0].exprptr)))
			    COMBINE((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),*=)
			  else
			    BINARY((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),O_MULT);
                        }
#line 2415 "gram.c"
    break;

  case 68:
#line 511 "gram.y"
                        {
			  if (TWOCON((yyvsp[-2].exprptr),(yyvsp[0].exprptr)))
			    COMBINE((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),/=)
			  else if (ONECON((yyvsp[0].exprptr)) && (yyvsp[0].exprptr)->ex_value!=0.) 
			    {
			      /* division by constant */
			      (yyvsp[0].exprptr)->ex_value = 1./(yyvsp[0].exprptr)->ex_value;
			      BINARY((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),O_MULT);
			    } 
			  else
			    BINARY((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),O_DIV);
                        }
#line 2432 "gram.c"
    break;

  case 69:
#line 524 "gram.y"
                        {
			  double f;
			  bool invert = false;
			  
			  if (TWOCON((yyvsp[-2].exprptr),(yyvsp[0].exprptr))) 
			    {
			      /* case const ^ const */
			      (yyvsp[-2].exprptr)->ex_value = pow((yyvsp[-2].exprptr)->ex_value,(yyvsp[0].exprptr)->ex_value);
			      efree((yyvsp[0].exprptr));
			    } 
			  else if (ONECON((yyvsp[-2].exprptr))) 
			    {
			      if ((yyvsp[-2].exprptr)->ex_value == 1.)
				{
				  /* case 1 ^ x */
				  efree((yyvsp[0].exprptr));
				  (yyval.exprptr) = (yyvsp[-2].exprptr);
                                }
			      else
				goto other;
			    }
			  else if (!ONECON((yyvsp[0].exprptr)))
			    goto other;
			  else 
			    {
			      f = (yyvsp[0].exprptr)->ex_value;
			      if (f < 0.) 
				{
				  /*
				   * negative exponent means
				   * to append an invert cmd
				   */
				  f = -f;
				  invert = true;
                                }
			      if (f == 2.) 
				{
				  /* case x ^ 2 */
				  (yyvsp[0].exprptr)->ex_oper = O_SQAR;
				  concat((yyvsp[-2].exprptr),(yyvsp[0].exprptr));
				  (yyval.exprptr) = (yyvsp[-2].exprptr);
                                }
			      else if (f == 3.) 
				{
				  /* case x ^ 3 */
				  (yyvsp[0].exprptr)->ex_oper = O_CUBE;
				  concat((yyvsp[-2].exprptr),(yyvsp[0].exprptr));
				  (yyval.exprptr) = (yyvsp[-2].exprptr);
                                }
			      else if (f == 0.5) 
				{
				  /* case x ^ .5 */
				  (yyvsp[0].exprptr)->ex_oper = O_SQRT;
				  concat((yyvsp[-2].exprptr),(yyvsp[0].exprptr));
				  (yyval.exprptr) = (yyvsp[-2].exprptr);
                                }
			      else if (f == 1.5) 
				{
				  /* case x ^ 1.5 */
				  (yyvsp[0].exprptr)->ex_oper = O_CUBE;
				  BINARY((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),O_SQRT);
                                }
			      else if (f == 1.) 
				{
				  /* case x ^ 1 */
				  efree((yyvsp[0].exprptr));
				  (yyval.exprptr) = (yyvsp[-2].exprptr);
                                }
			      else if (f == 0.) 
				{
				  /* case x ^ 0 */
				  efree((yyvsp[-2].exprptr));
				  (yyvsp[0].exprptr)->ex_value = 1.;
				  (yyval.exprptr) = (yyvsp[0].exprptr);
                                } 
			      else 
				{
				other:
				  /* default */
				  invert = false;
				  BINARY((yyvsp[-2].exprptr),(yyvsp[0].exprptr),(yyval.exprptr),O_POWER);
                                }
			      if (invert)
				UNARY((yyval.exprptr),(yyval.exprptr),O_INV)
			    }
                        }
#line 2523 "gram.c"
    break;

  case 70:
#line 611 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),sqrt)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_SQRT);
                        }
#line 2534 "gram.c"
    break;

  case 71:
#line 618 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr))) 
			    {
			      if ((yyvsp[-1].exprptr)->ex_value < 0)
				(yyvsp[-1].exprptr)->ex_value = -((yyvsp[-1].exprptr)->ex_value);
			      (yyval.exprptr) = (yyvsp[-1].exprptr);
			  } 
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ABS);
                        }
#line 2549 "gram.c"
    break;

  case 72:
#line 629 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),exp)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_EXP);
                        }
#line 2560 "gram.c"
    break;

  case 73:
#line 636 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),log)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_LOG);
                        }
#line 2571 "gram.c"
    break;

  case 74:
#line 643 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),log10)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_LOG10);
                        }
#line 2582 "gram.c"
    break;

  case 75:
#line 650 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),sin)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_SIN);
                        }
#line 2593 "gram.c"
    break;

  case 76:
#line 657 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),cos)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_COS);
                        }
#line 2604 "gram.c"
    break;

  case 77:
#line 664 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),tan)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_TAN);
                        }
#line 2615 "gram.c"
    break;

  case 78:
#line 671 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),asinh)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ASINH);
                        }
#line 2626 "gram.c"
    break;

  case 79:
#line 678 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),acosh)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ACOSH);
                        }
#line 2637 "gram.c"
    break;

  case 80:
#line 685 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),atanh)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ATANH);
                        }
#line 2648 "gram.c"
    break;

  case 81:
#line 692 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),asin)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ASIN);
                        }
#line 2659 "gram.c"
    break;

  case 82:
#line 699 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),acos)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ACOS);
                        }
#line 2670 "gram.c"
    break;

  case 83:
#line 706 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),atan)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ATAN);
                        }
#line 2681 "gram.c"
    break;

  case 84:
#line 713 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),sinh)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_SINH);
                        }
#line 2692 "gram.c"
    break;

  case 85:
#line 720 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),cosh)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_COSH);
                        }
#line 2703 "gram.c"
    break;

  case 86:
#line 727 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),tanh)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_TANH);
                        }
#line 2714 "gram.c"
    break;

  case 87:
#line 734 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),floor)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_FLOOR);
                        }
#line 2725 "gram.c"
    break;

  case 88:
#line 741 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),ceil)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_CEIL);
                        }
#line 2736 "gram.c"
    break;

  case 89:
#line 748 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),j0)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_J0);
                        }
#line 2747 "gram.c"
    break;

  case 90:
#line 755 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),j1)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_J1);
                        }
#line 2758 "gram.c"
    break;

  case 91:
#line 762 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),y0)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_Y0);
                        }
#line 2769 "gram.c"
    break;

  case 92:
#line 769 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),y1)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_Y1);
                        }
#line 2780 "gram.c"
    break;

  case 93:
#line 776 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),F_LGAMMA)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_LGAMMA);
                        }
#line 2791 "gram.c"
    break;

  case 94:
#line 783 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),f_gamma)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_GAMMA);
                        }
#line 2802 "gram.c"
    break;

  case 95:
#line 790 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),erfc)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ERFC);
                        }
#line 2813 "gram.c"
    break;

  case 96:
#line 797 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),erf)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_ERF);
                        }
#line 2824 "gram.c"
    break;

  case 97:
#line 804 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),inverf)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_INVERF);
                        }
#line 2835 "gram.c"
    break;

  case 98:
#line 811 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),norm)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_NORM);
                        }
#line 2846 "gram.c"
    break;

  case 99:
#line 818 "gram.y"
                        {
			  if (ONECON((yyvsp[-1].exprptr)))
			    CONFUNC((yyvsp[-1].exprptr),(yyval.exprptr),invnorm)
			  else
			    UNARY((yyvsp[-1].exprptr),(yyval.exprptr),O_INVNORM);
                        }
#line 2857 "gram.c"
    break;

  case 100:
#line 825 "gram.y"
                        {
			  if (TWOCON((yyvsp[-3].exprptr),(yyvsp[-1].exprptr))) 
			    {
			      (yyvsp[-3].exprptr)->ex_value = 
				igamma((yyvsp[-3].exprptr)->ex_value,(yyvsp[-1].exprptr)->ex_value);
			      efree((yyvsp[-1].exprptr));
			      (yyval.exprptr) = (yyvsp[-3].exprptr);
			    }
			  else 
			    BINARY((yyvsp[-3].exprptr),(yyvsp[-1].exprptr),(yyval.exprptr),O_IGAMMA);
		        }
#line 2873 "gram.c"
    break;

  case 101:
#line 837 "gram.y"
                        {
			  if (THREECON((yyvsp[-5].exprptr),(yyvsp[-3].exprptr),(yyvsp[-1].exprptr))) 
			    {
			      (yyvsp[-5].exprptr)->ex_value = 
				ibeta((yyvsp[-5].exprptr)->ex_value,(yyvsp[-3].exprptr)->ex_value,(yyvsp[-1].exprptr)->ex_value);
			      efree((yyvsp[-3].exprptr));
			      efree((yyvsp[-1].exprptr));
			      (yyval.exprptr) = (yyvsp[-5].exprptr);
			    }
			  else 
			    TERNARY((yyvsp[-5].exprptr),(yyvsp[-3].exprptr),(yyvsp[-1].exprptr),(yyval.exprptr),O_IBETA);
		        }
#line 2890 "gram.c"
    break;

  case 102:
#line 850 "gram.y"
                        {
			  if (ONECON((yyvsp[0].exprptr)))
			    CONFUNC((yyvsp[0].exprptr),(yyval.exprptr),-)
			  else
			    UNARY((yyvsp[0].exprptr),(yyval.exprptr),O_NEG);
                        }
#line 2901 "gram.c"
    break;

  case 103:
#line 857 "gram.y"
                        {
			  (yyval.exprptr) = ealloc();
			  (yyval.exprptr)->ex_oper = O_CONST;
			  (yyval.exprptr)->ex_value = (yyvsp[0].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[0].lexptr));
                        }
#line 2912 "gram.c"
    break;

  case 104:
#line 864 "gram.y"
                        {
			  (yyval.exprptr) = ealloc();
			  (yyval.exprptr)->ex_oper = O_IDENT;
			  (yyval.exprptr)->ex_sym = lookup((yyvsp[0].lexptr)->lx_u.lxu_name);
			  lfree((yyvsp[0].lexptr));
                        }
#line 2923 "gram.c"
    break;


#line 2927 "gram.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 871 "gram.y"


int
yyerror (const char *s)
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
concat (struct expr *e0, struct expr *e1)
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
prexq (const struct expr *ep)
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
