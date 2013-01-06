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

#include <boost/tuple/tuple.hpp>
// XercesExtensions
#include <XmlUtils.h>

#include <QtCore/QBuffer>
#include <QtCore/QDate>
#include <QtCore/QFileInfo>
#include <QtCore/QUuid>
#include <QRegularExpression>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Language.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/ImageResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"

using boost::tie;
namespace xe = XercesExt;

static const QString SIGIL_VERSION_META_NAME  = "Sigil version";
static const QString OPF_XML_NAMESPACE        = "http://www.idpf.org/2007/opf";
static const QString FALLBACK_MIMETYPE        = "text/plain";
static const QString ITEM_ELEMENT_TEMPLATE    = "<item id=\"%1\" href=\"%2\" media-type=\"%3\"/>";
static const QString ITEMREF_ELEMENT_TEMPLATE = "<itemref idref=\"%1\"/>";
static const QString OPF_REWRITTEN_COMMENT    = "<!-- Your OPF file was broken so Sigil "
        "was forced to create a new one from scratch. -->";

static const QString TEMPLATE_TEXT =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<package version=\"2.0\" xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"BookId\">\n\n"
    "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n"
    "    <dc:identifier opf:scheme=\"UUID\" id=\"BookId\">urn:uuid:%1</dc:identifier>\n"
    "  </metadata>\n\n"
    "  <manifest>\n"
    "    <item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\"/>\n"
    "  </manifest>\n\n"
    "  <spine toc=\"ncx\">\n"
    "  </spine>\n\n"
    "</package>";


OPFResource::OPFResource(const QString &fullfilepath, QObject *parent)
    : XMLResource(fullfilepath, parent)
{
    CreateMimetypes();
    FillWithDefaultText();
    // Make sure the file exists on disk.
    // Among many reasons, this also solves the problem
    // with the Book Browser not displaying an icon for this resource.
    SaveToDisk();
}


bool OPFResource::RenameTo(const QString &new_filename)
{
    // The user is not allowed to rename the OPF file.
    return false;
}


Resource::ResourceType OPFResource::Type() const
{
    return Resource::OPFResourceType;
}


GuideSemantics::GuideSemanticType OPFResource::GetGuideSemanticTypeForResource(const Resource &resource) const
{
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    return GetGuideSemanticTypeForResource(resource, *document);
}


int OPFResource::GetReadingOrder(const ::HTMLResource &html_resource) const
{
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    const Resource &resource = *static_cast< const Resource * >(&html_resource);
    QString resource_id = GetResourceManifestID(resource, *document);
    QList< xc::DOMElement * > itemrefs =
        XhtmlDoc::GetTagMatchingDescendants(*document, "itemref", OPF_XML_NAMESPACE);

    for (int i = 0; i < itemrefs.count(); ++i) {
        QString idref = XtoQ(itemrefs[ i ]->getAttribute(QtoX("idref")));

        if (resource_id == idref) {
            return i;
        }
    }

    return -1;
}


QString OPFResource::GetMainIdentifierValue() const
{
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    return XtoQ(GetMainIdentifier(*document).getTextContent());
}


void OPFResource::SaveToDisk(bool book_wide_save)
{
    QString text = GetText();
    // Work around for covers appearing on the Nook. Issue 942.
    text = text.replace(QRegularExpression("<meta content=\"([^\"]+)\" name=\"cover\""), "<meta name=\"cover\" content=\"\\1\"");
    SetText(text);
    TextResource::SaveToDisk(book_wide_save);
}

QString OPFResource::GetUUIDIdentifierValue()
{
    EnsureUUIDIdentifierPresent();
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement * > identifiers =
        XhtmlDoc::GetTagMatchingDescendants(*document, "identifier", DUBLIN_CORE_NS);
    foreach(xc::DOMElement * identifier, identifiers) {
        QString value = XtoQ(identifier->getTextContent()).remove("urn:uuid:");

        if (!QUuid(value).isNull()) {
            return value;
        }
    }
    // EnsureUUIDIdentifierPresent should ensure we
    // never reach here.
    Q_ASSERT(false);
    return QString();
}


void OPFResource::EnsureUUIDIdentifierPresent()
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement * > identifiers =
        XhtmlDoc::GetTagMatchingDescendants(*document, "identifier", DUBLIN_CORE_NS);
    foreach(xc::DOMElement * identifier, identifiers) {
        QString value = XtoQ(identifier->getTextContent()).remove("urn:uuid:");

        if (!QUuid(value).isNull()) {
            return;
        }
    }
    QString uuid = Utility::CreateUUID();
    WriteIdentifier("UUID", uuid, *document);
    UpdateTextFromDom(*document);
}

QString OPFResource::AddNCXItem(const QString &ncx_path)
{
    QWriteLocker locker(&GetLock());
    QString path_to_oebps_folder = QFileInfo(GetFullPath()).absolutePath() + "/";
    QString ncx_oebps_path  = Utility::URLEncodePath(QString(ncx_path).remove(path_to_oebps_folder));
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QHash< QString, QString > attributes;
    attributes[ "id"         ] = GetUniqueID("ncx", *document);
    attributes[ "href"       ] = Utility::URLEncodePath(ncx_oebps_path);
    attributes[ "media-type" ] = "application/x-dtbncx+xml";
    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument(
                                   "item", OPF_XML_NAMESPACE, *document, attributes);
    xc::DOMElement &manifest = GetManifestElement(*document);
    manifest.appendChild(new_item);
    UpdateTextFromDom(*document);
    return attributes[ "id" ];
}

