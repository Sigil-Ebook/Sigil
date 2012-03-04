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

#include <QtCore/QDate>
#include <QtGui/QShowEvent>

#include "Dialogs/MetaEditor.h"
#include "Dialogs/AddMetadata.h"
#include "Misc/SettingsStore.h"
#include "ResourceObjects/OPFResource.h"

static const int DEFAULT_EXPANDED_HEIGHT = 304;
static const QString SETTINGS_GROUP      = "meta_editor";

MetaEditor::MetaEditor( OPFResource &opf, QWidget *parent )
    :
    QDialog( parent ),
    m_OPF( opf ),
    m_Metadata( m_OPF.GetDCMetadata() )
{
    ui.setupUi( this );	

    PlatformSpecificTweaks();

    connect( ui.btMore,        SIGNAL( clicked()  ), this, SLOT( ToggleMoreLess()         ) );
    connect( ui.btAddBasic,    SIGNAL( clicked()  ), this, SLOT( AddBasic()	              ) );
    connect( ui.btAddAdvanced, SIGNAL( clicked()  ), this, SLOT( AddAdvanced()            ) );
    connect( ui.btRemove,      SIGNAL( clicked()  ), this, SLOT( Remove()                 ) );
    connect( this,             SIGNAL( accepted() ), this, SLOT( FillMetadataFromDialog() ) );

    connect( ui.tvMetaTable->horizontalHeader(),  SIGNAL( sectionClicked( int ) ),
             this,                                SLOT( RefreshVerticalHeader() ) );

    ReadSettings();
    ToggleMoreLess();
    SetUpMetaTable();

    FillLanguageComboBox();
    ReadMetadataFromBook();

    // Set the default language to the users preference if possible.
    if ( m_Metadata[ "Language" ].isEmpty() )
    {
        SettingsStore settings;
        int index = ui.cbLanguages->findText( settings.defaultMetadataLang() );
        if ( index == -1 )
        {
            index = ui.cbLanguages->findText( tr( "English" ) );
            if ( index == -1 )
            {
                index = 0;
            }
        }
        ui.cbLanguages->setCurrentIndex( index );
    }
}


MetaEditor::~MetaEditor()
{
    if ( m_isMore )

        m_ExpandedHeight = size().height();

    WriteSettings();
}


void MetaEditor::showEvent( QShowEvent* event )
{
    RefreshVerticalHeader();

    event->accept();
}


void MetaEditor::ToggleMoreLess()
{
    if ( m_isMore )
    {
        m_ExpandedHeight = size().height();
        ui.wgExtension->hide();
        ui.btMore->setText( tr( "More" ) );

        m_isMore = false;
    }

    else
    {		
        ui.wgExtension->show();

        if ( m_ExpandedHeight == 0 )

            resize( size().width(), DEFAULT_EXPANDED_HEIGHT );

        else
        
            resize( size().width(), m_ExpandedHeight );

        ui.btMore->setText( tr( "Less" ) );
        
        m_isMore = true;
    }
}


void MetaEditor::AddEmptyMetadataToTable( const QStringList &metanames )
{
    foreach ( QString metaname, metanames )
    {
        // If we are inserting a date, that needs special treatment;
        // We need to insert it as a QDate object so the table interface
        // can automatically impose input restrictions
        if ( metaname.contains( "Date" ) )
        {
            AddMetadataToTable( metaname, QDate::currentDate() );
        }
        // String-based metadata gets created normally
        else
        {
            AddMetadataToTable( metaname, QString() );
        }
    }
}


void MetaEditor::AddMetadataToTable( const QString &metaname, const QVariant &metavalue )
{
    m_MetaModel.insertRow( m_MetaModel.rowCount() );
    m_MetaModel.setData( m_MetaModel.index( m_MetaModel.rowCount() - 1, 0 ), metaname );

    // The user should not be able to edit
    // the field with the metadata's name
    m_MetaModel.item( m_MetaModel.rowCount() - 1, 0 )->setEditable( false );
    m_MetaModel.setData( m_MetaModel.index( m_MetaModel.rowCount() - 1, 1 ), metavalue );
}


void MetaEditor::AddBasic()
{
    AddMetadata addmeta( Metadata::Instance().GetBasicMetaMap(), this );

    connect( &addmeta, SIGNAL( MetadataToAdd( QStringList ) ), this, SLOT( AddEmptyMetadataToTable( QStringList ) ) );

    addmeta.exec();
}


void MetaEditor::AddAdvanced()
{
    AddMetadata addmeta( Metadata::Instance().GetRelatorMap(), this );

    connect( &addmeta, SIGNAL( MetadataToAdd( QStringList ) ), this, SLOT( AddEmptyMetadataToTable( QStringList ) ) );

    addmeta.exec();
}


void MetaEditor::Remove()
{
    while ( ui.tvMetaTable->selectionModel()->hasSelection() )
    {
        m_MetaModel.removeRow( ui.tvMetaTable->selectionModel()->selection().indexes().at( 0 ).row() );
    }
}


// Refreshes the vertical header of the table view widget
void MetaEditor::RefreshVerticalHeader()
{
    //    This whole function and all the calls to it are basically
    // a bug fix. Qt for some strange reason doesn't update the
    // vertical header of a table when the fields are sorted.
    // That leaves you with e.g. a vertically single-line field
    // occupying the space of a five-line field and vice versa.
    // The section sizes are also screwed up in certain situations
    // on first draw.
    //    Oh and the native slot that would handle all of this for us?
    // It's marked protected. Amazingly frustrating, isn't it?
    ui.tvMetaTable->verticalHeader()->resizeSections( QHeaderView::ResizeToContents );
}


