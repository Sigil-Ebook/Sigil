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

#include <QtCore/QStringList>
#include <QtGui/QStandardItem>
#include <QKeyEvent>

#include "BookManipulation/Book.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Dialogs/HeadingSelector.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_constants.h"

static const QString SETTINGS_GROUP   = "heading_selector";
static const int FIRST_COLUMN_PADDING = 30;

const QString SIGIL_TOC_ID_PREFIX = "sigil_toc_id_";
const QString OLD_SIGIL_TOC_ID_PREFIX = "heading_id_";

// Constructor;
// the first parameter is the book whose TOC
// is being edited, the second is the dialog's parent
HeadingSelector::HeadingSelector(QSharedPointer< Book > book, QWidget *parent)
    :
    QDialog(parent),
    m_Book(book),
    m_ContextMenu(new QMenu(this)),
    m_book_changed(false)
{
    ui.setupUi(this);
    ui.tvTOCDisplay->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.tvTOCDisplay->installEventFilter(this);
    CreateContextMenuActions();
    ConnectSignalsToSlots();
    ui.tvTOCDisplay->setModel(&m_TableOfContents);
    LockHTMLResources();
    QList< Headings::Heading > flat_headings = Headings::GetHeadingList(
                m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >(true), true);
    m_Headings = Headings::MakeHeadingHeirarchy(flat_headings);
    PopulateSelectHeadingCombo(GetMaxHeadingLevel(flat_headings));
    RefreshTOCModelDisplay();
    ReadSettings();
}


// Destructor
HeadingSelector::~HeadingSelector()
{
    WriteSettings();
    UnlockHTMLResources();
}

bool HeadingSelector::IsBookChanged()
{
    return m_book_changed;
}

int HeadingSelector::GetAbsoluteRowForIndex(QModelIndex current_index)
{
    QModelIndex index = m_TableOfContents.invisibleRootItem()->child(0)->index();
    int row = 0;
    bool past_last_row = false;

    while (index.isValid()) {
        if (current_index == index) {
            break;
        }

        index = ui.tvTOCDisplay->indexBelow(index);

        if (!index.isValid()) {
            past_last_row = true;
            break;
        }

        row++;
    }

    if (past_last_row) {
        row = -1;
    }

    return row;
}

QModelIndex HeadingSelector::GetIndexForAbsoluteRow(int row)
{
    QModelIndex found_index;

    if (row < 0) {
        return found_index;
    }

    QModelIndex index = m_TableOfContents.invisibleRootItem()->child(0)->index();
    bool past_last_row = false;

    for (int i = 0; i < row; i++) {
        index = ui.tvTOCDisplay->indexBelow(index);

        if (!index.isValid()) {
            past_last_row = true;
            break;
        }
    }

    if (!past_last_row) {
        found_index = index;
    }

    return found_index;
}

QModelIndex HeadingSelector::SelectAbsoluteRow(int row)
{
    // Select the item that was below the original item if any.
    if (m_TableOfContents.invisibleRootItem()->rowCount() == 0) {
        return QModelIndex();
    }

    QModelIndex first_index = m_TableOfContents.invisibleRootItem()->child(0)->index();
    QModelIndex index = first_index;
    QModelIndex previous_index = index;

    for (int i = 0; i < row; i++) {
        previous_index = index;
        index = ui.tvTOCDisplay->indexBelow(index);
    }

    if (!index.isValid()) {
        index = previous_index;
    }

    if (!index.isValid()) {
        index = first_index;
    }

    // Select the item
    ui.tvTOCDisplay->selectionModel()->clear();
    ui.tvTOCDisplay->setCurrentIndex(index);
    ui.tvTOCDisplay->selectionModel()->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    return index;
}


