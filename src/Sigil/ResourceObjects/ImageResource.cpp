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
#include "ImageResource.h"
#include "../Misc/Utility.h"

ImageResource::ImageResource( const QString &fullfilepath,
                              QHash< QString, Resource* > *hash_owner,
                              QHash< QString, QString > semantic_information,
                              QObject *parent )
    : 
    Resource( fullfilepath, hash_owner, parent ),
    m_IsCoverImage( false )  
{
    // There should only be one entry in the hash for ImageResources,
    // and entry merely states that the image is a cover image
    if ( semantic_information.keys().count() == 1 )
    {
        m_IsCoverImage = true;
    }
}


Resource::ResourceType ImageResource::Type() const
{
    return Resource::ImageResource;
}


bool ImageResource::IsCoverImage()
{
    return m_IsCoverImage;
}


void ImageResource::SetIsCoverImage( bool is_cover )
{
    m_IsCoverImage = is_cover;
}