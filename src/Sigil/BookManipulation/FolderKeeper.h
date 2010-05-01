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
#ifndef FOLDERKEEPER_H
#define FOLDERKEEPER_H

#include <QString>
#include <QHash>
#include <QMutex>

// These have to be included directly because
// of the template functions.
#include "../ResourceObjects/HTMLResource.h"
#include "../ResourceObjects/ImageResource.h"
#include "../ResourceObjects/CSSResource.h"
#include "../ResourceObjects/XPGTResource.h"
#include "../ResourceObjects/FontResource.h"


class Resource;


class FolderKeeper
{

public:

    // Constructor
    FolderKeeper();

    // Copy constructor
    FolderKeeper( const FolderKeeper& other );

    // Assignment operator
    FolderKeeper& operator= ( const FolderKeeper& other );

    // Destructor
    ~FolderKeeper();

    // A dispatcher function that routes the given *infrastructure* file
    // to the appropriate specific folder function; the name of the new file
    // needs to be specified
    void AddInfraFileToFolder( const QString &fullfilepath, const QString &newfilename );

    // The file is recognized according to its extension.
    Resource& AddContentFileToFolder( const QString &fullfilepath, 
                                      int reading_order = -1,
                                      QHash< QString, QString > semantic_information = QHash< QString, QString >() );

    int GetHighestReadingOrder() const;

    QString GetUniqueFilenameVersion( const QString &filename ) const;

    // Returns a list of all the content files in the directory
    // with a path relative to the OEBPS directory.
    // The list is alphabetically sorted.
    QStringList GetSortedContentFilesList() const;

    QList< Resource* > GetResourceList() const;

    template< class T >
    QList< T* > GetResourceTypeList( bool should_be_sorted = false ) const;

    template< class T >
    QList< Resource* > GetResourceTypeAsGenericList( bool should_be_sorted = false ) const;

    Resource& GetResourceByIdentifier( const QString &identifier ) const;

    // NOTE THAT RESOURCE FILENAMES CAN CHANGE,
    // while identifiers don't. Also, retrieving 
    // resources by identifier is O(1), this is O(n).
    // (and a *very* slow O(n) since we query the filesystem)
    Resource& GetResourceByFilename( const QString &filename ) const;

    // Returns the full path to the main folder of the publication
    QString GetFullPathToMainFolder() const;	

    // Returns the full path to the OEBPS folder of the publication
    QString GetFullPathToOEBPSFolder() const;	

    // Returns the full path to the OEBPS folder of the publication
    QString GetFullPathToTextFolder() const;

private:

    // Performs common constructor duties
    // for all constructors
    void Initialize();

    QStringList GetAllFilenames() const;

    void CopyFiles( const FolderKeeper &other );

    void DeleteAllResources( const QString &folderpath );

    // Creates the required folder structure:
    //	 META-INF
    //	 OEBPS
    //	    Images
    //	    Fonts
    //	    Text
    //      Styles
    //      Misc
    void CreateFolderStructure();

    template< typename T >
    static bool PointerLessThan( T* first_item, T* second_item );

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // Resources have to be pointers because
    // we cannot store references in a QHash
    QHash< QString, Resource* > m_Resources;

    QMutex m_AccessMutex;

    // Full paths to all the folders in the publication
    QString m_FullPathToMainFolder;
    QString m_FullPathToMetaInfFolder;
    QString m_FullPathToOEBPSFolder;

    QString m_FullPathToImagesFolder;
    QString m_FullPathToFontsFolder;
    QString m_FullPathToTextFolder;
    QString m_FullPathToStylesFolder;
    QString m_FullPathToMiscFolder;
};


template< class T >
QList< T* > FolderKeeper::GetResourceTypeList( bool should_be_sorted ) const
{
    QList< T* > onetype_resources;

    foreach( Resource *resource, m_Resources.values() )
    {
        T* type_resource = qobject_cast< T* >( resource );

        if ( type_resource )

            onetype_resources.append( type_resource );
    }

    if ( should_be_sorted )

        qSort( onetype_resources.begin(), onetype_resources.end(), FolderKeeper::PointerLessThan< T > );

    return onetype_resources;
}

template< class T >
QList< Resource* > FolderKeeper::GetResourceTypeAsGenericList( bool should_be_sorted ) const
{
    QList< Resource* > resources;

    foreach( Resource *resource, m_Resources.values() )
    {
        T* type_resource = qobject_cast< T* >( resource );

        if ( type_resource )

            resources.append( resource );
    }

    if ( should_be_sorted )

        qSort( resources.begin(), resources.end(), FolderKeeper::PointerLessThan< Resource > );

    return resources;
}

template< typename T >
bool FolderKeeper::PointerLessThan( T* first_item, T* second_item )
{
    Q_ASSERT( first_item );
    Q_ASSERT( second_item );

    return *first_item < *second_item;
}
#endif // FOLDERKEEPER_H


