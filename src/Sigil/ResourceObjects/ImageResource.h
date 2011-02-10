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
#ifndef IMAGERESOURCE_H
#define IMAGERESOURCE_H

#include "Resource.h"

class ImageResource : public Resource 
{
    Q_OBJECT

public:
    
    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param semantic_information The cover image information and other
     *                             semantic info in key-value pairs.
     * @param parent The object's parent.
     */
    ImageResource( const QString &fullfilepath,
                   QHash< QString, QString > semantic_information,
                   QObject *parent = NULL );

    // inherited
    virtual ResourceType Type() const;

    /**
     * Sets the cover image status.
     *
     * @param is_cover The new cover image status.
     * @warning Setting one image as a cover does \b not unset
     *          cover image status of other images.
     */
    void SetIsCoverImage( bool is_cover );

    /**
     * Returns the cover image status of this image.
     *
     * @return Cover image status.
     */
    bool IsCoverImage();

private:

    /**
     * The cover image state of this image.
     */
    bool m_IsCoverImage;
};

#endif // IMAGERESOURCE_H
