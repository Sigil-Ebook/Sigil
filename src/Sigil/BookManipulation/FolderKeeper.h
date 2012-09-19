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
#ifndef FOLDERKEEPER_H
#define FOLDERKEEPER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QFileSystemWatcher>

// These have to be included directly because
// of the template functions.
#include "ResourceObjects/CSSResource.h"
#include "ResourceObjects/FontResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/ImageResource.h"
#include "ResourceObjects/MiscTextResource.h"
#include "ResourceObjects/SVGResource.h"
#include "ResourceObjects/OPFResource.h"

#include "Misc/TempFolder.h"

class NCXResource;

/**
 * Stores the resources of a book.
 * The book uses one main folder with several
 * subfolders. Some are needed by the epub spec,
 * and some are for categorizing files.
 *
 * Also contains all the operations involving 
 * the stored files.
 */
class FolderKeeper : public QObject 
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The object's parent.
     */
    FolderKeeper( QObject *parent = NULL );

    /**
     *  Destructor.
     */
    ~FolderKeeper();

    /**
     * Adds a content file to the book folder and returns the
     * corresponding Resource object. The file type is recognized
     * according to the extension or mimetype if present.
     * 
     * @param fullfilepath The full path to the file to add.
     * @param update_opf If set to \c true, then the OPF will be notified
     *                   that a file was added. This will add entries in the
     *                   OPF manifest and potentially the spine as well.
     * @param mimetype The mimetype for the associated file.
     * @return The newly created resource.
     */
    Resource& AddContentFileToFolder( const QString &fullfilepath, 
                                      bool update_opf = true,
                                      const QString &mimetype = QString() );

    /**
     * Returns the highest reading order number present in the book.
     *
     * @return The highest reading order.
     */
    int GetHighestReadingOrder() const;

    /**
     * Returns a book-wide unique filename. Given a filename,
     * if a file with the same name already exists, a number suffix
     * is added and a new filename returned. Otherwise, the filename
     * is returned unchanged.
     *
     * @param filename The original filename.
     * @return The unique filename.
     */
    QString GetUniqueFilenameVersion( const QString &filename ) const;

    /**
     * Returns a sorted list of all the content filepaths.
     * The paths returned are relative to the OEBPS directory,
     * and the sort is alphabetical.
     * 
     * @return The sorted filepaths.
     */
    QStringList GetSortedContentFilesList() const;

    /**
     * Returns a list of all the resources in the book.
     * The order of the items is random.
     *
     * @return The resource list.
     */
    QList< Resource* > GetResourceList() const;

    /**
     * Returns a list of all resources of type T in a list
     * of pointers to type T. The list can be sorted.
     *
     * @param should_be_sorted If \c true, the list is sorted.
     * @return The resource list.
     */
    template< class T >
    QList< T* > GetResourceTypeList( bool should_be_sorted = false ) const;

    /**
     * Returns a list of all resources of type T in a list
     * of pointers to the Resource base class. The list can be sorted.
     *
     * @param should_be_sorted If \c true, the list is sorted.
     * @return The resource list.
     */
    template< class T >
    QList< Resource* > GetResourceTypeAsGenericList( bool should_be_sorted = false ) const;

    /**
     * Returns a resource with the given identifier. 
     * This function is a very fast O(1).
     * 
     * @param identifier The identifier to search for.
     * @return The searched-for resourcse.
     */
    Resource& GetResourceByIdentifier( const QString &identifier ) const;

    /**
     * Returns the resource with the given filename.
     * @note NOTE THAT RESOURCE FILENAMES CAN CHANGE,
     *       while identifiers don't. Also, retrieving 
     *       resources by identifier is O(1), this is O(n)
     *       (and a \b very slow O(n) since we query the filesystem).
     * @throws ResourceDoesNotExist if the filename is not found.
     *
     * @param filename The filename to search for.
     * @return The searched-for resource.
     */
    Resource& GetResourceByFilename( const QString &filename ) const;

    /**
     * Returns the book's OPF file.
     * 
     * @return The OPF.
     */
    OPFResource& GetOPF() const;

    /**
     * Returns the book's NCX file.
     * 
     * @return The NCX.
     */
    NCXResource& GetNCX() const;

    /**
     * Returns the full path to the main folder of the publication.
     *
     * @return The full path.
     */
    QString GetFullPathToMainFolder() const;	

    /**
     * Returns the full path to the OEBPS folder of the publication.
     *
     * @return The full path.
     */
    QString GetFullPathToOEBPSFolder() const;	

    /**
     * Returns the full path to the Text folder of the publication.
     *
     * @return The full path.
     */
    QString GetFullPathToTextFolder() const;

    /**
     * Returns the full path to the Image folder of the publication.
     *
     * @return The full path.
     */
    QString GetFullPathToImageFolder() const;

    /**
     * Returns a list of all the resource filenames in the book. 
     *
     * @return The filename list.
     */
    QStringList GetAllFilenames() const;

    /**
     * Registers certain file types to be watched for external modifications.
     */
    void WatchResourceFile( const Resource& resource, bool file_renamed = false );