void OPFResource::UpdateNCXOnSpine(const QString &new_ncx_id)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    xc::DOMElement &spine = GetSpineElement(*document);
    QString ncx_id = XtoQ(spine.getAttribute(QtoX("toc")));

    if (new_ncx_id != ncx_id) {
        spine.setAttribute(QtoX("toc"), QtoX(new_ncx_id));
        UpdateTextFromDom(*document);
    }
}

void OPFResource::UpdateNCXLocationInManifest(const ::NCXResource &ncx)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    xc::DOMElement &spine = GetSpineElement(*document);
    QString ncx_id = XtoQ(spine.getAttribute(QtoX("toc")));
    QList< xc::DOMElement * > items =
        XhtmlDoc::GetTagMatchingDescendants(*document, "item", OPF_XML_NAMESPACE);
    foreach(xc::DOMElement * item, items) {
        QString id = XtoQ(item->getAttribute(QtoX("id")));

        if (id == ncx_id) {
            item->setAttribute(QtoX("href"), QtoX(ncx.Filename()));
            break;
        }
    }
    UpdateTextFromDom(*document);
}


void OPFResource::AddSigilVersionMeta()
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement * > metas =
        XhtmlDoc::GetTagMatchingDescendants(*document, "meta", OPF_XML_NAMESPACE);
    foreach(xc::DOMElement * meta, metas) {
        QString name = XtoQ(meta->getAttribute(QtoX("name")));

        if (name == SIGIL_VERSION_META_NAME) {
            meta->setAttribute(QtoX("content"), QtoX(SIGIL_VERSION));
            UpdateTextFromDom(*document);
            return;
        }
    }
    xc::DOMElement *element = document->createElementNS(QtoX(OPF_XML_NAMESPACE), QtoX("meta"));
    element->setAttribute(QtoX("name"),    QtoX("Sigil version"));
    element->setAttribute(QtoX("content"), QtoX(SIGIL_VERSION));
    xc::DOMElement &metadata = GetMetadataElement(*document);
    metadata.appendChild(element);
    UpdateTextFromDom(*document);
}


bool OPFResource::IsCoverImage(const ::ImageResource &image_resource) const
{
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    return IsCoverImageCheck(image_resource, *document);
}

bool OPFResource::IsCoverImageCheck(const Resource &resource, xc::DOMDocument &document) const
{
    QString resource_id = GetResourceManifestID(resource, document);
    return IsCoverImageCheck(resource_id, document);
}

bool OPFResource::IsCoverImageCheck(QString resource_id, xc::DOMDocument &document) const
{
    xc::DOMElement *meta = GetCoverMeta(document);

    if (meta) {
        return XtoQ(meta->getAttribute(QtoX("content"))) == resource_id;
    }

    return false;
}


bool OPFResource::CoverImageExists() const
{
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    return GetCoverMeta(*document) != NULL;
}


void OPFResource::AutoFixWellFormedErrors()
{
    QWriteLocker locker(&GetLock());
    UpdateTextFromDom(*CreateOPFFromScratch());
}


QStringList OPFResource::GetSpineOrderFilenames() const
{
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement * > items =
        XhtmlDoc::GetTagMatchingDescendants(*document, "item", OPF_XML_NAMESPACE);
    QHash< QString, QString > id_to_filename_mapping;
    foreach(xc::DOMElement * item, items) {
        QString id   = XtoQ(item->getAttribute(QtoX("id")));
        QString href = XtoQ(item->getAttribute(QtoX("href")));
        id_to_filename_mapping[ id ] = QFileInfo(href).fileName();
    }
    QList< xc::DOMElement * > itemrefs =
        XhtmlDoc::GetTagMatchingDescendants(*document, "itemref", OPF_XML_NAMESPACE);
    QStringList filenames_in_reading_order;
    foreach(xc::DOMElement * itemref, itemrefs) {
        QString idref = XtoQ(itemref->getAttribute(QtoX("idref")));

        if (id_to_filename_mapping.contains(idref)) {
            filenames_in_reading_order.append(Utility::URLDecodePath(id_to_filename_mapping[ idref ]));
        }
    }
    return filenames_in_reading_order;
}


void OPFResource::SetSpineOrderFromFilenames(const QStringList spineOrder)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement * > items =
        XhtmlDoc::GetTagMatchingDescendants(*document, "item", OPF_XML_NAMESPACE);
    QHash< QString, QString > filename_to_id_mapping;
    foreach(xc::DOMElement * item, items) {
        QString id   = XtoQ(item->getAttribute(QtoX("id")));
        QString href = XtoQ(item->getAttribute(QtoX("href")));
    }
    QList< xc::DOMElement * > itemrefs =
        XhtmlDoc::GetTagMatchingDescendants(*document, "itemref", OPF_XML_NAMESPACE);
    QList< xc::DOMElement * > newSpine;
    foreach(QString spineItem, spineOrder) {
        QString id = filename_to_id_mapping[ spineItem ];
        bool found = false;
        QListIterator< xc::DOMElement * > spineElementSearch(itemrefs);

        while (spineElementSearch.hasNext() && !found) {
            xc::DOMElement *spineElement = spineElementSearch.next();

            if (XtoQ(spineElement->getAttribute(QtoX("idref"))) == spineItem) {
                newSpine.append(spineElement);
                found = true;
            }
        }
    }
    xc::DOMElement &spine = GetSpineElement(*document);
    XhtmlDoc::RemoveChildren(spine);
    QListIterator< xc::DOMElement * > spineWriter(newSpine);

    while (spineWriter.hasNext()) {
        spine.appendChild(spineWriter.next());
    }

    UpdateTextFromDom(*document);
}


