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

void HTMLResource::SetText(const QString &text)
{
    // We need to move the xml deceleration to the top
    // of the file because CKEditor will write the HTML doc type
    // above it.

    QString new_text = text;
    QString xmldec;
    QRegExp re_xmldec("(<\\?xml.+\\?>)");
    re_xmldec.setMinimal(true);

    // Pull out xml deceleration from the text.
    if (re_xmldec.indexIn(new_text) != -1) {
        xmldec = re_xmldec.cap(1) + "\n";
    }

    // Remove the matched xml deceleration.
    new_text.remove(re_xmldec);

    // Put the xml deceleration at the top.
    new_text = xmldec + new_text;

    XMLResource::SetText( ConvertToEntities( CleanSource::Clean( new_text) ) );
}


void HTMLResource::SaveToDisk(bool book_wide_save)
{
    // Just in case there was no initial load until now.
    InitialLoad();

    SetText(ConvertToEntities(CleanSource::PrettyPrint(GetText())));

    XMLResource::SaveToDisk(book_wide_save);
}


QStringList HTMLResource::SplitOnSGFChapterMarkers()
{
    QStringList chapters = XhtmlDoc::GetSGFChapterSplits(GetText());

    SetText(CleanSource::Clean(chapters.takeFirst()));

    return chapters;
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


void HTMLResource::TrackNewResources( const QStringList &filepaths )
{
    m_LinkedResourceIDs.clear();
    QStringList filenames;

    foreach( QString filepath, filepaths )
    {
        filenames.append( QFileInfo( filepath ).fileName() );
    }

    foreach( Resource *resource, m_Resources.values() )
    {
        if ( filenames.contains( resource->Filename() ) )

            m_LinkedResourceIDs.append( resource->GetIdentifier() );
    }
}


QString HTMLResource::ConvertToEntities( const QString &source )
{
    QString newsource = source;

    newsource = newsource.replace( QString::fromUtf8( "\u00ad" ), "&shy;" );
    newsource = newsource.replace( QString::fromUtf8( "\u2014" ), "&mdash;" );
    newsource = newsource.replace( QString::fromUtf8( "\u2013" ), "&ndash;" );

    return newsource;
}
