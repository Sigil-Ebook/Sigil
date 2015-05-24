/*
 * ODF parser class for MySpell
 *
 * Copyright (C) 2014, Laszlo Nemeth
 *
 */

#ifndef _ODFPARSER_HXX_
#define _ODFPARSER_HXX_

#include "xmlparser.hxx"

/*
 * HTML Parser
 *
 */

class ODFParser : public XMLParser
{
public:
 
  ODFParser(const char * wc);
  ODFParser(unsigned short * wordchars, int len);
  char *   next_token();
  virtual ~ODFParser();

};


#endif