QList< Metadata::MetaElement > OPFResource::GetDCMetadata() const
{
    QReadLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement * > dc_elements =
        XhtmlDoc::GetTagMatchingDescendants(*document, "*", DUBLIN_CORE_NS);
    QList< Metadata::MetaElement > metadata;
    foreach(xc::DOMElement * dc_element, dc_elements) {
        // Map the names in the OPF file to internal names
        Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata(*dc_element);

        if (!book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty()) {
            metadata.append(book_meta);
        }
    }
    return metadata;
}


QList< QVariant > OPFResource::GetDCMetadataValues(QString text) const
{
    QList< QVariant > values;
    foreach(Metadata::MetaElement meta, GetDCMetadata()) {
        if (meta.name == text) {
            values.append(meta.value);
        }
    }
    return values;
}


void OPFResource::SetDCMetadata(const QList< Metadata::MetaElement > &metadata)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    RemoveDCElements(*document);
    foreach(Metadata::MetaElement book_meta, metadata) {
        MetadataDispatcher(book_meta, *document);
    }
    SetMetaElementsLast(*document);
    UpdateTextFromDom(*document);
}


void OPFResource::AddResource(const Resource &resource)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QHash< QString, QString > attributes;
    attributes[ "id"         ] = GetUniqueID(GetValidID(resource.Filename()), *document);
    attributes[ "href"       ] = Utility::URLEncodePath(resource.GetRelativePathToOEBPS());
    attributes[ "media-type" ] = GetResourceMimetype(resource);
    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument(
                                   "item", OPF_XML_NAMESPACE, *document, attributes);
    xc::DOMElement &manifest = GetManifestElement(*document);
    manifest.appendChild(new_item);

    if (resource.Type() == Resource::HTMLResourceType) {
        AppendToSpine(attributes[ "id" ], *document);
    }

    UpdateTextFromDom(*document);
}

void OPFResource::RemoveCoverMetaForImage(const Resource &resource, xc::DOMDocument &document)
{
    xc::DOMElement *meta = GetCoverMeta(document);
    QString resource_id = GetResourceManifestID(resource, document);

    // Remove entry if there is a cover in meta and if this file is marked as cover
    if (meta && XtoQ(meta->getAttribute(QtoX("content"))) == resource_id) {
        GetMetadataElement(document).removeChild(meta);
    }
}

void OPFResource::AddCoverMetaForImage(const Resource &resource, xc::DOMDocument &document)
{
    xc::DOMElement *meta = GetCoverMeta(document);
    QString resource_id = GetResourceManifestID(resource, document);

    // If a cover entry exists, update its id, else create one
    if (meta) {
        meta->setAttribute(QtoX("content"), QtoX(resource_id));
    } else {
        QHash< QString, QString > attributes;
        attributes[ "name"    ] = "cover";
        attributes[ "content" ] = resource_id;
        xc::DOMElement *new_meta = XhtmlDoc::CreateElementInDocument(
                                       "meta", OPF_XML_NAMESPACE, document, attributes);
        xc::DOMElement &metadata = GetMetadataElement(document);
        metadata.appendChild(new_meta);
    }
}

void OPFResource::RemoveResource(const Resource &resource)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document  = GetDocument();
    xc::DOMElement &manifest                = GetManifestElement(*document);
    std::vector< xc::DOMElement * > children = xe::GetElementChildren(manifest);
    QString resource_oebps_path             = Utility::URLEncodePath(resource.GetRelativePathToOEBPS());
    QString item_id;

    // Delete the meta tag for cover images before deleting the manifest entry
    if (resource.Type() == Resource::ImageResourceType) {
        RemoveCoverMetaForImage(resource, *document);
    }

    foreach(xc::DOMElement * child, children) {
        QString href = XtoQ(child->getAttribute(QtoX("href")));

        if (href == resource_oebps_path) {
            item_id = XtoQ(child->getAttribute(QtoX("id")));
            manifest.removeChild(child);
            break;
        }
    }

    if (resource.Type() == Resource::HTMLResourceType) {
        RemoveFromSpine(item_id, *document);
        RemoveGuideReferenceForResource(resource, *document);
    }

    UpdateTextFromDom(*document);
}


void OPFResource::AddGuideSemanticType(
    const ::HTMLResource &html_resource,
    GuideSemantics::GuideSemanticType new_type)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document         = GetDocument();
    GuideSemantics::GuideSemanticType current_type = GetGuideSemanticTypeForResource(html_resource, *document);

    if (current_type != new_type) {
        RemoveDuplicateGuideTypes(new_type, *document);
        SetGuideSemanticTypeForResource(new_type, html_resource, *document);
    }
    // If the current type is the same as the new one,
    // we toggle it off.
    else {
        RemoveGuideReferenceForResource(html_resource, *document);
    }

    UpdateTextFromDom(*document);
}


void OPFResource::SetResourceAsCoverImage(const ::ImageResource &image_resource)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();

    if (IsCoverImageCheck(image_resource, *document)) {
        RemoveCoverMetaForImage(image_resource, *document);
    } else {
        AddCoverMetaForImage(image_resource, *document);
    }

    UpdateTextFromDom(*document);
}


