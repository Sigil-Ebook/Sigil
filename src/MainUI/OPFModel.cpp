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

#include <limits>

#include <QtWidgets/QApplication>
#include <QtWidgets/QFileIconProvider>
#include <QMessageBox>

#include "BookManipulation/Book.h"
#include "BookManipulation/FolderKeeper.h"
#include "MainUI/OPFModel.h"
#include "MainUI/OPFModelItem.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/NavProcessor.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "SourceUpdates/UniversalUpdates.h"

static const QList<QChar> FORBIDDEN_FILENAME_CHARS = QList<QChar>() << '<' << '>' << ':'
        << '"' << '/' << '\\'
        << '|' << '?' << '*';

OPFModel::OPFModel(QObject *parent)
    :
    QStandardItemModel(parent),
    m_RefreshInProgress(false),
    m_Book(NULL),
    m_TextFolderItem(new QStandardItem("Text")),
    m_StylesFolderItem(new QStandardItem("Styles")),
    m_ImagesFolderItem(new QStandardItem("Images")),
    m_FontsFolderItem(new QStandardItem("Fonts")),
    m_MiscFolderItem(new QStandardItem("Misc")),
    m_AudioFolderItem(new QStandardItem("Audio")),
    m_VideoFolderItem(new QStandardItem("Video"))
{
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(RowsRemovedHandler(const QModelIndex &, int, int)));
    connect(this, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(ItemChangedHandler(QStandardItem *)));
    QList<QStandardItem *> items;
    items.append(m_TextFolderItem);
    items.append(m_StylesFolderItem);
    items.append(m_ImagesFolderItem);
    items.append(m_FontsFolderItem);
    items.append(m_AudioFolderItem);
    items.append(m_VideoFolderItem);
    items.append(m_MiscFolderItem);
    QIcon folder_icon = QFileIconProvider().icon(QFileIconProvider::Folder);
    foreach(QStandardItem * item, items) {
        item->setIcon(folder_icon);
        item->setEditable(false);
        item->setDragEnabled(false);
        item->setDropEnabled(false);
        appendRow(item);
    }
    // We enable reordering of files in the text folder
    m_TextFolderItem->setDropEnabled(true);
    invisibleRootItem()->setDropEnabled(false);
}


void OPFModel::SetBook(QSharedPointer<Book> book)
{
    m_Book = book;
    connect(this, SIGNAL(BookContentModified()), m_Book.data(), SLOT(SetModified()));
    Refresh();
}


void OPFModel::Refresh()
{
    m_RefreshInProgress = true;
    InitializeModel();
    SortFilesByFilenames();
    SortHTMLFilesByReadingOrder();
    m_RefreshInProgress = false;
}


void OPFModel::SortHTML(QList <QModelIndex> index_list)
{
    m_RefreshInProgress = true;
    SortHTMLFilesByAlphanumeric(index_list);
    UpdateHTMLReadingOrders();
    m_RefreshInProgress = false;
}


QModelIndex OPFModel::GetFirstHTMLModelIndex()
{
    if (!m_TextFolderItem->hasChildren()) {
        throw(NoHTMLFiles(""));
    }

    return m_TextFolderItem->child(0)->index();
}


QModelIndex OPFModel::GetTextFolderModelIndex()
{
    return m_TextFolderItem->index();
}


QList <Resource *> OPFModel::GetResourceListInFolder(Resource *resource)
{
    return GetResourceListInFolder(resource->Type());
}


QList <Resource *> OPFModel::GetResourceListInFolder(Resource::ResourceType resource_type)
{
    QList <Resource *> resources;
    QStandardItem *folder = NULL;

    if (resource_type == Resource::HTMLResourceType) {
        folder = m_TextFolderItem;
    } else if (resource_type == Resource::CSSResourceType) {
        folder = m_StylesFolderItem;
    } else if (resource_type == Resource::ImageResourceType || resource_type == Resource::SVGResourceType) {
        folder = m_ImagesFolderItem;
    } else if (resource_type == Resource::FontResourceType) {
        folder = m_FontsFolderItem;
    } else if (resource_type == Resource::MiscTextResourceType) {
        folder = m_MiscFolderItem;
    } else if (resource_type == Resource::AudioResourceType) {
        folder = m_AudioFolderItem;
    } else if (resource_type == Resource::VideoResourceType) {
        folder = m_VideoFolderItem;
    } else if (resource_type != Resource::OPFResourceType && resource_type != Resource::NCXResourceType) {
        folder = m_MiscFolderItem;
    }

    if (folder) {
        for (int i = 0; i < folder->rowCount(); ++i) {
            QStandardItem *item = folder->child(i);
            QString identifier = item->data().toString();
            Resource *resource = m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);
            resources.append(resource);
        }
    }

    return resources;
}


