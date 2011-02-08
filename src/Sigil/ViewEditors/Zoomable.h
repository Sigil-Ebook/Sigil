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
#ifndef ZOOMABLE_H
#define ZOOMABLE_H

/**
 * An interface for widgets that can be zoomed.
 */
class Zoomable
{

public:

    /**
     * Sets a zoom factor for the view.
     * Thus, zooms in (factor > 1.0) or out (factor < 1.0).
     *
     * @param factor The zoom factor to use. 
     */
    virtual void SetZoomFactor( float factor ) = 0;

    /**
     * Returns the widget's current zoom factor.
     *
     * @return The current zoom factor.
     */
    virtual float GetZoomFactor() const = 0;
};

#endif // ZOOMABLE_H