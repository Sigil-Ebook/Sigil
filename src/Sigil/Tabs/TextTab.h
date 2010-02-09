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
#ifndef TextTAB_H
#define TextTAB_H

#include "ContentTab.h"
#include "../ViewEditors/CodeViewEditor.h"

class TextResource;
class QTextDocument;


class TextTab : public ContentTab
{
    Q_OBJECT

public:

    TextTab( Resource& resource, CodeViewEditor::HighlighterType type, QWidget *parent = 0 );

    // Overrides inherited from ContentTab
    virtual bool IsModified();

    virtual bool CutEnabled();

    virtual bool CopyEnabled();

    virtual bool PasteEnabled();

    virtual float GetZoomFactor() const;

    virtual void SetZoomFactor( float new_zoom_factor );

    virtual ViewState GetViewState();

signals:

    void SelectionChanged();

private:

    virtual void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    TextResource &m_TextResource;

    QString m_Source;

    // The plain text code editor 
    CodeViewEditor &m_wCodeView;

};

#endif // TextTAB_H