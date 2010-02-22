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

#include <stdafx.h>
#include "XHTMLDoc.h"
#include "../Misc/Utility.h"
#include "../BookManipulation/CleanSource.h"
#include <QDomDocument>

static const QString XHTML_DOCTYPE = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                                     "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"; 

// Use with <QRegExp>.setMinimal(true)
static const QString XML_DECLARATION = "(<\\?xml.+\\?>)";

static const QStringList BLOCK_LEVEL_TAGS = QStringList() << "address" << "blockquote" << "center" << "dir" << "div" << 
                                                            "dl" << "fieldset" << "form" << "h1" << "h2" << "h3" << 
                                                            "h4" << "h5" << "h6" << "hr" << "isindex" << "menu" << 
                                                            "noframes" << "noscript" << "ol" << "p" << "pre" <<
                                                            "table" << "ul" << "body";


// Returns a list of XMLElements representing all
// the elements of the specified tag name
// in the head section of the provided XHTML source code
QList< XHTMLDoc::XMLElement > XHTMLDoc::GetTagsInHead( const QString &source, const QString &tag_name )
{
    // TODO: how about replacing uses of this function
    // with XPath expressions? Profile for speed.

    QXmlStreamReader reader( source );

    bool in_head = false;

    QList< XMLElement > matching_elements;

    while ( !reader.atEnd() ) 
    {
        QXmlStreamReader::TokenType type = reader.readNext();

        if ( type == QXmlStreamReader::StartElement ) 
        {
            if ( reader.name() == "head" || reader.name() == "HEAD" )
            
                in_head = true;            

            else if ( in_head && reader.name() == tag_name )
            
                matching_elements.append( CreateXMLElement( reader ) );
        }

        else if ( type == QXmlStreamReader::EndElement &&
                  ( reader.name() == "head" || reader.name() == "HEAD" )
                )
        {
            break;        
        }
    }

    if ( reader.hasError() )
    {
        boost_throw( ErrorParsingXML() 
                     << errinfo_XML_parsing_error_string( reader.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number( reader.lineNumber() )
                     << errinfo_XML_parsing_column_number( reader.columnNumber() )
                   );
    }
    
    return matching_elements;
}


// Returns a list of XMLElements representing all
// the elements of the specified tag name
// in the entire document of the provided XHTML source code
QList< XHTMLDoc::XMLElement > XHTMLDoc::GetTagsInDocument( const QString &source, const QString &tag_name )
{
    // TODO: how about replacing uses of this function
    // with XPath expressions? Profile for speed.

    QXmlStreamReader reader( source );

    QList< XMLElement > matching_elements;

    while ( !reader.atEnd() ) 
    {
        QXmlStreamReader::TokenType type = reader.readNext();

        if ( ( type == QXmlStreamReader::StartElement ) && ( reader.name() == tag_name ) ) 
 
            matching_elements.append( CreateXMLElement( reader ) );        
    }

    if ( reader.hasError() )
    {
        boost_throw( ErrorParsingXML() 
                     << errinfo_XML_parsing_error_string( reader.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number( reader.lineNumber() )
                     << errinfo_XML_parsing_column_number( reader.columnNumber() )
                   );
    }

    return matching_elements;
}


QList< QDomNode > XHTMLDoc::GetTagMatchingChildren( const QDomNode &node, const QStringList &tag_names )
{
    Q_ASSERT( !node.isNull() );

    QList< QDomNode > matching_nodes;

    if ( tag_names.contains( GetNodeName( node ), Qt::CaseInsensitive ) )
    
        matching_nodes.append( node );

    if ( node.hasChildNodes() )
    {
        QDomNodeList children = node.childNodes();
        int num_children = children.count();

        for ( int i = 0; i < num_children; ++i )
        {
            matching_nodes.append( GetTagMatchingChildren( children.at( i ), tag_names ) );              
        }
    }    

    return matching_nodes;
}

QList< QString > XHTMLDoc::GetAllChildIDs( const QDomNode &node )
{
    Q_ASSERT( !node.isNull() );

    QDomElement element = node.toElement();

    if ( element.isNull() )

        return QList< QString >();

    QList< QString > IDs;

    if ( element.hasAttribute( "id" ) )
    
        IDs.append( element.attribute( "id" ) );
    
    else if ( element.hasAttribute( "name" ) )

        IDs.append( element.attribute( "name" ) );

    if ( node.hasChildNodes() )
    {
        QDomNodeList children = node.childNodes();
        int num_children = children.count();

        for ( int i = 0; i < num_children; ++i )
        {
            IDs.append( GetAllChildIDs( children.at( i ) ) );              
        }
    }    

    return IDs;
}


// We need to remove the XML carriage returns ("&#xD" sequences)
// that the default toString() method creates so we wrap it in this function
QString XHTMLDoc::GetQDomNodeAsString( const QDomNode &node )
{
    // This function used to be just this one line:
    //
    //    return document.toString().replace( "&#xd;", "" );
    //
    // But Qt has a bug with the toString() method if the XML
    // encoding is specified as "us-ascii"... so we work around it...
    // and other Qt bugs and weirdness...

    Q_ASSERT( !node.isNull() );

    QString document_text;
    QTextStream stream( &document_text );
    stream.setCodec( "UTF-8" );

    node.save( stream, 1, QDomNode::EncodingFromTextStream );
    document_text.remove( "&#xd;" );

    QRegExp xml_declaration( XML_DECLARATION );
    xml_declaration.setMinimal( true );

    // We need to add the XHTML doctype so XML parsers
    // don't flake-out on HTML character entities
    return document_text.replace( xml_declaration, "\\1\n" + XHTML_DOCTYPE );     
}


// Accepts a string with HTML and returns the text
// in that HTML fragment. For instance: 
//   <h1>Hello <b>Qt</b>&nbsp;this is great</h1>
// returns
//   Hello Qt this is great
QString XHTMLDoc::GetTextInHtml( const QString &source )
{
    QWebPage page;
    page.mainFrame()->setHtml( source );

    return page.mainFrame()->toPlainText();
}


// Resolves HTML entities in the provided string.
// For instance: 
//    Bonnie &amp; Clyde
// returns
//    Bonnie & Clyde
QString XHTMLDoc::ResolveHTMLEntities( const QString &text )
{
    // Faking some HTML... this is the easiest way to do it
    QString newsource = "<div>" + text + "</div>";

    return GetTextInHtml( newsource );
}


// Removes all the children of a node and
// returns that same node back.
// (QDomNodes objects are internally references)
QDomNode XHTMLDoc::RemoveChildren( QDomNode node )
{
    Q_ASSERT( !node.isNull() );

    QDomNodeList children = node.childNodes();

    while ( !children.isEmpty() )
    {
        node.removeChild( children.at( 0 ) );       
    }

    return node;
}


// Returns the node's "real" name. We don't care
// about namespace prefixes and whatnot.
QString XHTMLDoc::GetNodeName( const QDomNode &node )
{
    Q_ASSERT( !node.isNull() );

    QString local_name = node.localName();

    if ( local_name.isEmpty() )

        return node.nodeName();

    else

        return local_name;
}


QString XHTMLDoc::GetAttributeName( const QDomAttr &attribute )
{
    Q_ASSERT( !attribute.isNull() );

    QString name = attribute.name();
    int colon_index = name.lastIndexOf( QChar( ':' ) );

    if ( colon_index < 0 )

        return name;

    else

        return name.mid( colon_index + 1 );
}


// Converts a QDomNodeList to a regular QList
QList< QDomNode > XHTMLDoc::ConvertToRegularList( const QDomNodeList &list )
{
    // Since a QDomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.count();

    QList< QDomNode > nodes;

    for ( int i = 0; i < count; ++i )
    {
        nodes.append( list.at( i ) );
    }

    return nodes;
}


// Returns a list with only the element nodes
QList< QDomNode > XHTMLDoc::GetOnlyElementNodes( const QDomNodeList &list )
{
    // Since a QDomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.count();

    QList< QDomNode > element_nodes;

    for ( int i = 0; i < count; ++i )
    {
        if ( list.at( i ).nodeType() == QDomNode::ElementNode )

            element_nodes.append( list.at( i ) );
    }

    return element_nodes;
}


// Returns the node's real index in the list
int XHTMLDoc::GetRealIndexInList( const QDomNode &node, const QDomNodeList &list )
{
    // Since a QDomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.count();

    for ( int i = 0; i < count; ++i )
    {
        if ( list.at( i ) == node )

            return i;
    }
    
    return -1;
}

// Returns the node's "element" index 
// (pretending the list is only made up of element nodes).
int XHTMLDoc::GetElementIndexInList( const QDomNode &node, const QDomNodeList &list )
{
    // Since a QDomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.count();

    int element_index = 0;

    for ( int i = 0; i < count; ++i )
    {
        if ( list.at( i ) == node )

            return element_index;

        if ( list.at( i ).nodeType() == QDomNode::ElementNode )

            element_index++;
    }

    return -1;
}

// Returns the index of node in the specified list 
// depending on the node type. Text nodes get the "real"
// index, element nodes get the "element" index 
// (pretending the list is only made up of element nodes).
int XHTMLDoc::GetCustomIndexInList( const QDomNode &node, const QDomNodeList &list )
{
    if ( node.nodeType() == QDomNode::TextNode )

        return GetRealIndexInList( node, list );

    else

        return GetElementIndexInList( node, list );
}


// Returns a list of all the "visible" text nodes that are descendants
// of the specified node. "Visible" means we ignore style tags, script tags etc...
QList< QDomNode > XHTMLDoc::GetVisibleTextNodes( const QDomNode &node  )
{
    // TODO: investigate possible parallelization 
    // opportunities for this function (profile before and after!)

    Q_ASSERT( !node.isNull() );

    if ( node.nodeType() == QDomNode::TextNode )
    {
        return QList< QDomNode >() << node;
    }

    else
    {
        QString node_name = GetNodeName( node );

        if (    node.hasChildNodes() && 
                node_name != "script" && 
                node_name != "style" 
            )
        {
            QDomNodeList children = node.childNodes();
            QList< QDomNode > text_nodes;

            for ( int i = 0; i < children.count(); ++i )
            {
                text_nodes.append( GetVisibleTextNodes( children.at( i ) ) );              
            }

            return text_nodes;
        }
    }

    return QList< QDomNode >();
}


// Returns a list of ALL text nodes that are descendants
// of the specified node.
QList< QDomNode > XHTMLDoc::GetAllTextNodes( const QDomNode &node  )
{
    // TODO: investigate possible parallelization 
    // opportunities for this function (profile before and after!)

    Q_ASSERT( !node.isNull() );

    if ( node.nodeType() == QDomNode::TextNode )
    {
        return QList< QDomNode >() << node;
    }

    else
    {
        if ( node.hasChildNodes() )
        {
            QDomNodeList children = node.childNodes();
            QList< QDomNode > text_nodes;

            for ( int i = 0; i < children.count(); ++i )
            {
                text_nodes.append( GetAllTextNodes( children.at( i ) ) );              
            }

            return text_nodes;
        }
    }

    return QList< QDomNode >();
}


// Returns the first block element ancestor of the specified node
QDomNode XHTMLDoc::GetAncestorBlockElement( const QDomNode &node )
{
    QDomNode parent_node = node;

    while ( true )
    {
        parent_node = parent_node.parentNode();

        if ( BLOCK_LEVEL_TAGS.contains( GetNodeName( parent_node ) ) )

            break;
    }
    
    if ( !parent_node.isNull()  )

        return parent_node;

    else

        return node.ownerDocument().elementsByTagName( "body" ).at( 0 );
}


// Returns the node identified by the specified ViewEditor element hierarchy
QDomNode XHTMLDoc::GetNodeFromHierarchy( const QDomDocument &document, const QList< ViewEditor::ElementIndex > &hierarchy )
{
    QDomNode node = document.elementsByTagName( "html" ).at( 0 );
    QDomNode end_node;

    for ( int i = 0; i < hierarchy.count() - 1; ++i )
    {
        QList< QDomNode > children; 

        if ( hierarchy[ i + 1 ].name != "#text" )
        
            children = GetOnlyElementNodes( node.childNodes() );

        else
        
            children = ConvertToRegularList( node.childNodes() );

        // If the index is within the range, descend
        if ( hierarchy[ i ].index < children.count() )
        {
            node = children.at( hierarchy[ i ].index );

            if ( !node.isNull() )

                end_node = node;

            else

                break;
        }

        // Error handling. The asked-for node cannot be found,
        // so we stop where we are.
        else
        {
            end_node = node;  
            break;
        } 
    }

    return end_node;       
}


// Creates a ViewEditor element hierarchy from the specified node
QList< ViewEditor::ElementIndex > XHTMLDoc::GetHierarchyFromNode( const QDomNode &node )
{
    // TODO: Actually we should throw an exception here
    Q_ASSERT( !node.isNull() );

    QDomNode html_node    = node.ownerDocument().elementsByTagName( "html" ).at( 0 );
    QDomNode current_node = node;
    QList< ViewEditor::ElementIndex > element_list;

    while ( current_node != html_node )
    {
        QDomNode parent = current_node.parentNode();

        ViewEditor::ElementIndex element;
        element.name  = GetNodeName( parent );
        element.index = GetCustomIndexInList( current_node, parent.childNodes() );
    
        element_list.prepend( element );

        current_node = parent;
    }

    return element_list;
}


// Accepts a reference to an XML stream reader positioned on an XML element.
// Returns an XMLElement struct with the data in the stream.
XHTMLDoc::XMLElement XHTMLDoc::CreateXMLElement( QXmlStreamReader &reader )
{
    XMLElement element;

    foreach( QXmlStreamAttribute attribute, reader.attributes() )
    {
        QString attribute_name = attribute.name().toString();

        // We convert non-mixed case attribute names to lower case;
        // simplifies things later on so we for instance don't
        // have to check for both "src" and "SRC". 
        if ( !Utility::IsMixedCase( attribute_name ) )
        
            attribute_name = attribute_name.toLower();        

        element.attributes[ attribute_name ] = attribute.value().toString();
    }

    element.name = reader.name().toString();
    element.text = reader.readElementText();

    return element; 
}






