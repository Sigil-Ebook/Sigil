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
#include "XhtmlDoc.h"
#include "Misc/Utility.h"
#include "BookManipulation/CleanSource.h"
#include "XercesCppUse.h"
#include <XmlUtils.h>
#include <LocationAwareDOMParser.h>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

static const QStringList BLOCK_LEVEL_TAGS = QStringList() << "address" << "blockquote" << "center" << "dir" << "div" << 
                                                            "dl" << "fieldset" << "form" << "h1" << "h2" << "h3" << 
                                                            "h4" << "h5" << "h6" << "hr" << "isindex" << "menu" << 
                                                            "noframes" << "noscript" << "ol" << "p" << "pre" <<
                                                            "table" << "ul" << "body";
 
static const QStringList IMAGE_TAGS = QStringList() << "img" << "image";

const QString BREAK_TAG_SEARCH  = "(<div>\\s*)?<hr\\s*class\\s*=\\s*\"[^\"]*sigilChapterBreak[^\"]*\"\\s*/>(\\s*</div>)?";


// Returns a list of XMLElements representing all
// the elements of the specified tag name
// in the head section of the provided XHTML source code
QList< XhtmlDoc::XMLElement > XhtmlDoc::GetTagsInHead( const QString &source, const QString &tag_name )
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
QList< XhtmlDoc::XMLElement > XhtmlDoc::GetTagsInDocument( const QString &source, const QString &tag_name )
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


QList< xc::DOMNode* > XhtmlDoc::GetNodeChildren( const xc::DOMNode &node )
{
    xc::DOMNodeList *children = node.getChildNodes();
    int num_children = children->getLength();

    QList< xc::DOMNode* > qtchildren;

    for ( int i = 0; i < num_children; ++i )
    {
        qtchildren.append( children->item( i ) );              
    }

    return qtchildren;
}


QList< xc::DOMElement* > XhtmlDoc::GetTagMatchingDescendants( const xc::DOMNode &node, const QStringList &tag_names )
{
    QList< xc::DOMElement* > matching_nodes;

    if ( tag_names.contains( GetNodeName( node ), Qt::CaseInsensitive ) )
    
        matching_nodes.append( (xc::DOMElement*) &node );

    if ( node.hasChildNodes() )
    {
        QList< xc::DOMNode* > children = GetNodeChildren( node );

        for ( int i = 0; i < children.count(); ++i )
        {
            matching_nodes.append( GetTagMatchingDescendants( *children.at( i ), tag_names ) );              
        }
    }    

    return matching_nodes;
}


// TODO: turn the overloads into a template
QList< xc::DOMElement* > XhtmlDoc::GetTagMatchingDescendants( const xc::DOMElement &node, const QString &tag_name )
{
    xc::DOMNodeList *children = node.getElementsByTagName( QtoX( tag_name ) );
    int num_children = children->getLength();

    QList< xc::DOMElement* > qtchildren;

    for ( int i = 0; i < num_children; ++i )
    {
        qtchildren.append( static_cast< xc::DOMElement* >( children->item( i ) ) );              
    }

    return qtchildren;
}


QList< xc::DOMElement* > XhtmlDoc::GetTagMatchingDescendants( const xc::DOMDocument &node, const QString &tag_name )
{
    xc::DOMNodeList *children = node.getElementsByTagName( QtoX( tag_name ) );
    int num_children = children->getLength();

    QList< xc::DOMElement* > qtchildren;

    for ( int i = 0; i < num_children; ++i )
    {
        qtchildren.append( static_cast< xc::DOMElement* >( children->item( i ) ) );              
    }

    return qtchildren;
}


QList< QString > XhtmlDoc::GetAllDescendantIDs( const xc::DOMNode &node )
{
    const xc::DOMElement* element = static_cast< const xc::DOMElement* >( &node );

    if ( !element )

        return QList< QString >();    

    QList< QString > IDs;

    if ( element->hasAttribute( QtoX( "id" ) ) )
    
        IDs.append( XtoQ( element->getAttribute( QtoX( "id" ) ) ) );
    
    else if ( element->hasAttribute( QtoX( "name" ) ) )

        IDs.append( XtoQ( element->getAttribute( QtoX( "name" ) ) ) );

    if ( node.hasChildNodes() )
    {
        QList< xc::DOMNode* > children = GetNodeChildren( node );

        for ( int i = 0; i < children.count(); ++i )
        {
            IDs.append( GetAllDescendantIDs( *children.at( i ) ) );              
        }
    }    

    return IDs;
}


