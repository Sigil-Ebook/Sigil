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

// Constructor
About::About( QWidget *parent )
    : QDialog( parent )
{
    ui.setupUi( this );

    ui.lbLoadedQtDisplay->setText( QString( qVersion() ) );

    QRegExp version_number( VERSION_NUMBERS );
    QString( SIGIL_VERSION ).indexOf( version_number );

    QString version_text =  QString( "%1.%2.%3" )
                            .arg( version_number.cap( 1 ).toInt() )
                            .arg( version_number.cap( 2 ).toInt() )
                            .arg( version_number.cap( 3 ).toInt() );

    ui.lbVersionDisplay->setText( version_text );
}


