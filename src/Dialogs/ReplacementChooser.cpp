/************************************************************************
**
**  Copyright (C) 2022 Kevin B. Hendricks, Stratford, Ontario
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

#include <QKeySequence>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeView>
#include <QModelIndex>
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/TextResource.h"
#include "PCRE2/PCRECache.h"
#include "PCRE2/SPCRE.h"
#include "Dialogs/StyledTextDelegate.h"
#include "MainUI/FindReplace.h"
#include "Dialogs/ReplacementChooser.h"


static const QString SETTINGS_GROUP = "replacement_chooser";

// These need to be changed if columns added or deleted
static const int BEFORE_COL = 2;
static const int AFTER_COL = 3;

ReplacementChooser::ReplacementChooser(QWidget* parent)
    :
    QDialog(parent),
    m_ItemModel(new QStandardItemModel),
    m_TextDelegate(new StyledTextDelegate()),
    m_ContextMenu(new QMenu(this)),
    m_replacement_count(0),
    m_current_count(0)
{
    m_FindReplace = qobject_cast<FindReplace*>(parent);
    ui.setupUi(this);
    ui.amtcb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.amtcb->addItem("10",10);
    ui.amtcb->addItem("20",20);
    ui.amtcb->addItem("30",30);
    ui.amtcb->addItem("40",40);
    ui.amtcb->addItem("50",50);
    ui.amtcb->setEditable(false);
    ReadSettings();
    CreateContextMenuActions();
    connectSignalsSlots();
    // sorting must NOT be enabled as offset order is important
    // changes in a file must be made from bottom to top
    ui.chooserTree->setSortingEnabled(false);
    ui.chooserTree->setTextElideMode(Qt::ElideLeft);
}

ReplacementChooser::~ReplacementChooser()
{
    m_ItemModel->clear();
    delete m_ItemModel;
}

void ReplacementChooser::closeEvent(QCloseEvent *e)
{
    WriteSettings();
    QDialog::closeEvent(e);
}

void ReplacementChooser::reject()
{
    WriteSettings();
    QDialog::reject();
}

void ReplacementChooser::CreateTable()
{
    m_ItemModel->clear();
    m_Resources.clear();
    QStringList header;
    header.append(tr("Book Path"));
    header.append(tr("Offset"));
    header.append(tr("Before"));
    header.append(tr("After"));
    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.chooserTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.chooserTree->setModel(m_ItemModel);
    ui.chooserTree->setContextMenuPolicy(Qt::CustomContextMenu);

    QList<Resource*> resources = m_FindReplace->GetAllResourcesToSearch();
    QString search_regex = m_FindReplace->GetSearchRegex();
    QString replace_text = m_FindReplace->GetReplace();
    
    m_current_count = 0;
    foreach(Resource* resource, resources ) {
        qApp->processEvents();
        QString bookpath = resource->GetRelativePath();
        QString text;
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        TextResource *text_resource = qobject_cast<TextResource *>(resource);
        if (html_resource) {
            QReadLocker locker(&html_resource->GetLock());
            text = html_resource->GetText();
        } else if (text_resource) {
            QReadLocker locker(&text_resource->GetLock());
            text = text_resource->GetText();
        } 
        if (!text.isEmpty()) {

            // search the text using the search_regex and get all matches
            SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
            QList<SPCRE::MatchInfo> match_info = spcre->getEveryMatchInfo(text);

            // loop through matches to build up before and after snippets for table
            // and build table in reverse offset order
            for (int i = match_info.count() - 1; i >=  0; i--) {
                QString match_segment = Utility::Substring(match_info.at(i).offset.first,
                                                           match_info.at(i).offset.second, 
                                                           text);
                QString new_text;
                bool can_replace = spcre->replaceText(match_segment, match_info.at(i).capture_groups_offsets, 
                                                      replace_text, new_text);

                // set pre and post context strings
                QString prior_context  = GetPriorContext(match_info.at(i).offset.first, text, m_context_amt);
                QString post_context = GetPostContext(match_info.at(i).offset.second, text, m_context_amt);
                
                // finally create before and after snippets
                QString orig_snip = prior_context + match_segment + post_context;
                QString new_snip;
                if (can_replace) {
                    new_snip = prior_context + new_text + post_context;
                } else {
                    new_snip = orig_snip;
                    new_text = match_segment;
                }   
                int start = match_info.at(i).offset.first;

                // finally add a row to the table
                QList<QStandardItem *> rowItems;

                // Book Path
                QStandardItem *item;
                item = new QStandardItem();
                item->setText(bookpath);
                if (!m_Resources.contains(bookpath)) m_Resources[bookpath] = resource;
                rowItems << item;

                //  Offset
                if (start > -1) m_current_count++;
                NumericItem *count_item = new NumericItem();
                count_item->setText(QString::number(start));
                count_item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                rowItems << count_item;

                // Before
                item = new QStandardItem();
                item->setText(orig_snip);
                item->setData(prior_context.length(), Qt::UserRole+1);
                item->setData(prior_context.length() + match_segment.length(), Qt::UserRole+2);
                item->setData(match_segment, Qt::UserRole+3);
                rowItems << item;

                // After
                item = new QStandardItem();
                item ->setText(new_snip);
                item->setData(prior_context.length(), Qt::UserRole+1);
                item->setData(prior_context.length() + new_text.length(), Qt::UserRole+2);
                item->setData(new_text, Qt::UserRole+3);
                rowItems << item;

                // Add item to table
                m_ItemModel->appendRow(rowItems);
                for (int i = 0; i < rowItems.count(); i++) {
                    rowItems[i]->setEditable(false);
                }
            }
        }
    }

    // display the current count above the table (with a buffer to the right)
    ui.cntamt->setText(QString::number(m_current_count) + "   ");
    
    // set styled text item delegate for columns 2 (before) and 3 (after)
    ui.chooserTree->setItemDelegateForColumn(BEFORE_COL, m_TextDelegate);
    ui.chooserTree->setItemDelegateForColumn(AFTER_COL, m_TextDelegate);
    
    for (int i = 0; i < ui.chooserTree->header()->count(); i++) {
        ui.chooserTree->resizeColumnToContents(i);
    }
    ui.chooserTree->setSelectionBehavior(QAbstractItemView::SelectRows);
}


QString ReplacementChooser::GetPriorContext(int match_start, const QString& text, int amt)
{
    int context_start = match_start - amt;
    if  (context_start < 0) context_start = 0;
    // find first whitespace to break at
    while (context_start < match_start && !text.at(context_start).isSpace()) {
        context_start++;
    }
    QString prior_context = Utility::Substring(context_start, match_start, text);
    prior_context.replace('\n',' ');
    return prior_context;
}


QString ReplacementChooser::GetPostContext(int match_end, const QString& text, int amt)
{
    int context_end = match_end + amt;
    int end_pos = text.length();
    if  (context_end > end_pos) context_end = end_pos;
    // find last whitespace to break at
    while (context_end > match_end && !text.at(context_end-1).isSpace()) {
        context_end--;
    }
    QString post_context = Utility::Substring(match_end, context_end, text);
    post_context.replace('\n',' ');
    return post_context;
}


void ReplacementChooser::ApplyReplacements()
{
    // order of replacements is crucial
    // replacements must be made in reverse offset order (bottom to top) of file
    int rows = m_ItemModel->rowCount();
    for (int i=0; i < rows; i++) {
        QString bookpath = m_ItemModel->item(i, 0)->text();
        QString match_segment = m_ItemModel->item(i,2)->data(Qt::UserRole+3).toString();
        int startpos = m_ItemModel->item(i,1)->text().toInt();
        int n = match_segment.length();
        QString new_text = m_ItemModel->item(i,3)->data(Qt::UserRole+3).toString();
        Resource * resource = m_Resources[bookpath];
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        TextResource *text_resource = qobject_cast<TextResource *>(resource);
        if (html_resource) {
            QWriteLocker locker(&html_resource->GetLock());
            QString text = html_resource->GetText();
            QString updated_text = text.replace(startpos, n, new_text);
            html_resource->SetText(updated_text);
            m_replacement_count++;
        } else if (text_resource) {
            QWriteLocker locker(&text_resource->GetLock());
            QString text = text_resource->GetText();
            QString updated_text = text.replace(startpos, n, new_text);
            text_resource->SetText(updated_text);
            m_replacement_count++;
        }
    }
    close();
}

void ReplacementChooser::DeleteSelectedRows()
{
    if (!ui.chooserTree->selectionModel()->hasSelection()) return;
    // This QTreeView is limited to ContiguousSelection mode
    // so should be able to delete in blocks
#if 0
    // Delete one at a time as selection delets may change indexes
    int row = -1;
    QModelIndex parent_index;

    while (ui.chooserTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.chooserTree->selectionModel()->selectedRows(0).first();
        if (index.isValid()) {
            row = index.row();
            parent_index = index.parent();
            m_ItemModel->removeRows(row, 1, parent_index);
        }
    }
#else
    QModelIndex index = ui.chooserTree->selectionModel()->selectedRows(0).first();
    int count = ui.chooserTree->selectionModel()->selectedRows(0).count();
    int row = index.row();
    QModelIndex parent_index = index.parent();
    m_ItemModel->removeRows(row, count, parent_index);
    m_current_count = m_current_count - count;

    // display the current count above the table (with a buffer to the right)
    ui.cntamt->setText(QString::number(m_current_count) + "   ");

#endif

}

#if 0
void ReplacementChooser::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;
    // Hide rows that don't contain the filter text
    int first_visible_row = -1;

    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText)) {
            ui.chooserTree->setRowHidden(row, parent_index, false);

            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        } else {
            ui.chooserTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.chooserTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.chooserTree->setCurrentIndex(QModelIndex());
    }
}
#endif

void ReplacementChooser::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    m_context_amt = settings.value("context",20).toInt();
    SetContextCB(m_context_amt);
    settings.endGroup();
}

void ReplacementChooser::SetContextCB(int val)
{
    int index = 0;
    if (val >= 20) index = 1;
    if (val >= 30) index = 2;
    if (val >= 40) index = 3;
    if (val >= 50) index = 4;
    ui.amtcb->setCurrentIndex(index);
}

void ReplacementChooser::ChangeContext()
{
    int val = ui.amtcb->currentData().toInt();
    if (val != m_context_amt) {
        m_context_amt = val;
        CreateTable();
    }
}

void ReplacementChooser::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("context", m_context_amt);
    settings.endGroup();
}

void ReplacementChooser::CreateContextMenuActions()
{
    m_Delete    =   new QAction(tr("Delete Selected Rows"),  this);
    QList<QKeySequence> shortcuts;
    shortcuts << QKeySequence::Delete << QKeySequence::Cut << QKeySequence(Qt::Key_Backspace);
    m_Delete->setShortcuts(shortcuts);
    addAction(m_Delete);
}

void ReplacementChooser::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);
    m_ContextMenu->exec(ui.chooserTree->viewport()->mapToGlobal(point));
    if (!m_ContextMenu.isNull()) {
        m_ContextMenu->clear();
        m_Delete->setEnabled(true);
     }
}

void ReplacementChooser::SetupContextMenu(const QPoint &point)
{
    m_ContextMenu->addAction(m_Delete);
    m_Delete->setEnabled(ui.chooserTree->selectionModel()->hasSelection());
}


void ReplacementChooser::connectSignalsSlots()
{
    // connect(ui.leFilter,  SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));
    connect(m_Delete, SIGNAL(triggered()), this, SLOT(DeleteSelectedRows()));
    connect(ui.Apply, SIGNAL(clicked()), this, SLOT(ApplyReplacements()));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui.amtcb, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeContext()));
    connect(ui.chooserTree, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(OpenContextMenu(const QPoint &)));
}
