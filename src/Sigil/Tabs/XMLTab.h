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
#ifndef XMLTAB_H
#define XMLTAB_H

#include "TextTab.h"
#include "WellFormedContent.h"

class XMLResource;
class WellFormedCheckComponent;


class XMLTab : public TextTab, public WellFormedContent
{
    Q_OBJECT

public:

    XMLTab( XMLResource& resource,
            int line_to_scroll_to = -1,
            QWidget *parent = 0 );

    ~XMLTab();

    void ScrollToLine( int line );

    virtual void AutoFixWellFormedErrors();

    void TakeControlOfUI();

    QString GetFilename();

    bool GetCheckWellFormedErrors();

public slots:

    void SetWellFormedDialogsEnabledState( bool enabled );

    void SetCheckWellFormedErrorsState( bool enabled );
    
    bool IsDataWellFormed();

private:

    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    XMLResource &m_XMLResource;

    /**
     * The component used to display a dialog about 
     * well-formedness errors.
     */
    WellFormedCheckComponent& m_WellFormedCheckComponent;
};

#endif // XMLTAB_H
