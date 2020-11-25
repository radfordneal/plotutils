libplotdatadir = $(datadir)/libplot
libplotdata_DATA = README colors.txt demo-page h-demo.c h-fonts.doc h-glyphs.doc hershey.bib kana.doc kanji.doc

# files with nonstandard names in this directory
EXTRA_DIST = colors.txt demo-page h-demo.c h-fonts.doc h-glyphs.doc hershey.bib kana.doc kanji.doc

# remove documentation files installed by previous versions of the package,
# with obsolete names

OBSOLETEDOC = hershey-demo.c hershey-fonts hershey-number hershey.doc

install-data-hook:
	-for f in $(OBSOLETEDOC) ; do rm -f $(datadir)/libplot/$$f; done
