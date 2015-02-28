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

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtConcurrent/QtConcurrent>

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "Misc/GumboInterface.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "sigil_constants.h"
#include "SourceUpdates/AnchorUpdates.h"

using boost::make_tuple;
using boost::shared_ptr;
using boost::tie;
using boost::tuple;

void AnchorUpdates::UpdateAllAnchorsWithIDs(const QList<HTMLResource *> &html_resources)
{
    const QHash<QString, QString> &ID_locations = GetIDLocations(html_resources);
    QtConcurrent::blockingMap(html_resources, boost::bind(UpdateAnchorsInOneFile, _1, ID_locations));
}


void AnchorUpdates::UpdateExternalAnchors(const QList<HTMLResource *> &html_resources, const QString &originating_filename, const QList<HTMLResource *> new_files)
{
    const QHash<QString, QString> &ID_locations = GetIDLocations(new_files);
    QtConcurrent::blockingMap(html_resources, boost::bind(UpdateExternalAnchorsInOneFile, _1, originating_filename, ID_locations));
}


void AnchorUpdates::UpdateAllAnchors(const QList<HTMLResource *> &html_resources, const QStringList &originating_filenames, HTMLResource *new_file)
{
    QList<HTMLResource *> new_files;
    new_files.append(new_file);
    const QHash<QString, QString> &ID_locations = GetIDLocations(new_files);
    QList<QString> originating_filename_links;
    foreach(QString originating_filename, originating_filenames) {
        originating_filename_links.append("../" % TEXT_FOLDER_NAME % "/" % originating_filename);
    }
    const QString &new_filename_with_relative_path = "../" % TEXT_FOLDER_NAME % "/" % Utility::URLEncodePath(new_file->Filename());
    QtConcurrent::blockingMap(html_resources, boost::bind(UpdateAllAnchorsInOneFile, _1, originating_filename_links, ID_locations, new_filename_with_relative_path));
}


QHash<QString, QString> AnchorUpdates::GetIDLocations(const QList<HTMLResource *> &html_resources)
{
    const QList<tuple<QString, QList<QString>>> &IDs_in_files = QtConcurrent::blockingMapped(html_resources, GetOneFileIDs);
    QHash<QString, QString> ID_locations;

    for (int i = 0; i < IDs_in_files.count(); ++i) {
        QList<QString> file_element_IDs;
        QString resource_filename;
        tie(resource_filename, file_element_IDs) = IDs_in_files.at(i);

        for (int j = 0; j < file_element_IDs.count(); ++j) {
            ID_locations[ file_element_IDs.at(j) ] = resource_filename;
        }
    }

    return ID_locations;
}


tuple<QString, QList<QString>> AnchorUpdates::GetOneFileIDs(HTMLResource *html_resource)
{
    Q_ASSERT(html_resource);
    QReadLocker locker(&html_resource->GetLock());
    QString newsource = html_resource->GetText();
    GumboInterface gi = GumboInterface(newsource);
    gi.parse();
    QList<QString> ids = gi.get_all_values_for_attribute(QString("id"));
    return make_tuple(html_resource->Filename(), ids);
}


void AnchorUpdates::UpdateAnchorsInOneFile(HTMLResource *html_resource,
        const QHash<QString, QString> ID_locations)
{
    Q_ASSERT(html_resource);
    QWriteLocker locker(&html_resource->GetLock());
    GumboInterface gi = GumboInterface(html_resource->GetText());
    gi.parse();
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);
    const QString &resource_filename = html_resource->Filename();
    bool is_changed = false;

    for (uint i = 0; i < anchor_nodes.length(); ++i) {
        GumboNode* node = anchor_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (attr && QUrl(QString::fromUtf8(attr->value)).isRelative()) {
            QString href = QString::fromUtf8(attr->value);
            QStringList parts = href.split(QChar('#'), QString::KeepEmptyParts);

            if (parts.length() > 1) {
                QString fragment_id = href.right(href.size() - (parts.at(0).length() + 1));
                QString file_id = ID_locations.value(fragment_id);

                // If the ID is in a different file, update the link
                if (file_id != resource_filename && !file_id.isEmpty()) {
                    QString attribute_value = QString("../")
                                              .append(TEXT_FOLDER_NAME)
                                              .append("/")
                                              .append(Utility::URLEncodePath(file_id))
                                              .append("#")
                                              .append(fragment_id);
                    gumbo_attribute_set_value(attr, attribute_value.toUtf8());
                    is_changed = true;
                }
            }
        }
    }

    if (is_changed) {
        html_resource->SetText(gi.gettext());
    }
}


