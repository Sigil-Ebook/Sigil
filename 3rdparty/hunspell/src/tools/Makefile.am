bin_PROGRAMS=analyze chmorph hunspell munch unmunch hzip hunzip

AM_CPPFLAGS=-I${top_builddir}/src/hunspell -I${top_srcdir}/src/hunspell -I${top_srcdir}/src/parsers

hzip_SOURCES=hzip.cxx
hunzip_SOURCES=hunzip.cxx
hunzip_LDADD = ../hunspell/libhunspell-1.7.la

munch_SOURCES=munch.cxx munch.h
unmunch_SOURCES=unmunch.cxx unmunch.h

example_SOURCES=example.cxx
example_LDADD = ../hunspell/libhunspell-1.7.la

hunspell_SOURCES=hunspell.cxx
nodist_hunspell_SOURCES= ../../config.h
hunspell_LDADD = @LIBINTL@ @LIBICONV@ ../parsers/libparsers.a \
	../hunspell/libhunspell-1.7.la @CURSESLIB@ @READLINELIB@

analyze_SOURCES=analyze.cxx
analyze_LDADD = ../hunspell/libhunspell-1.7.la

chmorph_SOURCES=chmorph.cxx
chmorph_LDADD = ../parsers/libparsers.a ../hunspell/libhunspell-1.7.la

noinst_PROGRAMS=example 

dist_bin_SCRIPTS=makealias affixcompress wordforms ispellaff2myspell wordlist2hunspell