// Get the index of the given resource regardless of folder
QModelIndex OPFModel::GetModelItemIndex(Resource *resource, IndexChoice indexChoice)
{
    Resource::ResourceType resourceType = resource->Type();
    QStandardItem *folder = NULL;

    if (resourceType == Resource::OPFResourceType || resourceType == Resource::NCXResourceType) {
        folder = invisibleRootItem();
    } else {
        for (int i = 0; i < invisibleRootItem()->rowCount() && folder == NULL; ++i) {
            QStandardItem *child = invisibleRootItem()->child(i);

            if ((child == m_TextFolderItem && resourceType == Resource::HTMLResourceType) ||
                (child == m_ImagesFolderItem &&
                 (resourceType == Resource::ImageResourceType || resourceType == Resource::SVGResourceType)) ||
                (child == m_StylesFolderItem &&
                 (resourceType == Resource::CSSResourceType)) ||
                (child == m_FontsFolderItem && resourceType == Resource::FontResourceType) ||
                (child == m_MiscFolderItem && resourceType == Resource::GenericResourceType) ||
                (child == m_AudioFolderItem && resourceType == Resource::AudioResourceType) ||
                (child == m_VideoFolderItem && resourceType == Resource::VideoResourceType)
               ) {
                folder = child;
            }
        }
    }

    return GetModelFolderItemIndex(folder, resource, indexChoice);
}


// Get the index of the given resource in a specific folder
QModelIndex OPFModel::GetModelFolderItemIndex(QStandardItem const *folder, Resource *resource, IndexChoice indexChoice)
{
    if (folder != NULL) {
        int rowCount = folder->rowCount();

        for (int i = 0; i < rowCount ; ++i) {
            QStandardItem *item = folder->child(i);
            const QString &identifier = item->data().toString();

            if (!identifier.isEmpty() && identifier == resource->GetIdentifier()) {
                if (folder != invisibleRootItem()) {
                    if (indexChoice == IndexChoice_Previous && i > 0) {
                        i--;
                    } else if (indexChoice == IndexChoice_Next && (i + 1 < rowCount)) {
                        i++;
                    }
                }

                return index(i, 0, folder->index());
            }
        }
    }

    return index(0, 0);
}


Resource::ResourceType OPFModel::GetResourceType(QStandardItem const *item)
{
    Q_ASSERT(item);

    if (item == m_TextFolderItem) {
        return Resource::HTMLResourceType;
    }

    if (item == m_StylesFolderItem) {
        return Resource::CSSResourceType;
    }

    if (item == m_ImagesFolderItem) {
        return Resource::ImageResourceType;
    }

    if (item == m_FontsFolderItem) {
        return Resource::FontResourceType;
    }

    if (item == m_MiscFolderItem) {
        return Resource::GenericResourceType;
    }

    if (item == m_AudioFolderItem) {
        return Resource::AudioResourceType;
    }

    if (item == m_VideoFolderItem) {
        return Resource::VideoResourceType;
    }

    const QString &identifier = item->data().toString();
    return m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier)->Type();
}


void OPFModel::sort(int column, Qt::SortOrder order)
{
    return;
}


Qt::DropActions OPFModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


//   This function initiates HTML reading order updating when the user
// moves the HTML files in the Book Browser.
//   You would expect the use of QAbstractItemModel::rowsMoved, but that
// signal is never emitted because in QStandardItemModel, row moves
// are actually handled by creating a copy of the row in the new position
// and then deleting the old row.
//   Yes, it's stupid and violates the guarantees of the QAbstractItemModel
// class. Oh and it's not documented anywhere. Nokia FTW.
//   This also handles actual HTML item deletion.
void OPFModel::RowsRemovedHandler(const QModelIndex &parent, int start, int end)
{
    if (m_RefreshInProgress ||
        itemFromIndex(parent) != m_TextFolderItem) {
        return;
    }

    UpdateHTMLReadingOrders();
}


void OPFModel::ItemChangedHandler(QStandardItem *item)
{
    Q_ASSERT(item);
    const QString &identifier = item->data().toString();

    if (!identifier.isEmpty()) {
        const QString &new_filename = item->text();
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);

        if (new_filename != resource->Filename()) {
            if (!Utility::use_filename_warning(new_filename)) {
                item->setText(resource->Filename());
                return;
            }
            RenameResource(resource, new_filename);
        }
    }

    emit ResourceRenamed();
}


bool OPFModel:: RenameResource(Resource *resource, const QString &new_filename)
{
    QList<Resource *> resources;
    QStringList filenames;
    resources.append(resource);
    filenames.append(new_filename);
    return RenameResourceList(resources, filenames);
}

