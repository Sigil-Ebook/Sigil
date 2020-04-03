/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks, Stratford, Ontario Canada
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
#ifndef FONTTAB_H
#define FONTTAB_H

#include "Tabs/ContentTab.h"
#include "ResourceObjects/Resource.h"

class FontView;

class FontTab : public ContentTab
{
    Q_OBJECT

public:
    FontTab(Resource *resource, QWidget *parent=0);

public slots:
    void ShowFont();
    void RefreshContent();

    // dummy implementations for signals
    void Undo() { };
    void Redo() { };
    void Cut() { };
    void Copy() { };
    void Paste() { };
    void DeleteLine() { };
    void PrintPreview() { };
    void Print() { };

private:
    void ConnectSignalsToSlots();
    FontView *m_fv;
};

#endif // FONTTAB_H