// We need to filter the calls to functions that would normally
// connect directly to the itemChanged( QStandardItem* ) signal
// because some would-be slots delete the item in question.
// So, the signal connects here and this function calls the
// appropriate item-handling functions.
void HeadingSelector::ModelItemFilter(QStandardItem *item)
{
    Q_ASSERT(item);

    if (!item->isCheckable()) {
        // Heading changed
        UpdateOneHeadingTitle(item, item->text());
        return;
    }

    // Get the absolute row of the index in the table for later re-selection
    QModelIndex current_index = item->index().sibling(item->index().row(), 0);
    int row = GetAbsoluteRowForIndex(current_index);
    // Do the actual update of the list
    UpdateHeadingInclusion(item);
    QModelIndex new_index = SelectAbsoluteRow(row);

    // Expand the item and all its children
    if (new_index.isValid()) {
        QStandardItem *new_item = m_TableOfContents.itemFromIndex(new_index);
        QStandardItem *parent_item = GetActualItemParent(new_item);

        if (new_item) {
            for (int i = 0; i < parent_item->rowCount(); i++) {
                if (i >= new_item->row()) {
                    ExpandChildren(parent_item->child(i));
                }
            }
        }
    }
}

void HeadingSelector::ExpandChildren(QStandardItem *item)
{
    QModelIndexList indexes;

    if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); i++) {
            ExpandChildren(item->child(i, 0));
        }
    }

    ui.tvTOCDisplay->expand(item->index());
}


// Switches the display between showing all headings
// and showing only headings that are to be included in the TOC
void HeadingSelector::ChangeDisplayType(int new_check_state)
{
    // If checked, show only TOC items
    if (new_check_state == Qt::Checked) {
        RemoveExcludedItems(m_TableOfContents.invisibleRootItem());
    }
    // If unchecked, show all items
    else {
        CreateTOCModel();
        UpdateTreeViewDisplay();
    }
}


void HeadingSelector::UpdateHeadingElements()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // We recreate the model to make sure even those
    // headings marked as "don't include" are in the model.
    CreateTOCModel();
    // Identify any existing hrefs to an id that will indicate we can't change it
    QStringList used_ids = m_Book->GetIdsInHrefs();
    // Iterate through our headings, applying their changes to the underlying resources
    // if required, setting ids etc.
    int next_toc_id = 1;
    UpdateOneHeadingElement(m_TableOfContents.invisibleRootItem(), used_ids, next_toc_id);
    // Now iterate through the headings one final time, to save the underlying resources
    // that we have changed.
    QStringList ids;
    foreach(Headings::Heading heading, m_Headings) {
        ids = UpdateOneFile(heading, ids);
    }
    // Finally check to see whether we did actually make a change to the book.
    foreach(Headings::Heading heading, m_Headings) {
        if (heading.is_changed) {
            m_book_changed = true;
            break;
        }
    }
    QApplication::restoreOverrideCursor();
}

