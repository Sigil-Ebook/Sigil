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
#include "ViewEditor.h"

class BookViewEditor : public QWebView, public ViewEditor
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

    // Implements the "formatBlock" execCommand because
    // WebKit's default one has bugs.
    // It takes an element name as an argument (e.g. "p"),
    // and replaces the element the cursor is located in with it.
    void FormatBlock( const QString &element_name );

    // Returns the name of the element the caret is located in;
    // if text is selected, returns the name of the element
    // where the selection *starts*
    QString GetCaretElementName();

    // Returns a list of elements representing a "chain"
    // or "walk" through the XHTML document with which one
    // can identify a single element in the document.
    // This list identifies the element in which the 
    // keyboard caret is currently located.
    QList< ViewEditor::ElementIndex > GetCaretLocation(); 

    // Accepts a list returned by a view's GetCaretLocation
    // and creates and stores an update that sends the caret
    // in this view to the specified element.
    // The BookView implementation initiates the update in
    // the JavascriptOnDocumentLoad() function.
    void StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy );

signals:

    // The identically named QWebPage signal is wired to this one,
    // and we also emit it ourselves when necessary.
    void textChanged();

private slots:

    // Executes javascript that needs to be run when
    // the document has finished loading
    void JavascriptOnDocumentLoad();

    // Updates the state of the m_isLoadFinished variable
    // depending on the received loading progress; if the 
    // progress equals 100, the state is true, otherwise false.
    void UpdateFinishedState( int progress );

private:

    // Evaluates the provided javascript source code 
    // and returns the result of the last executed javascript statement
    QVariant EvaluateJavascript( const QString &javascript );

    // Executes the caret updating code
    // if an update is pending;
    // returns true if update was performed
    bool ExecuteCaretUpdate();

    // The javascript source code of the jQuery library
    const QString c_JQuery;

    // The javascript source code of the jQuery
    // ScrollTo extension library
    const QString c_JQueryScrollTo;

    // The javascript source code used
    // to get a hierarchy of elements from
    // the caret element to the top of the document
    const QString c_GetCaretLocation;

    // The javascript source code for the 
    // caret update when switching from 
    // CodeView to BookView
    QString m_CaretLocationUpdate;

    bool m_isLoadFinished;
};


#endif // BOOKVIEWEDITOR_H