void OPFResource::UpdateSpineOrder(const QList< ::HTMLResource * > html_files)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QHash< ::HTMLResource *, xc::DOMElement * > itemref_mapping =
        GetItemrefsForHTMLResources(html_files, *document);
    xc::DOMElement &spine = GetSpineElement(*document);
    XhtmlDoc::RemoveChildren(spine);
    foreach(::HTMLResource * resource, html_files) {
        xc::DOMElement *itemref = itemref_mapping.value(resource, NULL);

        if (itemref) {
            spine.appendChild(itemref);
        }
    }
    UpdateTextFromDom(*document);
}


void OPFResource::ResourceRenamed(const Resource &resource, QString old_full_path)
{
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QString path_to_oebps_folder = QFileInfo(GetFullPath()).absolutePath() + "/";
    QString resource_oebps_path  = Utility::URLEncodePath(QString(old_full_path).remove(path_to_oebps_folder));
    QList< xc::DOMElement * > items =
        XhtmlDoc::GetTagMatchingDescendants(*document, "item", OPF_XML_NAMESPACE);
    QString old_id;
    QString new_id;
    foreach(xc::DOMElement * item, items) {
        QString href = XtoQ(item->getAttribute(QtoX("href")));

        if (href == resource_oebps_path) {
            item->setAttribute(QtoX("href"), QtoX(Utility::URLEncodePath(resource.GetRelativePathToOEBPS())));
            old_id = XtoQ(item->getAttribute(QtoX("id")));
            new_id = GetUniqueID(GetValidID(resource.Filename()), *document);
            item->setAttribute(QtoX("id"), QtoX(new_id));
            break;
        }
    }
    UpdateItemrefID(old_id, new_id, *document);

    if (resource.Type() == Resource::ImageResourceType) {
        // Change meta entry for cover if necessary
        // Check using IDs since file is already renamed
        if (IsCoverImageCheck(old_id, *document)) {
            // Add will automatically replace an existing id
            // Assumes only one cover but removing duplicates
            // can cause timing issues
            AddCoverMetaForImage(resource, *document);
        }
    }

    UpdateTextFromDom(*document);
}


void OPFResource::AppendToSpine(const QString &id, xc::DOMDocument &document)
{
    QHash< QString, QString > attributes;
    attributes[ "idref" ] = id;
    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument(
                                   "itemref", OPF_XML_NAMESPACE, document, attributes);
    xc::DOMElement &spine = GetSpineElement(document);
    spine.appendChild(new_item);
}


void OPFResource::RemoveFromSpine(const QString &id, xc::DOMDocument &document)
{
    xc::DOMElement &spine = GetSpineElement(document);
    std::vector< xc::DOMElement * > children = xe::GetElementChildren(spine);
    foreach(xc::DOMElement * child, children) {
        QString idref = XtoQ(child->getAttribute(QtoX("idref")));

        if (idref == id) {
            spine.removeChild(child);
            break;
        }
    }
}


void OPFResource::UpdateItemrefID(const QString &old_id, const QString &new_id, xc::DOMDocument &document)
{
    xc::DOMElement &spine = GetSpineElement(document);
    std::vector< xc::DOMElement * > children = xe::GetElementChildren(spine);
    foreach(xc::DOMElement * child, children) {
        QString idref = XtoQ(child->getAttribute(QtoX("idref")));

        if (idref == old_id) {
            child->setAttribute(QtoX("idref"), QtoX(new_id));
            break;
        }
    }
}


shared_ptr< xc::DOMDocument > OPFResource::GetDocument() const
{
    // The call to ProcessXML is needed because even though we have well-formed
    // checks tied to "focus lost" events of the OPF tab, on Win XP those events
    // are sometimes not delivered at all. Blame MS. In the mean time, this
    // work-around makes sure we get valid XML into Xerces no matter what.
    shared_ptr< xc::DOMDocument > document =
        XhtmlDoc::LoadTextIntoDocument(CleanSource::ProcessXML(GetText()));

    if (!BasicStructurePresent(*document)) {
        document = CreateOPFFromScratch();
    }

    // For NCX files, the default of standalone == false should remain
    document->setXmlStandalone(true);
    return document;
}


xc::DOMElement &OPFResource::GetPackageElement(const xc::DOMDocument &document)
{
    QList< xc::DOMElement * > packages =
        XhtmlDoc::GetTagMatchingDescendants(document, "package", OPF_XML_NAMESPACE);
    Q_ASSERT(!packages.isEmpty());
    return *packages[ 0 ];
}


xc::DOMElement &OPFResource::GetMetadataElement(const xc::DOMDocument &document)
{
    QList< xc::DOMElement * > metadatas =
        XhtmlDoc::GetTagMatchingDescendants(document, "metadata", OPF_XML_NAMESPACE);
    Q_ASSERT(!metadatas.isEmpty());
    return *metadatas[ 0 ];
}


xc::DOMElement &OPFResource::GetManifestElement(const xc::DOMDocument &document)
{
    QList< xc::DOMElement * > manifests =
        XhtmlDoc::GetTagMatchingDescendants(document, "manifest", OPF_XML_NAMESPACE);
    Q_ASSERT(!manifests.isEmpty());
    return *manifests[ 0 ];
}


xc::DOMElement &OPFResource::GetSpineElement(const xc::DOMDocument &document)
{
    QList< xc::DOMElement * > spines =
        XhtmlDoc::GetTagMatchingDescendants(document, "spine", OPF_XML_NAMESPACE);
    Q_ASSERT(!spines.isEmpty());
    return *spines[ 0 ];
}


