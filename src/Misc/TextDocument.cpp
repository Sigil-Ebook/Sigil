/************************************************************************
 **
 **  Copyright (C) 2019 Kevin B. Hendricks Stratford, ON, Canada 
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

#include <QChar>
#include <QTextCursor>
#include <QDebug>

#include "Misc/TextDocument.h"

TextDocument::TextDocument(QObject *parent)
 :
  QTextDocument(parent)
{
}

// a proper replacement for toPlainText() that does not destroy
// non-breaking space characters
// see toPlainText() in qtbase/src/gui/text/qtextdocument.cpp

QString TextDocument::toText()
{
    QString txt;

#if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
    txt = toRawText();
#else
    // is the TextDocument itself is empty just return an empty string
    if (isEmpty()) return txt;

    // Use text cursors to get the TextDocument's contents
    QTextCursor cursor(this);
    cursor.select(QTextCursor::Document);
    txt = cursor.selectedText();
#endif

    QChar *uc = txt.data();
    QChar *e = uc + txt.size();

    for (; uc != e; ++uc) {
        switch (uc->unicode()) {
            case 0xfdd0: // QTextBeginningOfFrame                                                    
            case 0xfdd1: // QTextEndOfFrame                                                          
            case QChar::ParagraphSeparator:
            case QChar::LineSeparator:
	        *uc = QLatin1Char('\n');
	        break;
            default:
	    ;
        }
    }

    return txt;
}
