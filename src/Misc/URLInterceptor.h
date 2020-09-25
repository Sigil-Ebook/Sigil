/************************************************************************
**
**  Copyright (C) 2020  Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef URLINTERCEPTOR_H
#define URLINTERCEPTOR_H

#include <QWebEngineUrlRequestInterceptor>

class URLInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    explicit URLInterceptor(QObject* parent = Q_NULLPTR);

    void interceptRequest(QWebEngineUrlRequestInfo &info) Q_DECL_OVERRIDE;

};

#endif // URLINTERCEPTOR_H
