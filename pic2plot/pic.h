// -*- C++ -*-

// include system headers: stdio, string, math (+ M_PI, M_SQRT2), stdlib, errno
#include "sys-defines.h"

// include libgroff headers
#include "assert.h"
#include "cset.h"
#include "lib.h"
#include "stringclass.h"
#include "errarg.h"
#include "error.h"
#include "position.h"
#include "text.h"
#include "output.h"

class input 
{
public:
  // ctor, dtor
  input ();
  virtual ~input ();
  // public functions (all virtual)
  virtual int get (void) = 0;
  virtual int peek (void) = 0;
  virtual int get_location (const char **filenamep, int *linenop);
  // friend classes
  friend class input_stack;
  friend class copy_rest_thru_input;
private:
  input *next;
};

class file_input : public input 
{
public:
  // ctor, dtor
  file_input (FILE *, const char *);
  ~file_input ();
  // public functions
  int get (void);
  int peek (void);
  int get_location(const char **filenamep, int *linenop);
private:
  FILE *fp;
  const char *filename;
  int lineno;
  string line;
  const char *ptr;
  int read_line (void);
};

// interface to lexer in lex.cc
extern int yylex (void);
extern void copy_file_thru (const char *filename, const char *body, const char *until);
extern void copy_rest_thru (const char *body, const char *until);
extern void do_copy (const char *filename);
extern void do_for (char *var, double from, double to, int by_is_multiplicative, double by, char *body);
extern void do_lookahead (void);
extern void lex_cleanup (void);
extern void lex_error (const char *message, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);
extern void lex_init (input *top);
extern void lex_warning (const char *message, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);
extern void push_body (const char *s);
extern void yyerror (const char *s);

// interface to parser in gram.cc
extern int yyparse();
extern void parse_init (void);
extern void parse_cleanup (void);
extern int delim_flag;		// read-only variable

// command-line flags in main.cc, used by lexer, parser, or driver
extern int command_char;
extern int compatible_flag;
extern int safer_flag;
extern int no_centering_flag;
