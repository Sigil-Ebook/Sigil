/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks Stratford, ON, Canada 
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
#ifndef NAVPROCESSORH
#define NAVPROCESSORH

#include <QString>
#include <QList>
#include "ResourceObjects/HTMLResource.h"
#include "Misc/GumboInterface.h"

struct NavTOCEntry {
    int lvl;
    QString title;
    QString href;
};

struct NavPageListEntry {
    QString pagename;
    QString href;
};

struct NavLandmarkEntry {
    QString etype;
    QString title;
    QString href;
};

class NavProcessor
{
public:
    NavProcessor(HTMLResource * nav_resource);
    ~NavProcessor();

    QList<NavTOCEntry> GetTOC();
    QList<NavPageListEntry> GetPageList();
    QList<NavLandmarkEntry> GetLandmarks();
    
    QString BuildTOC(const QList<NavTOCEntry> & toclist);
    QString BuildLandmarks(const QList<NavLandmarkEntry> & landlist);
    QString BuildPageList(const QList<NavPageListEntry> & pagelist);
    
    void SetPageList(const QList<NavPageListEntry> & pagelist);
    void SetTOC(const QList<NavTOCEntry> & toclist);
    void SetLandmarks(const QList<NavLandmarkEntry> & landlist);

private:
    QList<NavTOCEntry> GetNodeTOC(GumboInterface gi, const GumboNode* node, int lvl);
    HTMLResource * m_NavResource;
};
#endif  // NAVPROCESSORH
