/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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

#include <QtCore/QtCore>
#include <QtGui/QApplication>
#include <QtGui/QProgressDialog>

#include "ResourceObjects/HTMLResource.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "MiscEditors/IndexEditorModel.h"
#include "BookManipulation/Index.h"
#include "MiscEditors/IndexEntries.h"
#include "sigil_constants.h"


using boost::shared_ptr;

const QString SIGIL_INDEX_CLASS = "sigil_index_marker";
const QString SIGIL_INDEX_ID_PREFIX = "sigil_index_id_";

bool Index::BuildIndex(QList<HTMLResource*> html_resources)
{
    IndexEntries::instance()->Clear();

    // Display progress dialog
    QProgressDialog progress(QObject::tr("Creating Index..."), QObject::tr("Cancel"), 0, html_resources.count(), QApplication::activeWindow());
    progress.setMinimumDuration(0);
    int progress_value = 0;
    progress.setValue(progress_value);
    qApp->processEvents();

    // Must do sequentially in order to keep sections in order
    foreach (HTMLResource *html_resource, html_resources) {
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
    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    QList< xc::DOMNode* > nodes = XhtmlDoc::GetIDNodes(*d.get());

    bool resource_updated = false;

    int index_id_number = 1;
    foreach(xc::DOMNode *node, nodes) {
        QString index_id_value;

        xc::DOMElement &element = static_cast< xc::DOMElement &>(*node);
        QString text_node_text = XhtmlDoc::GetIDElementText(*node);

        // Remove existing index ids
        if (element.hasAttribute(QtoX("id"))) {
            index_id_value = XtoQ(element.getAttribute(QtoX("id")));
            if (index_id_value.startsWith(SIGIL_INDEX_ID_PREFIX)) {
                element.removeAttribute(QtoX("id"));
                resource_updated = true;
            }
        }

        // If this node is a custom index entry make sure it gets included
        bool is_custom_index_entry = false;
        QString custom_index_value = text_node_text;
        if (element.hasAttribute(QtoX("class"))) {
            QString class_names = XtoQ(element.getAttribute(QtoX("class")));
            if (class_names.split(" ").contains(SIGIL_INDEX_CLASS)) {
                is_custom_index_entry = true;
                if (element.hasAttribute(QtoX("title"))) {
                    QString title = XtoQ(element.getAttribute(QtoX("title")));
                    if (!title.isEmpty()) {
                        custom_index_value = title;
                    }
                }
            }
        }

        // Use the existing id if there is one, else add one if node contains index item
        if (element.hasAttribute(QtoX("id"))) {
            CreateIndexEntry(text_node_text, html_resource, index_id_value, is_custom_index_entry, custom_index_value);
        }
        else {
            index_id_value = SIGIL_INDEX_ID_PREFIX + QString::number(index_id_number);

            if (CreateIndexEntry(text_node_text, html_resource, index_id_value, is_custom_index_entry, custom_index_value)) {
                element.setAttribute(QtoX("id"), QtoX(index_id_value));
                resource_updated = true;
                index_id_number++;
            }

        }
    }

    if (resource_updated) {
        html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(*d.get()));
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
    }
    else {
        entries = IndexEditorModel::instance()->GetEntries();
    }
    
    foreach (IndexEditorModel::indexEntry *entry, entries) {
        foreach(QString index_pattern, entry->pattern.split(";")) {
            if (index_pattern.isEmpty()) {
                continue;
            }
            QRegExp index_regex(index_pattern);
            if (text.contains(index_regex)) {
                created_index = true;
                foreach (QString index_entry, entry->index_entry.split(";")) {
                    if (index_entry.isEmpty()) {
                        // If no index text, use the pattern
                        IndexEntries::instance()->AddOneEntry(index_pattern, html_resource->Filename(), index_id_value);
                    }
                    else if (entry->index_entry.endsWith("/")) {
                        // If index text is a category then append the pattern
                        IndexEntries::instance()->AddOneEntry(index_entry + index_pattern, html_resource->Filename(), index_id_value);
                    }
                    else {
                        // Use the given index text
                        IndexEntries::instance()->AddOneEntry(index_entry, html_resource->Filename(), index_id_value);
                    }
                }
            }
        }
    }
    return created_index;
}
