/************************************************************************
**
**  Copyright (C) 2016-2021 Kevin B. Hendricks, Stratford, Ontario Canada
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
#include <functional>

#include "EmbedPython/EmbeddedPython.h"
#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

#include "Misc/Utility.h"
#include "Parsers/GumboInterface.h"
#include "BookManipulation/CleanSource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "sigil_constants.h"
#include "SourceUpdates/AnchorUpdates.h"

#define DBG if(0)

void AnchorUpdates::UpdateAllAnchorsWithIDs(const QList<HTMLResource *> &html_resources)
{
    const QHash<QString, QString> &ID_locations = GetIDLocations(html_resources);
    // The first file in the list is the orginating resource of the split
    QtConcurrent::blockingMap(html_resources, std::bind(UpdateAnchorsInOneFile, 
                                                        std::placeholders::_1, 
                                                        ID_locations));
}


void AnchorUpdates::UpdateExternalAnchors(const QList<HTMLResource *> &html_resources, const QString &originating_bookpath, const QList<HTMLResource *> new_files)
{
    const QHash<QString, QString> &ID_locations = GetIDLocations(new_files);
    QtConcurrent::blockingMap(html_resources, std::bind(UpdateExternalAnchorsInOneFile, std::placeholders::_1, originating_bookpath, ID_locations));
}


// used for merge to fix all anchors outside the merge to point to the sink
void AnchorUpdates::UpdateAllAnchors(const QList<HTMLResource *> &html_resources,
                                     const QStringList &originating_bookpaths,
                                     HTMLResource *sink_res,
                                     const QHash<QString,QString> &section_id_map)
{
    QList<HTMLResource *> new_files;
    new_files.append(sink_res);
    QString sink_bookpath = sink_res->GetRelativePath();
    QtConcurrent::blockingMap(html_resources, std::bind(UpdateAllAnchorsInOneFile, 
                                                        std::placeholders::_1,
                                                        originating_bookpaths,
                                                        sink_bookpath,
                                                        section_id_map));
}


QHash<QString, QString> AnchorUpdates::GetIDLocations(const QList<HTMLResource *> &html_resources)
{
    const QList<std::tuple<QString, QList<QString>>> &IDs_in_files = QtConcurrent::blockingMapped(html_resources, GetOneFileIDs);
    QHash<QString, QString> ID_locations;

    for (int i = 0; i < IDs_in_files.count(); ++i) {
        QList<QString> file_element_IDs;
        QString resource_bookpath;
        std::tie(resource_bookpath, file_element_IDs) = IDs_in_files.at(i);

        for (int j = 0; j < file_element_IDs.count(); ++j) {
            ID_locations[ file_element_IDs.at(j) ] = resource_bookpath;
        }
    }

    return ID_locations;
}


std::tuple<QString, QList<QString>> AnchorUpdates::GetOneFileIDs(HTMLResource *html_resource)
{
    Q_ASSERT(html_resource);
    QReadLocker locker(&html_resource->GetLock());
    QString newsource = html_resource->GetText();
    QString version = html_resource->GetEpubVersion();
    GumboInterface gi = GumboInterface(newsource, version);
    gi.parse();
    QList<QString> ids = gi.get_all_values_for_attribute(QString("id"));
    return std::make_tuple(html_resource->GetRelativePath(), ids);
}


void AnchorUpdates::UpdateAnchorsInOneFile(HTMLResource *html_resource,
                                           const QHash<QString, QString> ID_locations)
{
    Q_ASSERT(html_resource);
    DBG qDebug() << "In UpdateAnchorsInOneFile: " << html_resource->GetRelativePath();
    DBG qDebug() << "ID_locations" << ID_locations;
    QStringList bookpaths_impacted = ID_locations.values();
    DBG qDebug() << "bookpaths impacted: " << bookpaths_impacted;
    QWriteLocker locker(&html_resource->GetLock());
    QString version = html_resource->GetEpubVersion();
    GumboInterface gi = GumboInterface(html_resource->GetText(), version);
    gi.parse();
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);
    const QString &resource_bookpath = html_resource->GetRelativePath();
    bool is_changed = false;

    for (int i = 0; i < anchor_nodes.length(); ++i) {
        GumboNode* node = anchor_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (attr) {
            QString href = QString::fromUtf8(attr->value);
            if (href.indexOf(':') == -1) {
                std::pair<QString, QString> parts = Utility::parseRelativeHREF(href);
                 // if fragment exists
                if (!parts.second.isEmpty()) {
                    QString base_href = parts.first;
                    QString fragment_id = parts.second;
                    if (fragment_id.startsWith("#")) fragment_id = fragment_id.mid(1, -1);
                    QString dest_bookpath = Utility::buildBookPath(base_href, Utility::startingDir(resource_bookpath));
                    QString file_id = ID_locations.value(fragment_id);

                    DBG qDebug() << "base_href" << base_href;
                    DBG qDebug() << "destination bookpath" << dest_bookpath;
                    DBG qDebug() << "fragment_id" << fragment_id;
                    DBG qDebug() << "file_id" << file_id;
                    DBG qDebug() << "resource_bookpath" << resource_bookpath;

                    // Duplicates of this id may exist in other xhtml files in the epub
                    // not involved in the split, so only set the target file_id if the
                    // original dest_bookpath is one that was impacted by split
                    if (!base_href.isEmpty() && !bookpaths_impacted.contains(dest_bookpath)) {
                        file_id = "";
                        DBG qDebug() << "Not in bookpaths_impacted!" << dest_bookpath; 
                    }
            
                    // If the ID is in a different file, update the link
                    if (file_id != resource_bookpath && !file_id.isEmpty()) {
                        QString attpath = Utility::buildRelativePath(resource_bookpath, file_id);
                        QString attribute_value = Utility::buildRelativeHREF(attpath, "#" + fragment_id);
                        gumbo_attribute_set_value(attr, attribute_value.toUtf8().constData());
                        is_changed = true;
                    } else if ((file_id == resource_bookpath) && !base_href.isEmpty()) {
                        // this is a local internal link that needs to be fixed
                        QString attribute_value = QString("#").append(fragment_id);
                        gumbo_attribute_set_value(attr, attribute_value.toUtf8().constData());
                        is_changed = true;
                    }
                }
            }
        }
    }

    if (is_changed) {
        html_resource->SetText(CleanSource::CharToEntity(gi.getxhtml(), version));
    }
}


void AnchorUpdates::UpdateExternalAnchorsInOneFile(HTMLResource *html_resource, const QString &originating_bookpath, const QHash<QString, QString> ID_locations)
{
    Q_ASSERT(html_resource);
    DBG qDebug() << "UpdateExternalAnchorsInOneFile: " << html_resource->GetRelativePath() << originating_bookpath;
    DBG qDebug() << "ID_locations: " << ID_locations;
    QWriteLocker locker(&html_resource->GetLock());
    QString version = html_resource->GetEpubVersion();
    QString startdir = html_resource->GetFolder();
    GumboInterface gi = GumboInterface(html_resource->GetText(), version);
    gi.parse();
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);

    bool is_changed = false;

    for (int i = 0; i < anchor_nodes.length(); ++i) {
        GumboNode* node = anchor_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");

        // We're only interested in hrefs of the form "originating_filename#fragment_id".
        // But must be wary of hrefs that are "originating_filename", "originating_filename#" or "#fragment_id"
        // First, we find the hrefs that are relative and contain a fragment id.
        if (attr) {
            QString href = QString::fromUtf8(attr->value);
            if (href.indexOf(':') == -1) {
                std::pair<QString, QString> parts = Utility::parseRelativeHREF(href);
                DBG qDebug() << "....... with parts" << parts.first << parts.second;
                QString target_bookpath = Utility::buildBookPath(parts.first, startdir);
                DBG qDebug() << "....... target_bookpath" << target_bookpath;
      

                // If the href pointed to the original file then update the file_id.
                if (!parts.second.isEmpty() && (target_bookpath == originating_bookpath) && !(parts.second == "#")) {
                    QString fragment_id = parts.second;
                    DBG qDebug() << "....... fragment_id" << fragment_id;
                   
                    if (fragment_id.startsWith('#')) fragment_id = fragment_id.mid(1, -1);
                    target_bookpath = ID_locations.value(fragment_id);
                    DBG qDebug() << "....... NEW target_bookpath" << target_bookpath;

                    QString attpath = Utility::buildRelativePath(html_resource->GetRelativePath(), target_bookpath);
                    QString attribute_value = Utility::buildRelativeHREF(attpath, "#" + fragment_id);
                    DBG qDebug() << "....... attribute_value" << attribute_value;

                    gumbo_attribute_set_value(attr, attribute_value.toUtf8().constData());
                    is_changed = true;
                }
            }
        }
    }

    if (is_changed) {
        html_resource->SetText(CleanSource::CharToEntity(gi.getxhtml(), version));
    }
}


// Used for merge not splits
// walk all links in html_resource and look for any that end with a bookpath in
// originating_bookpaths and change it to be in the sink resource which is a 
// product of the merge
void AnchorUpdates::UpdateAllAnchorsInOneFile(HTMLResource *html_resource,
                                              const QList<QString> &originating_bookpaths,
                                              const QString &sink_bookpath,
                                              const QHash<QString, QString> &section_id_map)
{
    Q_ASSERT(html_resource);
    QWriteLocker locker(&html_resource->GetLock());
    QString startdir = html_resource->GetFolder();
    QString version = html_resource->GetEpubVersion();
    GumboInterface gi = GumboInterface(html_resource->GetText(), version);
    gi.parse();
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);
    bool is_changed = false;

    for (int i = 0; i < anchor_nodes.length(); ++i) {
        GumboNode* node = anchor_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");

        // We find the hrefs that are relative and contain an href.
        if (attr) {
            QString href = QString::fromUtf8(attr->value);
            if (href.indexOf(':') == -1) {
                std::pair<QString, QString> parts = Utility::parseRelativeHREF(href);
                DBG qDebug() << "In UpdateAllAnchorsInOneFile doing merge: " << parts.first << parts.second;
                // Does this href point to a bookpath in the originating_bookpaths
                QString target_bookpath = Utility::buildBookPath(parts.first, startdir);
                if (originating_bookpaths.contains(target_bookpath)) {
                    if (parts.second.isEmpty()) {
                        // this href went to the top of a merged file, so use the injected section links
                        // to update it
                        if (section_id_map.contains(target_bookpath)) {
                            parts.second = "#" + section_id_map[target_bookpath];
                        }
                    }
                    QString attpath = Utility::buildRelativePath(html_resource->GetRelativePath(), sink_bookpath);
                    QString attribute_value = Utility::buildRelativeHREF(attpath, parts.second);
                    gumbo_attribute_set_value(attr, attribute_value.toUtf8().constData());
                    is_changed = true;
                }
            }
        }
    }

    if (is_changed) {
        html_resource->SetText(CleanSource::CharToEntity(gi.getxhtml(), version));
    }
}


// use this after a split to update changed links in the NCX
void AnchorUpdates::UpdateTOCEntries(NCXResource *ncx_resource, const QString &originating_bookpath, const QList<HTMLResource *> new_files)
{
    
    // this routine should only be run on epub2
    Q_ASSERT(ncx_resource);
    const QHash<QString, QString> &ID_locations = GetIDLocations(new_files);
    // serialize the hash for passing to python
    QStringList dictkeys = ID_locations.keys();
    QStringList dictvals;
    foreach(QString key, dictkeys) {
      dictvals.append(ID_locations.value(key));
    }
    QWriteLocker locker(&ncx_resource->GetLock());
    QString source = ncx_resource->GetText();
    QString ncx_bookpath = ncx_resource->GetRelativePath();

    int rv = 0;
    QString error_traceback;

    QList<QVariant> args;
    args.append(QVariant(source));
    args.append(QVariant(ncx_bookpath));
    args.append(QVariant(originating_bookpath));
    args.append(QVariant(dictkeys));
    args.append(QVariant(dictvals));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("xmlprocessor"),
                                         QString("anchorNCXUpdates"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in xmlprocessor anchorNCXUpdates: ") + QString::number(rv), 
                                       error_traceback);
        // an error happened - make no changes
        return;
    }
    ncx_resource->SetText(res.toString());
}


// use this after a merge
void AnchorUpdates::UpdateTOCEntriesAfterMerge(NCXResource *ncx_resource, const QString &sink_bookpath, const QStringList & merged_bookpaths)
{
    
    // this routine should only be run on epub2
    Q_ASSERT(ncx_resource);
    QWriteLocker locker(&ncx_resource->GetLock());
    QString source = ncx_resource->GetText();
    QString ncx_bookpath = ncx_resource->GetRelativePath();

    int rv = 0;
    QString error_traceback;

    QList<QVariant> args;
    args.append(QVariant(source));
    args.append(QVariant(ncx_bookpath));
    args.append(QVariant(sink_bookpath));
    args.append(QVariant(merged_bookpaths));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("xmlprocessor"),
                                         QString("anchorNCXUpdatesAfterMerge"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in xmlprocessor anchorNCXUpdatesAfterMerge: ") + QString::number(rv), 
                                       error_traceback);
        // an error happened - make no changes
        return;
    }
    ncx_resource->SetText(res.toString());
}
