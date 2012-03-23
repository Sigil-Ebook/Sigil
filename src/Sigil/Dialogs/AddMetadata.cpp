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

#include "BookManipulation/Metadata.h"
#include "Dialogs/AddMetadata.h"
#include "Misc/SettingsStore.h"

static const QString SETTINGS_GROUP = "add_metadata";

AddMetadata::AddMetadata( const QHash< QString, Metadata::MetaInfo > &metadata, QWidget *parent )
    : 
    QDialog( parent ),
    m_Metadata( metadata )
{
    ui.setupUi( this );

    connect( ui.lwProperties, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ),
             this,	          SLOT(   UpdateDescription(  QListWidgetItem* ) ) );

    connect( this, SIGNAL( accepted() ), this, SLOT( SaveSelection() ) );

    // Fill the dialog with sorted translated metadata names
    QStringList names;
    foreach( QString code, m_Metadata.keys() )
    {
        names.append( Metadata::Instance().GetName(code));
    }
    names.sort();

    foreach( QString name, names )
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
    QString text = m_Metadata.value( Metadata::Instance().GetCode( current->text() ) ).description;

    if ( !text.isEmpty() )
    
        ui.lbDescription->setText( text );
}

QStringList AddMetadata::GetSelectedEntries()
{
    return m_SelectedEntries;
}

void AddMetadata::SaveSelection()
{
    m_SelectedEntries.clear();

    foreach( QListWidgetItem *item, ui.lwProperties->selectedItems() )
    {
        m_SelectedEntries.append( Metadata::Instance().GetCode( item->text() ) );
    }
}


void AddMetadata::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )
    {
        restoreGeometry( geometry );
    }

    QByteArray splitter_position = settings.value( "splitter" ).toByteArray();

    if ( !splitter_position.isNull() )
    {
        ui.splitter->restoreState( splitter_position );
    }

    settings.endGroup();
}


void AddMetadata::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );

    // The position of the splitter handle
    settings.setValue( "splitter", ui.splitter->saveState() );

    settings.endGroup();
}
