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
#ifndef OPFRESOURCE_H
#define OPFRESOURCE_H

#include <boost/shared_ptr.hpp>

#include "BookManipulation/GuideSemantics.h"
#include "ResourceObjects/XMLResource.h"
#include "BookManipulation/Metadata.h"

using boost::shared_ptr;

class HTMLResource;
class ImageResource;
class NCXResource;


class OPFResource : public XMLResource
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
    OPFResource(const QString &mainfolder, const QString &fullfilepath, QObject *parent = NULL);

    // inherited

    virtual bool RenameTo(const QString &new_filename);

    virtual ResourceType Type() const;

    GuideSemantics::GuideSemanticType GetGuideSemanticTypeForResource(const Resource &resource) const;
    QString GetGuideSemanticNameForResource(Resource *resource);

    QHash <QString, QString> GetGuideSemanticNameForPaths();

    int GetReadingOrder(const HTMLResource &html_resource) const;
    QHash <Resource *, int> GetReadingOrderAll( const QList <Resource *> resources);

    QString GetMainIdentifierValue() const;

    void SaveToDisk(bool book_wide_save = false);

    // Also creates such an ident if none was found
    QString GetUUIDIdentifierValue();

    void EnsureUUIDIdentifierPresent();

    QString AddNCXItem(const QString &ncx_path);

    void UpdateNCXOnSpine(const QString &new_ncx_id);

    void UpdateNCXLocationInManifest(const ::NCXResource &ncx);

    void AddModificationDateMeta();

    void AddSigilVersionMeta();

    bool IsCoverImage(const ::ImageResource &image_resource) const;

    bool IsCoverImageCheck(const Resource &resource, xc::DOMDocument &document) const;

    bool IsCoverImageCheck(QString resource_id, xc::DOMDocument &document) const;

    /**
     * Determines if a cover image exists.
     *
     * @return \c true if a cover image exists.
     */
    bool CoverImageExists() const;

    void AutoFixWellFormedErrors();

    QStringList GetSpineOrderFilenames() const;

    /**
     * SetSpineOrderFromFilenames
     * The setter to complement GetSpineOrderFilenames()
     *
     * @param An list of the content file names that must be written
     *        in order to the spine.
     **/
    void SetSpineOrderFromFilenames(const QStringList spineOrder);

    /**
     * Returns the book's Dublin Core metadata.
     *
     * @return The DC metadata, in the same format as the SetDCMetadata metadata parameter.
     */
    QList<Metadata::MetaElement> GetDCMetadata() const;

    /**
     * Returns the values for a specific metadata name.
     *
     * @return A list of values
     */
    QList<QVariant> GetDCMetadataValues(QString text) const;

    QString GetRelativePathToRoot() const;

public slots:

    /**
     * Writes metadata to the OPF <metadata> element.
     *
     * @param metadata A list with meta information about the book.
     */
    void SetDCMetadata(const QList<Metadata::MetaElement>  &metadata);

    void AddResource(const Resource &resource);

    void RemoveCoverMetaForImage(const Resource &resource, xc::DOMDocument &document);

    void AddCoverMetaForImage(const Resource &resource, xc::DOMDocument &document);

    void RemoveResource(const Resource &resource);

    void AddGuideSemanticType(const HTMLResource &html_resource, GuideSemantics::GuideSemanticType new_type);

    void SetResourceAsCoverImage(const ImageResource &image_resource);

    void UpdateSpineOrder(const QList<HTMLResource *> html_files);

    void ResourceRenamed(const Resource &resource, QString old_full_path);

