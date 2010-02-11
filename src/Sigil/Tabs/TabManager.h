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
#ifndef TABMANAGER_H
#define TABMANAGER_H

#include <QTabWidget>
#include <QWeakPointer>
#include <QUrl>

class ContentTab;
class Resource;

class TabManager : public QTabWidget
{
    Q_OBJECT

public:

    TabManager( QWidget *parent = 0 ); 

    ContentTab& GetCurrentContentTab();    

public slots:

    void OpenResource( Resource& resource, const QUrl &fragment= QUrl() );

signals:

    void TabChanged( ContentTab* old_tab, ContentTab* new_tab );

    void OpenUrlRequest( const QUrl &url );

private slots:

    void EmitTabChanged();

    void DeleteTab( ContentTab *tab_to_delete );

    void CloseTab( int tab_index );

private:

    int ResourceTabIndex( const Resource& resource ) const;

    bool SwitchedToExistingTab( Resource& resource, const QUrl &fragment );

    ContentTab* CreateTabForResource( Resource& resource, const QUrl &fragment );

    bool AddNewContentTab( ContentTab *new_tab );

    QWeakPointer< ContentTab > m_LastContentTab;
};

#endif // TABMANAGER_H


