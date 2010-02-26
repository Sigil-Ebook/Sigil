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
#ifndef ANCHORUPDATES_H
#define ANCHORUPDATES_H

class HTMLResource;

class AnchorUpdates
{

public:

    static void UpdateAllAnchors( const QList< HTMLResource* > &html_resources );

private:

    static QHash< QString, QString > GetIDLocations( const QList< HTMLResource* > &html_resources );

    static tuple< QString, QList< QString > > GetOneFileIDs( HTMLResource* html_resource );

    static void UpdateAnchorsInOneFile( HTMLResource *html_resource, 
                                        const QHash< QString, QString > ID_locations );  
};

#endif // ANCHORUPDATES_H