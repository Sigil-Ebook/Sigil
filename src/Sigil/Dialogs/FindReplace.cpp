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

#include <stdafx.h>
#include "FindReplace.h"

static const QString SETTINGS_GROUP = "find_replace";

// Constructor
// the first argument is the widget's parent.
FindReplace::FindReplace( QWidget *parent )
    : QDialog( parent )
{
    ui.setupUi( this );

    // Telling Qt to delete this window
    // from memory when it is closed
    setAttribute( Qt::WA_DeleteOnClose );

    ExtendUI();

    connect( ui.twTabs, SIGNAL( currentChanged( int ) ), this, SLOT( TabChanged() )     );
    connect( ui.btMore,	SIGNAL( clicked()  ),	         this, SLOT( ToggleMoreLess() ) );

    TabChanged();    
    ReadSettings();
    ToggleMoreLess();
}


// Destructor
FindReplace::~FindReplace()
{
    WriteSettings();
}


void FindReplace::ToggleMoreLess()
{
    if ( m_isMore == true )
    {
        // We hide then show the tab widget between
        // update calls to wOptions because this prevents
        // twTabs from doing layout twice and causing flicker.
        // It's a hack, but it works.
        ui.twTabs->hide();
        ui.wOptions->hide();
        ui.twTabs->show();        

        ui.btMore->setText( tr( "More" ) );

        m_isMore = false;
    }

    else // isMore == false
    {
        qDebug() << ui.wSearch->height();

        // We hide then show the tab widget between
        // update calls to wOptions because this prevents
        // twTabs from doing layout twice and causing flicker.
        // It's a hack, but it works.
        ui.twTabs->hide();
        ui.wOptions->show();
        ui.twTabs->show(); 

        qDebug() << ui.wSearch->height();

        ui.btMore->setText( tr( "Less" ) );

        m_isMore = true;
    }
}


void FindReplace::TabChanged()
{
    ui.twTabs->currentWidget()->layout()->addWidget( ui.wSearch );
    ui.twTabs->currentWidget()->layout()->addWidget( ui.wOptions );
  
    if ( ui.twTabs->currentIndex() == 0 )
    
        ToFindTab();

    else

        ToReplaceTab();
}


void FindReplace::ToFindTab()
{
    ui.btCount->show();
    ui.btReplace->hide();
    ui.btReplaceAll->hide();

    // We "hide" the replace label and field.
    // We use QStackedWidgets because we want the 
    // replace label and field to take up space 
    // in the layout even when they are not visible.
    // That way the dialog doesn't shift all the controls.
    ui.swReplaceLabelHider->setCurrentIndex( 1 );
    ui.swReplaceFieldHider->setCurrentIndex( 1 );
}


void FindReplace::ToReplaceTab()
{
    ui.btCount->hide();
    ui.btReplace->show();
    ui.btReplaceAll->show();

    // We "show" the replace label and field.
    // We use QStackedWidgets because we want the 
    // replace label and field to take up space 
    // in the layout even when they are not visible.
    // That way the dialog doesn't shift all the controls.
    ui.swReplaceLabelHider->setCurrentIndex( 0 );
    ui.swReplaceFieldHider->setCurrentIndex( 0 );
}

// Reads all the stored dialog settings like
// window position, geometry etc.
void FindReplace::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // We flip the stored isMore state because we have to pass through
    // the ToggleMoreLess function to actually set the widgets
    // (and the isMore variable) to the stored state
    m_isMore	= ! settings.value( "is_more" ).toBool();

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )
    {
        restoreGeometry( geometry );
    }

    else
    {
        // We call this to force the dialog to initially take up
        // as little space as possible. The reason for this is that
        // all the horizontal buttons are initially shown on the form,
        // yet only only a few are used on any tab.
        resize( 0, 0 );
    }
}

// Writes all the stored dialog settings like
// window position, geometry etc.
void FindReplace::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );

    // The window expansion state ("more" or "less")
    settings.setValue( "is_more", m_isMore );
}


void FindReplace::ExtendUI()
{
    QVBoxLayout *replace_tab_layout = new QVBoxLayout( ui.ReplaceTab );
}


