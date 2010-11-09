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
#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * Responsible for checking the current online version of
 * FlightCrew against the running one. If a newer version
 * exists, then a dialog is displayed informing the user
 * about it.
 *
 * Objects of this class should ALWAYS be created on the heap
 * and never explicitly deleted. The reason is that these objects
 * receive replies asynchronously from the web and need to persist.
 * Upon receiving and processing the network reply, the object
 * schedules its own deletion.
 */
class UpdateChecker : QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The object's parent.
     */
    UpdateChecker( QObject *parent );

    /**
     * Sends a request for the online version
     * if the last check was performed
     * a SECONDS_BETWEEN_CHECKS amount of time ago.
     */
    void CheckForUpdate();

private slots:

    /**
     * Gets called when the request posted by CheckForUpdate() 
     * gets a reply from the server.
     *
     * @param reply The network reply.
     */
    void ReplyRecieved( QNetworkReply* reply );

private:

    /**
     * Extracts the full text present in the network reply.
     *
     * @param reply The reply from which text should be extracted.
     * @return The full text content.
     */
    QString TextInReply( QNetworkReply* reply ) const;

    /**
     * Returns the version string present 
     * in the specified XML file, or an empty QString
     * if the required element is not present.
     *
     * @param online_version_xml The xml content from the reply.
     * @return The version string.
     */
    QString ReadOnlineVersion( const QString &online_version_xml ) const;

    /**
     * Compares the two provided version strings.
     *
     * @param current_version_string The version of the running program.
     * @param online_version_string The newest version.
     * @return \c true if the online string specifies
     *         that the online version is newer.
     */
    bool IsOnlineVersionNewer( const QString &current_version_string, 
                               const QString &online_version_string ) const;


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The network manager used to post 
     * network requests and receive replies.
     */ 
    QNetworkAccessManager *m_NetworkManager;

};

#endif // UPDATECHECKER_H