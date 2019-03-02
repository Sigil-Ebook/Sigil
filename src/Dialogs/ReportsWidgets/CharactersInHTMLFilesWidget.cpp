/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QWebView>
#include <QWebPage>
#include "Misc/SleepFunctions.h"

#include <QtCore/QFile>
#include <QtCore/QHashIterator>
#include <QtWidgets/QFileDialog>
#include <QtGui/QFont>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#include "sigil_constants.h"
#include "sigil_exception.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/ReportsWidgets/CharactersInHTMLFilesWidget.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/XMLEntities.h"
#include "ResourceObjects/HTMLResource.h"

static const QString SETTINGS_GROUP = "reports";
static const QString DEFAULT_REPORT_FILE = "CharactersInHTMLFilesReport.csv";

CharactersInHTMLFilesWidget::CharactersInHTMLFilesWidget()
    :
    m_ItemModel(new QStandardItemModel),
    m_LastDirSaved(QString()),
    m_LastFileSaved(QString()),
    m_PageLoaded(false)
{
    ui.setupUi(this);
    connectSignalsSlots();
}

CharactersInHTMLFilesWidget::~CharactersInHTMLFilesWidget()
{
    delete m_ItemModel;
}

void CharactersInHTMLFilesWidget::CreateReport(QSharedPointer<Book> book)
{
    m_Book = book;
    SetupTable();
    AddTableData();

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }

    ui.fileTree->sortByColumn(0, Qt::AscendingOrder);
}

void CharactersInHTMLFilesWidget::SetupTable()
{
    m_ItemModel->clear();
    QStringList header;
    header.append(tr("Character"));
    header.append(tr("Decimal"));
    header.append(tr("Hexadecimal"));
    header.append(tr("Entity Name"));
    header.append(tr("Entity Description"));
    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.fileTree->setModel(m_ItemModel);
    ui.fileTree->header()->setSortIndicatorShown(true);
    ui.fileTree->header()->setToolTip(
        tr("<p>This is a list of the characters used in all HTML files.<p>")
    );
}

void CharactersInHTMLFilesWidget::AddTableData()
{
    const QList<HTMLResource *> html_resources = m_Book->GetHTMLResources();
    QList <QChar> characters = GetDisplayedCharacters(html_resources);
    QString all_characters;
    foreach (QChar c, characters) {
        all_characters.append(c);
    }
    ui.Characters->setText(all_characters);

    foreach (QChar c, characters) {
        // Write the table entries
        QList<QStandardItem *> rowItems;
        // Character
        QStandardItem *item = new QStandardItem();
        item->setText(QString(c));
        rowItems << item;
        // Decimal number
        item = new QStandardItem();
        ushort char_number = c.unicode();
        item->setText(QString::number(char_number));
        rowItems << item;
        // Hex number
        item = new QStandardItem();
        QString hexadecimal;
        hexadecimal.setNum(char_number,16);
        item->setText(hexadecimal.toUpper());
        rowItems << item;
        // Name
        item = new QStandardItem();
        item->setText(XMLEntities::instance()->GetEntityName(char_number));
        rowItems << item;
        // Description
        item = new QStandardItem();
        item->setText(XMLEntities::instance()->GetEntityDescription(char_number));
        rowItems << item;

        for (int i = 0; i < rowItems.count(); i++) {
            rowItems[i]->setEditable(false);
        }

        m_ItemModel->appendRow(rowItems);
    }
}

void CharactersInHTMLFilesWidget::PageLoaded()
{
    m_PageLoaded = true;
}

