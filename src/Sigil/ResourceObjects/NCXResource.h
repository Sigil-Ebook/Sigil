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
#ifndef NCXRESOURCE_H
#define NCXRESOURCE_H

#include "MainUI/NCXModel.h"
#include "ResourceObjects/XMLResource.h"

class Book;

class NCXResource : public XMLResource
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
    NCXResource(const QString &mainfolder, const QString &fullfilepath, QObject *parent = NULL);

    // inherited

    virtual bool RenameTo(const QString &new_filename);

    virtual ResourceType Type() const;

    void SetMainID(const QString &main_id);

    bool GenerateNCXFromBookContents(const Book &book);
    void GenerateNCXFromTOCContents(const Book &book, NCXModel &ncx_model);
    void GenerateNCXFromTOCEntries(const Book &book, NCXModel::NCXEntry ncx_root_entry);

private:

    void FillWithDefaultText();

};

#endif // NCXRESOURCE_H
