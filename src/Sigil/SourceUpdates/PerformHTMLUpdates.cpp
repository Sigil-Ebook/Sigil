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

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "SourceUpdates/PerformCSSUpdates.h"
#include "SourceUpdates/PerformHTMLUpdates.h"


PerformHTMLUpdates::PerformHTMLUpdates( const QString &source,
                                        const QHash< QString, QString > &html_updates,
                                        const QHash< QString, QString > &css_updates )
    :
    PerformXMLUpdates( source, html_updates ),
    m_CSSUpdates( css_updates )
{
    InitPathTags();
}


PerformHTMLUpdates::PerformHTMLUpdates( const xc::DOMDocument &document, 
                                        const QHash< QString, QString > &html_updates, 
                                        const QHash< QString, QString > &css_updates )
    : 
    PerformXMLUpdates( document, html_updates ),
    m_CSSUpdates( css_updates )
{
    InitPathTags();
}


shared_ptr< xc::DOMDocument > PerformHTMLUpdates::operator()()
{
    UpdateXMLReferences();

    if ( !m_CSSUpdates.isEmpty() )
    {
        m_Document = XhtmlDoc::LoadTextIntoDocument( 
            PerformCSSUpdates( XhtmlDoc::GetDomDocumentAsString( *m_Document ), m_CSSUpdates )() );
    }

    return m_Document;
}


void PerformHTMLUpdates::InitPathTags()
{
    // We look at a different set of tags
    // This is the list of tags whose contents will be scanned for file references
    // that need to be updated.
    m_PathTags = QStringList() << "link" << "a" << "img" << "image" << "script";
}
