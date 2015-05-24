#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "../hunspell/csutil.hxx"
#include "firstparser.hxx"

#ifndef W32
using namespace std;
#endif

FirstParser::FirstParser(const char * wordchars)
{
	init(wordchars);
}

FirstParser::~FirstParser() 
{
}

char * FirstParser::next_token()
{
        char * tabpos = strchr(line[actual],'\t');
        if ((tabpos) && (tabpos - line[actual]>token))
        {
                char * t = (char *) malloc(tabpos - line[actual] + 1);
                if (!t)
                {
		    fprintf(stderr,"Error - Insufficient Memory\n");
                    return NULL;
                }
                t[tabpos - line[actual]] = '\0';
                token = tabpos - line[actual] +1;
                return strncpy(t, line[actual], tabpos - line[actual]);
        }
 	return NULL;
}