private:

    static void AppendToSpine(const QString &id, xc::DOMDocument &document);

    static void RemoveFromSpine(const QString &id, xc::DOMDocument &document);

    static void UpdateItemrefID(const QString &old_id, const QString &new_id, xc::DOMDocument &document);

    boost::shared_ptr<xc::DOMDocument> GetDocument() const;

    static xc::DOMElement *GetPackageElement(const xc::DOMDocument &document);

    static xc::DOMElement *GetMetadataElement(const xc::DOMDocument &document);

    static xc::DOMElement *GetManifestElement(const xc::DOMDocument &document);

    static xc::DOMElement *GetSpineElement(const xc::DOMDocument &document);

    static xc::DOMElement *GetGuideElement(const xc::DOMDocument &document);
    static xc::DOMElement *GetGuideElement(xc::DOMDocument &document);

    // CAN BE NULL! NULL means no reference for resource
    static xc::DOMElement *GetGuideReferenceForResource(
        const Resource &resource,
        const xc::DOMDocument &document);

    static void RemoveGuideReferenceForResource(
        const Resource &resource,
        xc::DOMDocument &document);

    static GuideSemantics::GuideSemanticType GetGuideSemanticTypeForResource(
        const Resource &resource,
        xc::DOMDocument &document);

    static void SetGuideSemanticTypeForResource(
        GuideSemantics::GuideSemanticType type,
        const Resource &resource,
        xc::DOMDocument &document);

    static void RemoveDuplicateGuideTypes(
        GuideSemantics::GuideSemanticType new_type,
        xc::DOMDocument &document);

    static QHash<HTMLResource *, xc::DOMElement *> GetItemrefsForHTMLResources(
        const QList<HTMLResource *> html_files,
        xc::DOMDocument &document);

    // CAN BE NULL! NULL means no cover meta element
    static xc::DOMElement *GetCoverMeta(const xc::DOMDocument &document);

    static xc::DOMElement &GetMainIdentifier(const xc::DOMDocument &document);

    static xc::DOMElement *GetMainIdentifierUnsafe(const xc::DOMDocument &document);

    static QString GetResourceManifestID(const Resource &resource, const xc::DOMDocument &document);

    static QHash<Resource *, QString> GetResourceManifestIDMapping(
        const QList<Resource *> resources,
        const xc::DOMDocument &document);

    static void SetMetaElementsLast(xc::DOMDocument &document);

    static void RemoveDCElements(xc::DOMDocument &document);

    /**
     * Dispatches each metadata entry based on its type.
     * The specialized Write* functions write the elements.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written.
     * @param document The OPF DOM document.
     */
    static void MetadataDispatcher(const Metadata::MetaElement &book_meta, xc::DOMDocument &document);

    /**
     * Writes <creator> and <contributor> metadata elements.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written.
     * @param document The OPF DOM document.
     */
    static void WriteCreatorOrContributor(const Metadata::MetaElement book_meta, xc::DOMDocument &document);

    /**
     * Writes simple metadata.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written.
     * @param document The OPF DOM document.
     */
    static void WriteSimpleMetadata(const QString &metaname, const QString &metavalue, xc::DOMDocument &document);

    /**
     * Writes the <identifier> elements.
     * The metaname will be used for the scheme.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written.
     * @param document The OPF DOM document.
     */
    static void WriteIdentifier(const QString &metaname, const QString &metavalue, xc::DOMDocument &document);

    /**
     * Writes the <date> elements.
     * The metaname will be used for the event.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written.
     * @param document The OPF DOM document.
     */
    static void WriteDate(const QString &metaname, const QVariant &metavalue, xc::DOMDocument &document);

    static bool BasicStructurePresent(const xc::DOMDocument &document);

    boost::shared_ptr<xc::DOMDocument> CreateOPFFromScratch(const xc::DOMDocument *document=NULL) const;

    QStringList GetRelativePathsToAllFilesInOEPBS() const;

    static QString GetOPFDefaultText();

    void FillWithDefaultText();

    QString GetUniqueID(const QString &preferred_id, const xc::DOMDocument &document) const;

    QString GetResourceMimetype(const Resource &resource) const;

    QString GetFileMimetype(const QString &filepath) const;

    /**
     * Initializes m_Mimetypes.
     */
    void CreateMimetypes();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * A mapping between file extensions
     * and appropriate MIME types.
     */
    QHash<QString, QString> m_Mimetypes;

};

#endif // OPFRESOURCE_H
