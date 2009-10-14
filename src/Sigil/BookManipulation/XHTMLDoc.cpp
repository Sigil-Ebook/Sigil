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
#include "../BookManipulation/XHTMLDoc.h"
#include <QDomDocument>


// Returns a list of XMLElements representing all
// the elements of the specified tag name
// in the head section of the provided XHTML source code
QList< XHTMLDoc::XMLElement > XHTMLDoc::GetTagsInHead( const QString &source, const QString &tag_name )
{
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

        else if (    type == QXmlStreamReader::EndElement &&
                    ( reader.name() == "head" || reader.name() == "HEAD" )
                )
        {
            break;        
        }
    }

    if ( reader.hasError() )
    {
        // TODO: error handling
    }
    
    return matching_elements;
}


// Returns a list of XMLElements representing all
// the elements of the specified tag name
// in the entire document of the provided XHTML source code
QList< XHTMLDoc::XMLElement > XHTMLDoc::GetTagsInDocument( const QString &source, const QString &tag_name )
{
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
        // TODO: error handling
    }

    return matching_elements;
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
    // encoding is specified as "us-ascii"... so we work around it.

    QString document_text;
    QTextStream stream( &document_text );
    stream.setCodec( "UTF-8" );

    node.save( stream, 1, QDomNode::EncodingFromTextStream );

    return document_text.replace( "&#xd;", "" );   
}


// Removes all the children of a node and
// returns that same node back.
// (QDomNodes objects are internally references)
QDomNode XHTMLDoc::RemoveChildren( QDomNode node )
{
    QDomNodeList children = node.childNodes();

    while ( !children.isEmpty() )
    {
        node.removeChild( children.at( 0 ) );       
    }

    return node;
}


// Accepts a reference to an XML stream reader positioned on an XML element.
// Returns an XMLElement struct with the data in the stream.
XHTMLDoc::XMLElement XHTMLDoc::CreateXMLElement( QXmlStreamReader &reader )
{
    XMLElement element;

    foreach( QXmlStreamAttribute attribute, reader.attributes() )
    {
        element.attributes[ attribute.name().toString() ] = attribute.value().toString();
    }

    element.name = reader.name().toString();
    element.text = reader.readElementText();

    return element; 
}


