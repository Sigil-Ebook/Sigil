/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#include "About.h"

const QString VERSION_NUMBERS = "(\\d+)\\.(\\d+)\\.(\\d+)";
const QString SIGIL_VERSION   = QString( SIGIL_FULL_VERSION );


About::About( QWidget *parent )
    : QDialog( parent )
{
    ui.setupUi( this );

    ui.lbBuildTimeDisplay->setText( GetUTCBuildTime().toString( "yyyy.MM.dd HH:mm:ss" ) + " UTC" );
    ui.lbLoadedQtDisplay->setText( QString( qVersion() ) );

    QRegExp version_number( VERSION_NUMBERS );
    QString( SIGIL_VERSION ).indexOf( version_number );

    QString version_text =  QString( "%1.%2.%3" )
                            .arg( version_number.cap( 1 ).toInt() )
                            .arg( version_number.cap( 2 ).toInt() )
                            .arg( version_number.cap( 3 ).toInt() );

    ui.lbVersionDisplay->setText( version_text );
}


QDateTime About::GetUTCBuildTime()
{
    QString time_string = QString::fromAscii( __TIME__ );
    QString date_string = QString::fromAscii( __DATE__ );

    Q_ASSERT( !date_string.isEmpty() );
    Q_ASSERT( !time_string.isEmpty() );

    QRegExp date_match( "(\\w{3}) (\\d{2}) (\\d{4})" );
    date_string.indexOf( date_match );

    QDate date( date_match.cap( 3 ).toInt(), 
                MonthIndexFromString( date_match.cap( 1 ) ),
                date_match.cap( 2 ).toInt() );

    return QDateTime( date, QTime::fromString( time_string, "hh:mm:ss" ) ).toUTC();
}


// Needed because if we use the "MMM" string in the QDate::fromString
// function, it will match on localized month names, not English ones.
// The __DATE__ macro *always* uses English month names.
int About::MonthIndexFromString( const QString& three_letter_string )
{
    Q_ASSERT( three_letter_string.count() == 3 );
    Q_ASSERT( three_letter_string[ 0 ].isUpper() );

    if ( three_letter_string == "Jan" )

        return 1;

    if ( three_letter_string == "Feb" )

        return 2;

    if ( three_letter_string == "Mar" )

        return 3;

    if ( three_letter_string == "Apr" )

        return 4;

    if ( three_letter_string == "May" )

        return 5;

    if ( three_letter_string == "Jun" )

        return 6;

    if ( three_letter_string == "Jul" )

        return 7;

    if ( three_letter_string == "Aug" )

        return 8;

    if ( three_letter_string == "Sep" )

        return 9;

    if ( three_letter_string == "Oct" )

        return 10;

    if ( three_letter_string == "Nov" )

        return 11;

    if ( three_letter_string == "Dec" )

        return 12;

    Q_ASSERT( false );
    return 0;
}
