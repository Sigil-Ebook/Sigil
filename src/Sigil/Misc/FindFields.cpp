/************************************************************************
**
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

#include "Misc/FindFields.h"

FindFields *FindFields::m_instance = 0;

FindFields *FindFields::instance()
{
    if ( m_instance == 0 ) {
        m_instance = new FindFields();
    }

    return m_instance;
}

FindFields::FindFields()
{
    // Use QList to keep order, but hashes for quick lookup

    // Search Mode
    m_SearchModeList.append( qMakePair( tr( "Normal" ),         FindFields::SearchMode_Normal  ) );
    m_SearchModeList.append( qMakePair( tr( "Case Sensitive" ), FindFields::SearchMode_Case_Sensitive ) );
    m_SearchModeList.append( qMakePair( tr( "Regex" ),          FindFields::SearchMode_Regex ) );
    m_SearchModeList.append( qMakePair( tr( "Spell Check" ),    FindFields::SearchMode_SpellCheck ) );

    foreach ( SearchModePair mode_pair, m_SearchModeList )
    {
        m_SearchModeHash[ mode_pair.first ] =  mode_pair.second;
        m_SearchModeText[ mode_pair.second ] = mode_pair.first;
    }

    // Look Where
    m_LookWhereList.append( qMakePair( tr( "Current File" ),        FindFields::LookWhere_CurrentFile ) );
    m_LookWhereList.append( qMakePair( tr( "All HTML Files" ),      FindFields::LookWhere_AllHTMLFiles ) );
    m_LookWhereList.append( qMakePair( tr( "Selected HTML Files" ), FindFields::LookWhere_SelectedHTMLFiles) );

    foreach ( LookWherePair look_pair, m_LookWhereList)
    {
        m_LookWhereHash[ look_pair.first ] =  look_pair.second;
        m_LookWhereText[ look_pair.second ] = look_pair.first;
    }

    // Search Direction
    m_SearchDirectionList.append( qMakePair( tr( "Up" ),   FindFields::SearchDirection_Up ) );
    m_SearchDirectionList.append( qMakePair( tr( "Down" ), FindFields::SearchDirection_Down ) );

    foreach ( SearchDirectionPair direction_pair, m_SearchDirectionList )
    {
        m_SearchDirectionHash[ direction_pair.first ] =  direction_pair.second;
        m_SearchDirectionText[ direction_pair.second ] = direction_pair.first;
    }

}

QString FindFields::GetSearchModeText( FindFields::SearchMode mode ) 
{
    if ( m_SearchModeText.contains( mode ) )
    {
        return m_SearchModeText.value( mode );
    }
    return QString();
}

QString FindFields::GetLookWhereText( FindFields::LookWhere look) 
{
    if ( m_LookWhereText.contains( look ) )
    {
        return m_LookWhereText.value( look );
    }
    return QString();
}

QString FindFields::GetSearchDirectionText( FindFields::SearchDirection direction ) 
{
    if ( m_SearchDirectionText.contains( direction ) )
    {
        return m_SearchDirectionText.value( direction );
    }
    return QString();
}

FindFields::SearchMode FindFields::GetSearchMode( QString mode_text )
{
    return m_SearchModeHash.value( mode_text );
}

FindFields::LookWhere FindFields::GetLookWhere( QString look_text )
{
    return m_LookWhereHash.value( look_text );
}

FindFields::SearchDirection FindFields::GetSearchDirection( QString direction_text )
{
    return m_SearchDirectionHash.value( direction_text );
}

FindFields::SearchMode FindFields::GetSearchMode( int mode )
{
    switch ( mode )
    {
    case FindFields::SearchMode_Regex:
        return static_cast<FindFields::SearchMode>( mode );
        break;
    case FindFields::SearchMode_Case_Sensitive:
        return static_cast<FindFields::SearchMode>( mode );
        break;
    case FindFields::SearchMode_SpellCheck:
        return static_cast<FindFields::SearchMode>( mode );
        break;
    default:
        return FindFields::SearchMode_Normal;
    }
}

FindFields::LookWhere FindFields::GetLookWhere( int look )
{
    switch ( look )
    {
    case FindFields::LookWhere_AllHTMLFiles:
        return static_cast<FindFields::LookWhere>( look );
        break;
    case FindFields::LookWhere_SelectedHTMLFiles:
        return static_cast<FindFields::LookWhere>( look );
        break;
    default:
        return FindFields::LookWhere_CurrentFile;
    }
}

FindFields::SearchDirection FindFields::GetSearchDirection( int direction )
{
    switch ( direction )
    {
    case FindFields::SearchDirection_Up:
        return static_cast<FindFields::SearchDirection>( direction );
        break;
    default:
        return FindFields::SearchDirection_Down;
    }
}

QList< FindFields::SearchModePair > FindFields::GetSearchModes()
{
    return m_SearchModeList;
}

QList< FindFields::LookWherePair > FindFields::GetLookWheres()
{
    return m_LookWhereList;
}

QList< FindFields::SearchDirectionPair > FindFields::GetSearchDirections()
{
    return m_SearchDirectionList;
}
