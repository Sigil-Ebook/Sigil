/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, Ontario Canada
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

#include <QString>
#include <QHash>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

#include "Misc/Utility.h"
#include "Parsers/GumboInterface.h"
#include "BookManipulation/CleanSource.h"
#include "ResourceObjects/HTMLResource.h"
#include "SourceUpdates/FragmentUpdates.h"

#define DBG if(0)

// Used to change ids and update any links to those ids from any other html file
// Given a QHash of bookpath+#+id to new_id
// No resources are being renamed or moved, just id changes

void FragmentUpdates::UpdateFragments(const QList<HTMLResource*> &html_resources, 
                                      const QHash<QString, QString> &updates)
{
    QtConcurrent::blockingMap(html_resources, std::bind(UpdateFragmentsInOneFile, 
                                                        std::placeholders::_1,
                                                        updates));
}


void FragmentUpdates::UpdateFragmentsInOneFile(HTMLResource * htmlresource, 
                                              const QHash<QString, QString> &updates)
{ 
    Q_ASSERT(htmlresource);
    QWriteLocker locker(&htmlresource->GetLock());
    QString version = htmlresource->GetEpubVersion();
    QString bookpath = htmlresource->GetRelativePath();
    bool is_changed = false;

    GumboInterface gi = GumboInterface(htmlresource->GetText(), version);
    gi.parse();

    // first check every id and if needed update them
    QList<GumboNode*> id_nodes = gi.get_all_nodes_with_attribute(QString("id"));
    id_nodes.append(gi.get_all_nodes_with_attribute(QString("name")));
    foreach(GumboNode * node, id_nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr) {
            QString idval = QString::fromUtf8(attr->value);
            QString key = bookpath + "#" + idval;
            QString newid = updates.value(key, "");
            if (!newid.isEmpty()) {
                gumbo_attribute_set_value(attr, newid.toUtf8().constData());
                is_changed = true;
            }
        }
    }

    // next check every href and see if it needs to be updated
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);

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
                    QString dest_bookpath;
                    if (base_href.isEmpty()) {
                        dest_bookpath = bookpath;
                    } else {
                        dest_bookpath = Utility::buildBookPath(base_href, Utility::startingDir(bookpath));
                    }
                    DBG qDebug() << "base_href" << base_href;
                    DBG qDebug() << "dest_bookpath" << dest_bookpath;
                    DBG qDebug() << "fragment_id" << fragment_id;
                    DBG qDebug() << "bookpath" << fragment_id;
                    if (!fragment_id.isEmpty()) {
                        QString key = dest_bookpath + "#" + fragment_id;
                        QString newid = updates.value(key,"");
                        if (!newid.isEmpty()) {
                            QString attpath = Utility::buildRelativePath(bookpath, dest_bookpath);
                            QString attribute_value = Utility::buildRelativeHREF(attpath, "#" + newid);
                            gumbo_attribute_set_value(attr, attribute_value.toUtf8().constData());
                            is_changed = true;
                        }
                    }
                }
            }
        }
    }
    if (is_changed) {
        htmlresource->SetText(CleanSource::CharToEntity(gi.getxhtml(), version));
    }
}
