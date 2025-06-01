/************************************************************************
**
**  Copyright (C) 2022-2025 Kevin B. Hendricks, Stratford, Ontario
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
#include "EmbedPython/PyObjectPtr.h"
#include "EmbedPython/PythonRoutines.h"

#include <QKeySequence>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
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
static const int BEFORE_COL = 3;
static const int AFTER_COL = 4;

ReplacementChooser::ReplacementChooser(QWidget* parent)
    :
    QDialog(parent),
    m_ItemModel(new QStandardItemModel),
    m_TextDelegate(new StyledTextDelegate()),
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
    ui.ToggleSelectAll->setCheckState(Qt::Checked);
    ReadSettings();
    connectSignalsSlots();
    // sorting must NOT be enabled as offset order is important
    // changes in a file must be made from bottom to top
    ui.chooserTable->setSortingEnabled(false);
    ui.chooserTable->setTextElideMode(Qt::ElideLeft);
    ui.chooserTable->setTabKeyNavigation(false);
    ui.chooserTable->setFocusPolicy(Qt::TabFocus);
    ui.chooserTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.amtcb->setFocusPolicy(Qt::TabFocus);
    ui.ToggleSelectAll->setFocusPolicy(Qt::TabFocus);
    CreateTable();
    ui.chooserTable->setFocus(Qt::OtherFocusReason);
}

ReplacementChooser::~ReplacementChooser()
{
    m_ItemModel->clear();
    delete m_ItemModel;
}

void ReplacementChooser::SelectUnselectAll(bool value)
{
    Qt::CheckState checkboxValue = (value ? Qt::Checked : Qt::Unchecked);
    m_current_count = 0;
    for (int row = 0; row < m_ItemModel->rowCount(); row++) {
        QStandardItem *checkbox = m_ItemModel->itemFromIndex(m_ItemModel->index(row, 0));
        checkbox->setCheckState(checkboxValue);
        if (value) m_current_count = m_current_count + 1;
    }
    // display the current count above the table (with a buffer to the right)
    ui.cntamt->setText(QString::number(m_current_count) + "   ");

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
    header.append(tr("Replace"));
    header.append(tr("Book Path"));
    header.append(tr("Offset"));
    header.append(tr("Before"));
    header.append(tr("After"));
    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.chooserTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.chooserTable->setModel(m_ItemModel);
    ui.chooserTable->setContextMenuPolicy(Qt::NoContextMenu);

    QList<Resource*> resources = m_FindReplace->GetAllResourcesToSearch();
    QString search_regex = m_FindReplace->GetSearchRegex();
    QString replace_text = m_FindReplace->GetReplace();

    // handle possible python function replacements
    QString functionname;
    QString rname = replace_text.trimmed();
    if (rname.startsWith("\\F<") && rname.endsWith(">")) {
        rname = rname.mid(3,-1);
        rname.chop(1);
        functionname = rname;
    }
    PyObjectPtr fsp = nullptr;
    if (!functionname.isEmpty()) {
        PythonRoutines pr;
        fsp = pr.SetupInitialFunctionSearchEnvInPython(functionname);
    }
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    
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
            QList<SPCRE::MatchInfo> match_info = spcre->getEveryMatchInfo(text);

            // loop through matches to build up before and after snippets for table
            // in forward order but apply them in reverse order
            for (int i = 0; i <  match_info.count(); i++) {
                QString match_segment = Utility::Substring(match_info.at(i).offset.first,
                                                           match_info.at(i).offset.second, 
                                                           text);
                QString new_text;
                bool can_replace;
                if (functionname.isEmpty()) {
                    can_replace = spcre->replaceText(match_segment, match_info.at(i).capture_groups_offsets, 
                                                      replace_text, new_text);
                } else {
                    can_replace = spcre->functionReplaceText(bookpath, match_segment,
                                                             match_info.at(i).capture_groups_offsets,
                                                             fsp, new_text);
                }
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

                // Checkbox
                QStandardItem *checkbox_item = new QStandardItem();
                checkbox_item->setCheckable(true);
                checkbox_item->setCheckState(Qt::Checked);
                rowItems << checkbox_item;

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
    ui.chooserTable->setItemDelegateForColumn(BEFORE_COL, m_TextDelegate);
    ui.chooserTable->setItemDelegateForColumn(AFTER_COL, m_TextDelegate);
    
    for (int i = 0; i < ui.chooserTable->horizontalHeader()->count(); i++) {
        ui.chooserTable->resizeColumnToContents(i);
    }
    ui.chooserTable->resizeRowsToContents();
    ui.chooserTable->setSelectionBehavior(QAbstractItemView::SelectRows);
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
    for (int i=rows-1; i>=0; i--) {
        bool checked = m_ItemModel->item(i,0)->checkState() == Qt::Checked;
        if (checked) {
            QString bookpath = m_ItemModel->item(i, 1)->text();
            int startpos = m_ItemModel->item(i,2)->text().toInt();
            QString match_segment = m_ItemModel->item(i,3)->data(Qt::UserRole+3).toString();
            int n = match_segment.length();
            QString new_text = m_ItemModel->item(i,4)->data(Qt::UserRole+3).toString();
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
    }
    close();
}

void ReplacementChooser::ChangeSelectedRows()
{
    if (!ui.chooserTable->selectionModel()->hasSelection()) return;
    // This QTableView is limited to ContiguousSelection mode
    // so should be able to select or deselect in blocks
    QModelIndex index = ui.chooserTable->selectionModel()->selectedRows(0).first();
    int count = ui.chooserTable->selectionModel()->selectedRows(0).count();
    int row = index.row();
    int i = row;
    while (count > 0) {
        bool checked = m_ItemModel->item(i,0)->checkState() == Qt::Checked;
        if (checked) {
            m_ItemModel->item(i, 0)->setCheckState(Qt::Unchecked);
            m_current_count--;
        } else {
            m_ItemModel->item(i, 0)->setCheckState(Qt::Checked);
            m_current_count++;
        }
        i++;
        count--;
    }
    // display the current count above the table (with a buffer to the right)
    ui.cntamt->setText(QString::number(m_current_count) + "   ");
}

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
    connect(ui.amtcb, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeContext()));
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
        // store away the current seletions to restore them
        // after table is recreated with changed context
        QList<bool> selections;
        int rows = m_ItemModel->rowCount();
        for (int i=0; i < rows; i++) {
            bool checked = m_ItemModel->item(i,0)->checkState() == Qt::Checked;
            selections << checked;
        }
        CreateTable();
        // update with saved selections
        rows = m_ItemModel->rowCount();
        for (int i=0; i < rows; i++) {
            if (selections.at(i)) {
                m_ItemModel->item(i,0)->setCheckState(Qt::Checked);
            } else {
                m_ItemModel->item(i,0)->setCheckState(Qt::Unchecked);
            }
        }
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

void ReplacementChooser::doActivated(const QModelIndex& index)
{
    ChangeSelectedRows();
}

void ReplacementChooser::keyPressEvent(QKeyEvent* event)
{
    if (ui.chooserTable->selectionModel()->hasSelection()) {
        if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
            ChangeSelectedRows();
            return;
        }
    }
    QDialog::keyPressEvent(event);
}

void ReplacementChooser::connectSignalsSlots()
{
    connect(ui.Apply, SIGNAL(clicked()), this, SLOT(ApplyReplacements()));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui.ToggleSelectAll, SIGNAL(clicked(bool)), this, SLOT(SelectUnselectAll(bool)));
    connect(ui.chooserTable, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(doActivated(const QModelIndex &)));
}
