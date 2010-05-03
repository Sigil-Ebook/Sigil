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
#include "AppEventFilter.h"
#include "../MainUI/MainWindow.h"


// Constructor;
// The argument is the object's parent.
AppEventFilter::AppEventFilter( QObject *parent )
    : QObject( parent )
{
    
}

// The event filter used to catch OS X's 
// QFileOpenEvents. These signal the user used the OS's
// services to start Sigil with an existing document
bool AppEventFilter::eventFilter( QObject *watched_object, QEvent *event )
{
    if ( event->type() == QEvent::FileOpen ) 
    {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        
        MainWindow *widget = new MainWindow( openEvent->file() );    
        widget->show();
        
        return true;
    } 
    
    // standard event processing
    return QObject::eventFilter( watched_object, event );
}