QString XhtmlDoc::GetDomNodeAsString( const xc::DOMNode &node )
{
    XMLCh LS[] = { xc::chLatin_L, xc::chLatin_S, xc::chNull };
    xc::DOMImplementation *impl = xc::DOMImplementationRegistry::getDOMImplementation( LS );

    shared_ptr< xc::DOMLSSerializer > serializer(
        ( (xc::DOMImplementationLS*) impl )->createLSSerializer(), 
        XercesExt::XercesDeallocator< xc::DOMLSSerializer > );

    serializer->getDomConfig()->setParameter( xc::XMLUni::fgDOMWRTDiscardDefaultContent, false );
    serializer->getDomConfig()->setParameter( xc::XMLUni::fgDOMWRTBOM, true );

    shared_ptr< XMLCh > xwritten( serializer->writeToString( &node ), XercesExt::XercesStringDeallocator );

    return XtoQ( xwritten.get() );
}


shared_ptr< xc::DOMDocument > XhtmlDoc::CopyDomDocument( const xc::DOMDocument &document )
{
    return RaiiWrapDocument( static_cast< xc::DOMDocument* >( document.cloneNode( true ) ) );
}


shared_ptr< xc::DOMDocument > XhtmlDoc::LoadTextIntoDocument( const QString &source )
{
    XercesExt::LocationAwareDOMParser parser;

    // This scanner ignores schemas
    parser.useScanner( xc::XMLUni::fgDGXMLScanner );
    parser.setValidationScheme( xc::AbstractDOMParser::Val_Never );
    parser.useCachedGrammarInParse( true );
    parser.setLoadExternalDTD( true );
    parser.setDoNamespaces( true );

    xc::MemBufInputSource dtd( XHTML_ENTITIES_DTD, XHTML_ENTITIES_DTD_LEN, XHTML_ENTITIES_DTD_ID );
    parser.loadGrammar( dtd, xc::Grammar::DTDGrammarType, true ); 

    // We use source.count() * 2 because count returns
    // the number of QChars, which are 2 bytes long
    xc::MemBufInputSource input( 
        reinterpret_cast< const XMLByte* >( source.utf16() ), 
        source.count() * 2, 
        "empty" );

    XMLCh UTF16[] = { xc::chLatin_U, xc::chLatin_T, xc::chLatin_F, xc::chDigit_1, xc::chDigit_6, xc::chNull };
    input.setEncoding( UTF16 );

    parser.parse( input );

    return RaiiWrapDocument( parser.adoptDocument() );
}


shared_ptr< xc::DOMDocument > XhtmlDoc::RaiiWrapDocument( xc::DOMDocument *document )
{
    return shared_ptr< xc::DOMDocument >( document, XercesExt::XercesDeallocator< xc::DOMDocument > );
}


int XhtmlDoc::NodeLineNumber( const xc::DOMNode &node )
{
    return XercesExt::GetNodeLocationInfo( node ).LineNumber;
}


int XhtmlDoc::NodeColumnNumber( const xc::DOMNode &node )
{
    return XercesExt::GetNodeLocationInfo( node ).ColumnNumber;
}


// Accepts a string with HTML and returns the text
// in that HTML fragment. For instance: 
//   <h1>Hello <b>Qt</b>&nbsp;this is great</h1>
// returns
//   Hello Qt this is great
QString XhtmlDoc::GetTextInHtml( const QString &source )
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
QString XhtmlDoc::ResolveHTMLEntities( const QString &text )
{
    // Faking some HTML... this is the easiest way to do it
    QString newsource = "<div>" + text + "</div>";

    return GetTextInHtml( newsource );
}


// A tree node class without a children() function...
// appallingly stupid, isn't it?
QList< QWebElement > XhtmlDoc::QWebElementChildren( const QWebElement &element )
{
    QList< QWebElement > children;

    const QWebElement &first_child = element.firstChild();
    
    if ( !first_child.isNull() )

        children.append( first_child );

    QWebElement next_sibling = first_child.nextSibling();

    while ( !next_sibling.isNull() )
    {
        children.append( next_sibling );
        next_sibling = next_sibling.nextSibling();
    }

    return children;
}


QStringList XhtmlDoc::GetSGFChapterSplits( const QString& source,
                                           const QString& custom_header )
{
    QRegExp body_start_tag( BODY_START );
    QRegExp body_end_tag( BODY_END );

    int body_begin = source.indexOf( body_start_tag, 0 ) + body_start_tag.matchedLength();
    int body_end   = source.indexOf( body_end_tag,   0 );

    int main_index = body_begin;

    QString header = !custom_header.isEmpty() ? custom_header + "<body>\n" : source.left( body_begin );
    
    QStringList chapters;
    QRegExp break_tag( BREAK_TAG_SEARCH );

    while ( main_index != body_end )
    {        
        // We search for our HR break tag
        int break_index = source.indexOf( break_tag, main_index );

        QString body;

        // We break up the remainder of the file on the HR tag index if it's found
        if ( break_index > -1 )
        {
            body = Utility::Substring( main_index, break_index, source );
            main_index = break_index + break_tag.matchedLength();
        }

        // Otherwise, we take the rest of the file
        else
        {
            body = Utility::Substring( main_index, body_end, source );
            main_index = body_end;
        }

        chapters.append( header + body + "</body> </html>" );
    }	

    return chapters;
}


