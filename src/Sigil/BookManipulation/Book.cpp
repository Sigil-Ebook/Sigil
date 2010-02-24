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

#include <stdafx.h>
#include "../BookManipulation/Book.h"
#include "../Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include <QDomDocument>

static const QString EMPTY_HTML_FILE =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                                        "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"							
                                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                                        "<head>\n"
                                        "<title></title>\n"
                                        "</head>\n"
                                        "<body>\n"

                                        // The "nbsp" is here so that the user starts writing
                                        // inside the <p> element; if it's not here, webkit
                                        // inserts text _outside_ the <p> element
                                        "<p>&nbsp;</p>\n"
                                        "</body>\n"
                                        "</html>";


// Constructor
Book::Book()
    : 
    m_PublicationIdentifier( Utility::CreateUUID() )
{
   
}


// Copy constructor
Book::Book( const Book& other )
{
    m_Metadata = other.m_Metadata;
    m_PublicationIdentifier = other.m_PublicationIdentifier;
    m_Mainfolder = other.m_Mainfolder;
}


// Assignment operator
Book& Book::operator = ( const Book& other )
{
    // Protect against invalid self-assignment
    if ( this != &other ) 
    {
        m_Metadata = other.m_Metadata;
        m_PublicationIdentifier = other.m_PublicationIdentifier;
        m_Mainfolder = other.m_Mainfolder;
    }

    // By convention, always return *this
    return *this;
}


// Returns the base url of the book,
// that is the location to the text folder
// within the main folder
QUrl Book::GetBaseUrl() const
{
    return QUrl::fromLocalFile( m_Mainfolder.GetFullPathToTextFolder() + "/" );
}


FolderKeeper& Book::GetFolderKeeper()
{
    return m_Mainfolder;
}


const FolderKeeper& Book::GetConstFolderKeeper()
{
    return m_Mainfolder;
}


QString Book::GetPublicationIdentifier()
{
    return m_PublicationIdentifier;
}


QHash< QString, QList< QVariant > > Book::GetMetadata()
{
    return m_Metadata;
}


void Book::SetMetadata( const QHash< QString, QList< QVariant > > metadata )
{
    m_Metadata = metadata;
}


void Book::SaveAllResourceCachesToDisk()
{
    foreach( Resource* resource, m_Mainfolder.GetResourceList() )
    {
        resource->SaveToDisk();
    }
}


// FIXME: Check if file with FIRST_CHAPTER_NAME already exists
// (in folderkeeper) and increment the number suffix.
void Book::CreateEmptyTextFile()
{
    QString folderpath = Utility::GetNewTempFolderPath();
    QDir dir( folderpath );
    dir.mkpath( folderpath );

    QString fullfilepath = folderpath + "/" + FIRST_CHAPTER_NAME;

    Utility::WriteUnicodeTextFile( EMPTY_HTML_FILE, fullfilepath );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( 
                                    &m_Mainfolder.AddContentFileToFolder( fullfilepath ) );

    Q_ASSERT( html_resource );

    QDomDocument document;
    document.setContent( EMPTY_HTML_FILE );

    html_resource->SetDomDocument( document );
}

