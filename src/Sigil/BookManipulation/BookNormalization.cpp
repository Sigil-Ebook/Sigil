/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <boost/shared_ptr.hpp>

#include <QtCore/QtCore>
#include <QtCore/QFileInfo>
#include <QtCore/QHashIterator>

#include "BookManipulation/Book.h"
#include "BookManipulation/BookNormalization.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/Headings.h"
#include "BookManipulation/GuideSemantics.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"

using boost::shared_ptr;

static const QString SIGIL_TOC_ID_PREFIX = "sigil_toc_id_";
static const QString SIGIL_TOC_ID_REG    = SIGIL_TOC_ID_PREFIX + "(\\d+)";


void BookNormalization::Normalize( QSharedPointer< Book > book )
{
    QList< HTMLResource* > html_resources = book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true );

    QStringList used_ids = book->GetIdsInHrefs();
    RemoveTOCIDs(html_resources, used_ids);

    int toc_id_index = MaxSigilHeadingIDIndex(book->GetIdsInHTMLFiles()) + 1;

    GiveIDsToHeadings( html_resources, toc_id_index );
}


void BookNormalization::GiveIDsToHeadings( const QList< HTMLResource* > &html_resources, int toc_id_index )
{
    foreach (HTMLResource *html_resource, html_resources) {
        toc_id_index = GiveIDsToHeadingsInResource(html_resource, toc_id_index);
    }
}


int BookNormalization::GiveIDsToHeadingsInResource( HTMLResource *html_resource, int toc_id_index)
{
    QWriteLocker locker( &html_resource->GetLock() );

    QList< Headings::Heading > headings = Headings::GetHeadingListForOneFile( html_resource );

    bool resource_updated = false;
    for ( int index = 0; index < headings.count(); index++ )
    {
        xc::DOMElement &element = *headings.at( index ).element;
        
        if ( !element.hasAttribute( QtoX( "id" ) ) )
        {
            element.setAttribute( 
                QtoX( "id" ), 
                QtoX( SIGIL_TOC_ID_PREFIX + QString::number( toc_id_index ) ) );
            
            toc_id_index++;
            resource_updated = true;
        }
    }
    if (resource_updated) {
        html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(*headings.at(0).document));
    }

    return toc_id_index;
}


int BookNormalization::MaxSigilHeadingIDIndex(QHash<QString, QStringList> file_ids)
{
    int maxindex = 0;

    QHashIterator<QString, QStringList> it (file_ids);

    QRegExp suffix( SIGIL_TOC_ID_REG );

    while (it.hasNext()) {
        it.next();
        foreach(QString id, it.value()) {
            if (id.contains(suffix)) {
                int index = suffix.cap(1).toInt();
                if (index > maxindex) {
                    maxindex = index;
                }
            }
        }
    }

    return maxindex;
}

void BookNormalization::RemoveTOCIDs( const QList< HTMLResource* > &html_resources, QStringList &used_ids)
{
    foreach (HTMLResource *html_resource, html_resources) {
        RemoveTOCIDsInResource(html_resource, used_ids);
    }
}

void BookNormalization::RemoveTOCIDsInResource( HTMLResource* html_resource,  QStringList &used_ids)
{
    Q_ASSERT( html_resource );

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    xc::DOMElement &doc_element = *d.get()->getDocumentElement();

    RemoveTOCIDsInNodes(doc_element, used_ids);
    html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(*d));
}

void BookNormalization::RemoveTOCIDsInNodes( xc::DOMNode &node, QStringList &used_ids )
{
    if ( node.getNodeType() != xc::DOMNode::ELEMENT_NODE ) {
        return;
    }

    xc::DOMElement* element = static_cast< xc::DOMElement* >( &node );

    if (element->hasAttribute(QtoX("id"))) {
        QString id_attribute = XtoQ(element->getAttribute(QtoX("id")));
        bool update_id = false;
        QString new_id_attribute;
        foreach (QString id, id_attribute.split(" ")) {
            if (used_ids.contains(id)) {
}
            if (id.startsWith(SIGIL_TOC_ID_PREFIX) && !used_ids.contains(id)) {
                update_id = true;
            }
            else {
                if (new_id_attribute.isEmpty()) {
                    new_id_attribute = id;
                }
                else {
                    new_id_attribute += " " + id;
                }
            }
        }
        if (update_id) {
            if (new_id_attribute.isEmpty()) {
                element->removeAttribute(QtoX("id"));
            }
            else {
                element->setAttribute(QtoX("id"), QtoX(new_id_attribute));
            }
        }
    }
    if ( node.hasChildNodes() ) {
        QList< xc::DOMNode* > children = XhtmlDoc::GetNodeChildren( node );

        for ( int i = 0; i < children.count(); ++i ) {
            RemoveTOCIDsInNodes( *children.at( i ), used_ids );
        }
    }
}
