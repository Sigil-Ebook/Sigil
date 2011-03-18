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
#include "PerformCSSUpdates.h"

PerformCSSUpdates::PerformCSSUpdates( const QString &source, const QHash< QString, QString > &css_updates )
    : 
    m_Source( source ), 
    m_CSSUpdates( css_updates )
{

}


QString PerformCSSUpdates::operator()()
{
    const QList< QString > &keys = m_CSSUpdates.keys();
    int num_keys = keys.count();

    for ( int i = 0; i < num_keys; ++i )
    {
        const QString &key_path = keys.at( i );
        const QString &filename = QFileInfo( key_path ).fileName();

        QRegExp reference = QRegExp( 
            "(?:(?:src|background|background-image)\\s*:|@import)\\s*\\w*\\(?[\"']*([^\\)\"']*/"
            + QRegExp::escape( filename ) + "|"
            + QRegExp::escape( filename ) + ")[\"']*\\)?" );

        int index = -1;

        while ( true )
        {
            int newindex = m_Source.indexOf( reference );

            // We need to make sure we don't end up
            // replacing the same thing over and over again
            if ( ( index == newindex ) || ( newindex == -1 ) )

                break;

            m_Source.replace( reference.cap( 1 ), m_CSSUpdates.value( key_path ) );

            index = newindex;
        }
    }

    return m_Source;
}