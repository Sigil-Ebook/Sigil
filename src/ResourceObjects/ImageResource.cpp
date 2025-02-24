/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QImage>
#include "Misc/Utility.h"
#include "ResourceObjects/ImageResource.h"

ImageResource::ImageResource(const QString &mainfolder, const QString &fullfilepath,
                             QObject *parent)
    :
    Resource(mainfolder, fullfilepath, parent)
{
}


Resource::ResourceType ImageResource::Type() const
{
    return Resource::ImageResourceType;
}

bool ImageResource::LoadFromDisk()
{
    emit ResourceUpdatedOnDisk();
    return true;
}

QString ImageResource::GetDescription() const
{
    const QString path = GetFullPath();
    const QImage img(path);
    QString colors_shades = img.isGrayscale() ? tr("shades") : tr("colors");
    QString grayscale_color = img.isGrayscale() ? tr("Grayscale") : tr("Color");
    QString colorsInfo = "";
    if (img.depth() == 32) {
        colorsInfo = QString(" %1bpp").arg(img.bitPlaneCount());
    } else if (img.depth() > 0) {
        colorsInfo = QString(" %1bpp (%2 %3)").arg(img.bitPlaneCount()).arg(img.colorCount()).arg(colors_shades);
    }
    QString description = QString("(%1px ✕ %2px) %3%4").arg(img.width()).arg(img.height()).arg(grayscale_color).arg(colorsInfo);
    return description;
}
