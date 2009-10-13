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

static const QString HEAD_ELEMENT = "<\\s*(?:head|HEAD)[^>]*>(.*)</\\s*(?:head|HEAD)[^>]*>";


// Returns a list of QDomNodes representing all
// the elements of the specified tag name
// in the head section of the provided XHTML source code
QList< QDomNode > XHTMLDoc::GetTagsInHead( const QString &source, const QString &tag_name )
{
    QRegExp head_element( HEAD_ELEMENT );

    source.contains( head_element );

    QDomDocument document;
    document.setContent( head_element.cap( 0 ) );
   
    return DeepCopyNodeList( document.elementsByTagName( tag_name ) );
}


// Returns a list of QDomNodes representing all
// the elements of the specified tag name
// in the entire document of the provided XHTML source code
QList< QDomNode > XHTMLDoc::GetTagsInDocument( const QString &source, const QString &tag_name )
{
    // TODO: This is painfully slow. Using the DOM approach
    // for this on the whole document is a bad idea since we don't
    // need the whole doc represented as a tree, and we are not going to
    // change the doc. We just need the required elements and their
    // attributes/values, so better to use a QXmlStreamReader that just streams
    // through the doc and stores values in a new proprietary XML element struct.
    // Same thing goes for GetTagsInHead.

    QDomDocument document;
    document.setContent( source );

    return DeepCopyNodeList( document.elementsByTagName( tag_name ) );
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


// Returns a list of deeply copied QDomNodes
// from the specified QDomNodeList
QList< QDomNode > XHTMLDoc::DeepCopyNodeList( QDomNodeList node_list )
{
    QList< QDomNode > new_node_list;

    for ( int i = 0; i < node_list.count(); i++ )
    {
        new_node_list.append( node_list.at( i ).cloneNode() );
    }

    return new_node_list;
}


