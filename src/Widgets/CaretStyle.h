/************************************************************************
**
**  Copyright (C) 2024 Kevin B. Hendricks, Stratford, Ontario Canada
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
#ifndef CARETSTYLE_H
#define CARETSTYLE_H

#include <QProxyStyle>
#include <QCommonStyle>

class QStyle;
class QStyleOption;

class CaretStyle : public QProxyStyle
{
public:
    CaretStyle(QStyle *style = 0) : QProxyStyle(style) { }

    int pixelMetric(QCommonStyle::PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

};

#endif // CARETSTYLE_H
