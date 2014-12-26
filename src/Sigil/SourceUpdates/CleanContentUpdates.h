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

    bool remove_empty_paragraphs;

    bool join_paragraphs;
};

struct ChangesCount
{
    int number_of_files;
    int number_of_changes;

    ChangesCount() :
        number_of_files(0),
        number_of_changes(0) {}
};

class CleanContentUpdates
{

public:

    static ChangesCount CleanContentInAllFiles(const QList<HTMLResource *> &html_resources,
                                               const CleanContentParams &params);

private:
    static ChangesCount CleanContentInOneFile(HTMLResource *html_resource,
                                              const CleanContentParams &params);
    static void ReduceFunction(ChangesCount &acc, ChangesCount one_result);

    static int RemovePageNumbers(xc::DOMDocument &doc, const QString &page_number_format);
    static int RemoveEmptyParagraphs(xc::DOMDocument &doc);
    static int JoinParagraphs(xc::DOMDocument &doc);

    static QString ConvertSamplePageNumberToRegExp(const QString &page_number_format);
    static void RemoveLastChar(xc::DOMElement &element);
    static void MoveChildren(xc::DOMElement &elementDest, xc::DOMElement &elementSrc);
    static bool ContainsLetters(const QString &str);
};

#endif // CLEANCONTENTUPDATES_H