int HeadingSelector::UpdateOneHeadingElement(QStandardItem *item, QStringList used_ids, int next_toc_id)
{
    Headings::Heading *heading = GetItemHeading(item);

    if (heading != NULL) {
        // Update heading inclusion: if a heading element
        // has one of the SIGIL_NOT_IN_TOC_CLASS classes, then it's not in the TOC
        const QString &class_attribute = XtoQ(heading->element->getAttribute(QtoX("class")));
        QString new_class_attribute = QString(class_attribute)
                                      .remove(SIGIL_NOT_IN_TOC_CLASS)
                                      .remove(OLD_SIGIL_NOT_IN_TOC_CLASS)
                                      .simplified();

        if (!heading->include_in_toc) {
            new_class_attribute = new_class_attribute.append(" " % SIGIL_NOT_IN_TOC_CLASS).simplified();
        }

        // Only apply the change if it is different
        if (new_class_attribute != class_attribute) {
            heading->is_changed = true;

            if (!new_class_attribute.isEmpty()) {
                heading->element->setAttribute(QtoX("class"), QtoX(new_class_attribute));
            } else {
                heading->element->removeAttribute(QtoX("class"));
            }
        }

        // Now apply the new id as needed.
        const QString &existing_id_attribute = heading->element->hasAttribute(QtoX("id"))
                                               ? XtoQ(heading->element->getAttribute(QtoX("id")))
                                               : QString();
        QString new_id_attribute(existing_id_attribute);

        if (!heading->include_in_toc) {
            // Since this heading is not being put in the toc, we will remove any existing id from it
            // provided it is Sigil generated and not in use already.
            if (!used_ids.contains(existing_id_attribute) &&
                (existing_id_attribute.startsWith(SIGIL_TOC_ID_PREFIX) ||
                 existing_id_attribute.startsWith(OLD_SIGIL_TOC_ID_PREFIX))) {
                new_id_attribute.clear();
            }
        } else {
            // We definitely need to give it an id if it does not already have one.
            // We can replace any existing id with a new autogenerated one
            // provided it is Sigil generated and not in use already.
            if (!used_ids.contains(existing_id_attribute) &&
                (existing_id_attribute.isEmpty() ||
                 existing_id_attribute.startsWith(SIGIL_TOC_ID_PREFIX) ||
                 existing_id_attribute.startsWith(OLD_SIGIL_TOC_ID_PREFIX))) {
                // We can replace it with a new sigil prefix version.
                do {
                    new_id_attribute = SIGIL_TOC_ID_PREFIX + QString::number(next_toc_id);
                    next_toc_id++;
                } while (used_ids.contains(new_id_attribute));
            }
        }

        // Only apply the change if it is different
        if (new_id_attribute.trimmed() != existing_id_attribute) {
            heading->is_changed = true;

            if (!new_id_attribute.isEmpty()) {
                heading->element->setAttribute(QtoX("id"), QtoX(new_id_attribute));
            } else {
                heading->element->removeAttribute(QtoX("id"));
            }
        }
    }

    if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); ++i) {
            next_toc_id = UpdateOneHeadingElement(item->child(i), used_ids, next_toc_id);
        }
    }

    return next_toc_id;
}

QStringList HeadingSelector::UpdateOneFile(Headings::Heading &heading, QStringList ids)
{
    // Only save the document if we have changed the heading
    if (heading.is_changed) {
        if (!ids.contains(heading.resource_file->GetIdentifier())) {
            heading.resource_file->SetText(XhtmlDoc::GetDomDocumentAsString(*heading.document.get()));
            ids << heading.resource_file->GetIdentifier();
        }
    }

    if (!heading.children.isEmpty()) {
        for (int i = 0; i < heading.children.count(); ++i) {
            ids = UpdateOneFile(heading.children[ i ], ids);
        }
    }

    return ids;
}

void HeadingSelector::UpdateOneHeadingTitle(QStandardItem *item, const QString &title)
{
    Headings::Heading *heading = GetItemHeading(item);

    if (heading != NULL) {
        QString title_attribute = heading->title;

        if (title != heading->title) {
            heading->title = title;
            heading->is_changed = true;
            heading->element->setAttribute(QtoX("title"), QtoX(title));
        }
    }
}

void HeadingSelector::DecreaseHeadingLevel()
{
    ChangeHeadingLevel(-1);
}

void HeadingSelector::IncreaseHeadingLevel()
{
    ChangeHeadingLevel(1);
}

void HeadingSelector::ChangeHeadingLevel(int change_amount)
{
    if (!ui.tvTOCDisplay->selectionModel()->hasSelection() || change_amount == 0) {
        return;
    }

    QModelIndex selected_index = ui.tvTOCDisplay->selectionModel()->selectedRows(0).first();
    QStandardItem *item = m_TableOfContents.itemFromIndex(selected_index);

    if (!item) {
        return;
    }

    Headings::Heading *heading = GetItemHeading(item);

    if (heading == NULL) {
        return;
    }

    // Can't change level if not a valid h1-h6 tag
    if (heading->level < 1 || heading->level > 6) {
        return;
    }

    // Change the heading level if valid to do so
    if ((heading->level + change_amount < 1) || (heading->level + change_amount > 6)) {
        return;
    }

    heading->level += change_amount;
    // Update whether we have made changes to the document for this heading element
    heading->is_changed = (heading->level != heading->orig_level) || (heading->title != heading->orig_title);
    // Get new tag name
    QString tag_name = XtoQ(heading->element->getTagName());
    QString new_tag_name = "h" + QString::number(heading->level);
    // Rename in document
    heading->element = XhtmlDoc::RenameElementInDocument(*heading->document, *heading->element, new_tag_name);
    // Clear all children information then rebuild hierarchy
    QList< Headings::Heading > flat_headings = Headings::GetFlattenedHeadings(m_Headings);

    for (int i = 0; i < flat_headings.count(); i++) {
        flat_headings[i].children.clear();
    }

    m_Headings = Headings::MakeHeadingHeirarchy(flat_headings);
    // Save selected row, refresh the display, restore selection
    int row = GetAbsoluteRowForIndex(selected_index);
    RefreshTOCModelDisplay();
    SelectAbsoluteRow(row);
    PopulateSelectHeadingCombo(GetMaxHeadingLevel(flat_headings));
}


