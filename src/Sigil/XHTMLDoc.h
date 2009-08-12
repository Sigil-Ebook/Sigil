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

#pragma once
#ifndef XHTMLDOC_H
#define XHTMLDOC_H

#include <QDomNodeList>
#include <QString>


class XHTMLDoc
{

public:

    // Returns a list of QDomNodes representing all
    // the style elements in the provided XHTML source code
    static QList< QDomNode > GetStyleTags( const QString &source );

private:

    // Returns a list of deeply copied QDomNodes
    // from the specified QDomNodeList
    static QList< QDomNode > DeepCopyNodeList( QDomNodeList node_list );

};

#endif // XHTMLDOC_H

