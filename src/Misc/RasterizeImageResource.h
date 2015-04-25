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
#ifndef RASTERIZEIMAGERESOURCE_H
#define RASTERIZEIMAGERESOURCE_H

#include <QtCore/QObject>

class QWebPage;
class QPixmap;
class ImageResource;


class RasterizeImageResource : public QObject
{
    Q_OBJECT

public:

    RasterizeImageResource(QWidget *parent = 0);
    ~RasterizeImageResource();

    QPixmap operator()(const ImageResource &resource, float zoom_factor);

private slots:

    void SetLoadFinishedFlag();

private:

    QWebPage *m_WebPage;

    bool m_LoadFinishedFlag;

};

#endif // RASTERIZEIMAGERESOURCE_H
