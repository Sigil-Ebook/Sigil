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


#include "stdafx.h"
#include "BookViewEditor.h"


BookViewEditor::BookViewEditor( QWidget *parent )
    : QWebView( parent )
{

}


// Executes the specified command on the document with javascript
void BookViewEditor::ExecCommand( const QString &command )
{       
    QString javascript = QString( "document.execCommand( '%1', false, null)" ).arg( command );

    page()->mainFrame()->evaluateJavaScript( javascript );
}


// Executes the specified command with the specified parameter
// on the document with javascript
void BookViewEditor::ExecCommand( const QString &command, const QString &parameter )
{       
    QString javascript = QString( "document.execCommand( '%1', false, '%2' )" ).arg( command ).arg( parameter );

    page()->mainFrame()->evaluateJavaScript( javascript );
}


// Returns the state of the JavaScript command provided
bool BookViewEditor::QueryCommandState( const QString &command )
{
    QString javascript = QString( "document.queryCommandState( '%1', false, null)" ).arg( command );

    return page()->mainFrame()->evaluateJavaScript( javascript ).toBool();
}


// Returns the name of the element the caret is located in;
// if text is selected, returns the name of the element
// where the selection *starts*
QString BookViewEditor::GetCursorElementName()
{
    QString javascript =  "var node = document.getSelection().anchorNode;"
        "var startNode = (node.nodeName == \"#text\" ? node.parentNode : node);"
        "startNode.nodeName;";

    return page()->mainFrame()->evaluateJavaScript( javascript ).toString();
}

