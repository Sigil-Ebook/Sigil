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
#ifndef OPFTAB_H
#define OPFTAB_H

#include "MiscEditors/ClipEditorModel.h"
#include "Tabs/XMLTab.h"

class OPFResource;

class OPFTab : public XMLTab
{
    Q_OBJECT

public:

    OPFTab(OPFResource *resource, int line_to_scroll_to = -1, QWidget *parent = 0);

    void AutoFixWellFormedErrors();

private:

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    OPFResource *m_OPFResource;

};

#endif // OPFTAB_H
