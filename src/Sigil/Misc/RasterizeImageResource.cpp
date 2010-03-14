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
#include "RasterizeImageResource.h"
#include "../ResourceObjects/ImageResource.h"
#include "../Misc/Utility.h"

static const QString PAGE_SOURCE =  "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">"
                                    "<head>"
                                    "<meta content=\"text/html; charset=UTF-8\"/>"
                                    "</head>"
                                    "<body style=\"margin: 0;\" >"
                                    "<img src=\"REPLACEME\" />"
                                    "</body>"
                                    "</html>";


RasterizeImageResource::RasterizeImageResource( QWidget *parent )
    :
    QObject( parent ),
    m_WebPage( *new QWebPage( this ) ),
    m_LoadFinishedFlag( false )
{
    connect( m_WebPage.mainFrame(), SIGNAL( loadFinished( bool ) ), this, SLOT( SetLoadFinishedFlag() ) );
}


// We could use the native QPixmap loading process for JPG, PNG and GIF images.
// But we have to use webkit to rasterize SVG's since QSvgRenderer supports
// only SVG Tiny. And if we're going to use webkit, then let's use it for all
// image types to reduce complexity. It also makes zooming/scaling much simpler.
QPixmap RasterizeImageResource::operator()( const ImageResource &resource, float zoom_factor )
{
    QString source( PAGE_SOURCE );
    source.replace( "REPLACEME", Utility::URLEncodePath( resource.Filename() ) );

    m_WebPage.mainFrame()->setHtml( source, resource.GetBaseUrl() );
    m_WebPage.mainFrame()->setZoomFactor( zoom_factor );

    while ( !m_LoadFinishedFlag )
    {
        // Make sure Qt processes events, signals and calls slots
        qApp->processEvents();
    }

    // The flag needs to be reset so the caller
    // can call this func multiple times on the same object
    m_LoadFinishedFlag = false;

    // Now we render the frame onto an image
    m_WebPage.setViewportSize( m_WebPage.mainFrame()->contentsSize() );
    QImage image( m_WebPage.viewportSize(), QImage::Format_ARGB32 );
    QPainter painter( &image );

    m_WebPage.mainFrame()->render( &painter );
    painter.end();

    return QPixmap::fromImage( image );
}


void RasterizeImageResource::SetLoadFinishedFlag()
{
    m_LoadFinishedFlag = true;
}