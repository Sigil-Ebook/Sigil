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
#ifndef BOOK_H
#define BOOK_H

#include "../BookManipulation/FolderKeeper.h"

#include <QHash>
#include <QUrl>
#include <QVariant>

class Book
{	

public:

    // Constructor
    Book();

    // Copy constructor
    Book( const Book& other );

    // Assignment operator
    Book& operator = ( const Book& other );

    // Returns the base url of the book,
    // that is the location to the text folder
    // within the main folder
    QUrl GetBaseUrl() const;

    FolderKeeper& GetFolderKeeper();

    const FolderKeeper& GetConstFolderKeeper();

    QString GetPublicationIdentifier();

    QHash< QString, QList< QVariant > > GetMetadata();

    void SetMetadata( const QHash< QString, QList< QVariant > > metadata );

    void CreateEmptyTextFile();

private:   

    // The FolderKeeper object that represents
    // this books presence on the hard drive
    FolderKeeper m_Mainfolder; 

    // The UUID that uniquely represents this book
    QString m_PublicationIdentifier;

    // Stores all the metadata for the book;
    // the key is the metadata name, the values
    // are the lists of metadata values
    QHash< QString, QList< QVariant > > m_Metadata;

};

#endif // BOOK_H


