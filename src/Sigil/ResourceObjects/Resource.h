/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#ifndef RESOURCE_H
#define RESOURCE_H

#include <QObject>
#include <QReadWriteLock>
#include <QIcon>
#include <QUrl>

/**
 * Represents a resource file on disk.
 * Stores data caches for improved performance, 
 * enables safe multi-threaded access to the data
 * and holds all sorts of meta information
 * on the file.
 */
class Resource : public QObject 
{
    Q_OBJECT

public:

    /**
     * Used to inspect a resource's type.
     */
    enum ResourceType
    {
        GenericResource, /**< A \em vulgaris resource; used for Misc resources. */
        TextResource,    /**< Used for Text resources, but \em not HTML files. */
        HTMLResource,    /**< Used for pure (X)HTML resources. */
        CSSResource,     /**< Used for CSS resources (stylesheets). */
        XPGTResource,    /**< Used for XPGT resources. */
        ImageResource,   /**< Used for image resource, of all types. */
        FontResource     /**< Used for font resources, both TTF and OTF. */
    };

    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param hash_owner The hash object that is the "owner" of this resource.
     *                   Needed so that the resource can remove itself from the
     *                   hash when it is deleted.
     * @param parent The object's parent.
     * @todo Remove the hash parameter and dependency. It was needed when the
     *       FolderKeeper was a copyable object and thus couldn't be a QObject
     *       to which we could send signals, but it's not anymore.
     */
    Resource( const QString &fullfilepath, QHash< QString, Resource* > *hash_owner, QObject *parent = NULL );

    /**
     * The less-than operator overload. By default, compares
     * the resources by filename (lexical).
     *
     * @param other The other Resource object we're comparing with.
     */
    virtual bool operator< ( const Resource& other );

    /**
     * Returns the resource's UUID.
     *
     * @return The resource's UUID.
     */
    QString GetIdentifier() const;

    /**
     * Returns the resource's filename.
     *
     * @return The resource's filename.
     */
    QString Filename() const;

    /**
     * Returns the resource's path relative to the OEBPS folder.
     * Returned \em without leading "../" characters.
     *
     * @return The resource's path relative to the OEBPS folder.
     */
    QString GetRelativePathToOEBPS() const;

    /**
     * Returns the resource's path relative to the publication's
     * root folder. Returned \em without leading "../" characters.
     *
     * @return The root-relative path.
     */
    QString GetRelativePathToRoot() const;

    /**
     * Returns the resource's full file path.
     * We \em really shouldn't be using this, 
     * it kinda breaks encapsulation.
     *
     * @return The resource's full file path.
     */
    QString GetFullPath() const;

    /**
     * Returns the URL to the parent folder ("base URL") of this resource.
     *
     * @return The base URL.
     */
    QUrl GetBaseUrl() const;

    /**
     * Returns a reference to the resource's ReadWriteLock.
     *
     * @return The resource's ReadWriteLock.
     */
    QReadWriteLock& GetLock();

    /**
     * Returns the resource's icon.
     *
     * @return The resource's icon.
     */
    QIcon Icon() const;

    /**
     * Renames the resource.
     *
     * @param new_filename The new name.
     * @return \c true if the operation was successful.
     */
    bool RenameTo( const QString &new_filename );

    /**
     * Deletes the resource.
     *
     * @return \c true if the operation was successful.
     */
    bool Delete();    

    /**
     * Returns the resource's type.
     *
     * @return The resource's type.
     */
    virtual ResourceType Type() const;

    /**
     * Instructs the resource to save any cached data to disk.
     * The default implementation does nothing,
     * and assumes the resource data is not being
     * cached in memory.
     *
     * @param book_wide_save If \c false (the default), a ResourceUpdatedOnDisk()
     *                       signal will be emitted.
     */
    virtual void SaveToDisk( bool book_wide_save = false );

signals:

    /**
     * Emitted whenever the resource changes its name.
     *
     * @param new_filename The resource's new name.
     */
    void RenamedTo( QString new_filename );

    /**
     * Emitted when the resource has been scheduled for deletion.
     */
    void Deleted();

    /**
     * Emitted when the resource has been updated on disk.
     */
    void ResourceUpdatedOnDisk();

protected:

    /**
     * The resources identifying UUID.
     */
    QString m_Identifier;

    /**
     * The full path to the resource on disk.
     */
    QString m_FullFilePath;

    /**
     * The ReaWriteLock guarding access to the resource's data.
     */
    QReadWriteLock m_ReadWriteLock;

    /**
     * The hash owner of the resource.
     */
    QHash< QString, Resource* > &m_HashOwner;
};

#endif // RESOURCE_H
