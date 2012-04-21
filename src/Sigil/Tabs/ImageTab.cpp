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

#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QScrollArea>

#include "Misc/RasterizeImageResource.h"
#include "Misc/SettingsStore.h"
#include "ResourceObjects/ImageResource.h"
#include "Tabs/ImageTab.h"

ImageTab::ImageTab( ImageResource& resource, QWidget *parent )
    :
    ContentTab( resource, parent ),
    m_ImageResource( resource ),
    m_ImageLabel( *new QLabel( this ) ),
    m_ScrollArea( *new QScrollArea( this ) )
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

    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore settings;
    m_CurrentZoomFactor = settings.zoomImage();
    Zoom();
}

float ImageTab::GetZoomFactor() const
{
    SettingsStore settings;
    return settings.zoomImage();
}

void ImageTab::SetZoomFactor( float new_zoom_factor )
{
    // Save the zoom for this type.
    SettingsStore settings;
    settings.setZoomImage(new_zoom_factor);
    m_CurrentZoomFactor = new_zoom_factor;

    Zoom();
    emit ZoomFactorChanged( m_CurrentZoomFactor );
}


void ImageTab::UpdateDisplay()
{
    // Update zoom.
    SettingsStore settings;
    float stored_factor = settings.zoomImage();
    if ( stored_factor != m_CurrentZoomFactor )
    {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
        //emit ZoomFactorChanged( m_CurrentZoomFactor );
    }
}


void ImageTab::Zoom()
{
    QPixmap pixmap = RasterizeImageResource()( m_ImageResource, m_CurrentZoomFactor );
    m_ImageLabel.setPixmap( pixmap );
    m_ImageLabel.resize( pixmap.size() );
}
