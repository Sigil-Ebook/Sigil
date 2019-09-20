/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks Stratford, ON, Canada 
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
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

#include <QtCore/QtCore>
#include <QtConcurrent/QtConcurrent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProgressDialog>
#include <QRegularExpression>

#include "ResourceObjects/HTMLResource.h"
#include "Misc/GumboInterface.h"
#include "BookManipulation/XhtmlDoc.h"
#include "MiscEditors/IndexEditorModel.h"
#include "BookManipulation/Index.h"
#include "MiscEditors/IndexEntries.h"
#include "sigil_constants.h"

const QString SIGIL_INDEX_CLASS = "sigil_index_marker";
const QString SIGIL_INDEX_ID_PREFIX = "sigil_index_id_";

bool Index::BuildIndex(QList<HTMLResource *> html_resources)
{
    IndexEntries::instance()->Clear();
    // Display progress dialog
    QProgressDialog progress(QObject::tr("Creating Index..."), QObject::tr("Cancel"), 0, html_resources.count(), QApplication::activeWindow());
    progress.setMinimumDuration(0);
    int progress_value = 0;
    progress.setValue(progress_value);
    qApp->processEvents();
    // Must do sequentially in order to keep sections in order
    foreach(HTMLResource * html_resource, html_resources) {
        // Set progress value and ensure dialog has time to display when doing extensive updates
        if (progress.wasCanceled()) {
            return false;
        }

        progress.setValue(progress_value++);
        qApp->processEvents();
        AddIndexIDsOneFile(html_resource);
    }
    return true;
}

void Index::AddIndexIDsOneFile(HTMLResource *html_resource)
{
    QWriteLocker locker(&html_resource->GetLock());
    QString source = html_resource->GetText();
    QString version = html_resource->GetEpubVersion();
    GumboInterface gi = GumboInterface(source, version);
    QList<GumboNode*> nodes = XhtmlDoc::GetIDNodes(gi, gi.get_root_node());
    bool resource_updated = false;
    int index_id_number = 1;
    foreach(GumboNode * node, nodes) {
        QString index_id_value;

        // Get the text of all sub-nodes.
        QString text_node_text = XhtmlDoc::GetIDElementText(gi, node);
        // Convert &nbsp; to space since Index Editor unfortunately does the same.
        text_node_text.replace(QChar(160), " ");

        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr) {
            index_id_value = QString::fromUtf8(attr->value);
            if (index_id_value.startsWith(SIGIL_INDEX_ID_PREFIX)) {
                GumboElement* element = &node->v.element;
                gumbo_element_remove_attribute(element, attr);
                resource_updated = true;
            }
        }

        // If this node is a custom index entry make sure it gets included
        bool is_custom_index_entry = false;
        QString custom_index_value = text_node_text;

        attr = gumbo_get_attribute(&node->v.element.attributes, "class");
        if (attr) {
            QString class_names = QString::fromUtf8(attr->value);

            if (class_names.split(" ").contains(SIGIL_INDEX_CLASS)) {
                is_custom_index_entry = true;
                
                GumboAttribute* titleattr = gumbo_get_attribute(&node->v.element.attributes, "title");
                if (titleattr) {
                    QString title = QString::fromUtf8(titleattr->value);
                    if (!title.isEmpty()) {
                        custom_index_value = title;
                    }
                }
            }

        }

        // Use the existing id if there is one, else add one if node contains index item
        attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr) {
            CreateIndexEntry(text_node_text, html_resource, index_id_value, is_custom_index_entry, custom_index_value);
        } else {
            index_id_value = SIGIL_INDEX_ID_PREFIX + QString::number(index_id_number);

            if (CreateIndexEntry(text_node_text, html_resource, index_id_value, is_custom_index_entry, custom_index_value)) {
                GumboElement* element = &node->v.element;
                gumbo_element_set_attribute(element, "id", index_id_value.toUtf8().constData()); 
                resource_updated = true;
                index_id_number++;
            }
        }
    }

    if (resource_updated) {
        html_resource->SetText(gi.getxhtml());
    }
}


bool Index::CreateIndexEntry(const QString text, HTMLResource *html_resource, QString index_id_value, bool is_custom_index_entry, QString custom_index_value)
{
    bool created_index = false;
    QList<IndexEditorModel::indexEntry *> entries;

    if (is_custom_index_entry) {
        IndexEditorModel::indexEntry *custom_entry = new IndexEditorModel::indexEntry();
        custom_entry->pattern = text;
        custom_entry->index_entry = custom_index_value;
        entries.append(custom_entry);
    } else {
        entries = IndexEditorModel::instance()->GetEntries();
    }

    foreach(IndexEditorModel::indexEntry * entry, entries) {
        QString index_pattern = entry->pattern;
        if (index_pattern.isEmpty()) {
            continue;
        }

        QRegularExpression index_regex(index_pattern);

        if (text.contains(index_regex)) {
            created_index = true;
            QString index_entry = entry->index_entry;
            if (index_entry.isEmpty()) {
                // If no index text, use the pattern
                IndexEntries::instance()->AddOneEntry(index_pattern, html_resource->ShortPathName(), index_id_value);
            } else if (entry->index_entry.endsWith("/")) {
                // If index text is a category then append the pattern
                IndexEntries::instance()->AddOneEntry(index_entry + index_pattern, html_resource->ShortPathName(), index_id_value);
            } else {
                // Use the given index text
                IndexEntries::instance()->AddOneEntry(index_entry, html_resource->ShortPathName(), index_id_value);
            }
        }
    }
    return created_index;
}
