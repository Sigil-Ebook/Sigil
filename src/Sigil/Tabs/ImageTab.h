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

#pragma once
#ifndef IMAGETAB_H
#define IMAGETAB_H

#include "ContentTab.h"

class ImageResource;
class QWebView;
class QLabel;
class QScrollArea;
class QPixmap;

class ImageTab : public ContentTab
{
    Q_OBJECT

public:

    ImageTab( ImageResource& resource, QWidget *parent = 0 );

    // Overrides inherited from ContentTab
    float GetZoomFactor() const;

    void SetZoomFactor( float new_zoom_factor );

    void UpdateDisplay();

private:

    void Zoom();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    ImageResource &m_ImageResource;

    QLabel &m_ImageLabel;

    QScrollArea &m_ScrollArea;

    float m_CurrentZoomFactor;
};

#endif // IMAGETAB_H