// Updates the inclusion of the heading in the TOC
// whenever that heading's "include in TOC" checkbox
// is checked/unchecked.
void HeadingSelector::UpdateHeadingInclusion(QStandardItem *checkbox_item)
{
    Q_ASSERT(checkbox_item);
    QStandardItem *item_parent = GetActualItemParent(checkbox_item);
    Headings::Heading *heading = GetItemHeading(item_parent->child(checkbox_item->row(), 0));
    Q_ASSERT(heading);
    QString heading_level = item_parent->child(checkbox_item->row(), 1)->text();

    if (checkbox_item->checkState() == Qt::Unchecked) {
        heading->include_in_toc = false;
        m_HeadingsIncluded[heading_level]--;
        m_HeadingsHidden[heading_level]++;
    } else {
        heading->include_in_toc = true;
        m_HeadingsIncluded[heading_level]++;
        m_HeadingsHidden[heading_level]--;
    }

    DisplayCounts();

    if (ui.cbTOCItemsOnly->checkState() == Qt::Checked) {
        RemoveExcludedItems(m_TableOfContents.invisibleRootItem());
    }
}



// Updates the display of the tree view
// (resizes columns etc.)
void HeadingSelector::UpdateTreeViewDisplay()
{
    ui.tvTOCDisplay->expandAll();
    // Make the header fill all the available space not used by the checkbox
    ui.tvTOCDisplay->header()->setStretchLastSection(false);
    ui.tvTOCDisplay->resizeColumnToContents(1);
    ui.tvTOCDisplay->resizeColumnToContents(2);
    ui.tvTOCDisplay->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void HeadingSelector::CountHeadings(QStandardItem *item)
{
    // Recursively call itself on the item's children
    for (int i = 0; i < item->rowCount(); i++) {
        CountHeadings(item->child(i));
    }

    // The root item is always present
    if (item == m_TableOfContents.invisibleRootItem()) {
        return;
    }

    // We query the "include in TOC" checkbox
    QStandardItem *item_parent = GetActualItemParent(item);
    Qt::CheckState check_state = item_parent->child(item->row(), 2)->checkState();
    QString heading_level = item_parent->child(item->row(), 1)->text();

    if (check_state) {
        m_HeadingsIncluded[heading_level]++;
    } else {
        m_HeadingsHidden[heading_level]++;
    }
}

void HeadingSelector::RefreshCounts()
{
    foreach(QString h, HEADING_TAGS) {
        m_HeadingsIncluded[h] = 0;
        m_HeadingsHidden[h] = 0;
    }
    CountHeadings(m_TableOfContents.invisibleRootItem());
    DisplayCounts();
}

void HeadingSelector::DisplayCounts()
{
    QString tooltip = QString() +
                      "<table cellpadding=\"5\">" +
                      "<tr><th>" + tr("Level") + "</th><th>" + tr("Included") + "</th><th>" + tr("Hidden") + "</th></tr>";
    foreach(QString h, HEADING_TAGS) {
        tooltip += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(h).arg(m_HeadingsIncluded[h]).arg(m_HeadingsHidden[h]);
    }
    tooltip += "</table>";
    ui.tvTOCDisplay->header()->setToolTip(tooltip);
}


// Creates the model that is displayed
// in the tree view
void HeadingSelector::CreateTOCModel()
{
    m_TableOfContents.clear();
    QStringList header;
    header.append(tr("TOC Entry / Heading Title"));
    header.append(tr("Level") % " ");
    header.append(tr("Include") % " ");
    m_TableOfContents.setHorizontalHeaderLabels(header);

    // Recursively inserts all headings
    for (int i = 0; i < m_Headings.count(); ++i) {
        InsertHeadingIntoModel(m_Headings[ i ], m_TableOfContents.invisibleRootItem());
    }

    RefreshCounts();
}


// Inserts the specified heading into the model
// as the child of the specified parent item;
// recursively calls itself on the headings children,
// thus building a TOC tree
void HeadingSelector::InsertHeadingIntoModel(Headings::Heading &heading, QStandardItem *parent_item)
{
    Q_ASSERT(parent_item);
    QString heading_text = !heading.title.isNull() ? heading.title : heading.text;
    QStandardItem *item_heading           = new QStandardItem(heading_text);
    QStandardItem *heading_level          = new QStandardItem("h" % QString::number(heading.level));
    QStandardItem *heading_included_check = new QStandardItem();
    heading_level->setEditable(false);
    heading_included_check->setEditable(false);
    heading_included_check->setCheckable(true);
    item_heading->setEditable(true);
    item_heading->setDragEnabled(false);
    item_heading->setDropEnabled(false);

    if (heading.include_in_toc) {
        heading_included_check->setCheckState(Qt::Checked);
    } else {
        heading_included_check->setCheckState(Qt::Unchecked);
    }

    // Storing a pointer to the heading that
    // is represented by this QStandardItem
    Headings::HeadingPointer wrap;
    wrap.heading = &heading;
    item_heading->setData(QVariant::fromValue(wrap));
    // Apparently using \n in the string means you don't have to replace < with &lt; or > with &gt;
    QString html = XhtmlDoc::GetDomNodeAsString(*heading.element).remove("xmlns=\"http://www.w3.org/1999/xhtml\"");
    item_heading->setToolTip(heading.resource_file->Filename() + ":\n\n" + html);
    QList< QStandardItem * > items;
    items << item_heading << heading_level << heading_included_check;
    parent_item->appendRow(items);

    if (!heading.children.isEmpty()) {
        for (int i = 0; i < heading.children.count(); ++i) {
            InsertHeadingIntoModel(heading.children[ i ], item_heading);
        }
    }
}


// Removes from the tree items that represent headings
// that are not to be included in the TOC; the children
// of those items rise to their parent's hierarchy level
// OR (more likely) are attached as children to the first
// previous heading that is lower in level.
void HeadingSelector::RemoveExcludedItems(QStandardItem *item)
{
    Q_ASSERT(item);

    // Recursively call itself on the item's children
    if (item->hasChildren()) {
        int row_index = 0;

        while (row_index < item->rowCount()) {
            QStandardItem *oldchild = item->child(row_index);
            RemoveExcludedItems(item->child(row_index));

            // We only increment the row_index if the
            // RemoveExcludedItems operation didn't end up
            // removing the child at that index.. if it did,
            // the next child is now at that index.
            if (oldchild == item->child(row_index)) {
                row_index++;
            }
        }
    }

    // The root item is always present
    if (item == m_TableOfContents.invisibleRootItem()) {
        return;
    }

    QStandardItem *item_parent = GetActualItemParent(item);
    // We query the "include in TOC" checkbox
    Qt::CheckState check_state = item_parent->child(item->row(), 2)->checkState();

    // We remove the current item if it shouldn't
    // be included in the TOC
    if (check_state == Qt::Unchecked) {
        if (item->hasChildren()) {
            while (item->rowCount() > 0) {
                QList< QStandardItem * > child_row = item->takeRow(0);

                if (!AddRowToVisiblePredecessorSucceeded(child_row, item)) {
                    item_parent->insertRow(item->row(), child_row);
                }
            }
        }

        // Item removes itself
        item_parent->removeRow(item->row());
    }
}


bool HeadingSelector::AddRowToVisiblePredecessorSucceeded(const QList< QStandardItem * > &child_row,
        QStandardItem *row_parent)
{
    Q_ASSERT(row_parent);
    QStandardItem *row_grandparent = GetActualItemParent(row_parent);

    if (row_grandparent == NULL) {
        return false;
    }

    return AddRowToCorrectItem(row_grandparent, child_row, row_parent->row());
}


// Basically we're looking for the first heading of a lower level that comes
// before the child_row heading whose parent heading is disappearing. The new parent
// needs to also be marked as "include_in_toc".
bool HeadingSelector::AddRowToCorrectItem(QStandardItem *item,
        const QList< QStandardItem * > &child_row,
        int child_index_limit)
{
    int child_start_index = child_index_limit != -1 ? child_index_limit - 1 : item->rowCount() - 1;

    for (int i = child_start_index; i > -1; --i) {
        bool row_placed = AddRowToCorrectItem(item->child(i), child_row);

        if (row_placed) {
            return true;
        }
    }

    Headings::Heading *heading = GetItemHeading(item);

    if (heading == NULL) {
        return false;
    }

    Headings::Heading *child_heading = GetItemHeading(child_row[ 0 ]);

    if (heading->include_in_toc &&
        heading->level < child_heading->level) {
        item->insertRow(child_start_index + 1, child_row);
        return true;
    }

    return false;
}


//    Unfortunately, while the invisible root of a QStandardItemModel
// can have children, those children do not have a parent set.
// Of course, their parent is the invisible root, but their
// parent() function returns 0. So now you have *two* tree levels
// for which parent() returns 0. This is clearly inconsistent
// and makes the whole idea of using the same recursive
// functions for tree traversal rather difficult to implement.
// The only item for which parent() should return 0 should be
// the invisible root.
//    Admittedly, Qt has some design issues. The next few lines
// try to work around them by manually setting an item parent.
QStandardItem *HeadingSelector::GetActualItemParent(const QStandardItem *item)
{
    Q_ASSERT(item);

    if (item == m_TableOfContents.invisibleRootItem()) {
        return NULL;
    }

    if (item->parent() == 0) {
        return m_TableOfContents.invisibleRootItem();
    }

    return item->parent();
}


// CAN RETURN NULL!
Headings::Heading *HeadingSelector::GetItemHeading(const QStandardItem *item)
{
    Q_ASSERT(item);

    if (item == m_TableOfContents.invisibleRootItem()) {
        return NULL;
    }

    Headings::Heading *heading = item->data().value< Headings::HeadingPointer >().heading;
    return heading;
}


// Get the maximum heading level for all headings
int HeadingSelector::GetMaxHeadingLevel(QList< Headings::Heading > flat_headings)
{
    int maxLevel = 0;
    foreach(Headings::Heading heading, flat_headings) {
        if (heading.level > maxLevel) {
            maxLevel = heading.level;
        }
    }
    return maxLevel;
}


// Add the selectable entries to the Select Heading combo box
void HeadingSelector::PopulateSelectHeadingCombo(int max_heading_level)
{
    QString entry = tr("Up to level");
    ui.cbTOCSetHeadingLevel->clear();
    ui.cbTOCSetHeadingLevel->addItem(tr("<Select headings to include in TOC>"));

    if (max_heading_level > 0) {
        ui.cbTOCSetHeadingLevel->addItem(tr("None"));

        for (int i = 1; i < max_heading_level; ++i) {
            ui.cbTOCSetHeadingLevel->addItem(entry + " " + QString::number(i));
        }

        ui.cbTOCSetHeadingLevel->addItem(tr("All"));
    }
}

void HeadingSelector::RefreshTOCModelDisplay()
{
    CreateTOCModel();

    if (ui.cbTOCItemsOnly->checkState() == Qt::Checked) {
        RemoveExcludedItems(m_TableOfContents.invisibleRootItem());
    }

    UpdateTreeViewDisplay();
}

// Set all headings to be in or not in the TOC
void HeadingSelector::SetAllHeadingInclusion(int upToLevel)
{
    // Recursively sets all headings
    for (int i = 0; i < m_Headings.count(); ++i) {
        SetOneHeadingInclusion(m_Headings[ i ], upToLevel);
    }

    RefreshTOCModelDisplay();
}


// Set one heading to be included/excluded from TOC
void HeadingSelector::SetOneHeadingInclusion(Headings::Heading &heading, int upToLevel)
{
    // Include if level is within range or All levels selected
    if (heading.level <= upToLevel || upToLevel < 0) {
        heading.include_in_toc = true;
    } else {
        heading.include_in_toc = false;
    }

    if (!heading.children.isEmpty()) {
        for (int i = 0; i < heading.children.count(); ++i) {
            SetOneHeadingInclusion(heading.children[ i ], upToLevel);
        }
    }
}


// Invoked when Select Heading is selected
void HeadingSelector::SelectHeadingLevelInclusion(const QString &heading_level)
{
    QChar last_char = heading_level[ heading_level.count() - 1 ];

    // For heading type == "Up to level #"
    if (last_char.isDigit()) {
        SetAllHeadingInclusion(last_char.digitValue());
    } else if (heading_level == tr("All")) {
        SetAllHeadingInclusion(-1);
    } else if (heading_level == tr("None")) {
        SetAllHeadingInclusion(0);
    }

    // else is "<Select heading level>" which does nothing
    // Reset selection to description
    ui.cbTOCSetHeadingLevel->setCurrentIndex(0);
}


// Reads all the stored dialog settings like
// window position, geometry etc.
void HeadingSelector::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}