signals:

    /**
     * Emitted when a resource is added to the FolderKeeper.
     * 
     * @param resource The new resource.
     */
    void ResourceAdded( const Resource& resource );

    /**
     * Emitted when a resource is removed from the FolderKeeper.
     * 
     * @param resource The removed resource.
     */
    void ResourceRemoved( const Resource& resource );

private slots:

    /**
     * Removes the provided resource from the m_Resources hash.
     * Usually called when a resource is being deleted.
     * 
     * @param resource The resource to remove.
     */
    void RemoveResource( const Resource& resource );

    /**
     * Tell the OPF object to updated itself,
     * and (optionally) register the new file with the FS watcher.
     */
    void ResourceRenamed( const Resource& resource, const QString& old_full_path );

    /**
     * Called by the FSWatcher when a watched file has changed on disk.
     */
    void ResourceFileChanged( const QString& path ) const;

private:

    /**
     * Creates the required subfolders of each book.
     */
    void CreateFolderStructure();

    /**
     * Creates the book's infrastructure files, like 
     * the NCX and the OPF.
     */
    void CreateInfrastructureFiles();

    /**
     * Dereferences two pointers and compares the values with "<".
     *
     * @param first_item The first item in the comparison.
     * @param second_item The second item in the comparison.
     * @return The less-than comparison result.
     */
    template< typename T >
    static bool PointerLessThan( T* first_item, T* second_item );

    template< typename T >
    QList< T* > ListResourceSort( const QList< T* > &resource_list ) const;


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The book's OPF file.
     */
    OPFResource *m_OPF;

    /**
     * The book's NCX file.
     */
    NCXResource *m_NCX;

    // Resources have to be pointers because
    // we cannot store references in a QHash
    /**
     * The hash store of the resources in this book.
     * The keys are the UUID identifiers, the values
     * are the pointers to the actual resources.
     */
    QHash< QString, Resource* > m_Resources;

    /**
     * Ensures thread-safe access to the m_Resources hash.
     */
    QMutex m_AccessMutex;

    /**
     * The main temp folder where files are stored.
     */
    TempFolder m_TempFolder;

    /**
     * Watches the files on disk for any changes in case the resources have been modified from outside Sigil.
     */
    QFileSystemWatcher *m_FSWatcher;

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

        onetype_resources = ListResourceSort( onetype_resources );

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

        resources = ListResourceSort( resources );

    return resources;
}


template< typename T > inline
QList< T* > FolderKeeper::ListResourceSort( const QList< T* > &resource_list )  const
{
    QList< T* > sorted_list = resource_list;
    qSort( sorted_list.begin(), sorted_list.end(), FolderKeeper::PointerLessThan< T > );

    return sorted_list;
}


// This has to be inline, otherwise we get linker errors about this
// specialization already being defined.
template<> inline
QList< HTMLResource* > FolderKeeper::ListResourceSort< HTMLResource >( const QList< HTMLResource* > &resource_list ) const
{
    QStringList spine_order_filenames = GetOPF().GetSpineOrderFilenames();

    QList< HTMLResource* > htmls = resource_list;
    QList< HTMLResource* > sorted_htmls;

    foreach( const QString &spine_filename, spine_order_filenames )
    {
        for ( int i = 0; i < htmls.count(); ++i )
        {
            if ( spine_filename == htmls[ i ]->Filename() )
            {
                sorted_htmls.append( htmls.takeAt( i ) );
                break;
            }
        }
    }

    // It's possible that there are certain HTML files in the
    // given resource list that are not in the spine filenames,
    // for several reasons. So we make sure we add them to the end
    // of the sorted list.
    sorted_htmls.append( htmls );

    return sorted_htmls;
}


template< typename T > inline
bool FolderKeeper::PointerLessThan( T* first_item, T* second_item )
{
    Q_ASSERT( first_item );
    Q_ASSERT( second_item );

    return *first_item < *second_item;
}
#endif // FOLDERKEEPER_H


