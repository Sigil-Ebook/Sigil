/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <stdafx.h>
#include "SigilMarkup.h"


// Adds all Sigil-specific markup
QString SigilMarkup::AddSigilMarkup( const QString &source )
{
    QString newsource = source;

    newsource = AddSigilStyleTags( newsource );

    return newsource;
}


// Adds our CSS style tags
QString SigilMarkup::AddSigilStyleTags( const QString &source )
{
    QString newsource = source;

    QString style_tag = "<style type=\"text/css\">\n"
                        "/*SG    DO NOT MODIFY.\n"
                        "        This style is used by Sigil.\n"
                        "        It will be removed on export\n"
                        "        along with the \"sigilChapterBreak\" HR tags. SG*/\n"
                        "hr.sigilChapterBreak {\n"   
                        "    border: none 0;\n" 
                        "    border-top: 3px double #c00;\n"
                        "    height: 3px;\n"
                        "    clear: both;\n"
                        "}\n"
                        "</style>\n";

    newsource.insert( newsource.indexOf( "</head>" ), style_tag );

    return newsource;
}



