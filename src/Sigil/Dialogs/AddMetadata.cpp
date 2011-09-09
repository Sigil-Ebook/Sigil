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

#include <stdafx.h>
#include "AddMetadata.h"
#include "BookManipulation/Metadata.h"

static const QString SETTINGS_GROUP = "add_metadata";


AddMetadata::AddMetadata( const QMap< QString, Metadata::MetaInfo > &metadata, QWidget *parent )
    : 
    QDialog( parent ),
    m_Metadata( metadata )
{
    ui.setupUi( this );

    connect( ui.lwProperties, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ),
             this,	          SLOT(   UpdateDescription(  QListWidgetItem* ) ) );

    connect( this, SIGNAL( accepted() ), this, SLOT( EmitSelection() ) );

    // Filling the dialog with metadata names
    foreach( QString name, m_Metadata.keys() )
    {
        ui.lwProperties->addItem( name );
    }

    ReadSettings();
}


AddMetadata::~AddMetadata()
{
    WriteSettings();
}


void AddMetadata::UpdateDescription( QListWidgetItem *current )
{
    QString text = m_Metadata.value( current->text() ).description;

    if ( !text.isEmpty() )
    
        ui.lbDescription->setText( text );
}


void AddMetadata::EmitSelection()
{
    QStringList metadata;

    foreach( QListWidgetItem *item, ui.lwProperties->selectedItems() )
    {
        metadata.append( item->text() );
    }

    emit MetadataToAdd( metadata );
}


void AddMetadata::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )
    {
        restoreGeometry( geometry );
    }

    QByteArray splitter_position = settings.value( "splitter" ).toByteArray();

    if ( !splitter_position.isNull() )

        ui.splitter->restoreState( splitter_position );
}


void AddMetadata::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );

    // The position of the splitter handle
    settings.setValue( "splitter", ui.splitter->saveState() );
}
