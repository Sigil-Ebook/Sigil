/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin B. Hendricks, Stratford ON
**  Copyright (C) 2013      John Schember <john@nachtimwald.com>
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

#pragma once
#ifndef OPFRESOURCE_H
#define OPFRESOURCE_H

#include <memory>
#include <QStringList>
#include <QHash>
#include <QString>
#include "Misc/GuideItems.h"
#include "ResourceObjects/XMLResource.h"
#include "Parsers/OPFParser.h"

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
    OPFResource(const QString &mainfolder, const QString &fullfilepath, const QString &version=QString(), QObject *parent = NULL);

    // inherited

    virtual bool RenameTo(const QString &new_filename);

    virtual bool MoveTo(const QString &newbookpath);

    virtual ResourceType Type() const;

    virtual QString GetText() const;

    virtual void SetText(const QString &text);

    virtual bool LoadFromDisk();

    QString GetGuideSemanticCodeForResource(const Resource *resource, QString tgt_id="") const;
    QString GetGuideSemanticNameForResource(Resource *resource, QString tgt_id="");
    QHash <QString, QStringList> GetSemanticCodeForPaths();
    QHash <QString, QStringList> GetGuideSemanticNameForPaths();
    QStringList GetAllGuideInfoByBookPath() const;

    void ClearSemanticCodesInGuide();

    void MoveReadingOrder(const HTMLResource* from_htmlresource, const HTMLResource* to_after_htmlresource);
    int GetReadingOrder(const HTMLResource *html_resource) const;
    QHash <Resource *, int> GetReadingOrderAll( const QList <Resource *> resources);

    QList<Resource*> GetSpineOrderResources(const QList<Resource *> &resources);

    QString GetMainIdentifierValue() const;

    void SaveToDisk(bool book_wide_save = false);

    QString GetPackageVersion() const;

    // Also creates such an ident if none was found
    QString GetUUIDIdentifierValue();

    void EnsureUUIDIdentifierPresent();

    QString AddNCXItem(const QString &ncx_path, QString id="ncx");

    void RemoveNCXOnSpine();

    void UpdateNCXOnSpine(const QString &new_ncx_id);

    void UpdateNCXLocationInManifest(const NCXResource *ncx);

    QString AddModificationDateMeta();

    void AddSigilVersionMeta();

    bool IsCoverImage(const ImageResource *image_resource) const;

    void AutoFixWellFormedErrors();

    QStringList GetSpineOrderBookPaths() const;

    void SetItemRefLinear(Resource * resource, bool linear);

    /**
     * SetSpineOrderFromFilenames
     * The setter to complement GetSpineOrderFilenames()
     *
     * @param An list of the content file names that must be written
     *        in order to the spine.
     **/
    // void SetSpineOrderFromFilenames(const QStringList spineOrder);

    // returns first (primary) dc:language value found
    QString GetPrimaryBookLanguage() const;

    // returns first (primary) dc:title value found
    QString GetPrimaryBookTitle() const;

    // returns the metadata tag and all its contents as an xml fragment
    QString GetMetadataXML() const;

    /**
     * Returns the book's Dublin Core metadata.
     *
     * @return The DC metadata, in the same format as the SetDCMetadata metadata parameter.
     */
    QList<MetaEntry> GetDCMetadata() const;

    /**
     * Returns list of any Media Overlay Active Class Selectors if defined in OPF metadata
     */
    QStringList GetMediaOverlayActiveClassSelectors() const;
    
    /**
     * Returns the values for a specific dc: metadata name.
     *
     * @return A list of values
     */
    QStringList GetDCMetadataValues(QString text) const;

    void SetNavResource(HTMLResource* nav);
    HTMLResource* GetNavResource() const;

    void UpdateGuideAfterMerge(QList<Resource*> &merged_resources, QHash<QString,QString> &section_id_map);
    void UpdateGuideFragments(QHash<QString,QString> &idupdates);

 signals:
    void TextChanging();
    void LoadedFromDisk();