bool OPFModel:: RenameResourceList(QList<Resource *> resources, QList<QString> new_filenames)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QStringList not_renamed;
    QHash<QString, QString> update;
    foreach(Resource * resource, resources) {
        const QString &old_bookrelpath = resource->GetRelativePathToRoot();
        QString old_filename = resource->Filename();
        QString extension = old_filename.right(old_filename.length() - old_filename.lastIndexOf('.'));

        QString new_filename = new_filenames.first();
        new_filenames.removeFirst();
        QString new_filename_with_extension = new_filename;

        if (!new_filename.contains('.')) {
            new_filename_with_extension.append(extension);
        }

        if (old_filename == new_filename_with_extension) {
            continue;
        }

        if (!FilenameIsValid(old_filename, new_filename_with_extension)) {
            not_renamed.append(resource->Filename());
            continue;
        }

        bool rename_success = resource->RenameTo(new_filename_with_extension);

        if (!rename_success) {
            not_renamed.append(resource->Filename());
            continue;
        }

        update[ old_bookrelpath ] = "../" + resource->GetRelativePathToOEBPS();
    }

    if (update.count() > 0) {
        UniversalUpdates::PerformUniversalUpdates(true, m_Book->GetFolderKeeper()->GetResourceList(), update);
        emit BookContentModified();
    }

    Refresh();
    QApplication::restoreOverrideCursor();

    if (not_renamed.isEmpty()) {
        return true;
    }

    return false;
}

void OPFModel::InitializeModel()
{
    Q_ASSERT(m_Book);
    ClearModel();
    QList<Resource *> resources = m_Book->GetFolderKeeper()->GetResourceList();
    QHash <Resource *, int> reading_order_all = m_Book->GetOPF()->GetReadingOrderAll(resources);
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    QHash <QString, QString> semantic_type_all;
    QHash <QString, QString> manifest_properties_all;
    if (version.startsWith('3')) {
        NavProcessor navproc(m_Book->GetConstOPF()->GetNavResource());
        semantic_type_all = navproc.GetLandmarkNameForPaths();
        manifest_properties_all = m_Book->GetOPF()->GetManifestPropertiesForPaths();
    } else { 
        semantic_type_all = m_Book->GetOPF()->GetGuideSemanticNameForPaths();
    }

    foreach(Resource * resource, resources) {
        AlphanumericItem *item = new AlphanumericItem(resource->Icon(), resource->Filename());
        item->setDropEnabled(false);
        item->setData(resource->GetIdentifier());
        QString tooltip = resource->Filename();
        QString path = resource->GetRelativePathToOEBPS();

        if (semantic_type_all.contains(path)) {
            tooltip += " (" + semantic_type_all[path] + ")";
        }
        if (manifest_properties_all.contains(path)) {
            tooltip += " [" + manifest_properties_all[path] + "]";
        }
        item->setToolTip(tooltip);

        if (resource->Type() == Resource::HTMLResourceType) {
            int reading_order = -1;
            if (reading_order_all.contains(resource)) {
                reading_order = reading_order_all[resource];
            } else {
                reading_order = NO_READING_ORDER;
            }

            item->setData(reading_order, READING_ORDER_ROLE);
            // Remove the extension for alphanumeric sorting
            QString name = resource->Filename().left(resource->Filename().lastIndexOf('.'));
            item->setData(name, ALPHANUMERIC_ORDER_ROLE);
            m_TextFolderItem->appendRow(item);
        } else if (resource->Type() == Resource::CSSResourceType) {
            item->setDragEnabled(false);
            m_StylesFolderItem->appendRow(item);
        } else if (resource->Type() == Resource::ImageResourceType ||
                   resource->Type() == Resource::SVGResourceType
                  ) {
            m_ImagesFolderItem->appendRow(item);
        } else if (resource->Type() == Resource::FontResourceType) {
            item->setDragEnabled(false);
            m_FontsFolderItem->appendRow(item);
        } else if (resource->Type() == Resource::AudioResourceType) {
            item->setDragEnabled(false);
            m_AudioFolderItem->appendRow(item);
        } else if (resource->Type() == Resource::VideoResourceType) {
            item->setDragEnabled(false);
            m_VideoFolderItem->appendRow(item);
        } else if (resource->Type() == Resource::OPFResourceType ||
                   resource->Type() == Resource::NCXResourceType) {
            item->setEditable(false);
            item->setDragEnabled(false);
            appendRow(item);
        } else {
            m_MiscFolderItem->appendRow(item);
        }
    }
}


