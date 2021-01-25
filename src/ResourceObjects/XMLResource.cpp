/************************************************************************
**
**  Copyright (C) 2015-2020 Kevin B. Hendricks, Stratford Ontario canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "Misc/AsciiFy.h"
#include "ResourceObjects/XMLResource.h"

XMLResource::XMLResource(const QString &mainfolder, const QString &fullfilepath, QObject *parent)
    : TextResource(mainfolder, fullfilepath, parent)
{
}


Resource::ResourceType XMLResource::Type() const
{
    return Resource::XMLResourceType;
}


bool XMLResource::FileIsWellFormed() const
{
    // TODO: expand this with a dialog to fix the problem
    QReadLocker locker(&GetLock());
    QString mtype = GetMediaType();
    if ((mtype == "application/xhtml+xml") || (mtype == "application/x-dtbook+xml")) { 
        XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(GetText());
        bool well_formed = error.line == -1;
        return well_formed;
    }
    bool well_formed = CleanSource::IsWellFormedXML(GetText(),mtype);
    return well_formed;
}


XhtmlDoc::WellFormedError XMLResource::WellFormedErrorLocation() const
{
    QReadLocker locker(&GetLock());
    QString mtype = GetMediaType();
    XhtmlDoc::WellFormedError error;
    if ((mtype == "application/xhtml+xml") || (mtype == "application/x-dtbook+xml")) { 
        error = XhtmlDoc::WellFormedErrorForSource(GetText());
    } else {
        error = CleanSource::WellFormedXMLCheck(GetText(), mtype);
    }
    return error;
}

// The actual xml spec for allowed char in xml ids
//
//  NameStartChar ::=   ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] |
//                            [#xD8-#xF6] | [#xF8-#x2FF] |
//                            [#x370-#x37D] | [#x37F-#x1FFF] |
//                            [#x200C-#x200D] | [#x2070-#x218F] |
//                            [#x2C00-#x2FEF] | [#x3001-#xD7FF] |
//                            [#xF900-#xFDCF] | [#xFDF0-#xFFFD] |
//                            [#x10000-#xEFFFF]
// 
// NameChar     ::=      NameStartChar | "-" | "." | [0-9] | #xB7 |
//                         [#x0300-#x036F] | [#x203F-#x2040]
// 

// to create an epub that will work with even old readers
// we will limit ourselves to the ascii set as the old
// standard proscribed when creating new ids from scratch

static const QString ID_VALID_FIRST_CHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const QString ID_VALID_CHARS = ID_VALID_FIRST_CHAR + "_-.0123456789";

QString XMLResource::GetValidID(const QString &value)
{
    // simplify to trim any leading or trailing whitespace
    QString new_value = value.simplified();
    // convert it to a "close" pure ascii representation
    new_value = AsciiFy::instance()->convertToPlainAscii(new_value);
    int i = 0;
    // Remove all forbidden characters.
    while (i < new_value.size()) {
        if (!IsValidIDCharacter(new_value.at(i))) {
            new_value.remove(i, 1);
        } else {
            ++i;
        }
    }

    if (new_value.isEmpty()) new_value = Utility::CreateUUID();

    QChar first_char = new_value.at(0);

    // IDs cannot start with a number, a dash or a dot
    // and should start with a character in range [A-Z,a-z]
    if (!ID_VALID_FIRST_CHAR.contains(first_char)) {
        new_value.prepend("x");
    }

    return new_value;
}


// This is probably more rigorous
// than the XML spec, but it's simpler.
// (spec ref: http://www.w3.org/TR/xml-id/#processing)
bool XMLResource::IsValidIDCharacter(const QChar &character)
{
    return ID_VALID_CHARS.contains(character);
}

