/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#ifndef AVTAB_H
#define AVTAB_H

#include "Tabs/ContentTab.h"
#include "ResourceObjects/Resource.h"

class QWebView;

class AVTab : public ContentTab
{
    Q_OBJECT

public:
    AVTab(Resource &resource, QWidget *parent=0);

public slots:
    void RefreshContent();

private:
    void ConnectSignalsToSlots();

    QWebView *m_WebView;
};

#endif // IMAGETAB_H
