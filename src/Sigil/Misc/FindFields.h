/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify **  it under the terms of the GNU General Public License as published by
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
#ifndef FINDFIELDS_H
#define FINDFIELDS_H

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QObject>

class FindFields: public QObject
{
    Q_OBJECT

public:

    FindFields();
    static FindFields *instance();

    /**
     * Defines possible areas where the search can be performed.
     */
    enum LookWhere
    {
        LookWhere_CurrentFile = 0,
        LookWhere_AllHTMLFiles = 10,
        LookWhere_SelectedHTMLFiles = 20
    };

    enum SearchMode
    {
        // Normal is Case insensitive
        SearchMode_Normal = 0,
        SearchMode_Case_Sensitive = 10,
        SearchMode_Regex = 20
    };

    enum SearchDirection
    {
        SearchDirection_Down = 0,
        SearchDirection_Up = 10
    };

    typedef QPair<QString, FindFields::SearchMode> SearchModePair;
    typedef QPair<QString, FindFields::LookWhere> LookWherePair;
    typedef QPair<QString, FindFields::SearchDirection> SearchDirectionPair;

    // Return the ordered list of mode text and enumerated value
    QList< SearchModePair > GetSearchModes();
    QList< LookWherePair > GetLookWheres();
    QList< SearchDirectionPair > GetSearchDirections();

    // Convert enumerated value to translated text
    QString GetSearchModeText( FindFields::SearchMode mode );
    QString GetLookWhereText( FindFields::LookWhere look );
    QString GetSearchDirectionText( FindFields::SearchDirection direction );

    // Convert translated text to enumerated value
    FindFields::SearchMode GetSearchMode( QString mode_text );
    FindFields::LookWhere GetLookWhere( QString look_text );
    FindFields::SearchDirection GetSearchDirection( QString direction_text );

    // Convert saved int form of mode to enumerated value
    static FindFields::SearchMode GetSearchMode( int index );
    static FindFields::LookWhere GetLookWhere( int index );
    static FindFields::SearchDirection GetSearchDirection( int index );

private:

    // Ordered list of mode text and enumerated value
    QList< SearchModePair > m_SearchModeList;
    QList< LookWherePair > m_LookWhereList;
    QList< SearchDirectionPair > m_SearchDirectionList;

    // Save lookup of mode text to enumerated value
    QHash<QString, FindFields::SearchMode > m_SearchModeHash;
    QHash<QString, FindFields::LookWhere > m_LookWhereHash;
    QHash<QString, FindFields::SearchDirection > m_SearchDirectionHash;

    // Save lookup of enumerated value to mode text 
    QHash<FindFields::SearchMode, QString> m_SearchModeText;
    QHash<FindFields::LookWhere, QString> m_LookWhereText;
    QHash<FindFields::SearchDirection, QString> m_SearchDirectionText;

    static FindFields *m_instance;
};

#endif // FINDFIELDS_H
