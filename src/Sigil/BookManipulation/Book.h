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
#include <QMutex>

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

    // Returns the status of the m_ReportToCalibre
    // variable. Thread-safe.
    bool GetReportToCalibreStatus();

    // Sets the status of the m_ReportToCalibre
    // variable. Thread-safe.
    void SetReportToCalibreStatus( bool new_status );

    // This used to be a struct so these are public.
    // TODO: Find the time to update the codebase
    // to use getters and setters.

    // Stores the full XHTML source code of the book
    QString source;

    // Stores all the metadata for the book;
    // the key is the metadata name, the values
    // are the lists of metadata values
    QHash< QString, QList< QVariant > > metadata;

    // The 30 character random identifier
    // that uniquely represents this book
    QString PublicationIdentifier;

    // The FolderKeeper object that represents
    // this books presence on the hard drive
    FolderKeeper mainfolder;

private:    

    bool m_ReportToCalibre;

    QMutex m_ReportToCalibreSync;

    static bool s_IgnoreCalibreEnvFlag;

    QMutex m_IgnoreCalibreEnvFlagSync;    
};

#endif // BOOK_H


