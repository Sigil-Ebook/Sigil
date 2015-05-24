/*
 * HTML parser class for MySpell
 *
 * implemented: text, HTML, TeX
 *
 * Copyright (C) 2014, Laszlo Nemeth
 *
 */

#ifndef _XMLPARSER_HXX_
#define _XMLPARSER_HXX_


#include "textparser.hxx"

/*
 * XML Parser
 *
 */

class XMLParser : public TextParser
{

public:
  XMLParser();
  XMLParser(const char * wc);
  XMLParser(unsigned short * wordchars, int len);
  char *              next_token(const char * p[][2], unsigned int len, const char * p2[][2], unsigned int len2);
  char *              next_token();
  int                 change_token(const char * word);
  virtual ~XMLParser();

private:

  int                 look_pattern(const char * p[][2], unsigned int len, int column);
  int                 pattern_num;
  int                 pattern2_num;
  int		      prevstate;
  int                 checkattr;
  char		      quotmark;
};

#endif

