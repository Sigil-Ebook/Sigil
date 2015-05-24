noinst_LIBRARIES=libparsers.a
libparsers_a_SOURCES=firstparser.cxx xmlparser.cxx \
		      latexparser.cxx  manparser.cxx \
		      textparser.cxx htmlparser.cxx \
		      odfparser.cxx

noinst_PROGRAMS=testparser
testparser_SOURCES=firstparser.cxx firstparser.hxx xmlparser.cxx \
		      xmlparser.hxx latexparser.cxx latexparser.hxx \
		      manparser.cxx manparser.hxx testparser.cxx \
		      textparser.cxx textparser.hxx htmlparser.cxx \
		      htmlparser.hxx odfparser.hxx odfparser.cxx

# need mystrdup()
LDADD = ../hunspell/libhunspell-1.3.la