QList <QChar> CharactersInHTMLFilesWidget::GetDisplayedCharacters(QList<HTMLResource *> resources)
{
    QWebView *view = new QWebView();
    view->setGeometry(0,0,200,200);
    connect(view->page(), SIGNAL(loadFinished(bool)), this, SLOT(PageLoaded()));
    m_PageLoaded = false;

    QString all_characters;
    foreach (HTMLResource *resource, resources) {
        QString replaced_html = resource->GetText();
        replaced_html = replaced_html.replace("<html>", "<html xmlns=\"http://www.w3.org/1999/xhtml\">");
        // Skip displaying images, etc. to improve performance
        replaced_html = replaced_html.replace("<(img|video|audio)", "<ignore");
        replaced_html = replaced_html.replace("</(img|video|audio)", "</ignore");
        view->setContent(replaced_html.toUtf8(), "application/xhtml+xml", QUrl::fromLocalFile(resource->GetFullPath()));

        while (!m_PageLoaded) {
            qApp->processEvents();
            SleepFunctions::msleep(100);
        }
        m_PageLoaded = false;

        view->page()->triggerAction(QWebPage::SelectAll);
        QString text = view->page()->selectedText();
        all_characters.append(text);
    }

    QMap <QChar, QChar> character_map;
    foreach (const QChar c, all_characters) {
        if (c != '\n') {
            character_map.insert(c, c);
        }
    }
    QList <QChar> character_list;
    character_list = character_map.values();

    return character_list;
}


void CharactersInHTMLFilesWidget::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;
    // Hide rows that don't contain the filter text
    int first_visible_row = -1;

    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 1)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 2)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 3)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 4)->text().toLower().contains(lowercaseText)) {
            ui.fileTree->setRowHidden(row, parent_index, false);

            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        } else {
            ui.fileTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.fileTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.fileTree->setCurrentIndex(QModelIndex());
    }
}

void CharactersInHTMLFilesWidget::DoubleClick()
{
    QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
    QString text = m_ItemModel->itemFromIndex(index)->text();
    emit FindText(text);
}

void CharactersInHTMLFilesWidget::Save()
{
    QString report_info;
    QString row_text;

    // Get headings
    for (int col = 0; col < ui.fileTree->header()->count(); col++) {
        QStandardItem *item = m_ItemModel->horizontalHeaderItem(col);
        QString text = "";
        if (item) {
            text = item->text();
        }
        if (col == 0) {
            row_text.append(text);
        } else {
            row_text.append("," % text);
        }
    }

    report_info.append(row_text % "\n");

    // Get data from table
    for (int row = 0; row < m_ItemModel->rowCount(); row++) {
        row_text = "";

        for (int col = 0; col < ui.fileTree->header()->count(); col++) {
            QStandardItem *item = m_ItemModel->item(row, col);
            QString text = "";
            if (item) {
                text = item->text();
            }
            if (col == 0) {
                row_text.append(text);
            } else {
                row_text.append("," % text);
            }
        }

        report_info.append(row_text % "\n");
    }

    // Save the file
    ReadSettings();
    QString filter_string = "*.csv;;*.txt;;*.*";
    QString default_filter = "";
    QString save_path = m_LastDirSaved + "/" + m_LastFileSaved;
    QString destination = QFileDialog::getSaveFileName(this,
                          tr("Save Report As Comma Separated File"),
                          save_path,
                          filter_string,
                          &default_filter
                                                      );

    if (destination.isEmpty()) {
        return;
    }

    try {
        Utility::WriteUnicodeTextFile(report_info, destination);
    } catch (CannotOpenFile) {
        QMessageBox::warning(this, tr("Sigil"), tr("Cannot save report file."));
    }

    m_LastDirSaved = QFileInfo(destination).absolutePath();
    m_LastFileSaved = QFileInfo(destination).fileName();
    WriteSettings();
}

void CharactersInHTMLFilesWidget::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    m_LastDirSaved = settings.value("last_dir_saved").toString();
    m_LastFileSaved = settings.value("last_file_saved_characters_in_html").toString();

    if (m_LastFileSaved.isEmpty()) {
        m_LastFileSaved = "CharactersInHTMLFilesReport.csv";
    }

    settings.endGroup();
}

void CharactersInHTMLFilesWidget::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    settings.setValue("last_dir_saved", m_LastDirSaved);
    settings.setValue("last_file_saved_characters_html", m_LastFileSaved);
    settings.endGroup();
}


void CharactersInHTMLFilesWidget::connectSignalsSlots()
{
    connect(ui.Filter,    SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClick()));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SIGNAL(CloseDialog()));
    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}