void OPFModel::UpdateHTMLReadingOrders()
{
    QList<HTMLResource *> reading_order_htmls;

    for (int i = 0; i < m_TextFolderItem->rowCount(); ++i) {
        QStandardItem *html_item = m_TextFolderItem->child(i);
        Q_ASSERT(html_item);
        html_item->setData(i, READING_ORDER_ROLE);
        HTMLResource *html_resource =  qobject_cast<HTMLResource *>(
                                           m_Book->GetFolderKeeper()->GetResourceByIdentifier(html_item->data().toString()));

        if (html_resource != NULL) {
            reading_order_htmls.append(html_resource);
        }
    }

    m_Book->GetOPF()->UpdateSpineOrder(reading_order_htmls);
    m_Book->SetModified();
}


void OPFModel::ClearModel()
{
    while (m_TextFolderItem->rowCount() != 0) {
        m_TextFolderItem->removeRow(0);
    }

    while (m_StylesFolderItem->rowCount() != 0) {
        m_StylesFolderItem->removeRow(0);
    }

    while (m_ImagesFolderItem->rowCount() != 0) {
        m_ImagesFolderItem->removeRow(0);
    }

    while (m_FontsFolderItem->rowCount() != 0) {
        m_FontsFolderItem->removeRow(0);
    }

    while (m_MiscFolderItem->rowCount() != 0) {
        m_MiscFolderItem->removeRow(0);
    }

    while (m_AudioFolderItem->rowCount() != 0) {
        m_AudioFolderItem->removeRow(0);
    }

    while (m_VideoFolderItem->rowCount() != 0) {
        m_VideoFolderItem->removeRow(0);
    }

    int i = 0;

    while (i < invisibleRootItem()->rowCount()) {
        QStandardItem *child = invisibleRootItem()->child(i, 0);

        if (child != m_TextFolderItem   &&
            child != m_StylesFolderItem &&
            child != m_ImagesFolderItem &&
            child != m_FontsFolderItem  &&
            child != m_MiscFolderItem   &&
            child != m_AudioFolderItem  &&
            child != m_VideoFolderItem) {
            invisibleRootItem()->removeRow(i);
        } else {
            ++i;
        }
    }
}


void OPFModel::SortFilesByFilenames()
{
    for (int i = 0; i < invisibleRootItem()->rowCount(); ++i) {
        invisibleRootItem()->child(i)->sortChildren(0);
    }
}


void OPFModel::SortHTMLFilesByReadingOrder()
{
    int old_sort_role = sortRole();
    setSortRole(READING_ORDER_ROLE);
    m_TextFolderItem->sortChildren(0);
    setSortRole(old_sort_role);
}

void OPFModel::SortHTMLFilesByAlphanumeric(QList <QModelIndex> index_list)
{
    // Get items for all selected indexes
    QList<QStandardItem *> item_list;
    foreach(QModelIndex index, index_list) {
        QStandardItem *qitem = itemFromIndex(index);
        item_list.append(qitem);
    }
    // Create new model for selected items to allow temporary sorting
    QStandardItemModel sort_model;
    sort_model.setSortRole(ALPHANUMERIC_ORDER_ROLE);
    QStandardItem *items = new QStandardItem();
    sort_model.setItem(0, items);
    int first_item_position = -1;
    foreach(QStandardItem *item, item_list) {
        int i = 0;

        while (i < m_TextFolderItem->rowCount()) {
            if (m_TextFolderItem->child(i) == item) {
                if (first_item_position < 0) {
                    first_item_position = i;
                }

                QList<QStandardItem *> removed_items = m_TextFolderItem->takeRow(i--);
                foreach(QStandardItem *one_item, removed_items) {
                    items->appendRow(one_item);
                }
                break;
            }

            i++;
        }
    }
    items->sortChildren(0);

    for (int i = 0; i < items->rowCount(); ++i) {
        QList<QStandardItem *> removed_items = items->takeRow(i--);
        foreach(QStandardItem *one_item, removed_items) {
            m_TextFolderItem->insertRow(first_item_position++, one_item);
        }
    }
}


bool OPFModel::FilenameIsValid(const QString &old_filename, const QString &new_filename)
{
    foreach(QChar character, new_filename) {
        if (FORBIDDEN_FILENAME_CHARS.contains(character)) {
            Utility::DisplayStdErrorDialog(
                tr("A filename cannot contains the character \"%1\".")
                .arg(character)
            );
            return false;
        }
    }
    const QString &extension = QFileInfo(new_filename).suffix();
    QString new_filename_without_extension = new_filename.left(new_filename.length() - extension.length() - 1);

    if (new_filename.isEmpty() || new_filename_without_extension.isEmpty()) {
        Utility::DisplayStdErrorDialog(
            tr("The filename cannot be empty.")
        );
        return false;
    }

    if (new_filename != m_Book->GetFolderKeeper()->GetUniqueFilenameVersion(new_filename)) {
        Utility::DisplayStdErrorDialog(
            tr("The filename \"%1\" is already in use.\n")
            .arg(new_filename)
        );
        return false;
    }

    return true;
}


