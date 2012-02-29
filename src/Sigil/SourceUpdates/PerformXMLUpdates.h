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
#ifndef PERFORMXMLUPDATES_H
#define PERFORMXMLUPDATES_H

#include <boost/shared_ptr.hpp>

#include <QtCore/QHash>
#include <QtCore/QStringList>

#include "BookManipulation/XercesHUse.h"

class QString;
class QStringList;

using boost::shared_ptr;

/**
 * Performs path updates on XML documents.
 */
class PerformXMLUpdates
{

public:

    /**
     * Constructor.
     *
     * @param source The raw text source of the XML file to update.
     * @param xml_updates The path updates.
     */
    PerformXMLUpdates( const QString &source,
                       const QHash< QString, QString > &xml_updates );
    
    /**
     * Constructor.
     *
     * @param document The already loaded XML document to update.
     * @param xml_updates The path updates.
     */
    PerformXMLUpdates( const xc::DOMDocument &document,
                       const QHash< QString, QString > &xml_updates );

    /**
     * Performs the updates.
     *
     * @return The updated DOM of the provided XML file.
     */
    virtual shared_ptr< xc::DOMDocument > operator()();

protected:

    /**
     * Updates the resource references in the XML.
     */
    void UpdateXMLReferences();

    /**
     * Holds all the tags with paths that should be looked at during
     * path updates.
     *
     * @note DON'T FORGET TO INITIALIZE THIS IN A SUBCLASS!
     *       IT'S EMPTY BY DEFAULT!
     */
    QStringList m_PathTags;

    /**
     * Holds all the attributes of path tags that should be looked at
     * during path updates.
     */
    QStringList m_PathAttributes;

    /**
     * The parsed DOM document
     */
    shared_ptr< xc::DOMDocument > m_Document;

private:

    /**
     * Updates the resource references in the attributes 
     * of the one specified node in the XML.
     */
    void UpdateReferenceInNode( xc::DOMElement *node );

    /**
     * Initializes the m_PathAttributes variable.
     */
    void InitPathAttributes();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The updates that need to be performed. Keys are old paths,
     * values are new paths.
     */
    const QHash< QString, QString > &m_XMLUpdates;
};

#endif // PERFORMXMLUPDATES_H
