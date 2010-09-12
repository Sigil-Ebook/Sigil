/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#ifndef PERFORMHTMLUPDATES_H
#define PERFORMHTMLUPDATES_H

#include "BookManipulation/XercesHUse.h"
class QString;

class PerformHTMLUpdates
{

public:

    PerformHTMLUpdates( const QString &source,
                        const QHash< QString, QString > &html_updates,
                        const QHash< QString, QString > &css_updates );

    PerformHTMLUpdates( const xc::DOMDocument &document,
                        const QHash< QString, QString > &html_updates,
                        const QHash< QString, QString > &css_updates );

    shared_ptr< xc::DOMDocument > operator()();

private:

    // Updates the resource references in the HTML.
    void UpdateHTMLReferences();

    // Updates the resource references in the attributes 
    // of the one specified node in the HTML.
    void UpdateReferenceInNode( xc::DOMElement *node );

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    shared_ptr< xc::DOMDocument > m_Document;

    const QHash< QString, QString > &m_HTMLUpdates;
    const QHash< QString, QString > &m_CSSUpdates;
};

#endif // PERFORMHTMLUPDATES_H