void MetaEditor::FillMetadataFromDialog()
{
    // Clear the book metadata so we don't duplicate something...
    // Nothing should be lost as everything was loaded into the dialog
    ClearBookMetadata();

    // For string-based metadata, create multiple entries
    // if the typed in value contains semicolons
    m_Metadata[ "Title"    ].append( InputsInField( ui.leTitle->text()            ) );
    m_Metadata[ "Author"   ].append( InputsInField( ui.leAuthor->text()           ) );
    m_Metadata[ "Language" ].append( InputsInField( ui.cbLanguages->currentText() ) );

    for ( int row = 0; row < m_MetaModel.rowCount(); row++ )
    {
        QString name   = m_MetaModel.data( m_MetaModel.index( row, 0 ) ).toString();
        QVariant value = m_MetaModel.data( m_MetaModel.index( row, 1 ) );

        // For string-based metadata, create multiple entries
        // if the typed in value contains semicolons
        if ( value.type() == QVariant::String && OkToSplitInput( name ) )
        
            m_Metadata[ name ].append( InputsInField( value.toString() ) );

        else
            
            m_Metadata[ name ].append( value );
    }

    m_OPF.SetDCMetadata( m_Metadata );
}


void MetaEditor::ReadMetadataFromBook()
{
    foreach ( QString name, m_Metadata.keys() )
    {
        foreach ( QVariant single_value, m_Metadata[ name ] )
        {
            if ( name == "Title" )

                ui.leTitle->setText( AddValueToField( ui.leTitle->text(), single_value.toString() ) );

            else if ( name == "Author" )

                ui.leAuthor->setText( AddValueToField( ui.leAuthor->text(), single_value.toString() ) );

            else if ( name == "Language" )

                ui.cbLanguages->setCurrentIndex( ui.cbLanguages->findText( single_value.toString() ) );

            else
            
                AddMetadataToTable( name, single_value );				
        }
    }
}


void MetaEditor::ClearBookMetadata()
{
    foreach ( QString name, m_Metadata.keys() )
    {
        m_Metadata[ name ].clear();
    }
}


bool MetaEditor::OkToSplitInput( const QString &metaname )
{
    // The "description" and "rights" fields could have a semicolon
    // in the text and there's also little point in providing multiple
    // entries for these so we don't split them.
    if ( metaname == "Description" ||
         metaname == "Rights"
        )
    {
        return false;
    }		

    return true;
}


QList< QVariant > MetaEditor::InputsInField( const QString &field_value )
{
    QList< QVariant > inputs;

    foreach ( QString input, field_value.split( ";", QString::SkipEmptyParts ) )
    {
        inputs.append( input.simplified() );
    }

    return inputs;
}


QString MetaEditor::AddValueToField( const QString &field_value, const QString &value )
{
    if ( field_value.isEmpty() )

        return value;

    else

        return field_value + "; " + value;
}


void MetaEditor::FillLanguageComboBox()
{
    foreach ( QString lang, Metadata::Instance().GetLanguageMap().keys() )
    {
        ui.cbLanguages->addItem( lang );
    }	
}


void MetaEditor::SetUpMetaTable()
{
    QStringList header;
    
    header.append( tr( "Name" ) );
    header.append( tr( "Value" ) );

    m_MetaModel.setHorizontalHeaderLabels( header );

    ui.tvMetaTable->setModel( &m_MetaModel );

    // Make the header fill all the available space
    ui.tvMetaTable->horizontalHeader()->setStretchLastSection( true );
    ui.tvMetaTable->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );

    ui.tvMetaTable->setSortingEnabled( true );
    ui.tvMetaTable->setWordWrap( true );
    ui.tvMetaTable->setAlternatingRowColors( true );    
}


void MetaEditor::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // We flip the stored isMore state because we have to pass through
    // the ToggleMoreLess function to actually set the widgets
    // (and the isMore variable) to the stored state
    m_isMore = !settings.value( "is_more" ).toBool();

    // Window width and the height after expansion
    int width        = settings.value( "width" ).toInt();
    m_ExpandedHeight = settings.value( "expanded_height" ).toInt();

    if ( ( width != 0 ) && ( m_ExpandedHeight != 0 ) )
    {
        resize( width, m_ExpandedHeight );
    }

    // The window's position on the screen
    QPoint position = settings.value( "position" ).toPoint();

    if ( !position.isNull() )
    {
        move( position );
    }

    settings.endGroup();
}


void MetaEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The window expansion state ("more" or "less")
    settings.setValue( "is_more", m_isMore );

    // Window width and the height after expansion
    settings.setValue( "width", size().width() );
    settings.setValue( "expanded_height", m_ExpandedHeight );

    // The window's position on the screen
    settings.setValue( "position", pos() );

    settings.endGroup();
}


void MetaEditor::PlatformSpecificTweaks()
{
#ifdef Q_WS_WIN
    // Increasing the spacing between the controls so they
    // line up nicely with the buttons on Windows. Setting 
    // this for other platforms has the opposite effect.
    ui.formLayout->setVerticalSpacing( 13 );
#endif
}









