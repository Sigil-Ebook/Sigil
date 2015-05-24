#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "../hunspell/csutil.hxx"
#include "odfparser.hxx"


#ifndef W32
using namespace std;
#endif

static const char * PATTERN[][2] = {
	{ "<office:meta>", "</office:meta>" },
	{ "<office:settings>", "</office:settings>" },
	{ "<office:binary-data>", "</office:binary-data>" },
	{ "<!--", "-->" },
	{ "<[cdata[", "]]>" }, // XML comment
	{ "<", ">" }
};

#define PATTERN_LEN (sizeof(PATTERN) / (sizeof(char *) * 2))

static const char * PATTERN2[][2] = {
};

#define PATTERN_LEN2 (sizeof(PATTERN2) / (sizeof(char *) * 2))

ODFParser::ODFParser(const char * wordchars)
{
	init(wordchars);
}

ODFParser::ODFParser(unsigned short * wordchars, int len)
{
	init(wordchars, len);
}

char * ODFParser::next_token()
{
	return XMLParser::next_token(PATTERN, PATTERN_LEN, PATTERN2, PATTERN_LEN2);
}

ODFParser::~ODFParser() 
{
}
