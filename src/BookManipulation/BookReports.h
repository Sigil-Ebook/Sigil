/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef BOOKREPORTS_H
#define BOOKREPORTS_H

#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "BookManipulation/Book.h"

class QString;


class BookReports
{

public:

    // Data used by the style reports
    struct StyleData {
        QString html_filename;
        QString html_element_name;
        QString html_class_name;
        QString css_filename;
        QString css_selector_text;
        int css_selector_line;
        int css_selector_position;
    };

    static QList<BookReports::StyleData *> GetHTMLClassUsage(QSharedPointer<Book> book, bool show_progress = false);
    static QList<BookReports::StyleData *> GetCSSSelectorUsage(QSharedPointer<Book> book, QList<BookReports::StyleData *> html_classes_usage);
};

#endif // BOOKREPORTS_H
