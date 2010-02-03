/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#ifndef CSSTAB_H
#define CSSTAB_H

#include "ContentTab.h"

class CodeViewEditor;
class CSSResource;
class QTextDocument;


class CSSTab : public ContentTab
{
    Q_OBJECT

public:

    CSSTab( Resource& resource, QWidget *parent = 0 );

    // Overrides inherited from ContentTab
    bool IsModified();

    bool CutEnabled();

    bool CopyEnabled();

    bool PasteEnabled();

    float GetZoomFactor();

    void SetZoomFactor( float new_zoom_factor );

signals:

    void SelectionChanged();

private:

    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    CSSResource &m_CSSResource;

    //QTextDocument &m_TextDocument;

    QString m_Source;

    // The plain text code editor 
    CodeViewEditor &m_wCodeView;

};

#endif // FLOWTAB_H