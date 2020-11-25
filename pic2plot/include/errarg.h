// -*- C++ -*-
/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

class errarg {
 public:
  errarg();
  errarg(const char *);
  errarg(char);
  errarg(unsigned char);
  errarg(int);
  errarg(double);
  int empty() const;
  void print() const;
private:
  enum { EMPTY, STRING, CHAR, INTEGER, DOUBLE } type;
  union {
    const char *s;
    int n;
    char c;
    double d;
  };
};

extern errarg empty_errarg;

extern void errprint(const char *,
		     const errarg &arg1 = empty_errarg,
		     const errarg &arg2 = empty_errarg,
		     const errarg &arg3 = empty_errarg);

