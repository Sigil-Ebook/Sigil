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
#ifndef BOOKVIEWEDITOR_H
#define BOOKVIEWEDITOR_H

#include <QWebView>

class Book;

class BookViewEditor : public QWebView
{
    Q_OBJECT

public:

    // Constructor;
    // the parameters is the object's parent
    BookViewEditor( QWidget *parent = 0 );

    // Sets the content of the View to the specified book
    void SetBook( const Book &book );

    // Executes the specified command on the document with javascript
    void ExecCommand( const QString &command );

    // Executes the specified command with the specified parameter
    // on the document with javascript
    void ExecCommand( const QString &command, const QString &parameter );

    // Returns the state of the JavaScript command provided
    bool QueryCommandState( const QString &command );

    // Returns the name of the element the caret is located in;
    // if text is selected, returns the name of the element
    // where the selection *starts*
    QString GetCursorElementName();

private slots:

    // Loads custom javascript used by Sigil;
    // should be called every time the Book View
    // is loaded with new content
    void LoadCustomJavascript();

private:

    const QString jQuery;
};


#endif // BOOKVIEWEDITOR_H

