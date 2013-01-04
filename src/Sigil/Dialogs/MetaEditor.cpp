/************************************************************************
**
**  Copyright (C) 2011, 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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

#include <QtCore/QDate>
#include <QtGui/QShowEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "Dialogs/MetaEditor.h"
#include "Dialogs/AddMetadata.h"
#include "Dialogs/MetaEditorItemDelegate.h"
#include "Misc/Language.h"
#include "Misc/SettingsStore.h"
#include "ResourceObjects/OPFResource.h"

static const QString SETTINGS_GROUP      = "meta_editor";

MetaEditor::MetaEditor(QWidget *parent)
    :
    QDialog(parent),
    m_cbDelegate(new MetaEditorItemDelegate()),
    m_IsDataModified(false)
{
    ui.setupUi(this);
    ConnectSignals();
}

MetaEditor::~MetaEditor()
{
    if (m_cbDelegate) {
        delete m_cbDelegate;
        m_cbDelegate = 0;
    }
}

void MetaEditor::SetBook(QSharedPointer< Book > book)
{
    m_Book = book;
    m_OPF = &(m_Book->GetOPF());
    m_Metadata = m_OPF->GetDCMetadata();

    SetUpMetaTable();
    // Update position of window only if it was closed
    if (!isVisible()) {
        ReadSettings();
    }
    FillLanguageComboBox();
    ReadMetadataFromBook();
    SetLanguage();
    SetOriginalData();
    m_IsDataModified = false;
}

void MetaEditor::SetOriginalData()
{
    m_OriginalData.title = ui.leTitle->text();
    m_OriginalData.author = ui.leAuthor->text();
    m_OriginalData.file_as = ui.leAuthorFileAs->text();
    m_OriginalData.language = ui.cbLanguages->currentText();
}

void MetaEditor::SetLanguage()
{
    QString metadata_language;
    foreach(Metadata::MetaElement meta, m_Metadata) {
        if (meta.name == "language") {
            metadata_language = meta.value.toString();
            break;
        }
    }

    // Set language from preferences if none in book
    if (metadata_language.isEmpty()) {
        SettingsStore settings;
        int index = ui.cbLanguages->findText(Language::instance()->GetLanguageName(settings.defaultMetadataLang()), Qt::MatchExactly);

        if (index == -1) {
            index = ui.cbLanguages->findText(Language::instance()->GetLanguageName("en"), Qt::MatchExactly);

            if (index == -1) {
                index = 0;
            }
        }

        ui.cbLanguages->setCurrentIndex(index);
    }
}

void MetaEditor::AddEmptyMetadataToTable(const QStringList &metanames)
{
    foreach(QString metaname, metanames) {
        Metadata::MetaElement book_meta;
        book_meta.name = metaname;

        if (Metadata::Instance().IsRelator(book_meta.name)) {
            book_meta.role_type = "contributor";
        }

        // Map dates and identifiers from user-friendly version to standard entries

        // If we are inserting a date, that needs special treatment;
        // We need to insert it as a QDate object so the table interface
        // can automatically impose input restrictions
        if (metaname == "creation" || metaname == "modification" || metaname == "publication"  || metaname == "customdate") {
            book_meta.name = "date";
            book_meta.value = QDate::currentDate();
            book_meta.file_as = metaname;
        } else if (metaname == "DOI" || metaname == "ISBN" || metaname == "ISSN"  || metaname == "customidentifier") {
            book_meta.name = "identifier";
            book_meta.value = QString();
            book_meta.file_as = metaname;
        }
        // String-based metadata gets created normally
        else {
            book_meta.value = QString();
        }

        AddMetadataToTable(book_meta);
    }
}


void MetaEditor::AddMetadataToTable(Metadata::MetaElement book_meta, int row)
{
    if (row < 0) {
        row = m_MetaModel.rowCount();
    }

    m_MetaModel.insertRow(row);
    // Set the display name
    // All translations done in Metadata for consistency
    QString fullname;

    if (book_meta.name == "date") {
        fullname = Metadata::Instance().GetText("date");
    } else if (book_meta.name == "identifier") {
        fullname = Metadata::Instance().GetText("identifier");
    } else {
        fullname = Metadata::Instance().GetName(book_meta.name);
    }

    // Add name, making sure its not editable
    m_MetaModel.setData(m_MetaModel.index(row, 0), fullname);
    m_MetaModel.item(row, 0)->setEditable(false);
    // Add value
    m_MetaModel.setData(m_MetaModel.index(row, 1), book_meta.value);

    // Add file_as and role_type and set editable based on type of entry
    if (Metadata::Instance().IsRelator(book_meta.name)) {
        // File As
        m_MetaModel.setData(m_MetaModel.index(row, 2), book_meta.file_as);
        // Role Type - always ensure a default value
        QString role = book_meta.role_type;

        if (role.isEmpty()) {
            role = "contributor";
        }

        m_MetaModel.setData(m_MetaModel.index(row, 3), Metadata::Instance().GetText(role));
    } else if (book_meta.name == "date" || book_meta.name == "identifier") {
        // File As or Event description
        m_MetaModel.setData(m_MetaModel.index(row, 2), book_meta.file_as);
        // Role Type not used
        m_MetaModel.setData(m_MetaModel.index(row, 3), "");
        m_MetaModel.item(row, 3)->setEditable(false);
    } else {
        // File As not used
        m_MetaModel.setData(m_MetaModel.index(row, 3), "");
        m_MetaModel.item(m_MetaModel.rowCount() - 1, 3)->setEditable(false);
        // Role Type not used
        m_MetaModel.setData(m_MetaModel.index(row, 2), "");
        m_MetaModel.item(row, 2)->setEditable(false);
    }
}


void MetaEditor::AddBasic()
{
    AddMetadata addmeta(Metadata::Instance().GetBasicMetaMap(), this);

    if (addmeta.exec() == QDialog::Accepted) {
        AddEmptyMetadataToTable(addmeta.GetSelectedEntries());
    }
}


void MetaEditor::AddRole()
{
    AddMetadata addmeta(Metadata::Instance().GetRelatorMap(), this);

    if (addmeta.exec() == QDialog::Accepted) {
        AddEmptyMetadataToTable(addmeta.GetSelectedEntries());
    }
}

void MetaEditor::Copy()
{
    if (ui.tvMetaTable->selectionModel()->selectedIndexes().isEmpty()) {
        return;
    }

    Metadata::MetaElement book_meta;
    // Copy only the first selected row
    int row = ui.tvMetaTable->selectionModel()->selectedIndexes().first().row();
    QString code = m_MetaModel.data(m_MetaModel.index(row, 0)).toString();

    if (code == Metadata::Instance().GetText("date")) {
        code = "date";
    } else if (code == Metadata::Instance().GetText("identifier")) {
        code = "identifier";
    } else {
        code = Metadata::Instance().GetCode(code);
    }

    book_meta.name = code;
    book_meta.value = m_MetaModel.data(m_MetaModel.index(row, 1));
    book_meta.file_as = m_MetaModel.data(m_MetaModel.index(row, 2)).toString();
    book_meta.role_type = m_MetaModel.data(m_MetaModel.index(row, 3)).toString();
    AddMetadataToTable(book_meta, row + 1);
    ui.tvMetaTable->selectRow(row + 1);
}

void MetaEditor::Remove()
{
    while (ui.tvMetaTable->selectionModel()->hasSelection()) {
        m_MetaModel.removeRow(ui.tvMetaTable->selectionModel()->selection().indexes().at(0).row());
    }
}

void MetaEditor::AddMetaElements(QString name, QList<QVariant> values, QString role_type, QString file_as)
{
    foreach(QVariant value, values) {
        AddMetaElement(name, value, role_type, file_as);
    }
}

void MetaEditor::AddMetaElement(QString name, QVariant value, QString role_type, QString file_as)
{
    Metadata::MetaElement book_meta;
    book_meta.name = name;
    book_meta.value = value;

    if (role_type == Metadata::Instance().GetText("creator")) {
        book_meta.role_type = "creator";
    } else if (role_type == Metadata::Instance().GetText("contributor")) {
        book_meta.role_type = "contributor";
    }

    book_meta.file_as = file_as;
    m_Metadata.append(book_meta);
}

void MetaEditor::SetDataModifiedIfNeeded()
{
    if (m_OriginalData.title != ui.leTitle->text() ||
            m_OriginalData.author != ui.leAuthor->text() ||
            m_OriginalData.file_as != ui.leAuthorFileAs->text() ||
            m_OriginalData.language != ui.cbLanguages->currentText()) {
        m_IsDataModified = true;
    }
}

bool MetaEditor::SaveData()
{
    // Clear the book metadata so we don't duplicate something...
    // Nothing should be lost as everything was loaded into the dialog
    m_Metadata.clear();
    // Save the required fields
    // Author is stored using relator
    AddMetaElements("title", InputsInField(ui.leTitle->text()));
    QString author_name = ui.leAuthor->text();
    QString author_file_as = ui.leAuthorFileAs->text();

    if (author_name.isEmpty()) {
        author_name = author_file_as;
    }

    AddMetaElements("aut", InputsInField(author_name), Metadata::Instance().GetText("creator"), author_file_as);
    AddMetaElements("language", InputsInField(ui.cbLanguages->currentText()));

    // Save the table
    for (int row = 0; row < m_MetaModel.rowCount(); row++) {
        QString name   = m_MetaModel.data(m_MetaModel.index(row, 0)).toString();
        QVariant value = m_MetaModel.data(m_MetaModel.index(row, 1));
        QString file_as = m_MetaModel.data(m_MetaModel.index(row, 2)).toString();
        QString role_type = m_MetaModel.data(m_MetaModel.index(row, 3)).toString();
        // Mapping of translations to code done in Metadata for consistency
        QString code;

        // Handle date differently due to event name stored in file_as
        if (name == Metadata::Instance().GetText("date")) {
            code = "date";
        }
        // Handle identifier differently due to scheme name stored in file_as
        else if (name == Metadata::Instance().GetText("identifier")) {
            code = "identifier";
        } else {
            code = Metadata::Instance().GetCode(name);
        }

        // For string-based metadata, create multiple entries
        // if the typed in value contains semicolons
        if (value.type() == QVariant::String && OkToSplitInput(code)) {
            AddMetaElements(code, InputsInField(value.toString()), role_type, file_as);
        } else {
            AddMetaElement(code, value, role_type, file_as);
        }
    }

    m_OPF->SetDCMetadata(m_Metadata);
    m_Book->SetModified(true);
    m_IsDataModified = false;
    SetOriginalData();

    emit ShowStatusMessageRequest(tr("Metadata saved."));
    return true;
}


void MetaEditor::ReadMetadataFromBook()
{
    bool have_title = false;
    bool have_author = false;
    bool have_language = false;
    foreach(Metadata::MetaElement book_meta, m_Metadata) {
        // Load data into one of the main fields or into the table
        // Only load the first of each type
        if (!have_title && book_meta.name == "title") {
            ui.leTitle->setText(book_meta.value.toString());
            have_title = true;
        }
        // Convert basic and relator authors to main Author field - but only first creator Author
        else if (!have_author && (book_meta.name == "author" || (book_meta.name == "aut" && book_meta.role_type == "creator"))) {
            QString author_name = book_meta.value.toString();
            QString author_file_as = book_meta.file_as;

            if (author_name.isEmpty()) {
                author_name = author_file_as;
            }

            ui.leAuthor->setText(author_name);
            ui.leAuthorFileAs->setText(author_file_as);
            have_author = true;
        } else if (!have_language &&  book_meta.name == "language") {
            ui.cbLanguages->setCurrentIndex(ui.cbLanguages->findText(book_meta.value.toString()));
            have_language = true;
        } else {
            AddMetadataToTable(book_meta);
        }
    }
}



bool MetaEditor::OkToSplitInput(const QString &metaname)
{
    // The "description" and "rights" fields could have a semicolon
    // in the text and there's also little point in providing multiple
    // entries for these so we don't split them.
    if (metaname == "description" || metaname == "rights") {
        return false;
    }

    return true;
}


QList< QVariant > MetaEditor::InputsInField(const QString &field_value)
{
    QList< QVariant > inputs;
    foreach(QString input, field_value.split(";", QString::SkipEmptyParts)) {
        inputs.append(input.simplified());
    }
    return inputs;
}


void MetaEditor::FillLanguageComboBox()
{
    foreach(QString lang, Language::instance()->GetSortedPrimaryLanguageNames()) {
        ui.cbLanguages->addItem(lang);
    }
}

void MetaEditor::SetUpMetaTable()
{
    m_MetaModel.clear();
    ui.leTitle->setText("");
    ui.leAuthor->setText("");
    ui.leAuthorFileAs->setText("");

    QStringList header;
    header.append(tr("Name"));
    header.append(tr("Value"));
    header.append(tr("File As"));
    header.append(tr("Role Type"));
    m_MetaModel.setHorizontalHeaderLabels(header);
    ui.tvMetaTable->setModel(&m_MetaModel);
    // Make the header fill all the available space
    ui.tvMetaTable->horizontalHeader()->setStretchLastSection(true);
    ui.tvMetaTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Divide column widths roughly equally for default view
    int columnCount = m_MetaModel.columnCount();

    for (int i = 0; i < columnCount; i++) {
        ui.tvMetaTable->setColumnWidth(i, 155);
    }

    ui.tvMetaTable->setSortingEnabled(false);
    ui.tvMetaTable->setWordWrap(true);
    ui.tvMetaTable->setAlternatingRowColors(true);
    // The table's role_type column uses a combobox for editing its values
    ui.tvMetaTable->setItemDelegateForColumn(3, m_cbDelegate);
}

void MetaEditor::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // Column widths
    int size = settings.beginReadArray("column_data");

    for (int i = 0; i < size && i < ui.tvMetaTable->horizontalHeader()->count(); i++) {
        settings.setArrayIndex(i);
        int column_width = settings.value("width").toInt();

        if (column_width) {
            ui.tvMetaTable->setColumnWidth(i, column_width);
        }
    }

    settings.endArray();
    settings.endGroup();
}


void MetaEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    // Column widths
    settings.beginWriteArray("column_data");

    for (int i = 0; i < ui.tvMetaTable->horizontalHeader()->count(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("width", ui.tvMetaTable->columnWidth(i));
    }

    settings.endArray();
    settings.endGroup();
}

void MetaEditor::MoveUp()
{
    QModelIndexList selected_indexes = ui.tvMetaTable->selectionModel()->selectedIndexes();

    if (selected_indexes.isEmpty()) {
        return;
    }

    // Get just the row numbers to move
    QList<int> rows;
    foreach(QModelIndex index, selected_indexes) {
        int row = index.row();

        if (row == 0) {
            return;
        }

        if (!rows.contains(row)) {
            rows.append(row);
        }
    }
    qSort(rows);
    // Move the rows as a block starting from the top
    foreach(int row, rows) {
        QList< QStandardItem * > items =  m_MetaModel.invisibleRootItem()->takeRow(row - 1);
        m_MetaModel.invisibleRootItem()->insertRow(row, items);
    }
}

void MetaEditor::MoveDown()
{
    QModelIndexList selected_indexes = ui.tvMetaTable->selectionModel()->selectedIndexes();

    if (selected_indexes.isEmpty()) {
        return;
    }

    // Get just the row numbers to move
    QList<int> rows;
    foreach(QModelIndex index, selected_indexes) {
        int row = index.row();

        if (row == m_MetaModel.invisibleRootItem()->rowCount() - 1) {
            return;
        }

        if (!rows.contains(row)) {
            rows.append(row);
        }
    }
    qSort(rows);

    // Move the rows as a block starting from the bottom
    for (int i = rows.count() - 1; i >= 0; i--) {
        int row = rows.at(i);
        QList< QStandardItem * > items =  m_MetaModel.invisibleRootItem()->takeRow(row + 1);
        m_MetaModel.invisibleRootItem()->insertRow(row, items);
    }
}

void MetaEditor::Save()
{
    SaveData();
    accept();
}

void MetaEditor::reject()
{
    WriteSettings();

    if (MaybeSaveDialogSaysProceed(false)) {
        QDialog::reject();
    }
}

void MetaEditor::ForceClose()
{
    MaybeSaveDialogSaysProceed(true);
    WriteSettings();
    QDialog::reject();
}

bool MetaEditor::MaybeSaveDialogSaysProceed(bool is_forced)
{
    SetDataModifiedIfNeeded();

    if (m_IsDataModified) {
        QMessageBox::StandardButton button_pressed;
        QMessageBox::StandardButtons buttons = is_forced ? QMessageBox::Save | QMessageBox::Discard
                                               : QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel;
        button_pressed = QMessageBox::warning(this,
                                              tr("Sigil: Metadata Editor"),
                                              tr("The metadata may have been modified.\n"
                                                      "Do you want to save your changes?"),
                                              buttons
                                             );

        if (button_pressed == QMessageBox::Save) {
            return SaveData();
        } else if (button_pressed == QMessageBox::Cancel) {
            return false;
        }
    }

    return true;
}

void MetaEditor::ItemChangedHandler(QStandardItem *item)
{
    m_IsDataModified = true;
}

void MetaEditor::RowsRemovedHandler(const QModelIndex &parent, int start, int end)
{
    m_IsDataModified = true;
}

void MetaEditor::ConnectSignals()
{
    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(Save()));
    connect(ui.btAddBasic,    SIGNAL(clicked()), this, SLOT(AddBasic()));
    connect(ui.btAddRole,     SIGNAL(clicked()), this, SLOT(AddRole()));
    connect(ui.btCopy,        SIGNAL(clicked()), this, SLOT(Copy()));
    connect(ui.btRemove,      SIGNAL(clicked()), this, SLOT(Remove()));
    connect(ui.tbMoveUp,      SIGNAL(clicked()), this, SLOT(MoveUp()));
    connect(ui.tbMoveDown,    SIGNAL(clicked()), this, SLOT(MoveDown()));

    connect(&m_MetaModel, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(ItemChangedHandler(QStandardItem *)));
    connect(&m_MetaModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(RowsRemovedHandler(const QModelIndex &, int, int)));
}
