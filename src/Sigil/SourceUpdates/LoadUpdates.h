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
#ifndef LOADUPDATES_H
#define LOADUPDATES_H

#include <QMutex>
#include <QFutureSynchronizer>

class QString;
class QDomNode;

class LoadUpdates
{

public:

    LoadUpdates( const QString &source, const QHash< QString, QString > &updates );

    QString operator()( );

private:

    // Updates the resource references in the HTML.
    void UpdateHTMLReferences();

    // Updates the resource references in the attributes 
    // of the one specified node in the HTML.
    void UpdateReferenceInNode( QDomNode node );

    // Updates the resource references in the CSS.
    void UpdateCSSReferences();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    QString m_Source;

    QHash< QString, QString > m_HTMLUpdates;

    QHash< QString, QString > m_CSSUpdates;

    // This synchronizer is used to wait for all
    // the HTML node updates to finish before moving on
    QFutureSynchronizer< void > m_NodeUpdateSynchronizer;

    // The mutex used for locking access
    // to the QFuture synchronizer
    QMutex m_SynchronizerMutex;
};

#endif // LOADUPDATES_H