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

#pragma once
#ifndef XHTMLDOC_H
#define XHTMLDOC_H

#include "ViewEditors/ViewEditor.h"
#include <QWebElement>
#include "XercesHUse.h"

class QString;
class QStringList;

class XhtmlDoc
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

    // Resolves custom ENTITY declarations
    static QString ResolveCustomEntities( const QString &source );

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the head section of the provided XHTML source code
    static QList< XMLElement > GetTagsInHead( const QString &source, const QString &tag_name );

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the entire document of the provided XHTML source code
    static QList< XMLElement > GetTagsInDocument( const QString &source, const QString &tag_name );

    static QList< xc::DOMNode* > GetNodeChildren( const xc::DOMNode &node );

    static QList< xc::DOMElement* > GetTagMatchingDescendants( const xc::DOMNode &node, const QStringList &tag_names );
   
    static QList< xc::DOMElement* > GetTagMatchingDescendants( const xc::DOMElement &node, const QString &tag_name );

    static QList< xc::DOMElement* > GetTagMatchingDescendants( const xc::DOMDocument &node, const QString &tag_name );

    static QList< QString > GetAllDescendantIDs( const xc::DOMNode &node ); 

    static QString GetDomNodeAsString( const xc::DOMNode &node );

    static QString GetDomDocumentAsString( const xc::DOMDocument &document );

    /**
     * Parses the source text into a DOM and returns a shared pointer
     * to the heap-created document. 
     */
    static shared_ptr< xc::DOMDocument > LoadTextIntoDocument( const QString &source );

    static shared_ptr< xc::DOMDocument > CopyDomDocument( const xc::DOMDocument &document );

    static shared_ptr< xc::DOMDocument > RaiiWrapDocument( xc::DOMDocument *document );

    static int NodeLineNumber( const xc::DOMNode &node );

    static int NodeColumnNumber( const xc::DOMNode &node );

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

    static QList< QWebElement > QWebElementChildren( const QWebElement &element );

    /**
     * Splits the provided source on SGF chapter breaks.
     *
     * @param source The source which we want to split.
     * @param custom_header An option custom header to be used instead of 
     *                      the one in the current source.
     * @return The split chapters.
     */
    static QStringList GetSGFChapterSplits( const QString& source, 
                                            const QString& custom_header = QString() );

    // Removes all the children of a node
    static void RemoveChildren( xc::DOMNode &node );

    // Returns the node's "real" name. We don't care
    // about namespace prefixes and whatnot.
    static QString GetNodeName( const xc::DOMNode &node );

    // Returns the attribute's "real" name. We don't care
    // about namespace prefixes and whatnot.
    static QString GetAttributeName( const xc::DOMAttr &attribute );

    /**
     * Converts a DomNodeList of nodes (and all their descendants)
     * into a document fragment that can then be easily inserted 
     * into other documents.
     *
     * @param list The list of nodes to go into the fragment.
     * @return The new document fragment.
     */
    static xc::DOMDocumentFragment* ConvertToDocumentFragment( const xc::DOMNodeList &list );

    // Converts a DomNodeList to a regular QList
    static QList< xc::DOMNode* > ConvertToRegularList( const xc::DOMNodeList &list );

    // Returns a list with only the element nodes
    static QList< xc::DOMNode* > ExtractElements( const xc::DOMNodeList &list );

    // Returns the node's real index in the list
    static int GetRealIndexInList( const xc::DOMNode &node, const xc::DOMNodeList &list );

    // Returns the node's "element" index 
    // (pretending the list is only made up of element nodes).
    static int GetElementIndexInList( const xc::DOMNode &node, const xc::DOMNodeList &list );

    // Returns the index of node in the specified list 
    // depending on the node type. Text nodes get the "real"
    // index, element nodes get the "element" index 
    // (pretending the list is only made up of element nodes).
    static int GetCustomIndexInList( const xc::DOMNode &node, const xc::DOMNodeList &list );

    // Returns a list of all the "visible" text nodes that are descendants
    // of the specified node. "Visible" means we ignore style tags, script tags etc...
    static QList< xc::DOMNode* > GetVisibleTextNodes( const xc::DOMNode &node );

    // Returns a list of ALL text nodes that are descendants
    // of the specified node.
    static QList< xc::DOMNode* > GetAllTextNodes( const xc::DOMNode &node );

    // Returns the first block element ancestor of the specified node
    static xc::DOMNode& GetAncestorBlockElement( const xc::DOMNode &node );

    // Returns the node identified by the specified ViewEditor element hierarchy
    static xc::DOMNode* GetNodeFromHierarchy( const xc::DOMDocument &document, 
                                              const QList< ViewEditor::ElementIndex > &hierarchy );


    // Creates a ViewEditor element hierarchy from the specified node
    static QList< ViewEditor::ElementIndex > GetHierarchyFromNode( const xc::DOMNode &node ); 

    static QStringList GetImagePathsFromImageChildren( const xc::DOMNode &node );

private:

    // Accepts a reference to an XML stream reader positioned on an XML element.
    // Returns an XMLElement struct with the data in the stream.
    static XMLElement CreateXMLElement( QXmlStreamReader &reader );

    static QString PrepareSourceForXerces( const QString &source );
};

#endif // XHTMLDOC_H

