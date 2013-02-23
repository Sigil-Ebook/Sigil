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
#ifndef XMLRESOURCE_H
#define XMLRESOURCE_H

#include "BookManipulation/XercesHUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "ResourceObjects/TextResource.h"


class XMLResource : public TextResource
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param parent The object's parent.
     */
    XMLResource(const QString &mainfolder, const QString &fullfilepath, QObject *parent = NULL);

    // inherited

    virtual ResourceType Type() const;

    bool FileIsWellFormed() const;

    XhtmlDoc::WellFormedError WellFormedErrorLocation() const;

protected:

    void UpdateTextFromDom(const xc::DOMDocument &document);

    /**
     * Creates a valid ID from the requested value.
     *
     * @param value What the caller wants the ID value to be.
     * @return The potentially modified value to make the ID valid.
     */
    static QString GetValidID(const QString &value);

    /**
     * Determines if the provided character can appear
     * in an XML ID attribute.
     *
     * @param character The character that needs to be checked.
     * @return True if the character is valid.
     */
    static bool IsValidIDCharacter(const QChar &character);

};

#endif // XMLRESOURCE_H
