# Configure template for GNU libxmi.
# This is hugely cut down from the template for the GNU plotutils package.
# Copyright (C) 1989-2000 Free Software Foundation, Inc.
# Process this file with autoconf to produce a configure script.

AC_INIT([GNU libxmi],[1.3],[bug-libxmi@gnu.org])
AC_PREREQ(2.59)
AC_CONFIG_SRCDIR(xmi.h)
AM_INIT_AUTOMAKE([libxmi],[1.3])
AM_CONFIG_HEADER(config.h:config.hin)

# The following will be defined in config.h, if appropriate.
AH_TEMPLATE([HAVE_BOOL_IN_CC], 
	[Define to 1 if your C compiler supports the bool type.])

# Did installer set the CFLAGS and CXXFLAGS environ variables before
# running configure?  Our default CFLAGS and CXXFLAGS differ from
# autoconf's, but we won't override installer-specified values.
if test "x$CFLAGS" = "x"; then
  CFLAGS_NOT_SET_BY_INSTALLER="yes"
else
  CFLAGS_NOT_SET_BY_INSTALLER="no"
fi

# We now use libtool to make and installed a shared library.  This should
# invoke the tests AC_PROG_CC, AC_OBJEXT, AC_PROG_INSTALL, and
# AC_PROG_MAKE_SET, or equivalents, so we don't perform them explicitly.
# Note: this can apparently alter CFLAGS, on a few platforms, e.g., on
# SCO OpenServer 5 (i.e. *-*-sco3.2v5*), "-belf" is added.
AM_PROG_LIBTOOL

# Determine extension (e.g. ".exe") on executables, if any.
AC_EXEEXT

# Compiler characteristics and typedefs.
AC_C_CONST
AC_TYPE_SIZE_T

# Does the C compiler support the bool datatype?  If it's really
# a C++ compiler, which could be arranged by `CC=g++ ./configure',
# then it could.
AC_MSG_CHECKING(whether ${CC-gcc} supports bool)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([bool foo = true;],[])],[AC_MSG_RESULT(yes); AC_DEFINE(HAVE_BOOL_IN_CC)],[AC_MSG_RESULT(no)])

# Checks for header files: ANSI C, POSIX, and nonstandard Unix headers.
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h limits.h float.h)
AC_CHECK_HEADERS(malloc.h strings.h values.h)
AC_CHECK_HEADERS(sys/stdtypes.h)

# Other library functions.
AC_CHECK_FUNCS(memmove)

# This may do some good.
LIBS="$LIBS -lm"

# Override default autoconf value "-g -O2" or "-g" for CFLAGS, set by
# AC_PROG_CC.  Provided, that is, installer didn't set CFLAGS via an
# environment variable before running configure.  We don't use "-g" when
# compiling libxmi, since a debugging version of libxmi would be large.

# Note: on a very few platforms where libtool adds a command-line option
# to CFLAGS (see above; this includes SCO OpenServer 5, where "-belf" is
# added), this way of doing things will not work, i.e., the installer
# will need to add by hand the option that libtool would have added.
# E.g., on SCO OpenServer5 the installer may need to set the environment
# variable CFLAGS to "-O -belf" or "-O2 -belf".

if test "x$CFLAGS_NOT_SET_BY_INSTALLER" = "xyes"; then
if test "x$GCC" = "xyes"; then
  CFLAGS="-O2"
else
  CFLAGS="-O"
fi
fi

# Check for gcc strength-reduce bug (taken from WINE config).  Could do
# the same for g++, but we'll assume anyone doing any C++ compiling has
# installed a modern compiler.
if test "x${GCC}" = "xyes"; then
  AC_CACHE_CHECK(for gcc strength-reduce bug, ac_cv_c_gcc_strength_bug,
                 AC_RUN_IFELSE([AC_LANG_SOURCE([[
int main()
{
  static int Array[[3]];
  unsigned int B = 3;
  int i;
  for (i=0; i < B; i++)
    Array[[i]] = i - 3;
  exit (Array[[1]] != -2);
}]])],
[ac_cv_c_gcc_strength_bug="no"],[ac_cv_c_gcc_strength_bug="yes"],[ac_cv_c_gcc_strength_bug="yes"]) )
  if test "$ac_cv_c_gcc_strength_bug" = "yes"
  then
    CFLAGS="$CFLAGS -fno-strength-reduce"
  fi
fi

AC_CONFIG_FILES(Makefile info/Makefile)
AC_OUTPUT
