/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks Stratford, ON, Canada 
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

#include <memory>

#include <QtCore/QFileInfo>
#include <QtCore/QString>
// #include <QDebug>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "Misc/GumboInterface.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_exception.h"

static const QString LOADED_CONTENT_MIMETYPE = "application/xhtml+xml";
const QString XML_NAMESPACE_CRUFT = "xmlns=\"http://www.w3.org/1999/xhtml\"";
const QString REPLACE_SPANS = "<span class=\"SigilReplace_\\d*\"( id=\"SigilReplace_\\d*\")*>";

const QString XML_TAG = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>";

HTMLResource::HTMLResource(const QString &mainfolder, const QString &fullfilepath,
                           const QHash<QString, Resource *> &resources,
                           QObject *parent)
    :
    XMLResource(mainfolder, fullfilepath, parent),
    m_Resources(resources),
    m_TOCCache("")
{
}


Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResourceType;
}

bool HTMLResource::LoadFromDisk()
{
    try {
        const QString &text = Utility::ReadUnicodeTextFile(GetFullPath());
        SetText(text);
        emit LoadedFromDisk();
        return true;
    } catch (CannotOpenFile) {
        //
    }

    return false;
}

void HTMLResource::SetText(const QString &text)
{
    emit TextChanging();

    XMLResource::SetText(text);

    // Track resources whose change will necessitate an update of the BV and PV.
    // At present this only applies to css files and images.
    TrackNewResources(GetPathsToLinkedResources());
}

QString HTMLResource::GetTOCCache()
{
    if (m_TOCCache.isEmpty()) {
        m_TOCCache = TextResource::GetText();
    }
    return m_TOCCache;
}

void HTMLResource::SetTOCCache(const QString & text)
{
    m_TOCCache = text;
}

void HTMLResource::SaveToDisk(bool book_wide_save)
{
    SetText(GetText());
    XMLResource::SaveToDisk(book_wide_save);
}


QStringList HTMLResource::GetLinkedStylesheets()
{
    QStringList hreflist = XhtmlDoc::GetLinkedStylesheets(GetText());
    QString startdir = GetFolder();
    QStringList stylesheet_bookpaths;
    foreach(QString ahref, hreflist) {
        stylesheet_bookpaths << Utility::buildBookPath(ahref,startdir); 
    }
    return stylesheet_bookpaths;
}


QStringList HTMLResource::GetManifestProperties() const
{
    QStringList properties;
    QReadLocker locker(&GetLock());
    GumboInterface gi = GumboInterface(GetText(), GetEpubVersion());
    gi.parse();
    QStringList props = gi.get_all_properties();
    props.removeDuplicates();
    if (props.contains("math")) properties.append("mathml");
    if (props.contains("svg")) properties.append("svg");
    // nav as a property should only be used on the nav document and no where else
    // if (props.contains("nav")) properties.append("nav");
    if (props.contains("script")) properties.append("scripted");
    if (props.contains("epub:switch")) properties.append("switch");
    if (props.contains("remote-resources")) properties.append("remote-resources");
    return properties;
}


QStringList HTMLResource::SplitOnSGFSectionMarkers()
{
    QStringList sections = XhtmlDoc::GetSGFSectionSplits(GetText());
    SetText(CleanSource::Mend(sections.takeFirst(),GetEpubVersion()));
    return sections;
}


// returns a list of book paths
QStringList HTMLResource::GetPathsToLinkedResources()
{
    QStringList linked_resources;
    // Can NOT grab Read Lock here as this is also invoked in SetText which has write lock!
    // leading to instant lockup when renaming any resource
    // QReadLocker locker(&GetLock());
    GumboInterface gi = GumboInterface(GetText(),GetEpubVersion());
    gi.parse();
    QList<GumboTag> tags;
    tags << GUMBO_TAG_IMG << GUMBO_TAG_LINK << GUMBO_TAG_AUDIO << GUMBO_TAG_VIDEO;
    const QList<GumboNode*> linked_rsc_nodes = gi.get_all_nodes_with_tags(tags);
    for (int i = 0; i < linked_rsc_nodes.count(); ++i) {
        GumboNode* node = linked_rsc_nodes.at(i);

        // We skip the link elements that are not stylesheets
        if (node->v.element.tag == GUMBO_TAG_LINK) {
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "rel");
            if (attr && (QString::fromUtf8(attr->value) != "stylesheet")) { 
                continue;
            }
        }
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (attr) {
	    QString attpath = Utility::URLDecodePath(QString::fromUtf8(attr->value));
	    if (attpath.indexOf(":") == -1) {
	        linked_resources.append(Utility::buildBookPath(attpath,GetFolder()));
	    }
            continue;
        }
        attr = gumbo_get_attribute(&node->v.element.attributes, "src");
        if (attr) {
	    QString attpath = Utility::URLDecodePath(QString::fromUtf8(attr->value));
	    if (attpath.indexOf(":") == -1) {
	        linked_resources.append(Utility::buildBookPath(attpath,GetFolder()));
	    }
        }
    }
    return linked_resources;
}


void HTMLResource::TrackNewResources(const QStringList &filepaths)
{
    QStringList bookpaths;
    QStringList linkedResourceIDs;
    foreach(QString bkpath, filepaths) {
        bookpaths.append(bkpath);
    }
    foreach(Resource * resource, m_Resources.values()) {
        disconnect(resource, SIGNAL(ResourceUpdatedOnDisk()),    this, SIGNAL(LinkedResourceUpdated()));
        disconnect(resource, SIGNAL(Deleted(const Resource *)), this, SIGNAL(LinkedResourceUpdated()));

        if (bookpaths.contains(resource->GetRelativePath())) {
            linkedResourceIDs.append(resource->GetIdentifier());
        }
    }
    foreach(QString resource_id, linkedResourceIDs) {
        Resource *resource = m_Resources.value(resource_id);
        if (resource) {
            connect(resource, SIGNAL(ResourceUpdatedOnDisk()),    this, SIGNAL(LinkedResourceUpdated()));
            connect(resource, SIGNAL(Deleted(const Resource *)), this, SIGNAL(LinkedResourceUpdated()));
        }
    }
}

bool HTMLResource::DeleteCSStyles(QList<CSSInfo::CSSSelector *> css_selectors)
{
    CSSInfo css_info(GetText(), false);
    // Search for selectors with the same definition and line and remove from text
    const QString &new_resource_text = css_info.removeMatchingSelectors(css_selectors);

    if (!new_resource_text.isNull()) {
        // At least one of the selector(s) was removed.
        SetText(new_resource_text);
        emit Modified();
        return true;
    }

    return false;
}
