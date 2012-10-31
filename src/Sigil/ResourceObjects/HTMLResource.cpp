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

#include <boost/shared_ptr.hpp>

#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/GuideSemantics.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_exception.h"

using boost::shared_ptr;

static const QString LOADED_CONTENT_MIMETYPE = "application/xhtml+xml";
const QString XML_NAMESPACE_CRUFT = "xmlns=\"http://www.w3.org/1999/xhtml\"";
const QString REPLACE_SPANS = "<span class=\"SigilReplace_\\d*\"( id=\"SigilReplace_\\d*\")*>";

const QString XML_TAG = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>";

HTMLResource::HTMLResource( const QString &fullfilepath,
                            const QHash< QString, Resource* > &resources,
                            QObject *parent )
    :
    XMLResource( fullfilepath, parent ),
    m_Resources( resources )
{

}


Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResourceType;
}

bool HTMLResource::LoadFromDisk(bool load_raw)
{
    try {
        const QString &text = Utility::ReadUnicodeTextFile(GetFullPath());

        SetText(text, load_raw);

        emit LoadedFromDisk();

        return true;
    }
    catch (CannotOpenFile)
    {
        //
    }

    return false;
}

void HTMLResource::SetText(const QString &text, bool load_raw)
{
    emit TextChanging();
    if (load_raw) {
        XMLResource::SetText(text);
    }
    else {
        XMLResource::SetText(CleanSource::Clean(text));
    }

    // Track resources whose change will necessitate an update of the BV and PV.
    // At present this only applies to css files and images.
    TrackNewResources(GetPathsToLinkedResources());
}


void HTMLResource::SaveToDisk(bool book_wide_save)
{
    SetText(GetText());

    XMLResource::SaveToDisk(book_wide_save);
}


QStringList HTMLResource::GetLinkedStylesheets()
{
    return XhtmlDoc::GetLinkedStylesheets( GetText() );
}


QStringList HTMLResource::SplitOnSGFSectionMarkers()
{
    QStringList sections = XhtmlDoc::GetSGFSectionSplits(GetText());

    SetText(CleanSource::Clean(sections.takeFirst()));

    return sections;
}


QStringList HTMLResource::GetPathsToLinkedResources()
{
    QStringList linked_resources;

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(GetText());
    xc::DOMDocument &document = *d.get();
    QStringList tags = QStringList() << "link" << "img";
    Q_FOREACH(QString tag, tags) {
        xc::DOMNodeList *elems = document.getElementsByTagName( QtoX( tag ) );
        for (uint i = 0; i < elems->getLength(); ++i) {
            xc::DOMElement &element = *static_cast< xc::DOMElement *>(elems->item(i));

            Q_ASSERT(&element);

            // We skip the link elements that are not stylesheets
            if (tag == "link" && element.hasAttribute(QtoX("rel")) &&
                XtoQ(element.getAttribute(QtoX("rel"))).toLower() != "stylesheet")
            {
                continue;
            }

            if (element.hasAttribute(QtoX("href"))) {
                linked_resources.append(XtoQ(element.getAttribute(QtoX("href"))));
            } else if (element.hasAttribute(QtoX("src"))) {
                linked_resources.append(XtoQ(element.getAttribute(QtoX("src"))));
            }
        }
    }

    return linked_resources;
}


void HTMLResource::TrackNewResources(const QStringList &filepaths)
{
    disconnect(this, 0, 0, 0);

    QStringList filenames;
    QStringList linkedResourceIDs;

    foreach(QString filepath, filepaths) {
        filenames.append(QFileInfo(filepath).fileName());
    }

    foreach(Resource *resource, m_Resources.values())
    {
        if (filenames.contains(resource->Filename())) {
            linkedResourceIDs.append(resource->GetIdentifier());
        }
    }

    foreach(QString resource_id, linkedResourceIDs)
    {
        Resource *resource = m_Resources.value(resource_id);
        if (resource) {
            connect(resource, SIGNAL(ResourceUpdatedOnDisk()),    this, SIGNAL(LinkedResourceUpdated()));
            connect(resource, SIGNAL(Deleted( const Resource &)), this, SIGNAL(LinkedResourceUpdated()));
        }
    }
}

bool HTMLResource::DeleteCSStyles( QList<CSSInfo::CSSSelector*> css_selectors)
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
