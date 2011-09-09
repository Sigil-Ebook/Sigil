/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#include "FindReplaceOptions.h"

FindReplaceOptions::FindReplaceOptions( QWidget *parent )
    :
      QDialog( parent )
{
    ui.setupUi( this );

    ExtendUI();
}


FindReplace::LookWhere FindReplaceOptions::GetLookWhere()
{
    return (FindReplace::LookWhere) ui.lookWhere->itemData( ui.lookWhere->currentIndex() ).toInt();
}


FindReplace::SearchMode FindReplaceOptions::GetSearchMode()
{
    if ( ui.searchWildcard->isChecked() )
    {
        return FindReplace::SearchMode_Wildcard;
    }
    else if ( ui.searchRegex->isChecked() )
    {
        return FindReplace::SearchMode_Regex;
    }
    else
    {
        return FindReplace::SearchMode_Normal;
    }
}


bool FindReplaceOptions::GetMatchWholeWord()
{
    return ui.matchWholeWord->isChecked();
}


bool FindReplaceOptions::GetMatchCase()
{
    return ui.matchCase->isChecked();
}


bool FindReplaceOptions::GetMatchMinimal()
{
    return ui.matchMinimal->isChecked();
}


void FindReplaceOptions::SetLookWhere( FindReplace::LookWhere look_where)
{
    switch ( look_where )
    {
    case FindReplace::LookWhere_AllHTMLFiles:
        ui.lookWhere->setCurrentIndex( 1 );
        break;
    default:
        ui.lookWhere->setCurrentIndex( 0 );
    }
}


void FindReplaceOptions::SetSearchMode( FindReplace::SearchMode search_mode )
{
    switch ( search_mode )
    {
    case FindReplace::SearchMode_Wildcard:
        ui.searchWildcard->setChecked( true );
        break;
    case FindReplace::SearchMode_Regex:
        ui.searchRegex->setChecked( true );
        break;
    default:
        ui.searchNormal->setChecked( true );
    }
}


void FindReplaceOptions::SetMatchWholeWord( bool enabled )
{
    ui.matchWholeWord->setChecked( enabled );
}


void FindReplaceOptions::SetMatchCase( bool enabled )
{
    ui.matchCase->setChecked( enabled );
}


void FindReplaceOptions::SetMatchMinimal( bool enabled )
{
    ui.matchMinimal->setChecked( enabled );
}

void FindReplaceOptions::ExtendUI()
{
    ui.lookWhere->addItem( tr( "Current File" ),   FindReplace::LookWhere_CurrentFile  );
    ui.lookWhere->addItem( tr( "All HTML Files" ), FindReplace::LookWhere_AllHTMLFiles );

    ui.searchNormal->setChecked( true );
}