void XhtmlDoc::RemoveChildren( xc::DOMNode &node )
{
    while ( true )
    {
        xc::DOMNode *child = node.getFirstChild(); 

        if ( !child )

            break;

        node.removeChild( child );       
    }
}


// Returns the node's "real" name. We don't care
// about namespace prefixes and whatnot.
QString XhtmlDoc::GetNodeName( const xc::DOMNode &node )
{
    QString local_name = XtoQ( node.getLocalName() );

    if ( local_name.isEmpty() )

        return XtoQ( node.getNodeName() );

    else

        return local_name;
}


// TODO: this should be covered by attribute.localName(), no?
QString XhtmlDoc::GetAttributeName( const xc::DOMAttr &attribute )
{
    QString name = XtoQ( attribute.getName() );
    int colon_index = name.lastIndexOf( QChar( ':' ) );

    if ( colon_index < 0 )

        return name;

    else

        return name.mid( colon_index + 1 );
}


xc::DOMDocumentFragment* XhtmlDoc::ConvertToDocumentFragment( const xc::DOMNodeList &list )
{
    if ( list.getLength() == 0 )

        return NULL;

    xc::DOMDocumentFragment *fragment = list.item( 0 )->getOwnerDocument()->createDocumentFragment();

    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();

    for ( int i = 0; i < count; ++i )
    {
        // We need to clone the node before inserting it in the 
        // fragment so as to pick up the node's descendants too
        fragment->appendChild( list.item( i )->cloneNode( true ) );
    }

    return fragment;
}


// Converts a DomNodeList to a regular QList
QList< xc::DOMNode* > XhtmlDoc::ConvertToRegularList( const xc::DOMNodeList &list )
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();

    QList< xc::DOMNode* > nodes;

    for ( int i = 0; i < count; ++i )
    {
        nodes.append( list.item( i ) );
    }

    return nodes;
}


// Returns a list with only the element nodes
QList< xc::DOMNode* > XhtmlDoc::ExtractElements( const xc::DOMNodeList &list )
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();

    QList< xc::DOMNode* > element_nodes;

    for ( int i = 0; i < count; ++i )
    {
        xc::DOMNode *node = list.item( i );

        if ( node->getNodeType() == xc::DOMNode::ELEMENT_NODE )

            element_nodes.append( node );
    }

    return element_nodes;
}


// Returns the node's real index in the list
int XhtmlDoc::GetRealIndexInList( const xc::DOMNode &node, const xc::DOMNodeList &list )
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();

    for ( int i = 0; i < count; ++i )
    {
        if ( list.item( i )->isSameNode( &node ) )

            return i;
    }
    
    return -1;
}

// Returns the node's "element" index 
// (pretending the list is only made up of element nodes).
int XhtmlDoc::GetElementIndexInList( const xc::DOMNode &node, const xc::DOMNodeList &list )
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();

    int element_index = 0;

    for ( int i = 0; i < count; ++i )
    {
        if ( list.item( i )->isSameNode( &node ) )

            return element_index;

        if ( list.item( i )->getNodeType() == xc::DOMNode::ELEMENT_NODE )

            element_index++;
    }

    return -1;
}

// Returns the index of node in the specified list 
// depending on the node type. Text nodes get the "real"
// index, element nodes get the "element" index 
// (pretending the list is only made up of element nodes).
int XhtmlDoc::GetCustomIndexInList( const xc::DOMNode &node, const xc::DOMNodeList &list )
{
    if ( node.getNodeType() == xc::DOMNode::TEXT_NODE )

        return GetRealIndexInList( node, list );

    else

        return GetElementIndexInList( node, list );
}


// Returns a list of all the "visible" text nodes that are descendants
// of the specified node. "Visible" means we ignore style tags, script tags etc...
QList< xc::DOMNode* > XhtmlDoc::GetVisibleTextNodes( const xc::DOMNode &node  )
{
    // TODO: investigate possible parallelization 
    // opportunities for this function (profile before and after!)

    if ( node.getNodeType() == xc::DOMNode::TEXT_NODE )
    {
        return QList< xc::DOMNode* >() << const_cast< xc::DOMNode* >( &node );
    }

    else
    {
        QString node_name = GetNodeName( node );

        if ( node.hasChildNodes()  && 
             node_name != "script" && 
             node_name != "style" 
           )
        {
            QList< xc::DOMNode* > children = GetNodeChildren( node );
            QList< xc::DOMNode* > text_nodes;

            for ( int i = 0; i < children.count(); ++i )
            {
                text_nodes.append( GetVisibleTextNodes( *children.at( i ) ) );              
            }

            return text_nodes;
        }
    }

    return QList< xc::DOMNode* >();
}


