/************************************************************************
**
**  Copyright (C) 2016-2021 Kevin B. Hendricks Stratford, ON, Canada 
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
#include "BookManipulation/Book.h"
#include "BookManipulation/Headings.h"
#include "ResourceObjects/HTMLResource.h"
#include "Parsers/GumboInterface.h"
#include "MainUI/TOCModel.h"

struct NavTOCEntry {
    int lvl;
    QString title;
    QString href; // hrefs must be stored in URLEncoded form since fragments may be present
    // allow for either flat or hierarchical use
    QList<NavTOCEntry> children;
};

struct NavPageListEntry {
    QString pagename;
    QString href; // hrefs must be stored in URLEncoded form since fragments may be present
};

struct NavLandmarkEntry {
    QString etype;
    QString title;
    QString href; // hrefs must be stored in URLEncoded form since fragments may be present
};

class NavProcessor
{
public:
    NavProcessor(HTMLResource * nav_resource);
    ~NavProcessor();

    QList<NavTOCEntry> GetTOC();
    QList<NavLandmarkEntry> GetLandmarks();
    QList<NavPageListEntry> GetPageList();

    // Set Nav Section from Actual Book Headings
    bool GenerateTOCFromBookContents(const Book* book);

    // Set Nav Section from TOCEntry Tree
    void GenerateNavTOCFromTOCEntries(const TOCModel::TOCEntry& root);

    // Get current Nav as TOCEntry Tree
    TOCModel::TOCEntry GetRootTOCEntry();

    // For Working with Landmarks
    void AddLandmarkCode(const Resource * resource, QString new_code, bool toggle = true);
    void RemoveLandmarkForResource(const Resource * resource);
    QString GetLandmarkCodeForResource(const Resource * resource);
    QString GetLandmarkNameForResource(const Resource * resource);
    QHash<QString, QString> GetLandmarkCodeForPaths();
    QHash<QString, QString> GetLandmarkNameForPaths();


private:    
    QString BuildTOC(const QList<NavTOCEntry> & toclist);
    QString BuildLandmarks(const QList<NavLandmarkEntry> & landlist);
    QString BuildPageList(const QList<NavPageListEntry> & pagelist);
    
    void SetTOC(const QList<NavTOCEntry> & toclist);
    void SetLandmarks(const QList<NavLandmarkEntry> & landlist);
    void SetPageList(const QList<NavPageListEntry> & pagelist);

    int GetResourceLandmarkPos(const Resource * resource, const QList<NavLandmarkEntry> & landlist);
    QList<NavTOCEntry> GetNodeTOC(GumboInterface & gi, const GumboNode* node, int lvl);
    QList<NavTOCEntry> HeadingWalker(const Headings::Heading & heading, int lvl);

    void AddTOCEntry(const NavTOCEntry & nav_entry, TOCModel::TOCEntry & parent);

    QList<NavTOCEntry> MakeHierarchy(const QList<NavTOCEntry> & toclist);
    void AddChildEntry(NavTOCEntry &parent, NavTOCEntry new_child);
    
    QList<NavTOCEntry> AddEditTOCEntry(TOCModel::TOCEntry & rentry, int lvl);

    QString ConvertHREFToBookPath(const QString & href);
    QString ConvertBookPathToNavRelative(const QString & href);
    
    HTMLResource * m_NavResource;
    QString m_language;
};
#endif  // NAVPROCESSORH