// Writes all the stored dialog settings like
// window position, geometry etc.
void HeadingSelector::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}


void HeadingSelector::LockHTMLResources()
{
    foreach(HTMLResource * resource, m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >(true)) {
        resource->GetLock().lockForWrite();
    }
}


void HeadingSelector::UnlockHTMLResources()
{
    foreach(HTMLResource * resource, m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >(true)) {
        resource->GetLock().unlock();
    }
}


void HeadingSelector::Rename()
{
    if (!ui.tvTOCDisplay->selectionModel()->hasSelection()) {
        return;
    }

    if (ui.tvTOCDisplay->selectionModel()->selectedRows(0).count() > 1) {
        return;
    }

    ui.tvTOCDisplay->edit(ui.tvTOCDisplay->currentIndex());
}


void HeadingSelector::CreateContextMenuActions()
{
    m_Rename = new QAction(tr("Rename"),     this);
    m_Rename->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_R));
    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_Rename);
}

void HeadingSelector::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);
    m_ContextMenu->exec(ui.tvTOCDisplay->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();
}

void HeadingSelector::SetupContextMenu(const QPoint &point)
{
    m_ContextMenu->addAction(m_Rename);
}

bool HeadingSelector::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.tvTOCDisplay) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();

            if (key == Qt::Key_Left) {
                DecreaseHeadingLevel();
                return true;
            } else if (key == Qt::Key_Right) {
                IncreaseHeadingLevel();
                return true;
            }
        }
    }

    // pass the event on to the parent class
    return QDialog::eventFilter(obj, event);
}

void HeadingSelector::ConnectSignalsToSlots()
{
    connect(&m_TableOfContents, SIGNAL(itemChanged(QStandardItem *)),
            this,               SLOT(ModelItemFilter(QStandardItem *))
           );
    connect(ui.cbTOCItemsOnly,  SIGNAL(stateChanged(int)),
            this,               SLOT(ChangeDisplayType(int))
           );
    connect(this,               SIGNAL(accepted()),
            this,               SLOT(UpdateHeadingElements())
           );
    connect(ui.cbTOCSetHeadingLevel,
            SIGNAL(activated(const QString &)),
            this,               SLOT(SelectHeadingLevelInclusion(const QString &))
           );
    connect(ui.left,             SIGNAL(clicked()), this, SLOT(DecreaseHeadingLevel()));
    connect(ui.right,            SIGNAL(clicked()), this, SLOT(IncreaseHeadingLevel()));
    connect(ui.rename,           SIGNAL(clicked()), this, SLOT(Rename()));
    connect(ui.tvTOCDisplay,     SIGNAL(customContextMenuRequested(const QPoint &)),
            this,                SLOT(OpenContextMenu(const QPoint &)));
    connect(m_Rename,            SIGNAL(triggered()), this, SLOT(Rename()));
}


