/************************************************************************
**
**  Copyright (C) 2010  Strahinja Markovic
**
**  This file is part of FlightCrew.
**
**  FlightCrew is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  FlightCrew is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public License
**  along with FlightCrew.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef NODELOCATIONINFO_H
#define NODELOCATIONINFO_H

namespace XercesExt
{

/**
 * Stores line/column number information for DOM nodes.
 * The numbers realte to the location where the node was
 * encountered when its parent document was parsed.
 */
struct NodeLocationInfo
{
    NodeLocationInfo()
        : LineNumber( -1 ), ColumnNumber( -1 ) {}
    NodeLocationInfo( int line_number, int column_number )
        : LineNumber( line_number ), ColumnNumber( column_number ) {}

    int LineNumber;
    int ColumnNumber;
};

}

#endif // NODELOCATIONINFO_H
