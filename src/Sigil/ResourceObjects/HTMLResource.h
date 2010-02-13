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
#ifndef HTMLRESOURCE_H
#define HTMLRESOURCE_H

#include "Resource.h"

class QWebPage;
class QString;

class HTMLResource : public Resource 
{
    Q_OBJECT

public:
    
    HTMLResource( const QString &fullfilepath, 
                  QHash< QString, Resource* > *hash_owner,
                  int reading_order,
                  QObject *parent = NULL );

    virtual ResourceType Type() const;

    QWebPage& GetWebPage();

    void SetHtml( const QString &source );

    QString GetHtml();

    void SyncToDisk();

    // The contents of the file on the disk should only
    // be newer right after this resource object is created
    // and before this function is called for the first time
    void SyncFromDisk();

    int GetReadingOrder();

    void SetReadingOrder( int reading_order );
    
    void RemoveWebkitClasses();

private:

    void SetRawHTML( const QString &source );

    QWebPage &m_WebPage;

    int m_ReadingOrder;

    bool m_InitialLoadFromDiskDone;
};

#endif // HTMLRESOURCE_H
