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
#include <QDomDocument>
#include "../Misc/Utility.h"
#include "../BookManipulation/Book.h"
#include "BookNormalization.h"
#include "../BookManipulation/CleanSource.h"
#include "../BookManipulation/Headings.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "../BookManipulation/GuideSemantics.h"
#include "ResourceObjects/HTMLResource.h"

static const QString SIGIL_HEADING_ID_PREFIX = "heading_id_";
static const QString SIGIL_HEADING_ID_REG    = SIGIL_HEADING_ID_PREFIX + "(\\d+)";
static const int FLOW_SIZE_THRESHOLD         = 1000;


void BookNormalization::Normalize( QSharedPointer< Book > book )
{
    QList< HTMLResource* > html_resources = book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true );

    GiveIDsToHeadings( html_resources );

    if ( !CoverPageExists( html_resources ) )
    
        TryToSetCoverPage( html_resources );

    QList< ImageResource* > image_resources = book->GetFolderKeeper().GetResourceTypeList< ImageResource >();

    if ( !CoverImageExists( image_resources ) )

        TryToSetCoverImage( html_resources, image_resources );
}


void BookNormalization::GiveIDsToHeadings( QList< HTMLResource* > html_resources )
{
    QtConcurrent::blockingMap( html_resources, GiveIDsToHeadingsInResource );
}


void BookNormalization::GiveIDsToHeadingsInResource( HTMLResource *html_resource )
{
    QReadLocker locker( &html_resource->GetLock() );

    QList< Headings::Heading > headings = Headings::GetHeadingListForOneFile( html_resource );

    int heading_id_index = MaxSigilHeadingIDIndex( headings ) + 1;

    for ( int index = 0; index < headings.count(); index++ )
    {
        QDomElement element = headings.at( index ).element;
        
        if ( !element.hasAttribute( "id" ) )
        {
            element.setAttribute( "id", SIGIL_HEADING_ID_PREFIX + QString::number( heading_id_index ) );
            
            heading_id_index++;
        }
    }    
}


int BookNormalization::MaxSigilHeadingIDIndex( const QList< Headings::Heading > headings )
{
    int maxindex = 1;
    
    for ( int index = 0; index < headings.count(); index++ )
    {
        QDomElement element = headings.at( index ).element;

        QRegExp suffix( SIGIL_HEADING_ID_REG );

        if ( element.attribute( "id" ).contains( suffix ) )
        {
            int index = suffix.cap( 1 ).toInt();

            if ( index > maxindex )

                maxindex = index;
        }
    }

    return maxindex;
}


HTMLResource* BookNormalization::GetCoverPage( QList< HTMLResource* > html_resources )
{
    foreach( HTMLResource* html_resource, html_resources )
    {
        if ( html_resource->GetGuideSemanticType() == GuideSemantics::Cover )

            return html_resource;
    }

    return NULL;
}


bool BookNormalization::CoverPageExists( QList< HTMLResource* > html_resources )
{
    return GetCoverPage( html_resources ) != NULL;
}


void BookNormalization::TryToSetCoverPage( QList< HTMLResource* > html_resources )
{
    HTMLResource *first_html = html_resources[ 0 ];

    if ( IsFlowUnderThreshold( first_html, FLOW_SIZE_THRESHOLD ) &&
         FlowHasOnlyOneImage( first_html )
       )
    {
        first_html->SetGuideSemanticType( GuideSemantics::Cover );
    }
}


bool BookNormalization::CoverImageExists( QList< ImageResource* > image_resources )
{
    foreach( ImageResource* image_resource, image_resources )
    {
        if ( image_resource->IsCoverImage() )

            return true;
    }

    return false;
}


void BookNormalization::TryToSetCoverImage( QList< HTMLResource* > html_resources, 
                                            QList< ImageResource* > image_resources )
{
    HTMLResource *cover_page = GetCoverPage( html_resources );

    if ( !cover_page )

        return;

    QReadLocker locker( &cover_page->GetLock() );

    QStringList image_paths = XHTMLDoc::GetImagePathsFromImageChildren( cover_page->GetDomDocumentForReading() );

    if ( image_paths.count() == 0 )
         
        return;

    QString first_image_name = QFileInfo( image_paths[ 0 ] ).fileName();

    foreach( ImageResource *image_resource, image_resources )
    {
        if ( image_resource->Filename() == first_image_name )
        {
            image_resource->SetIsCoverImage( true );
            break;
        }
    }
}


bool BookNormalization::IsFlowUnderThreshold( HTMLResource *html_resource, int threshold )
{
    QReadLocker locker( &html_resource->GetLock() );

    QDomElement doc_element = html_resource->GetDomDocumentForReading().documentElement();
    return doc_element.text().count() < threshold;
}

bool BookNormalization::FlowHasOnlyOneImage( HTMLResource* html_resource )
{
    return XHTMLDoc::GetImagePathsFromImageChildren( html_resource->GetDomDocumentForReading() ).count() == 1;
}


