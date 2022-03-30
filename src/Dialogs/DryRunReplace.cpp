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

#include <QFont>
#include <QMessageBox>
#include <QPushButton>
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/TextResource.h"
#include "PCRE2/PCRECache.h"
#include "PCRE2/SPCRE.h"
#include "Dialogs/StyledTextDelegate.h"
#include "Dialogs/DryRunReplace.h"


static const QString SETTINGS_GROUP = "dryrun_report";


DryRunReplace::DryRunReplace(QWidget* parent)
    :
    QDialog(parent),
    m_ItemModel(new QStandardItemModel),
    m_TextDelegate(new StyledTextDelegate())
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
}

DryRunReplace::~DryRunReplace()
{
    WriteSettings();
    delete m_ItemModel;
}

void DryRunReplace::CreateReport(const QString& search_regex, const QString& replace_text, const QList<Resource*> resources )
{
    m_search_regex = search_regex;
    m_replace_text = replace_text;
    m_resources = resources;
    SetupTable();
}

void DryRunReplace::SetupTable(int sort_column, Qt::SortOrder sort_order)
{
    int context_amt = 30;
    m_ItemModel->clear();
    QStringList header;
    header.append(tr("Book Path"));
    header.append(tr("Offset"));
    header.append(tr("Before"));
    header.append(tr("After"));
    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.dryrunTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.dryrunTree->setModel(m_ItemModel);
    ui.dryrunTree->header()->setSortIndicatorShown(true);
    int count = 0;
    foreach(Resource* resource, m_resources ) {
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
            SPCRE *spcre = PCRECache::instance()->getObject(m_search_regex);
            QList<SPCRE::MatchInfo> match_info = spcre->getEveryMatchInfo(text);

            // loop through matches to build up before and after snippets for table
            for (int i = 0; i < match_info.count(); i++) {
                QString match_segment = Utility::Substring(match_info.at(i).offset.first,
                                                           match_info.at(i).offset.second, 
                                                           text);
                QString new_text;
                bool can_replace = spcre->replaceText(match_segment, match_info.at(i).capture_groups_offsets, 
                                                      m_replace_text, new_text);

                // set pre and post context strings
                QString prior_context  = GetPriorContext(match_info.at(i).offset.first, text, context_amt);
                QString post_context = GetPostContext(match_info.at(i).offset.second, text, context_amt);
                
                // finally create before and after snippets
                QString orig_snip = prior_context + match_segment + post_context;
                QString new_snip = orig_snip;
                if (can_replace) new_snip = prior_context + new_text + post_context;

                int start = match_info.at(i).offset.first;

                // finally add a row to the table
                QList<QStandardItem *> rowItems;

                // Book Path
                QStandardItem *item;
                item = new QStandardItem();
                item->setText(bookpath);
                rowItems << item;

                //  Offset
                if (start > -1) count += 1;
                NumericItem *count_item = new NumericItem();
                count_item->setText(QString::number(start));
                count_item->setTextAlignment(Qt::AlignRight);
                rowItems << count_item;

                // Before
                item = new QStandardItem();
                item->setText(orig_snip);
                item->setData(prior_context.length(), Qt::UserRole+1);
                item->setData(prior_context.length() + match_segment.length(), Qt::UserRole+2);
                rowItems << item;
                
                // After
                item = new QStandardItem();
                item ->setText(new_snip);
                item->setData(prior_context.length(), Qt::UserRole+1);
                item->setData(prior_context.length() + new_text.length(), Qt::UserRole+2);
                rowItems << item;
                
                // Add item to table
                m_ItemModel->appendRow(rowItems);
                for (int i = 0; i < rowItems.count(); i++) {
                    rowItems[i]->setEditable(false);
                }
            }
        }
    }
    // Sort before adding the totals row
    // Since sortIndicator calls this routine, must disconnect/reconnect while resorting
    disconnect(ui.dryrunTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    ui.dryrunTree->header()->setSortIndicator(sort_column, sort_order);
    connect(ui.dryrunTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    // Totals
    NumericItem *nitem;
    QList<QStandardItem *> rowItems;
    // BookPath
    nitem = new NumericItem();
    nitem->setText(QString::number(count));
    nitem->setTextAlignment(Qt::AlignRight);
    rowItems << nitem;
    // Offset
    nitem = new NumericItem();
    rowItems << nitem;
    // Before
    nitem = new NumericItem();
    rowItems << nitem;
    // After
    nitem = new NumericItem();
    rowItems << nitem;
    QFont font;
    font.setWeight(QFont::Bold);
    for (int i = 0; i < rowItems.count(); i++) {
        rowItems[i]->setEditable(false);
        rowItems[i]->setFont(font);
    }
    // set styled text item delegate for columns 2 (before) and 3 (after)
    ui.dryrunTree->setItemDelegateForColumn(2, m_TextDelegate);
    ui.dryrunTree->setItemDelegateForColumn(3, m_TextDelegate);
    
    m_ItemModel->appendRow(rowItems);

    for (int i = 0; i < ui.dryrunTree->header()->count(); i++) {
        ui.dryrunTree->resizeColumnToContents(i);
    }
}

QString DryRunReplace::GetPriorContext(int match_start, const QString& text, int amt)
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

QString DryRunReplace::GetPostContext(int match_end, const QString& text, int amt)
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


void DryRunReplace::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;
    // Hide rows that don't contain the filter text
    int first_visible_row = -1;

    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText)) {
            ui.dryrunTree->setRowHidden(row, parent_index, false);

            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        } else {
            ui.dryrunTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.dryrunTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.dryrunTree->setCurrentIndex(QModelIndex());
    }
}

void DryRunReplace::Sort(int logicalindex, Qt::SortOrder order)
{
    SetupTable(logicalindex, order);
}

void DryRunReplace::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}

void DryRunReplace::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}


void DryRunReplace::connectSignalsSlots()
{
    connect(ui.leFilter,  SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.dryrunTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(accept()));
}
