/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#include "ui_MetaEditor.h"
#include <QStandardItemModel>

class Book;


class MetaEditor : public QDialog
{
    Q_OBJECT

public:

    // Constructor
    MetaEditor( Book &book, QWidget *parent = 0 );

    // Destructor
    ~MetaEditor();	

protected:

    // Overrides
    void showEvent( QShowEvent * event );  

private slots:

    // More/Less button functionality;
    // Switches the display between the "more" version with 
    // the metadata table and the "less" version without it
    void ToggleMoreLess();

    // Inserts a metadata field with the provided name
    // into the table; the value of the field is empty
    void AddEmptyMetadataToTable( const QString &metaname );

    // Inserts a metadata field with the provided name
    // and value into the table
    void AddMetadataToTable( const QString &metaname, const QVariant &metavalue );

    // Add Basic button functionality;
    // Shows the window for selecting the new basic
    // metadata type which creates the new type upon return
    void AddBasic();

    // Add Advanced button functionality;
    // Shows the window for selecting the new advance
    // metadata type which creates the new type upon return
    void AddAdvanced();

    // Remove button functionality;
    // removes the currently selected row
    // from the metadata table
    void Remove();

    // Refreshes the vertical header of the table view widget
    void RefreshVerticalHeader();

    // Reads the metadata in the metadata table and transfers it 
    // to the m_Book variable; this is usually called before closing the dialog
    void FillMetadataFromDialog();

private:

    // Reads the metadata from the Book and fills
    // the metadata table with it
    void ReadMetadataFromBook();

    // Clears all the metadata stored in the Book
    void ClearBookMetadata();

    // Returns true if it's ok to split this metadata field
    bool OkToSplitInput( const QString &metaname ) const;

    // Returns a list of all entries in the specified field;
    // Entries are separated by semicolons, so for instance
    // "Doe, John;Doe, Jane" would return "Doe, John" and "Doe, Jane" in a list
    QList< QVariant > InputsInField( const QString &field_value ) const;

    // Adds a value to a field; if a value already exists, 
    // the new value is appended after a semicolon
    QString AddValueToField( const QString &field, const QString &value ) const;

    // Fills the language combobox with all the supported languages
    void FillLanguageComboBox();	

    // Sets up the metadata table
    void SetUpMetaTable();	

    // Reads all the stored dialog settings like
    // window position, geometry etc.
    void ReadSettings();

    // Writes all the stored dialog settings like
    // window position, geometry etc.
    void WriteSettings();

    // Performs specific changes based on the OS platform
    void PlatformSpecificTweaks();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // The item model that stores the meta information
    QStandardItemModel m_MetaModel;

    // Stores the current display state
    // of the dialog
    bool m_isMore;

    // The Book whose metadata is being edited
    Book &m_Book;

    // The window height after expansion
    int m_ExpandedHeight;

    // Holds all the widgets Qt Designer created for us
    Ui::MetaEditor ui;
};

#endif // METAEDITOR_H