void AnchorUpdates::UpdateExternalAnchorsInOneFile(HTMLResource *html_resource, const QString &originating_filename, const QHash<QString, QString> ID_locations)
{
    Q_ASSERT(html_resource);
    QWriteLocker locker(&html_resource->GetLock());
    GumboInterface gi = GumboInterface(html_resource->GetText());
    gi.parse();
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);
    QString original_filename_with_relative_path = "../" % TEXT_FOLDER_NAME % "/" % originating_filename;
    bool is_changed = false;

    for (uint i = 0; i < anchor_nodes.length(); ++i) {
        GumboNode* node = anchor_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");

        // We're only interested in hrefs of the form "originating_filename#fragment_id".
        // But must be wary of hrefs that are "originating_filename", "originating_filename#" or "#fragment_id"
        // First, we find the hrefs that are relative and contain a fragment id.
        if (attr && QUrl(QString::fromUtf8(attr->value)).isRelative()) {
            QString href = QString::fromUtf8(attr->value);
            QStringList parts = href.split(QChar('#'), QString::KeepEmptyParts);

            // If the href pointed to the original file then update the file_id.
            if (parts.length() > 1 && parts.at(0) == original_filename_with_relative_path && !parts.at(1).isEmpty()) {
                QString fragment_id = href.right(href.size() - (parts.at(0).length() + 1));
                QString attribute_value = QString("../")
                                          .append(TEXT_FOLDER_NAME)
                                          .append("/")
                                          .append(Utility::URLEncodePath(ID_locations.value(fragment_id)))
                                          .append("#")
                                          .append(fragment_id);
                gumbo_attribute_set_value(attr, attribute_value.toUtf8());
                is_changed = true;
            }
        }
    }

    if (is_changed) {
        html_resource->SetText(gi.gettext());
    }
}


void AnchorUpdates::UpdateAllAnchorsInOneFile(HTMLResource *html_resource,
        const QList<QString> &originating_filename_links,
        const QHash<QString, QString> ID_locations,
        const QString &new_filename)
{
    Q_ASSERT(html_resource);
    QWriteLocker locker(&html_resource->GetLock());
    GumboInterface gi = GumboInterface(html_resource->GetText());
    gi.parse();
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);
    bool is_changed = false;

    for (uint i = 0; i < anchor_nodes.length(); ++i) {
        GumboNode* node = anchor_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");

        // We find the hrefs that are relative and contain an href.
        if (attr && QUrl(QString::fromUtf8(attr->value)).isRelative()) {
            QString href = QString::fromUtf8(attr->value);

            // Is this href in the form "originating_filename#fragment_id" or "originating_filename" or "#fragment_id"?
            QStringList parts = href.split(QChar('#'), QString::KeepEmptyParts);

            // If the href pointed to the original file then update the file_id.
            if (originating_filename_links.contains(parts.at(0))) {
                if (parts.count() == 1 || parts.at(1).isEmpty()) {
                    // This is a straight href with no anchor fragment
                    gumbo_attribute_set_value(attr, new_filename.toUtf8());
                } else {
                    // Rather than using parts.at(1) we will allow a # being part of the anchor
                    QString fragment_id = href.right(href.size() - (parts.at(0).length() + 1));
                    QString attribute_value = QString("../")
                                              .append(TEXT_FOLDER_NAME)
                                              .append("/")
                                              .append(Utility::URLEncodePath(ID_locations.value(fragment_id)))
                                              .append("#")
                                              .append(fragment_id);
                    gumbo_attribute_set_value(attr, attribute_value.toUtf8());
                }
                is_changed = true;
            }
        }
    }

    if (is_changed) {
        html_resource->SetText(gi.gettext());
    }
}


void AnchorUpdates::UpdateTOCEntries(NCXResource *ncx_resource, const QString &originating_filename, const QList<HTMLResource *> new_files)
{
    Q_ASSERT(ncx_resource);
    const QHash<QString, QString> &ID_locations = GetIDLocations(new_files);
    QWriteLocker locker(&ncx_resource->GetLock());
    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(ncx_resource->GetText());
    xc::DOMDocument &document = *d.get();
    xc::DOMNodeList *anchors  = document.getElementsByTagName(QtoX("content"));
    QString original_filename_with_relative_path = TEXT_FOLDER_NAME % "/" % originating_filename;

    for (uint i = 0; i < anchors->getLength(); ++i) {
        xc::DOMElement &element = *static_cast<xc::DOMElement *>(anchors->item(i));
        Q_ASSERT(&element);

        // We're only interested in src links of the form "originating_filename#fragment_id".
        // First, we find the hrefs that are relative and contain a fragment id.
        if (element.hasAttribute(QtoX("src")) &&
            QUrl(XtoQ(element.getAttribute(QtoX("src")))).isRelative()) {
            QString src = XtoQ(element.getAttribute(QtoX("src")));
            QStringList parts = src.split(QChar('#'), QString::KeepEmptyParts);

            // If the src pointed to the original file then update the file_id.
            if (parts.count() > 1 && parts.at(0) == original_filename_with_relative_path && !parts.at(1).isEmpty()) {
                QString fragment_id = src.right(src.size() - (parts.at(0).length() + 1));
                QString attribute_value = QString("%1").arg(TEXT_FOLDER_NAME)
                                          .append("/")
                                          .append(Utility::URLEncodePath(ID_locations.value(fragment_id)))
                                          .append("#")
                                          .append(fragment_id);
                element.setAttribute(QtoX("src"), QtoX(attribute_value));
            }
        }
    }

    ncx_resource->SetText(XhtmlDoc::GetDomDocumentAsString(document));
}
