typedef union {
        struct  lex     *lexptr;
        struct  expr    *exprptr;
        struct  prt     *prtptr;
        int     simple;
} YYSTYPE;
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


extern YYSTYPE yylval;
