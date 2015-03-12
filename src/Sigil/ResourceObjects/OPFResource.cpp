/************************************************************************
**
**  Copyright (C) 2013              John Schember <john@nachtimwald.com>
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

#include <memory>

#include "Misc/EmbeddedPython.h"

// XercesExtensions
#include <XmlUtils.h>

#include <QtCore/QBuffer>
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUuid>
#include <QRegularExpression>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Language.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/ImageResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"

static const QString SIGIL_VERSION_META_NAME  = "Sigil version";
static const QString OPF_XML_NAMESPACE        = "http://www.idpf.org/2007/opf";
static const QString FALLBACK_MIMETYPE        = "text/plain";
static const QString ITEM_ELEMENT_TEMPLATE    = "<item id=\"%1\" href=\"%2\" media-type=\"%3\"/>";
static const QString ITEMREF_ELEMENT_TEMPLATE = "<itemref idref=\"%1\"/>";
static const QString OPF_REWRITTEN_COMMENT    = "<!-- Your OPF file was broken so Sigil "
        "tried to rebuild it for you. -->";

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
    "  <guide>\n\n</guide>\n\n"
    "</package>";


OPFResource::OPFResource(const QString &mainfolder, const QString &fullfilepath, QObject *parent)
  : XMLResource(mainfolder, fullfilepath, parent), m_idpos(QHash<QString,int>()), m_hrefpos(QHash<QString,int>())
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


QString OPFResource::GetText() const
{
    QMutexLocker locker(&m_AccessMutex);
    return convert_to_xml();
}


void OPFResource::SetText(const QString &text)
{
    QMutexLocker locker(&m_AccessMutex);
    QString source = CleanSource::ProcessXML(text);
    int rv = 0;
    QString traceback;

    QList<QVariant> args;
    args.append(QVariant(source));
    EmbeddedPython* epp = EmbeddedPython::instance();
    QVariant res = epp->runInPython( QString("opf_newparser"), QString("parseopf"), args, &rv, traceback, true);
    if (rv) fprintf(stderr, "setext parseropf error %d traceback %s\n",rv, traceback.toStdString().c_str());
    PyObjectPtr mpo = PyObjectPtr(res);

    args.clear();
    res = epp->callPyObjMethod(mpo, QString("get_package"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext package error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_package = PackageEntry(res);

    res = epp->callPyObjMethod(mpo, QString("get_metadata_attr"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext meta_attr error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_metans = MetaNSEntry(res);

    res = epp->callPyObjMethod(mpo, QString("get_metadata"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext metadata error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_metadata.clear();
    QList<QVariant> lst = res.toList();
    foreach(QVariant qv, lst) {
      m_metadata.append(MetaEntry(qv));
    }

    m_idpos.clear();
    m_hrefpos.clear();

    res = epp->callPyObjMethod(mpo, QString("get_manifest"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext manifest error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_manifest.clear();
    lst = res.toList();
    for (int i = 0; i < lst.count(); i++) {
      ManifestEntry me = ManifestEntry(lst.at(i));
      m_idpos[me.m_id] = i;
      m_hrefpos[me.m_href] = i;
      m_manifest.append(me);
    }

    res = epp->callPyObjMethod(mpo, QString("get_spine_attr"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext spineattr error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_spineattr = SpineAttrEntry(res);

    res = epp->callPyObjMethod(mpo, QString("get_spine"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext spine error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_spine.clear();
    lst = res.toList();
    foreach(QVariant qv, lst) {
      m_spine.append(SpineEntry(qv));
    }

    res = epp->callPyObjMethod(mpo, QString("get_guide"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext guide error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_guide.clear();
    lst = res.toList();
    foreach(QVariant qv, lst) {
      m_guide.append(GuideEntry(qv));
    }

    res = epp->callPyObjMethod(mpo, QString("get_bindings"), args, &rv, traceback);
    if (rv) fprintf(stderr, "setext bindings error %d traceback %s\n",rv, traceback.toStdString().c_str());
    m_bindings.clear();
    lst = res.toList();
    foreach(QVariant qv, lst) {
      m_bindings.append(BindingsEntry(qv));
    }

    TextResource::SetText(text);
}


GuideSemantics::GuideSemanticType OPFResource::GetGuideSemanticTypeForResource(const Resource &resource) const
{
    QMutexLocker locker(&m_AccessMutex);
    return GetGuideSemanticTypeForResource2(resource);
}


QString OPFResource::GetGuideSemanticNameForResource(Resource *resource)
{
    QMutexLocker locker(&m_AccessMutex);
    return GuideSemantics::Instance().GetGuideName(GetGuideSemanticTypeForResource(*resource));
}

QHash <QString, QString>  OPFResource::GetGuideSemanticNameForPaths()
{
    QMutexLocker locker(&m_AccessMutex);
    QHash <QString, QString> semantic_types;

    foreach(GuideEntry ge, m_guide) {
        QString href = ge.m_href;
        QStringList parts = href.split('#', QString::KeepEmptyParts);

        QString type_text = ge.m_type;
        GuideSemantics::GuideSemanticType type =
            GuideSemantics::Instance().MapReferenceTypeToGuideEnum(type_text);
        semantic_types[parts.at(0)] = GuideSemantics::Instance().GetGuideName(type);
    }

    // Cover image semantics don't use reference
    int pos  = GetCoverMeta();
    if (pos > -1) {
        MetaEntry me = m_metadata.at(pos);
        QString cover_id = me.m_atts.value(QString("content"),QString(""));
        ManifestEntry man = m_manifest.at(m_idpos[cover_id]);
        QString href = man.m_href;
        GuideSemantics::GuideSemanticType type =
                    GuideSemantics::Instance().MapReferenceTypeToGuideEnum("cover");
        semantic_types[href] = GuideSemantics::Instance().GetGuideName(type);
    }

    return semantic_types;
}

QHash <Resource *, int>  OPFResource::GetReadingOrderAll( const QList <Resource *> resources)
{
    QMutexLocker locker(&m_AccessMutex);
    QHash <Resource *, int> reading_order;
    QHash<QString, int> id_order;
    for (int i = 0; i < m_spine.count(); ++i) {
      id_order[m_spine.at(i).m_idref] = i;
    }
    QHash<Resource *, QString> id_mapping = GetResourceManifestIDMapping(resources);
    foreach(Resource *resource, resources) {
        reading_order[resource] = id_order[id_mapping[resource]];
    }
    return reading_order;
}

int OPFResource::GetReadingOrder(const ::HTMLResource &html_resource) const
{
    QMutexLocker locker(&m_AccessMutex);
    const Resource &resource = *static_cast<const Resource *>(&html_resource);
    QString resource_id = GetResourceManifestID(resource);
    for (int i = 0; i < m_spine.count(); ++i) {
      QString idref = m_spine.at(i).m_idref;
      if (resource_id == idref) {
          return i;
      }
    }
    return -1;
}

QString OPFResource::GetMainIdentifierValue() const
{
    QMutexLocker locker(&m_AccessMutex);
    int i = GetMainIdentifier();
    if (i > -1) {
        return QString(m_metadata.at(i).m_content);
    }
    return QString();
}

void OPFResource::SaveToDisk(bool book_wide_save)
{
    QString text = GetText();
    // Work around for covers appearing on the Nook. Issue 942.
    text = text.replace(QRegularExpression("<meta content=\"([^\"]+)\" name=\"cover\""), "<meta name=\"cover\" content=\"\\1\"");
    TextResource::SetText(text);
    TextResource::SaveToDisk(book_wide_save);
}

QString OPFResource::GetUUIDIdentifierValue()
{
    EnsureUUIDIdentifierPresent();
    QMutexLocker locker(&m_AccessMutex);
    for (int i=0; i < m_metadata.count(); ++i) {
        MetaEntry me = m_metadata.at(i);
        if(me.m_name.startsWith("dc:identifier")) {
            QString value = QString(me.m_content).remove("urn:uuid:");
            if (!QUuid(value).isNull()) {
              return value;
            }
        }
    }
    // EnsureUUIDIdentifierPresent should ensure we
    // never reach here.
    Q_ASSERT(false);
    return QString();
}


void OPFResource::EnsureUUIDIdentifierPresent()
{
    QMutexLocker locker(&m_AccessMutex);
    for (int i=0; i < m_metadata.count(); ++i) {
        MetaEntry me = m_metadata.at(i);
        if(me.m_name.startsWith("dc:identifier")) {
            QString value = QString(me.m_content).remove("urn:uuid:");
            if (!QUuid(value).isNull()) {
                return;
            }
        }
    }
    QString uuid = Utility::CreateUUID();
    WriteIdentifier("UUID", uuid);
}

QString OPFResource::AddNCXItem(const QString &ncx_path)
{
    QMutexLocker locker(&m_AccessMutex);
    QString path_to_oebps_folder = QFileInfo(GetFullPath()).absolutePath() + "/";
    QString ncx_oebps_path  = QString(ncx_path).remove(path_to_oebps_folder);
    int n = m_manifest.count();
    ManifestEntry me;
    me.m_id = GetUniqueID("ncx");
    me.m_href = ncx_oebps_path;
    me.m_mtype = "application/x-dtbncx+xml";
    m_manifest.append(me);
    m_idpos[me.m_id] = n;
    m_hrefpos[me.m_href] = n;
    return me.m_id;
}


void OPFResource::UpdateNCXOnSpine(const QString &new_ncx_id)
{
    QMutexLocker locker(&m_AccessMutex);
    QString ncx_id = m_spineattr.m_atts.value(QString("toc"),"");
    if (new_ncx_id != ncx_id) {
        m_spineattr.m_atts[QString("toc")] = new_ncx_id;
    }
}


void OPFResource::UpdateNCXLocationInManifest(const ::NCXResource &ncx)
{
    QMutexLocker locker(&m_AccessMutex);
    QString ncx_id = m_spineattr.m_atts.value(QString("toc"),"");
    int pos = m_idpos.value(ncx_id, -1);
    if (pos > -1) {
        ManifestEntry me = m_manifest.at(pos);
        QString href = me.m_href;
        me.m_href = ncx.Filename();
        m_manifest.replace(pos, me);
        m_hrefpos.remove(href);
        m_hrefpos[ncx.Filename()] = pos;
    }
}


void OPFResource::AddSigilVersionMeta()
{
    QMutexLocker locker(&m_AccessMutex);
    for (int i=0; i < m_metadata.count(); ++i) {
        MetaEntry me = m_metadata.at(i);
        if ((me.m_name == "meta") && (me.m_atts.contains("name"))) {  
            QString name = me.m_atts[QString("name")];
            if (name == SIGIL_VERSION_META_NAME) {
                me.m_atts["content"] = QString(SIGIL_VERSION);
                m_metadata.replace(i, me);
                return;
            }
        }
    }
    MetaEntry me;
    me.m_name = "meta";
    me.m_atts[QString("name")] = QString("Sigil version");
    me.m_atts[QString("content")] = QString(SIGIL_VERSION);
    m_metadata.append(me);
}


bool OPFResource::IsCoverImage(const ::ImageResource &image_resource) const
{
    QMutexLocker locker(&m_AccessMutex);
    QString resource_id = GetResourceManifestID(image_resource);
    return IsCoverImageCheck(resource_id);
}


bool OPFResource::IsCoverImageCheck(QString resource_id) const
{
    int pos = GetCoverMeta();
    if (pos > -1) {
        MetaEntry me = m_metadata.at(pos);
        return me.m_atts.value(QString("content"),QString("")) == resource_id;
    }
    return false;
}


bool OPFResource::CoverImageExists() const
{
    return GetCoverMeta() > -1;
}


void OPFResource::AutoFixWellFormedErrors()
{
    QMutexLocker locker(&m_AccessMutex);
    QString source = CleanSource::ProcessXML(GetText());
    TextResource::SetText(source);
}


QStringList OPFResource::GetSpineOrderFilenames() const
{
    QMutexLocker locker(&m_AccessMutex);
    QStringList filenames_in_reading_order;
    for (int i=0; i < m_spine.count(); ++i) {
        SpineEntry sp = m_spine.at(i);
        QString idref = sp.m_idref;
        int pos = m_idpos.value(idref,-1);
        if (pos > -1) {
            QString href = m_manifest.at(pos).m_href;
            QString filename = QFileInfo(href).fileName();
            filenames_in_reading_order.append(filename);
        }
    }
    return filenames_in_reading_order;
}

#if 0
void OPFResource::SetSpineOrderFromFilenames(const QStringList spineOrder)
{
    QMutexLocker locker(&m_AccessMutex);
    QWriteLocker locker(&GetLock());
    std::shared_ptr<xc::DOMDocument> document = GetDocument();
    QList<xc::DOMElement *> items =
        XhtmlDoc::GetTagMatchingDescendants(*document, "item", OPF_XML_NAMESPACE);
    QHash<QString, QString> filename_to_id_mapping;
    foreach(xc::DOMElement * item, items) {
        QString id   = XtoQ(item->getAttribute(QtoX("id")));
        QString href = XtoQ(item->getAttribute(QtoX("href")));
    }
    QList<xc::DOMElement *> itemrefs =
        XhtmlDoc::GetTagMatchingDescendants(*document, "itemref", OPF_XML_NAMESPACE);
    QList<xc::DOMElement *> newSpine;
    foreach(QString spineItem, spineOrder) {
        QString id = filename_to_id_mapping[ spineItem ];
        bool found = false;
        QListIterator<xc::DOMElement *> spineElementSearch(itemrefs);

        while (spineElementSearch.hasNext() && !found) {
            xc::DOMElement *spineElement = spineElementSearch.next();

            if (XtoQ(spineElement->getAttribute(QtoX("idref"))) == spineItem) {
                newSpine.append(spineElement);
                found = true;
            }
        }
    }
    xc::DOMElement *spine = GetSpineElement(*document);
    if (!spine) {
        return;
    }
    XhtmlDoc::RemoveChildren(*spine);
    QListIterator<xc::DOMElement *> spineWriter(newSpine);

    while (spineWriter.hasNext()) {
        spine->appendChild(spineWriter.next());
    }

    UpdateTextFromDom(*document);
}
#endif

QList<Metadata::MetaElement> OPFResource::GetDCMetadata() const
{
    QMutexLocker locker(&m_AccessMutex);
    QList<Metadata::MetaElement> metadata;
    for (int i=0; i < m_metadata.count(); ++i) {
        MetaEntry me = m_metadata.at(i);
        if (me.m_name.startsWith("dc:")) {
            Metadata::MetaElement book_meta = Metadata::Instance().MapMetaEntryToBookMetadata(me);
            if (!book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty()) {
                metadata.append(book_meta);
            }
        }
    }
    return metadata;
}


QList<QVariant> OPFResource::GetDCMetadataValues(QString text) const
{
    QMutexLocker locker(&m_AccessMutex);
    QList<QVariant> values;
    foreach(Metadata::MetaElement meta, GetDCMetadata()) {
        if (meta.name == text) {
            values.append(meta.value);
        }
    }
    return values;
}


void OPFResource::SetDCMetadata(const QList<Metadata::MetaElement> &metadata)
{
    QMutexLocker locker(&m_AccessMutex);
    RemoveDCElements();
    foreach(Metadata::MetaElement book_meta, metadata) {
        MetadataDispatcher(book_meta);
    }
}


void OPFResource::AddResource(const Resource &resource)
{
    QMutexLocker locker(&m_AccessMutex);
    ManifestEntry me;
    me.m_id = GetUniqueID(GetValidID(resource.Filename()));
    me.m_href = resource.GetRelativePathToOEBPS();
    me.m_mtype = GetResourceMimetype(resource);
    int n = m_manifest.count();
    m_manifest.append(me);
    m_idpos[me.m_id] = n;
    m_hrefpos[me.m_href] = n;
    if (resource.Type() == Resource::HTMLResourceType) {
        SpineEntry se;
        se.m_idref = me.m_id;
        m_spine.append(se);
    }
}

void OPFResource::RemoveCoverMetaForImage(const Resource &resource)
{
    int pos = GetCoverMeta();
    QString resource_id = GetResourceManifestID(resource);

    // Remove entry if there is a cover in meta and if this file is marked as cover
    if (pos > -1) {
       MetaEntry me = m_metadata.at(pos);
       if (me.m_atts.value(QString("content"),QString("")) == resource_id) {
           m_metadata.removeAt(pos);
       }
    }
}

void OPFResource::AddCoverMetaForImage(const Resource &resource)
{
    int pos = GetCoverMeta();
    QString resource_id = GetResourceManifestID(resource);

    // If a cover entry exists, update its id, else create one
    if (pos > -1) {
        MetaEntry me = m_metadata.at(pos);
        me.m_atts["content"] = resource_id;
        m_metadata.replace(pos, me);
    } else {
        MetaEntry me;
        me.m_name = "meta";
        me.m_atts["name"] = "cover";
        me.m_atts["content"] = QString(resource_id);
        m_metadata.append(me);
    }
}

void OPFResource::RemoveResource(const Resource &resource)
{
    QMutexLocker locker(&m_AccessMutex);
    if (m_manifest.isEmpty()) return;

    QString resource_oebps_path = resource.GetRelativePathToOEBPS();
    int pos = m_hrefpos.value(resource_oebps_path, -1);
    QString item_id = "";

    // Delete the meta tag for cover images before deleting the manifest entry
    if (resource.Type() == Resource::ImageResourceType) {
        RemoveCoverMetaForImage(resource);
    }
    if (pos > -1) {
        item_id = m_manifest.at(pos).m_id;
    }
    if (resource.Type() == Resource::HTMLResourceType) {
        for (int i=0; i < m_spine.count(); ++i) {
            QString idref = m_spine.at(i).m_idref;
            if (idref == item_id) {
                m_spine.removeAt(i);
                break;
            }
        }
        RemoveGuideReferenceForResource(resource);
    }
    if (pos > -1) {
        m_manifest.removeAt(pos);
        // rebuild the maps since updating them item by item would be slower
        m_idpos.clear();
        m_hrefpos.clear();
        for (int i=0; i < m_manifest.count(); ++i) {
            m_idpos[m_manifest.at(i).m_id] = i;
            m_idpos[m_manifest.at(i).m_href] = i;
        }
    }
}


void OPFResource::AddGuideSemanticType(
    const ::HTMLResource &html_resource,
    GuideSemantics::GuideSemanticType new_type)
{
    QMutexLocker locker(&m_AccessMutex);
    GuideSemantics::GuideSemanticType current_type = GetGuideSemanticTypeForResource(html_resource);

    if (current_type != new_type) {
        RemoveDuplicateGuideTypes(new_type);
        SetGuideSemanticTypeForResource(new_type, html_resource);
    }
    // If the current type is the same as the new one,
    // we toggle it off.
    else {
        RemoveGuideReferenceForResource(html_resource);
    }
}


void OPFResource::SetResourceAsCoverImage(const ::ImageResource &image_resource)
{
    QMutexLocker locker(&m_AccessMutex);
    if (IsCoverImage(image_resource)) {
        RemoveCoverMetaForImage(image_resource);
    } else {
        AddCoverMetaForImage(image_resource);
    }
}


// note: under epub3 spine elements may have page properties set, so simply clearing the
// spine will lose these attributes.  We should try to keep as much of the spine properties
// and linear attributes as we can.  Either that or make the HTML Resource remember its own
// spine page properties, linear attribute, etc

void OPFResource::UpdateSpineOrder(const QList<::HTMLResource *> html_files)
{
    QMutexLocker locker(&m_AccessMutex);
    QList<SpineEntry> new_spine;
    foreach(HTMLResource * html_resource, html_files) {
        const Resource &resource = *static_cast<const Resource *>(html_resource);
        QString id = GetResourceManifestID(resource);
        int found = -1;
        for (int i = 0; i < m_spine.count(); ++i) {
           SpineEntry se = m_spine.at(i);
           if (se.m_idref == id) {
               found = i;
               break;
           }
        }
        if (found > -1) {
            new_spine.append(m_spine.at(found));
        } else {
            SpineEntry se;
            se.m_idref = id;
            new_spine.append(se);
        }
    }
    m_spine.clear();
    m_spine = new_spine;
}


void OPFResource::ResourceRenamed(const Resource &resource, QString old_full_path)
{
    QMutexLocker locker(&m_AccessMutex);
    QString path_to_oebps_folder = QFileInfo(GetFullPath()).absolutePath() + "/";
    QString resource_oebps_path  = QString(old_full_path).remove(path_to_oebps_folder);
    QString old_id;
    QString new_id;
    for (int i=0; i < m_manifest.count(); ++i) {
        QString href = m_manifest.at(i).m_href;
        if (href == resource_oebps_path) {
            ManifestEntry me = m_manifest.at(i);
            QString old_href = me.m_href;
            me.m_href = resource.GetRelativePathToOEBPS();
            old_id = me.m_id;
            new_id = GetUniqueID(GetValidID(resource.Filename()));
            me.m_id = new_id;
            m_idpos.remove(old_id);
            m_idpos[new_id] = i;
            m_hrefpos.remove(old_href);
            m_hrefpos[me.m_href] = i;
            m_manifest.replace(i, me);
            break;
        }
    }
    for (int i=0; i < m_spine.count(); ++i) {
        QString idref = m_spine.at(i).m_idref;
        if (idref == old_id) {
            SpineEntry se = m_spine.at(i);
            se.m_idref = new_id;
            m_spine.replace(i, se);
            break;
        }
    }

    if (resource.Type() == Resource::ImageResourceType) {
        // Change meta entry for cover if necessary
        // Check using IDs since file is already renamed
        if (IsCoverImageCheck(old_id)) {
            // Add will automatically replace an existing id
            // Assumes only one cover but removing duplicates
            // can cause timing issues
            AddCoverMetaForImage(resource);
        }
    }
}


# if 0
QString OPFResource::GetDocument() const
{
    QString source = GetText();
    return source;
}
#endif


int OPFResource::GetGuideReferenceForResourcePos(const Resource &resource) const
{
    QString resource_oebps_path = resource.GetRelativePathToOEBPS();
    for (int i=0; i < m_guide.count(); ++i) {
        GuideEntry ge = m_guide.at(i);
        QString href = ge.m_href;
        QStringList parts = href.split('#', QString::KeepEmptyParts);
        if (parts.at(0) == resource_oebps_path) {
            return i;
        }
    }
    return -1;
}


void OPFResource::RemoveGuideReferenceForResource(const Resource &resource)
{
    if (m_guide.isEmpty()) return;
    int pos = GetGuideReferenceForResourcePos(resource);
    if (pos > -1) {
        m_guide.removeAt(pos);
    }
}


GuideSemantics::GuideSemanticType OPFResource::GetGuideSemanticTypeForResource2(const Resource &resource) const
{
    int pos = GetGuideReferenceForResourcePos(resource);
    if (pos > -1) {
        GuideEntry ge = m_guide.at(pos);
        QString type = ge.m_type;
        return GuideSemantics::Instance().MapReferenceTypeToGuideEnum(type);
    }
    return GuideSemantics::NoType;
}


void OPFResource::SetGuideSemanticTypeForResource(
    GuideSemantics::GuideSemanticType type,
    const Resource &resource)
{
    int pos = GetGuideReferenceForResourcePos(resource);
    QString type_attribute;
    QString title_attribute;
    std::tie(type_attribute, title_attribute) = GuideSemantics::Instance().GetGuideTypeMapping()[ type ];

    if (pos > -1) {
        GuideEntry ge = m_guide.at(pos);
        ge.m_type = type_attribute;
        ge.m_title = title_attribute;
        m_guide.replace(pos, ge);
    } else {
        GuideEntry ge;
        ge.m_type = type_attribute;
        ge.m_title = title_attribute;
        ge.m_href = resource.GetRelativePathToOEBPS();
        m_guide.append(ge);
    }
}


void OPFResource::RemoveDuplicateGuideTypes(
    GuideSemantics::GuideSemanticType new_type)
{
    // Industry best practice is to have only one
    // <guide> reference type instance per book.
    // For NoType, there is nothing to remove.
    if (new_type == GuideSemantics::NoType) {
        return;
    }

    if (m_guide.isEmpty()) return;

    // build up the list to be deleted in reverse order
    QList<int> dellist;
    for (int i = m_guide.count() - 1; i >= 0; --i) {
        GuideEntry ge = m_guide.at(i);
        QString type_text = ge.m_type;
        GuideSemantics::GuideSemanticType current_type = GuideSemantics::Instance().MapReferenceTypeToGuideEnum(type_text);
        if (current_type == new_type) {
            dellist.append(i);
        }
    }
    // remove them from the list in reverse order
    foreach(int index, dellist) {
        m_guide.removeAt(index);
    }
}


int OPFResource::GetCoverMeta() const
{
    for (int i = 0; i < m_metadata.count(); ++i) {
        MetaEntry me = m_metadata.at(i);
        if ((me.m_name == "meta") && (me.m_atts.contains(QString("name")))) {
            QString name = me.m_atts[QString("name")];
            if (name == "cover") {
                return i;
            }
        }
    }
    return -1;
}


int OPFResource::GetMainIdentifier() const
{
    QString unique_identifier = m_package.m_uniqueid;
    for (int i=0; i < m_metadata.count(); ++i) {
        MetaEntry me = m_metadata.at(i);
        if (me.m_name == "dc:identifier") {
            QString id = me.m_atts.value("id", "");
            if (id == unique_identifier) {
                return i;
            }
        }
    }
    return -1;
}


QString OPFResource::GetResourceManifestID(const Resource &resource) const
{
    QString oebps_path = resource.GetRelativePathToOEBPS();
    int pos = m_hrefpos.value(oebps_path,-1);
    if (pos > -1) { 
        return QString(m_manifest.at(pos).m_id); 
    }
    return QString();
}


QHash<Resource *, QString> OPFResource::GetResourceManifestIDMapping(
    const QList<Resource *> resources)
{
    QHash<Resource *, QString> id_mapping;
    foreach(Resource * resource, resources) {
        QString oebps_path = resource->GetRelativePathToOEBPS();
        int pos = m_hrefpos.value(oebps_path,-1);
        if (pos > -1) { 
            id_mapping[ resource ] = m_manifest.at(pos).m_id;
        }
    }
    return id_mapping;
}


#if 0
void OPFResource::SetMetaElementsLast(xc::DOMDocument &document)
{
    QList<xc::DOMElement *> metas =
        XhtmlDoc::GetTagMatchingDescendants(document, "meta", OPF_XML_NAMESPACE);
    xc::DOMElement *metadata = GetMetadataElement(document);
    if (!metadata) {
        return;
    }
    foreach(xc::DOMElement * meta, metas) {
        // This makes sure that the <meta> elements come last
        metadata->removeChild(meta);
        metadata->appendChild(meta);
    }
}
#endif

void OPFResource::RemoveDCElements()
{
    int pos = GetMainIdentifier();
    // build list to be delted in reverse order
    QList<int> dellist;
    int n = m_metadata.count();
    for (int i = n-1; i >= 0; --i) {
        MetaEntry me = m_metadata.at(i);
        if (me.m_name.startsWith("dc:")) {
            if (i != pos) {
               dellist.append(i);
            }
        }
    }
    // delete the MetaEntries in reverse order to not mess up indexes
    foreach(int index, dellist) {
        m_metadata.removeAt(index);
    }
}


void OPFResource::MetadataDispatcher(const Metadata::MetaElement &book_meta)
{
    // We ignore badly formed meta elements.
    if (book_meta.name.isEmpty() || book_meta.value.isNull()) {
        return;
    }

    // Write Relator codes (always write author as relator code)
    if (Metadata::Instance().IsRelator(book_meta.name) || book_meta.name == "author") {
        WriteCreatorOrContributor(book_meta);
    }
    // There is a relator for the publisher, but there is
    // also a special publisher element that we would rather use
    else if (book_meta.name == "pub") {
        WriteSimpleMetadata("publisher", book_meta.value.toString());
    } else if (book_meta.name  == "language") {
        WriteSimpleMetadata(book_meta.name,
                            Language::instance()->GetLanguageCode(book_meta.value.toString()));
    } else if (book_meta.name  == "identifier") {
        WriteIdentifier(book_meta.file_as, book_meta.value.toString());
    } else if (book_meta.name == "date") {
        WriteDate(book_meta.file_as, book_meta.value);
    } else {
        WriteSimpleMetadata(book_meta.name, book_meta.value.toString());
    }
}


void OPFResource::WriteCreatorOrContributor(const Metadata::MetaElement book_meta)
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

    MetaEntry me;
    me.m_name = QString("dc:") + role_type;
    me.m_atts[QString("opf:role")] = name;
    if (!file_as.isEmpty()) {
        me.m_atts[QString("opf:file-as")] = file_as;
    }
    me.m_content = value;
    m_metadata.append(me);
}


void OPFResource::WriteSimpleMetadata(
    const QString &metaname,
    const QString &metavalue)
{
    MetaEntry me;
    me.m_name = QString("dc:") + metaname;
    me.m_content = metavalue;
    m_metadata.append(me);
}


void OPFResource::WriteIdentifier(
    const QString &metaname,
    const QString &metavalue)
{
    int pos = GetMainIdentifier();
    if (pos > -1) {
        MetaEntry me = m_metadata.at(pos);
        QString scheme = me.m_atts.value(QString("scheme"),QString(""));
        if ((metavalue == me.m_content) && (metaname == scheme)) {
            return;
        }
    }
    MetaEntry me;
    me.m_name = QString("dc:identifier");
    me.m_atts[QString("opf:scheme")] = metaname;
    if (metaname.toLower() == "uuid" && !metavalue.contains("urn:uuid:")) {
        me.m_content = QString("urn:uuid:")  + metavalue;
    } else {
        me.m_content = metavalue;
    }
    m_metadata.append(me);
}

void OPFResource::AddModificationDateMeta()
{
    QMutexLocker locker(&m_AccessMutex);
    QString date;
    QDate d = QDate::currentDate();
    // We can't use QDate.toString() because it will take into account the locale. Which mean we may not get Arabic 
    // numerals if the local uses it's own numbering system. So we use this instead to ensure we get a valid date per
    // the epub spec.
    QTextStream(&date) << d.year() << "-" << (d.month() < 10 ? "0" : "") << d.month() << "-" << (d.day() < 10 ? "0" : "") << d.day();
    for (int i=0; i < m_metadata.count(); ++i) {
        MetaEntry me = m_metadata.at(i);
        if (me.m_name == QString("dc:date")) {
            QString etype = me.m_atts.value(QString("opf:event"), QString(""));
            if (etype == QString("modification")) {
                me.m_content = date;
                m_metadata.replace(i, me);
                return;
            }
            
        }
    }
    MetaEntry me;
    me.m_name = QString("dc:date");
    me.m_content = date;
    me.m_atts[QString("opf:event")] = QString("modification");
    m_metadata.append(me);
}

void OPFResource::WriteDate(
    const QString &metaname,
    const QVariant &metavalue)
{
    QString date;
    QDate d = metavalue.toDate();
    // We can't use QDate.toString() because it will take into account the locale. Which mean we may not get Arabic 
    // numerals if the local uses it's own numbering system. So we use this instead to ensure we get a valid date per
    // the epub spec.
    QTextStream(&date) << d.year() << "-" << (d.month() < 10 ? "0" : "") << d.month() << "-" << (d.day() < 10 ? "0" : "") << d.day();

    // This assumes that the "dc" prefix has been declared for the DC namespace
    QHash<QString,QString> atts;
    atts["opf:event"] = metaname;
    MetaEntry me;
    me.m_name=QString("dc:date");
    me.m_content = date;
    me.m_atts[QString("opf:event")] = metaname;
    m_metadata.append(me);
}

#if 0
bool OPFResource::BasicStructurePresent(const xc::DOMDocument &document)
{
    QList<xc::DOMElement *> packages =
        XhtmlDoc::GetTagMatchingDescendants(document, "package", OPF_XML_NAMESPACE);

    if (packages.count() != 1) {
        return false;
    }

    QList<xc::DOMElement *> metadatas =
        XhtmlDoc::GetTagMatchingDescendants(document, "metadata", OPF_XML_NAMESPACE);

    if (metadatas.count() != 1) {
        return false;
    }

    QList<xc::DOMElement *> manifests =
        XhtmlDoc::GetTagMatchingDescendants(document, "manifest", OPF_XML_NAMESPACE);

    if (manifests.count() != 1) {
        return false;
    }

    QList<xc::DOMElement *> spines =
        XhtmlDoc::GetTagMatchingDescendants(document, "spine", OPF_XML_NAMESPACE);

    if (spines.count() != 1) {
        return false;
    }

    if (GetMainIdentifierUnsafe() == -1) {
        return false;
    }

    return true;
}

std::shared_ptr<xc::DOMDocument> OPFResource::CreateOPFFromScratch(const xc::DOMDocument *d) const
{
    xc::DOMElement *elem;
    QList<xc::DOMElement *> children;
    QString xml_source;
    QString manifest;
    QString spine;
    QString metadata_content;
    QList<std::pair<QString, QString>> manifest_file;
    QList<std::pair<QString, QString>> manifest_recovered;
    QString manifest_content;
    QList<QString> ids_in_manifest;
    QList<QString> spine_file;
    QList<QString> spine_recovered;
    QString spine_content;
    QString guide_content;
    QString item_id;
    QString path;
    QStringList relative_oebps_paths;
    QString id;
    QString href;
    QString mime;
    bool exists;

    // Try to pull as much as we can out of the originial OPF is present.
    if (d) {
        metadata_content = XhtmlDoc::GetNodeChildrenAsString(GetMetadataElement(*d));
        guide_content = XhtmlDoc::GetNodeChildrenAsString(GetGuideElement(*d));

        elem = GetManifestElement(*d);
        if (elem) {
            children = XhtmlDoc::GetTagMatchingDescendants(*elem, "item");;
            for (int i=0; i<children.length(); ++i) {
                if (XtoQ(children.at(i)->getAttribute(QtoX("media-type"))).toLower() == NCX_MIMETYPE) {
                    continue;
                }
                id = XtoQ(children.at(i)->getAttribute(QtoX("id")));
                href = XtoQ(children.at(i)->getAttribute(QtoX("href")));
                if (!id.isEmpty() && !href.isEmpty()) {
                    manifest_recovered.append(std::pair<QString, QString>(id, href));
                }
            }
        }

        elem = GetSpineElement(*d);
        if (elem) {
            children = XhtmlDoc::GetTagMatchingDescendants(*elem, "itemref");;
            for (int i=0; i<children.length(); ++i) {
                id = XtoQ(children.at(i)->getAttribute(QtoX("idref")));
                if (!id.isEmpty()) {
                    spine_recovered.append(id);
                }
            }
        }
    }

    // Get a list of all items on disk.
    relative_oebps_paths = GetRelativePathsToAllFilesInOEPBS();
    foreach(path, relative_oebps_paths) {
        // The OPF is not allowed to be in the manifest and the NCX
        // is already in the template.
        if (path.contains(OPF_FILE_NAME) || path.contains(NCX_FILE_NAME)) {
            continue;
        }

        item_id = GetValidID(QFileInfo(path).fileName());
        manifest_file.append(std::pair<QString, QString>(item_id, path));

        if (TEXT_EXTENSIONS.contains(QFileInfo(path).suffix().toLower())) {
            spine_file.append(item_id);
        }
    }

    // Compare the recovered with on disk content to ensure we aren't missing anything.
    // Put anything recovered that exists on disk into content.
    for (int i=0; i<manifest_recovered.count(); ++i) {
        std::pair<QString, QString> rec = manifest_recovered.at(i);
        item_id = rec.first;
        path = rec.second;
        for (int j=0; j<manifest_file.count(); ++j) {
            std::pair<QString, QString> frec = manifest_file.at(j);
            if (frec.second == path) {
                mime = GetFileMimetype(path);
                manifest_content.append(QString(ITEM_ELEMENT_TEMPLATE).arg(item_id).arg(path).arg(mime));
                if (mime == "application/xhtml+xml") {
                    ids_in_manifest.append(item_id);
                }
                break;
            }
        }
    }
    // Put anything that exists on disk that isn't in recovered into content.
    for (int i=0; i<manifest_file.count(); ++i) {
        std::pair<QString, QString> frec = manifest_file.at(i);
        exists = false;
        item_id = frec.first;
        path = frec.second;
        for (int j=0; i<manifest_recovered.count(); ++j) {
            std::pair<QString, QString> rec = manifest_recovered.at(j);
            if (rec.second == path) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            mime = GetFileMimetype(path);
            manifest_content.append(QString(ITEM_ELEMENT_TEMPLATE).arg(item_id).arg(path).arg(mime));
            if (mime == "application/xhtml+xml") {
                ids_in_manifest.append(item_id);
            }
        }
    }

    // Compare the recovered sine with the items that are in the manifest. We only
    // use a recovered spine item if the id exists in the manfiest.
    foreach (id, spine_recovered) {
        if (ids_in_manifest.contains(id)) {
            spine_content.append(ITEMREF_ELEMENT_TEMPLATE.arg(id));
        }
    }
    // Add any spine items that are on disk that are not in the recovered spine to the spine.
    foreach (id, spine_file) {
        if (!spine_recovered.contains(id) && ids_in_manifest.contains(id)) {
            spine_content.append(ITEMREF_ELEMENT_TEMPLATE.arg(id));
        }
    }

    // Build the OPF.
    xml_source = GetOPFDefaultText();
    xml_source.replace("</manifest>", manifest_content + "</manifest>")
    .replace("</spine>", spine_content + "</spine>")
    .replace("<metadata", OPF_REWRITTEN_COMMENT + "<metadata")
    .replace("</metadata>", metadata_content + "</metadata>")
    .replace("</guide>", guide_content + "</guide>")
    .replace("<guide>\n\n</guide>\n\n", "");

    std::shared_ptr<xc::DOMDocument> document = XhtmlDoc::LoadTextIntoDocument(xml_source);
    document->setXmlStandalone(true);
    return document;
}
#endif

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


QString OPFResource::GetUniqueID(const QString &preferred_id) const
{
    if (m_idpos.contains(preferred_id)) {
        return Utility::CreateUUID();
    }
    return preferred_id;
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


QString OPFResource::GetRelativePathToRoot() const
{
    QFileInfo info(GetFullPath());
    QDir parent_dir = info.dir();
    QString parent_name = parent_dir.dirName();
    return parent_name + "/" + Filename();
}


QString OPFResource::convert_to_xml() const
{
    QStringList xmlres;
    xmlres << m_package.convert_to_xml();
    xmlres << m_metans.convert_to_xml();
    foreach (MetaEntry me, m_metadata) {
        xmlres << me.convert_to_xml();
    }
    xmlres << "  </metadata>\n";
    xmlres << "  <manifest>\n";
    foreach (ManifestEntry me, m_manifest) {
        xmlres << me.convert_to_xml();
    }
    xmlres << "  </manifest>\n";
    xmlres << m_spineattr.convert_to_xml();
    foreach(SpineEntry sp, m_spine) {
        xmlres << sp.convert_to_xml();
    }
    xmlres << "  </spine>\n";
    if ((m_guide.size() > 0) || (m_package.m_version.startsWith("2"))) {
        xmlres << "  <guide>\n";
        foreach(GuideEntry ge, m_guide) {
            xmlres << ge.convert_to_xml();
        }
        xmlres << "  </guide>\n";
    }
    if ((m_bindings.size() > 0) && (!m_package.m_version.startsWith("2"))) {
        xmlres << "  <bindings>\n";
        foreach(BindingsEntry be, m_bindings) {
            xmlres << be.convert_to_xml();
        }
        xmlres << "  </bindings>\n";
    }
    xmlres << "</package>\n";
    return xmlres.join("");
}