xc::DOMElement &OPFResource::GetGuideElement(xc::DOMDocument &document)
{
    QList< xc::DOMElement * > guides =
        XhtmlDoc::GetTagMatchingDescendants(document, "guide", OPF_XML_NAMESPACE);

    if (!guides.isEmpty()) {
        return *guides[ 0 ];
    }

    xc::DOMElement *guide = XhtmlDoc::CreateElementInDocument(
                                "guide", OPF_XML_NAMESPACE, document, QHash< QString, QString >());
    xc::DOMElement &package = *document.getDocumentElement();
    package.appendChild(guide);
    return *guide;
}


xc::DOMElement *OPFResource::GetGuideReferenceForResource(const Resource &resource, const xc::DOMDocument &document)
{
    QString resource_oebps_path         = Utility::URLEncodePath(resource.GetRelativePathToOEBPS());
    QList< xc::DOMElement * > references =
        XhtmlDoc::GetTagMatchingDescendants(document, "reference", OPF_XML_NAMESPACE);
    foreach(xc::DOMElement * reference, references) {
        const QString &href = XtoQ(reference->getAttribute(QtoX("href")));
        QStringList parts = href.split('#', QString::KeepEmptyParts);

        if (parts.at(0) == resource_oebps_path) {
            return reference;
        }
    }
    return NULL;
}


void OPFResource::RemoveGuideReferenceForResource(const Resource &resource, xc::DOMDocument &document)
{
    xc::DOMElement &guide = GetGuideElement(document);
    xc::DOMElement *elem = GetGuideReferenceForResource(resource, document);

    if (elem) {
        guide.removeChild(elem);
    }
}


GuideSemantics::GuideSemanticType OPFResource::GetGuideSemanticTypeForResource(
    const Resource &resource,
    xc::DOMDocument &document)
{
    xc::DOMElement *reference = GetGuideReferenceForResource(resource, document);

    if (reference) {
        QString type = XtoQ(reference->getAttribute(QtoX("type")));
        return GuideSemantics::Instance().MapReferenceTypeToGuideEnum(type);
    }

    return GuideSemantics::NoType;
}


void OPFResource::SetGuideSemanticTypeForResource(
    GuideSemantics::GuideSemanticType type,
    const Resource &resource,
    xc::DOMDocument &document)
{
    xc::DOMElement *reference = GetGuideReferenceForResource(resource, document);
    QString type_attribute;
    QString title_attribute;
    tie(type_attribute, title_attribute) = GuideSemantics::Instance().GetGuideTypeMapping()[ type ];

    if (reference) {
        reference->setAttribute(QtoX("type"), QtoX(type_attribute));
    } else {
        QHash< QString, QString > attributes;
        attributes[ "type"  ] = type_attribute;
        attributes[ "title" ] = title_attribute;
        attributes[ "href"  ] = Utility::URLEncodePath(resource.GetRelativePathToOEBPS());
        xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument(
                                       "reference", OPF_XML_NAMESPACE, document, attributes);
        xc::DOMElement &guide = GetGuideElement(document);
        guide.appendChild(new_item);
    }
}


void OPFResource::RemoveDuplicateGuideTypes(
    GuideSemantics::GuideSemanticType new_type,
    xc::DOMDocument &document)
{
    // Industry best practice is to have only one
    // <guide> reference type instance per book.
    // For NoType, there is nothing to remove.
    if (new_type == GuideSemantics::NoType) {
        return;
    }

    xc::DOMElement &guide               = GetGuideElement(document);
    QList< xc::DOMElement * > references =
        XhtmlDoc::GetTagMatchingDescendants(document, "reference", OPF_XML_NAMESPACE);
    foreach(xc::DOMElement * reference, references) {
        QString type_text = XtoQ(reference->getAttribute(QtoX("type")));
        GuideSemantics::GuideSemanticType current_type =
            GuideSemantics::Instance().MapReferenceTypeToGuideEnum(type_text);

        if (current_type == new_type) {
            guide.removeChild(reference);
            // There is no "break" statement here because we might
            // load an epub that has several instance of one guide type.
            // We preserve them on load, but if the user is intent on
            // changing them, then we enforce "one type instance per book".
        }
    }
}

// If there is no itemref for the resource, one will be created (but NOT
// attached to the spine element!).
// Also, it's possible that a NULL will be set as an itemref for a resource
// if that resource doesns't have an entry in the manifest.
QHash< ::HTMLResource *, xc::DOMElement * > OPFResource::GetItemrefsForHTMLResources(
    const QList< ::HTMLResource * > html_files,
    xc::DOMDocument &document)
{
    QList< xc::DOMElement * > itemrefs =
        XhtmlDoc::GetTagMatchingDescendants(document, "itemref", OPF_XML_NAMESPACE);
    QList< Resource * > resource_list;
    foreach(::HTMLResource * html_resource, html_files) {
        resource_list.append(static_cast< Resource * >(html_resource));
    }
    QHash< Resource *, QString > id_mapping = GetResourceManifestIDMapping(resource_list, document);
    QList< Resource * > htmls_without_itemrefs;
    QHash< ::HTMLResource *, xc::DOMElement * > itmeref_mapping;
    foreach(Resource * resource, resource_list) {
        ::HTMLResource *html_resource = qobject_cast< ::HTMLResource * >(resource);
        QString resource_id = id_mapping.value(resource, "");
        foreach(xc::DOMElement * itemref, itemrefs) {
            QString idref = XtoQ(itemref->getAttribute(QtoX("idref")));

            if (idref == resource_id) {
                itmeref_mapping[ html_resource ] = itemref;
                break;
            }
        }

        if (!itmeref_mapping.contains(html_resource)) {
            htmls_without_itemrefs.append(resource);
        }
    }
    foreach(Resource * resource, htmls_without_itemrefs) {
        QHash< QString, QString > attributes;
        QString resource_id = id_mapping.value(resource, "");
        ::HTMLResource *html_resource = qobject_cast< ::HTMLResource * >(resource);

        if (resource_id.isEmpty()) {
            itmeref_mapping[ html_resource ] = NULL;
        }

        attributes[ "idref" ] = resource_id;
        xc::DOMElement *new_itemref = XhtmlDoc::CreateElementInDocument(
                                          "itemref", OPF_XML_NAMESPACE, document, attributes);
        itmeref_mapping[ html_resource ] = new_itemref;
    }
    return itmeref_mapping;
}