public slots:

    /**
     * Writes metadata to the OPF <metadata> element.
     *
     * @param metadata A list with meta information about the book.
     */

    void SetDCMetadata(const QList<MetaEntry>  &metadata);

    void AddResource(const Resource *resource);

    void RemoveResource(const Resource *resource);
    void BulkRemoveResources(const QList<Resource *>resources);

    void AddGuideSemanticCode(HTMLResource *html_resource, QString code, bool toggle = true, QString tgt_id="");

    void SetResourceAsCoverImage(ImageResource *image_resource);

    void UpdateSpineOrder(const QList<HTMLResource *> html_files);

    void ResourceRenamed(const Resource *resource, QString old_full_path);
    void BulkResourcesRenamed(const QHash<QString, Resource *> renamedDict);

    void ResourceMoved(const Resource *resource, QString old_full_path);
    void BulkResourcesMoved(const QHash<QString, Resource *> movedDict);

    void UpdateManifestProperties(const QList<Resource *> resources);

    void UpdateManifestMediaTypes(const QList<Resource*> resources);

    QString GetManifestPropertiesForResource(const Resource * resource);

    QHash <QString, QString> GetManifestPropertiesForPaths();

    void RebaseManifestIDs();

private:

    /**
     * Determines if a cover image exists.
     *
     * @return \c true if a cover image exists.
     */
    bool CoverImageExists() const;

    bool IsCoverImageCheck(QString resource_id, const OPFParser& p) const;

    void AddCoverImageProperty(QString& resource_id, OPFParser& p);

    void RemoveCoverImageProperty(QString& resource_id, OPFParser& p);

    void AddCoverMetaForImage(const Resource *resource, OPFParser &p);

    void RemoveCoverMetaForImage(const Resource *resource, OPFParser &p);


    // static void AppendToSpine(const QString &id);

    // static void RemoveFromSpine(const QString &id);

    // static void UpdateItemrefID(const QString &old_id, const QString &new_id);

    int GetMainIdentifier(const OPFParser &p) const;

    // CAN BE -1 which means no reference for resource
    int GetGuideReferenceForResourcePos(const Resource *resource, const OPFParser &p, QString tgt_id="") const;

    void RemoveGuideReferenceForResource(const Resource *resource, OPFParser &p, QString tgt_id="");
    
    void RemoveAllGuideReferencesForResource(const Resource *resource, OPFParser& p);

    QString GetGuideSemanticCodeForResource(const Resource *resource, const OPFParser &p, QString tgt_id="") const;

    void SetGuideSemanticCodeForResource(QString code, const Resource *resource, OPFParser &p,
                                         const QString &lang, QString tgt_id="");

    void RemoveDuplicateGuideCodes(QString code, OPFParser &p);

        // CAN BE -1 means no cover meta element
    int GetCoverMeta(const OPFParser &p) const;

    QString GetResourceManifestID(const Resource *resource, const OPFParser &p) const;

    QHash<Resource *, QString> GetResourceManifestIDMapping(const QList<Resource *> &resources, const OPFParser &p);

    QHash<QString, Resource *> GetManifestIDResourceMapping(const QList<Resource *> &resources, const OPFParser &p);

    void RemoveDCElements(OPFParser &p);

    /**
     * Writes simple metadata.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written.
     * @param document The OPF DOM document.
     */
    void WriteSimpleMetadata(const QString &metaname, const QString &metavalue, OPFParser &p);

    /**
     * Writes the <identifier> elements.
     * The metaname will be used for the scheme.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written.
     * @param document The OPF DOM document.
     */
    void WriteIdentifier(const QString &metaname, const QString &metavalue, OPFParser &p);

    static QString GetOPFDefaultText(const QString &version);

    void FillWithDefaultText(const QString &version=QString());

    QString GetUniqueID(const QString &preferred_id, const OPFParser &p) const;

    QString GetResourceMimetype(const Resource *resource) const;

    void UpdateText(const OPFParser &p);

    QString ValidatePackageVersion(const QString &source);

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    HTMLResource * m_NavResource;
    bool m_WarnedAboutVersion;
};

#endif // OPFRESOURCE_H
