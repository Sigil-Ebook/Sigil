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

#pragma once
#ifndef METAEDITOR_H
#define METAEDITOR_H

#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include <BookManipulation/Metadata.h>

#include "ui_MetaEditor.h"

class OPFResource;
class MetaEditorItemDelegate;

/**
 * The editor used to create and modify the book's metadata.
 */
class MetaEditor : public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param opf The OPF whose metadata we want to edit.
     * @param parent The object's parent.
     */
    MetaEditor(OPFResource &opf, QWidget *parent = 0);
    ~MetaEditor();

protected:

    /**
     * Overridden to work around a Qt bug.
     */
    void showEvent(QShowEvent *event);

    void reject();

private slots:

    /**
     * Inserts a metadata field with the provided name
     * into the table. The value of the new field is empty by default.
     *
     * @param metaname The name of the new metadata field.
     */
    void AddEmptyMetadataToTable(const QStringList &metanames);

    /**
     * Add Basic button functionality.
     * Shows the window for selecting the new basic
     * metadata type which creates the new type upon return.
     */
    void AddBasic();

    /**
     * Add Role button functionality.
     * Shows the window for selecting the new advanced
     * metadata type which creates the new type upon return.
     */
    void AddRole();

    /**
     * Copy button functionality.
     * Copies the first selected entry and creates a new
     * entry with the same contents.
     */
    void Copy();

    /**
     * Remove button functionality.
     * Removes the currently selected rows
     * from the metadata table.
     */
    void Remove();

    /**
     * Move up button functionality.
     * Moves the first selected entry up one row
     */
    void MoveUp();

    /**
     * Move down button functionality.
     * Moves the first selected entry down one row
     */
    void MoveDown();

    /**
     * Refreshes the vertical header of the table view widget.
     */
    void RefreshVerticalHeader();

    /**
     * Adds a list of elements from the dialog to the saved metadata one at a time
     */
    void AddMetaElements(QString name, QList<QVariant> values, QString role_type = "", QString file_as = "");

    /**
     * Adds one element from the dialog to the saved metadata
     */
    void AddMetaElement(QString name, QVariant value, QString role_type = "", QString file_as = "");

    /**
     * Reads the metadata from the metadata table and transfers it
     * to the m_Book variable. This is usually called before closing the dialog.
     */
    void FillMetadataFromDialog();

    /**
     * Writes all the stored dialog settings like
     * window position, geometry etc.
     */
    void WriteSettings();

private:

    // We need this to be able to use a forward
    // declaration of Book in the QSharedPointer
    Q_DISABLE_COPY(MetaEditor)

    // Set the language choice from book or preferences
    void SetLanguage();

    /**
     * Inserts a metadata field with the provided name
     * and value into the table.
     *
     * @param metaname The name of the new metadata field.
     * @param metavalue The value of the new metadata field.
     */
    void AddMetadataToTable(Metadata::MetaElement book_meta, int row = -1);

    /**
     * Reads the metadata from the Book and fills
     * the metadata table with it.
     */
    void ReadMetadataFromBook();

    /**
     * Checks if it's ok to split this metadata field.
     * We interpret (most) metadata fields containing semicolons
     * as containing more than one value, but not for all fields.
     *
     * @param metaname The name of the metadata field.
     * @return \c true if it's ok to split this field.
     */
    static bool OkToSplitInput(const QString &metaname);

    /**
     * Returns a list of all entries in the specified field value.
     * Entries are separated by semicolons, so for instance
     * "Doe, John;Doe, Jane" would return "Doe, John" and "Doe, Jane" in a list.
     *
     * @param field_value The raw string value of the field.
     * @return The list of actual metadata values.
     */
    static QList< QVariant > InputsInField(const QString &field_value);

    /**
     * Fills the language combobox with all the supported languages.
     */
    void FillLanguageComboBox();

    /**
     * Sets up the metadata table.
     */
    void SetUpMetaTable();

    /**
     * Reads all the stored dialog settings like
     * window position, geometry etc.
     */
    void ReadSettings();

    // Setup signal connections
    void ConnectSignals();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The item model that stores the meta information.
     */
    QStandardItemModel m_MetaModel;

    /**
     * The OPF whose metadata is being edited.
     */
    OPFResource &m_OPF;

    /**
     * A working copy of the book's metadata store.
     * This is what we are changing with the dialog.
     *
     * @see Book::m_Metadata
     */
    QList< Metadata::MetaElement > m_Metadata;

    /**
     * The window height after expansion.
     */
    int m_ExpandedHeight;

    MetaEditorItemDelegate *m_cbDelegate;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::MetaEditor ui;
};

#endif // METAEDITOR_H


