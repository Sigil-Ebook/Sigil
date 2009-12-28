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
#ifndef XHTMLDOC_H
#define XHTMLDOC_H

#include "../ViewEditors/ViewEditor.h"

class QDomNode;
class QDomDocument;
class QString;
class QDomNodeList;

class XHTMLDoc
{

public:

    struct XMLElement
    {
        // The name of the element
        QString name;

        // The text of the element
        QString text;

        // The attributes of the element;
        // the keys are the attribute names,
        // the values are the attribute values
        QHash< QString, QString > attributes;
    };

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the head section of the provided XHTML source code
    static QList< XMLElement > GetTagsInHead( const QString &source, const QString &tag_name  );

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the entire document of the provided XHTML source code
    static QList< XMLElement > GetTagsInDocument( const QString &source, const QString &tag_name );

    // We need to remove the XML carriage returns ("&#xD" sequences)
    // that the default toString() method creates so we wrap it in this function
    static QString GetQDomNodeAsString( const QDomNode &node );

    // Accepts a string with HTML and returns the text
    // in that HTML fragment. For instance: 
    //   <h1>Hello <b>Qt</b>&nbsp;this is great</h1>
    // returns
    //   Hello Qt this is great
    static QString GetTextInHtml( const QString &source );

    // Resolves HTML entities in the provided string.
    // For instance: 
    //    Bonnie &amp; Clyde
    // returns
    //    Bonnie & Clyde
    static QString ResolveHTMLEntities( const QString &text );

    // Removes all the children of a node and
    // returns that same modified node back.
    // (QDomNodes objects are internally references)
    static QDomNode RemoveChildren( QDomNode node );

    // Returns the node's "real" name. We don't care
    // about namespace prefixes and whatnot.
    static QString GetNodeName( const QDomNode &node );

    // Converts a QDomNodeList to a regular QList
    static QList< QDomNode > ConvertToRegularList( const QDomNodeList &list );

    // Returns a list with only the element nodes
    static QList< QDomNode > GetOnlyElementNodes( const QDomNodeList &list );

    // Returns the node's real index in the list
    static int GetRealIndexInList( const QDomNode &node, const QDomNodeList &list );

    // Returns the node's "element" index 
    // (pretending the list is only made up of element nodes).
    static int GetElementIndexInList( const QDomNode &node, const QDomNodeList &list );

    // Returns the index of node in the specified list 
    // depending on the node type. Text nodes get the "real"
    // index, element nodes get the "element" index 
    // (pretending the list is only made up of element nodes).
    static int GetCustomIndexInList( const QDomNode &node, const QDomNodeList &list );

    // Returns a list of all the "visible" text nodes that are descendants
    // of the specified node. "Visible" means we ignore style tags, script tags etc...
    static QList< QDomNode > GetVisibleTextNodes( const QDomNode &node );

    // Returns a list of ALL text nodes that are descendants
    // of the specified node.
    static QList< QDomNode > GetAllTextNodes( const QDomNode &node );

    // Returns the first block element ancestor of the specified node
    static QDomNode GetAncestorBlockElement( const QDomNode &node );

    // Returns the node identified by the specified ViewEditor element hierarchy
    static QDomNode GetNodeFromHierarchy( const QDomDocument &document, 
                                          const QList< ViewEditor::ElementIndex > &hierarchy );


    // Creates a ViewEditor element hierarchy from the specified node
    static QList< ViewEditor::ElementIndex > GetHierarchyFromNode( const QDomNode &node ); 

private:

    // Accepts a reference to an XML stream reader positioned on an XML element.
    // Returns an XMLElement struct with the data in the stream.
    static XMLElement CreateXMLElement( QXmlStreamReader &reader );
};

#endif // XHTMLDOC_H