// Returns a list of ALL text nodes that are descendants
// of the specified node.
QList< xc::DOMNode* > XhtmlDoc::GetAllTextNodes( const xc::DOMNode &node  )
{
    // TODO: investigate possible parallelization 
    // opportunities for this function (profile before and after!)

    if ( node.getNodeType() == xc::DOMNode::TEXT_NODE )
    {
        return QList< xc::DOMNode* >() << const_cast< xc::DOMNode* >( &node );
    }

    else
    {
        if ( node.hasChildNodes() )
        {
            QList< xc::DOMNode* > children = GetNodeChildren( node );
            QList< xc::DOMNode* > text_nodes;

            for ( int i = 0; i < children.count(); ++i )
            {
                text_nodes.append( GetAllTextNodes( *children.at( i ) ) );              
            }

            return text_nodes;
        }
    }

    return QList< xc::DOMNode* >();
}


// Returns the first block element ancestor of the specified node
xc::DOMNode& XhtmlDoc::GetAncestorBlockElement( const xc::DOMNode &node )
{
    const xc::DOMNode *parent_node = &node;

    while ( true )
    {
        parent_node = parent_node->getParentNode();

        if ( BLOCK_LEVEL_TAGS.contains( GetNodeName( *parent_node ) ) )

            break;
    }
    
    if ( parent_node )

        return const_cast< xc::DOMNode& >( *parent_node );

    else

        return *( node.getOwnerDocument()->getElementsByTagName( QtoX( "body" ) )->item( 0 ) );
}


// Returns the node identified by the specified ViewEditor element hierarchy
xc::DOMNode* XhtmlDoc::GetNodeFromHierarchy( const xc::DOMDocument &document,
                                             const QList< ViewEditor::ElementIndex > &hierarchy )
{
    xc::DOMNode *node = document.getElementsByTagName( QtoX( "html" ) )->item( 0 );
    xc::DOMNode *end_node = NULL;

    for ( int i = 0; i < hierarchy.count() - 1; ++i )
    {
        QList< xc::DOMNode* > children; 

        if ( hierarchy[ i + 1 ].name != "#text" )
        
            children = ExtractElements( *node->getChildNodes() );

        else
        
            children = ConvertToRegularList( *node->getChildNodes() );

        // If the index is within the range, descend
        if ( hierarchy[ i ].index < children.count() )
        {
            node = children.at( hierarchy[ i ].index );

            if ( node )

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
QList< ViewEditor::ElementIndex > XhtmlDoc::GetHierarchyFromNode( const xc::DOMNode &node )
{
    xc::DOMNode *html_node = node.getOwnerDocument()->getElementsByTagName( QtoX( "html" ) )->item( 0 );
    const xc::DOMNode *current_node = &node;

    QList< ViewEditor::ElementIndex > element_list;

    while ( current_node != html_node )
    {
        xc::DOMNode *parent = current_node->getParentNode();

        ViewEditor::ElementIndex element;
        element.name  = GetNodeName( *parent );
        element.index = GetCustomIndexInList( *current_node, *parent->getChildNodes() );
    
        element_list.prepend( element );

        current_node = parent;
    }

    return element_list;
}


QStringList XhtmlDoc::GetImagePathsFromImageChildren( const xc::DOMNode &node )
{
    // "Normal" HTML image elements
    QList< xc::DOMElement* > image_nodes = GetTagMatchingDescendants( node, IMAGE_TAGS );

    QStringList image_links;

    // Get a list of all images referenced
    foreach( xc::DOMElement *image_node, image_nodes )
    {
        QString url_reference;

        if ( image_node->hasAttribute( QtoX( "src" ) ) )

            url_reference = Utility::URLDecodePath( XtoQ( image_node->getAttribute( QtoX( "src" ) ) ) );

        else // This covers the SVG "image" tags

            url_reference = Utility::URLDecodePath( XtoQ( image_node->getAttribute( QtoX( "xlink:href" ) ) ) );

        if ( !url_reference.isEmpty() )

            image_links << url_reference;
    }

    // Remove duplicate references
    image_links.removeDuplicates();
    
    return image_links;
}


// Accepts a reference to an XML stream reader positioned on an XML element.
// Returns an XMLElement struct with the data in the stream.
XhtmlDoc::XMLElement XhtmlDoc::CreateXMLElement( QXmlStreamReader &reader )
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














