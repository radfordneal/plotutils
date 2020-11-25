## Makefile.am for plotutils.
## Process this file with automake to produce Makefile.in in all directories.

# plot, tek2plot, plotfont, and graph are linked with libplot.
# spline, double, and ode are not, however.  All use the routines in lib.
#
# pic2plot is written in C++ but is currently linked with libplot, not
# libplotter.  It could easily be modified to use libplotter rather than
# libplot.

if NO_LIBPLOTTER
SUBDIRS = lib spline double ode ode-examples libplot plot tek2plot plotfont graph include info doc fonts test
else
SUBDIRS = lib spline double ode ode-examples libplot plot tek2plot plotfont graph pic2plot libplotter include info doc fonts test
endif

# files with nonstandard names in this directory
EXTRA_DIST = COMPAT INSTALL.pkg INSTALL.fonts KNOWN_BUGS ONEWS PROBLEMS

# remove files that are symlinks and shouldn't be part of the distribution
dist-hook:
	rm -f $(distdir)/libplotter/*.cc
	rm -f $(distdir)/libplotter/*.h
