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

#pragma once
#ifndef APPEVENTFILTER_H
#define APPEVENTFILTER_H

#include <QObject>

class QEvent;

class AppEventFilter : public QObject
{
    Q_OBJECT
    
public:
    
    // Constructor;
    // The argument is the object's parent.
    AppEventFilter( QObject *parent );
       
protected:
    
    // The event filter used to catch OS X's 
    // QFileOpenEvents. These signal the user used the OS's
    // services to start Sigil with an existing document
    bool eventFilter( QObject *watched_object, QEvent *event );

};

#endif // APPEVENTFILTER_H


