/************************************************************************
**
**  Copyright (C) 2014 Marek Gibek
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
#ifndef CLEANCONTENTUPDATES_H
#define CLEANCONTENTUPDATES_H

#include "BookManipulation/XercesHUse.h"

class HTMLResource;

struct CleanContentParams
{
    bool remove_page_numbers;
    QString page_number_format;

    bool join_paragraphs;
};

class CleanContentUpdates
{

public:

    static void CleanContentInAllFiles(const QList<HTMLResource *> &html_resources,
                                       const CleanContentParams &params);

private:
    static void CleanContentInOneFile(HTMLResource *html_resource,
                                      const CleanContentParams &params);

    static void RemovePageNumbers(xc::DOMDocument &doc, const QString &page_number_format);
    static QString ConvertSamplePageNumberToRegExp(const QString &page_number_format);
};

#endif // CLEANCONTENTUPDATES_H