xc::DOMElement *OPFResource::GetCoverMeta(const xc::DOMDocument &document)
{
    QList< xc::DOMElement * > metas =
        XhtmlDoc::GetTagMatchingDescendants(document, "meta", OPF_XML_NAMESPACE);
    foreach(xc::DOMElement * meta, metas) {
        QString name = XtoQ(meta->getAttribute(QtoX("name")));

        if (name == "cover") {
            return meta;
        }
    }
    return NULL;
}


xc::DOMElement &OPFResource::GetMainIdentifier(const xc::DOMDocument &document)
{
    xc::DOMElement *identifier = GetMainIdentifierUnsafe(document);
    Q_ASSERT(identifier);
    return *identifier;
}


// This is here because we use it to check for the presence of the main identifier
// during DOM validation. But after that, the GetMainIdentifier func should be used.
xc::DOMElement *OPFResource::GetMainIdentifierUnsafe(const xc::DOMDocument &document)
{
    xc::DOMElement &package = GetPackageElement(document);
    QString unique_identifier = XtoQ(package.getAttribute(QtoX("unique-identifier")));
    QList< xc::DOMElement * > identifiers =
        XhtmlDoc::GetTagMatchingDescendants(document, "identifier", DUBLIN_CORE_NS);
    foreach(xc::DOMElement * identifier, identifiers) {
        QString id = XtoQ(identifier->getAttribute(QtoX("id")));

        if (id == unique_identifier) {
            return identifier;
        }
    }
    return NULL;
}

QString OPFResource::GetResourceManifestID(const Resource &resource, const xc::DOMDocument &document)
{
    QString oebps_path = Utility::URLEncodePath(resource.GetRelativePathToOEBPS());
    QList< xc::DOMElement * > items =
        XhtmlDoc::GetTagMatchingDescendants(document, "item", OPF_XML_NAMESPACE);
    foreach(xc::DOMElement * item, items) {
        QString href = XtoQ(item->getAttribute(QtoX("href")));

        if (href == oebps_path) {
            return XtoQ(item->getAttribute(QtoX("id")));
        }
    }
    return QString();
}


QHash< Resource *, QString > OPFResource::GetResourceManifestIDMapping(
    const QList< Resource * > resources,
    const xc::DOMDocument &document)
{
    QHash< Resource *, QString > id_mapping;
    QList< xc::DOMElement * > items =
        XhtmlDoc::GetTagMatchingDescendants(document, "item", OPF_XML_NAMESPACE);
    foreach(Resource * resource, resources) {
        QString oebps_path = Utility::URLEncodePath(resource->GetRelativePathToOEBPS());
        foreach(xc::DOMElement * item, items) {
            QString href = XtoQ(item->getAttribute(QtoX("href")));

            if (href == oebps_path) {
                id_mapping[ resource ] = XtoQ(item->getAttribute(QtoX("id")));
                break;
            }
        }
    }
    return id_mapping;
}


void OPFResource::SetMetaElementsLast(xc::DOMDocument &document)
{
    QList< xc::DOMElement * > metas =
        XhtmlDoc::GetTagMatchingDescendants(document, "meta", OPF_XML_NAMESPACE);
    xc::DOMElement &metadata = GetMetadataElement(document);
    foreach(xc::DOMElement * meta, metas) {
        // This makes sure that the <meta> elements come last
        metadata.removeChild(meta);
        metadata.appendChild(meta);
    }
}


void OPFResource::RemoveDCElements(xc::DOMDocument &document)
{
    QList< xc::DOMElement * > dc_elements = XhtmlDoc::GetTagMatchingDescendants(document, "*", DUBLIN_CORE_NS);
    xc::DOMElement &main_identifier = GetMainIdentifier(document);
    foreach(xc::DOMElement * dc_element, dc_elements) {
        // We preserve the original main identifier. Users
        // complain when we don't.
        if (dc_element->isSameNode(&main_identifier)) {
            continue;
        }

        xc::DOMNode *parent = dc_element->getParentNode();

        if (parent) {
            parent->removeChild(dc_element);
        }
    }
}


