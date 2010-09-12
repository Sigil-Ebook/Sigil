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
#include "ImageTab.h"
#include "ResourceObjects/ImageResource.h"
#include "Misc/RasterizeImageResource.h"


ImageTab::ImageTab( Resource& resource, QWidget *parent )
    :
    ContentTab( resource, parent ),
    m_ImageResource( *( qobject_cast< ImageResource* >( &resource ) ) ),
    m_ImageLabel( *new QLabel( this ) ),
    m_ScrollArea( *new QScrollArea( this ) ),
    m_CurrentZoomFactor( 1.0 )
{
    // There are two pairs of parentheses: one calls the constructor,
    // the other calls operator()
    QPixmap pixmap = RasterizeImageResource()( m_ImageResource, m_CurrentZoomFactor );

    m_ImageLabel.setPixmap( pixmap );
    m_ImageLabel.resize( pixmap.size() );
    m_ImageLabel.setFrameStyle( QFrame::Plain | QFrame::StyledPanel );
    m_ImageLabel.setStyleSheet( "border: 1px solid rgb(210, 210, 210)" );
    
    m_ScrollArea.setStyleSheet( "QScrollArea { background: white }" );
    m_ScrollArea.setFrameStyle( QFrame::NoFrame );
    m_ScrollArea.setAlignment( Qt::AlignCenter );
    m_ScrollArea.setWidget( &m_ImageLabel );
    
    m_Layout.addWidget( &m_ScrollArea );
}

float ImageTab::GetZoomFactor() const
{
    return m_CurrentZoomFactor;
}

void ImageTab::SetZoomFactor( float new_zoom_factor )
{
    m_CurrentZoomFactor = new_zoom_factor;

    QPixmap pixmap = RasterizeImageResource()( m_ImageResource, m_CurrentZoomFactor );
    m_ImageLabel.setPixmap( pixmap );
    m_ImageLabel.resize( pixmap.size() );

    emit ZoomFactorChanged( m_CurrentZoomFactor );
}