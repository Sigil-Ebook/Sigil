/*
 * HTML parser class for MySpell
 *
 * implemented: text, HTML, TeX
 *
 * Copyright (C) 2002, Laszlo Nemeth
 *
 */

#ifndef _HTMLPARSER_HXX_
#define _HTMLPARSER_HXX_

#include "xmlparser.hxx"

/*
 * HTML Parser
 *
 */

class HTMLParser : public XMLParser
{

public:

  HTMLParser(const char * wc);
  HTMLParser(unsigned short * wordchars, int len);
  char *              next_token();
  virtual ~HTMLParser();

};


#endif