void OPFResource::MetadataDispatcher(const Metadata::MetaElement &book_meta, xc::DOMDocument &document)
{
    // We ignore badly formed meta elements.
    if (book_meta.name.isEmpty() || book_meta.value.isNull()) {
        return;
    }

    // Write Relator codes (always write author as relator code)
    if (Metadata::Instance().IsRelator(book_meta.name) || book_meta.name == "author") {
        WriteCreatorOrContributor(book_meta, document);
    }
    // There is a relator for the publisher, but there is
    // also a special publisher element that we would rather use
    else if (book_meta.name == "pub") {
        WriteSimpleMetadata("publisher", book_meta.value.toString(), document);
    } else if (book_meta.name  == "language") {
        WriteSimpleMetadata(book_meta.name,
                            Language::instance()->GetLanguageCode(book_meta.value.toString()),
                            document);
    } else if (book_meta.name  == "identifier") {
        WriteIdentifier(book_meta.file_as, book_meta.value.toString(), document);
    } else if (book_meta.name == "date") {
        WriteDate(book_meta.file_as, book_meta.value, document);
    } else {
        WriteSimpleMetadata(book_meta.name, book_meta.value.toString(), document);
    }
}


void OPFResource::WriteCreatorOrContributor(const Metadata::MetaElement book_meta, xc::DOMDocument &document)
{
    QString value = book_meta.value.toString();
    QString file_as = book_meta.file_as;
    QString role_type = book_meta.role_type;
    QString name = book_meta.name;

    if (name == "author") {
        name = "aut";
    }

    // Must have a role type
    if (role_type.isEmpty()) {
        role_type = "contributor";
    }

    // This assumes that the "dc" prefix has been declared for the DC namespace
    xc::DOMElement *element = document.createElementNS(QtoX(DUBLIN_CORE_NS), QtoX("dc:" + role_type));
    element->setAttributeNS(QtoX(OPF_XML_NAMESPACE), QtoX("opf:role"), QtoX(name));

    if (!file_as.isEmpty()) {
        element->setAttributeNS(QtoX(OPF_XML_NAMESPACE), QtoX("opf:file-as"), QtoX(file_as));
    }

    element->setTextContent(QtoX(value));
    xc::DOMElement &metadata = GetMetadataElement(document);
    metadata.appendChild(element);
}


void OPFResource::WriteSimpleMetadata(
    const QString &metaname,
    const QString &metavalue,
    xc::DOMDocument &document)
{
    try {
        // This assumes that the "dc" prefix has been declared for the DC namespace
        xc::DOMElement *element = document.createElementNS(QtoX(DUBLIN_CORE_NS), QtoX("dc:" + metaname));
        element->setTextContent(QtoX(metavalue));
        xc::DOMElement &metadata = GetMetadataElement(document);
        metadata.appendChild(element);
    } catch (...) {
        // "dc" prefix isn't always declared when importing HTML.
    }
}


void OPFResource::WriteIdentifier(
    const QString &metaname,
    const QString &metavalue,
    xc::DOMDocument &document)
{
    xc::DOMElement &main_identifier = GetMainIdentifier(document);

    // There's a possibility that this identifier is a duplicate
    // of the main identifier that we preserved, so we don't write
    // it out if it is.
    if (metavalue == XtoQ(main_identifier.getTextContent()) &&
        metaname == XtoQ(main_identifier.getAttributeNS(QtoX(OPF_XML_NAMESPACE), QtoX("scheme")))) {
        return;
    }

    // This assumes that the "dc" prefix has been declared for the DC namespace
    xc::DOMElement *element = document.createElementNS(QtoX(DUBLIN_CORE_NS), QtoX("dc:identifier"));
    element->setAttributeNS(QtoX(OPF_XML_NAMESPACE), QtoX("opf:scheme"), QtoX(metaname));

    if (metaname.toLower() == "uuid" && !metavalue.contains("urn:uuid:")) {
        element->setTextContent(QtoX("urn:uuid:" + metavalue));
    } else {
        element->setTextContent(QtoX(metavalue));
    }

    xc::DOMElement &metadata = GetMetadataElement(document);
    metadata.appendChild(element);
}

void OPFResource::AddModificationDateMeta()
{
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    QWriteLocker locker(&GetLock());
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement * > metas =
        XhtmlDoc::GetTagMatchingDescendants(*document, "date", DUBLIN_CORE_NS);
    foreach(xc::DOMElement * meta, metas) {
        QString name = XtoQ(meta->getAttribute(QtoX("opf:event")));

        if (name == "modification") {
            meta->setTextContent(QtoX(date));
            UpdateTextFromDom(*document);
            return;
        }
    }
    xc::DOMElement *element = document->createElementNS(QtoX(DUBLIN_CORE_NS), QtoX("dc:date"));
    element->setAttributeNS(QtoX(OPF_XML_NAMESPACE), QtoX("opf:event"), QtoX("modification"));
    element->setTextContent(QtoX(date));
    xc::DOMElement &metadata = GetMetadataElement(*document);
    metadata.appendChild(element);
    UpdateTextFromDom(*document);
}

void OPFResource::WriteDate(
    const QString &metaname,
    const QVariant &metavalue,
    xc::DOMDocument &document)
{
    QString date = metavalue.toDate().toString("yyyy-MM-dd");
    // This assumes that the "dc" prefix has been declared for the DC namespace
    xc::DOMElement *element = document.createElementNS(QtoX(DUBLIN_CORE_NS), QtoX("dc:date"));
    element->setAttributeNS(QtoX(OPF_XML_NAMESPACE), QtoX("opf:event"), QtoX(metaname));
    element->setTextContent(QtoX(date));
    xc::DOMElement &metadata = GetMetadataElement(document);
    metadata.appendChild(element);
}


