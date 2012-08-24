/************************************************************************
**
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Grant Drake
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
#ifndef WINCLIPBOARDERRORHANDLER_H
#define WINCLIPBOARDERRORHANDLER_H

#include <QtCore/QObject>

class QEvent;
class QPlainTextEdit;
class QWebPage;

class WinClipboardErrorHandler : public QObject
{
    Q_OBJECT
    
public:
    
    // Constructor;
    // The argument is the object's parent.
    WinClipboardErrorHandler( QObject *parent=0 );
       
    bool HandleError();

private slots:
    void AttemptAnotherCopy();

private:
    QPlainTextEdit *m_textEdit;
    QWebPage *m_webPage;
};

#endif // WINCLIPBOARDERRORHANDLER


