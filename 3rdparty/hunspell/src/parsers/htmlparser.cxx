#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "../hunspell/csutil.hxx"
#include "htmlparser.hxx"


#ifndef W32
using namespace std;
#endif

static const char * PATTERN[][2] = {
	{ "<script", "</script>" },
	{ "<style", "</style>" },
	{ "<code", "</code>" },
	{ "<samp", "</samp>" },
	{ "<kbd", "</kbd>" },
	{ "<var", "</var>" },
	{ "<listing", "</listing>" },
	{ "<address", "</address>" },
	{ "<pre", "</pre>" },
	{ "<!--", "-->" },
	{ "<[cdata[", "]]>" }, // XML comment
	{ "<", ">" }
};

#define PATTERN_LEN (sizeof(PATTERN) / (sizeof(char *) * 2))

static const char * PATTERN2[][2] = {
	{ "<img", "alt=" }, // ALT and TITLE attrib handled spec.
	{ "<img", "title=" },
	{ "<a ", "title=" }
};

#define PATTERN_LEN2 (sizeof(PATTERN2) / (sizeof(char *) * 2))

HTMLParser::HTMLParser(const char * wordchars)
{
	init(wordchars);
}

HTMLParser::HTMLParser(unsigned short * wordchars, int len)
{
	init(wordchars, len);
}

char * HTMLParser::next_token()
{
	return XMLParser::next_token(PATTERN, PATTERN_LEN, PATTERN2, PATTERN_LEN2);
}

HTMLParser::~HTMLParser() 
{
}