bool OPFResource::BasicStructurePresent(const xc::DOMDocument &document)
{
    QList< xc::DOMElement * > packages =
        XhtmlDoc::GetTagMatchingDescendants(document, "package", OPF_XML_NAMESPACE);

    if (packages.count() != 1) {
        return false;
    }

    QList< xc::DOMElement * > metadatas =
        XhtmlDoc::GetTagMatchingDescendants(document, "metadata", OPF_XML_NAMESPACE);

    if (metadatas.count() != 1) {
        return false;
    }

    QList< xc::DOMElement * > manifests =
        XhtmlDoc::GetTagMatchingDescendants(document, "manifest", OPF_XML_NAMESPACE);

    if (manifests.count() != 1) {
        return false;
    }

    QList< xc::DOMElement * > spines =
        XhtmlDoc::GetTagMatchingDescendants(document, "spine", OPF_XML_NAMESPACE);

    if (spines.count() != 1) {
        return false;
    }

    xc::DOMElement *identifier = GetMainIdentifierUnsafe(document);

    if (!identifier) {
        return false;
    }

    return true;
}


shared_ptr< xc::DOMDocument > OPFResource::CreateOPFFromScratch() const
{
    QString xml_source = GetOPFDefaultText();
    QString manifest_content;
    QString spine_content;
    QStringList relative_oebps_paths = GetRelativePathsToAllFilesInOEPBS();
    foreach(QString path, relative_oebps_paths) {
        // The OPF is not allowed to be in the manifest and the NCX
        // is already in the template.
        if (path.contains(OPF_FILE_NAME) || path.contains(NCX_FILE_NAME)) {
            continue;
        }

        QString item_id = GetValidID(QFileInfo(path).fileName());
        QString item = ITEM_ELEMENT_TEMPLATE
                       .arg(item_id)
                       .arg(path)
                       .arg(GetFileMimetype(path));
        manifest_content.append(item);

        if (TEXT_EXTENSIONS.contains(QFileInfo(path).suffix().toLower())) {
            spine_content.append(ITEMREF_ELEMENT_TEMPLATE.arg(item_id));
        }
    }
    xml_source.replace("</manifest>", manifest_content + "</manifest>")
    .replace("</spine>", spine_content + "</spine>")
    .replace("<metadata", OPF_REWRITTEN_COMMENT + "<metadata");
    shared_ptr< xc::DOMDocument > document =
        XhtmlDoc::LoadTextIntoDocument(xml_source);
    document->setXmlStandalone(true);
    return document;
}


// Yeah, we could get this list of paths with the GetSortedContentFilesList()
// func from FolderKeeper, but let's not create a strong coupling from
// the opf to the FK just yet. If we can work without that dependency,
// then let's do so.
QStringList OPFResource::GetRelativePathsToAllFilesInOEPBS() const
{
    // The parent folder of the OPF will always be the OEBPS folder.
    QString path_to_oebps_folder = QFileInfo(GetFullPath()).absolutePath();
    QStringList paths = Utility::GetAbsolutePathsToFolderDescendantFiles(path_to_oebps_folder);
    paths.replaceInStrings(Utility::URLEncodePath(path_to_oebps_folder) + "/", "");
    paths.sort();
    return paths;
}


QString OPFResource::GetOPFDefaultText()
{
    return TEMPLATE_TEXT.arg(Utility::CreateUUID());
}


void OPFResource::FillWithDefaultText()
{
    SetText(GetOPFDefaultText());
}


QString OPFResource::GetUniqueID(const QString &preferred_id, const xc::DOMDocument &document) const
{
    xc::DOMElement *element = document.getElementById(QtoX(preferred_id));

    if (!element) {
        return preferred_id;
    }

    return Utility::CreateUUID();
}


QString OPFResource::GetResourceMimetype(const Resource &resource) const
{
    return GetFileMimetype(resource.Filename());
}


QString OPFResource::GetFileMimetype(const QString &filepath) const
{
    return m_Mimetypes.value(QFileInfo(filepath).suffix().toLower(), FALLBACK_MIMETYPE);
}


// Initializes m_Mimetypes
void OPFResource::CreateMimetypes()
{
    m_Mimetypes[ "jpg"   ] = "image/jpeg";
    m_Mimetypes[ "jpeg"  ] = "image/jpeg";
    m_Mimetypes[ "png"   ] = "image/png";
    m_Mimetypes[ "gif"   ] = "image/gif";
    m_Mimetypes[ "tif"   ] = "image/tiff";
    m_Mimetypes[ "tiff"  ] = "image/tiff";
    m_Mimetypes[ "bm"    ] = "image/bmp";
    m_Mimetypes[ "bmp"   ] = "image/bmp";
    m_Mimetypes[ "svg"   ] = "image/svg+xml";
    m_Mimetypes[ "ncx"   ] = NCX_MIMETYPE;
    // We convert all HTML document types to XHTML
    m_Mimetypes[ "xml"   ] = "application/xhtml+xml";
    m_Mimetypes[ "xhtml" ] = "application/xhtml+xml";
    m_Mimetypes[ "html"  ] = "application/xhtml+xml";
    m_Mimetypes[ "htm"   ] = "application/xhtml+xml";
    m_Mimetypes[ "css"   ] = "text/css";
    // Until the standards gods grace us with font mimetypes,
    // these will have to do
    m_Mimetypes[ "otf"   ] = "application/vnd.ms-opentype";
    m_Mimetypes[ "ttf"   ] = "application/x-font-ttf";
    m_Mimetypes[ "ttc"   ] = "application/x-font-truetype-collection";
}
