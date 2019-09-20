/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef WELLFORMEDCONTENT_H
#define WELLFORMEDCONTENT_H

#include <QtCore/QString>


class WellFormedContent
{

public:

    /**
     * Destructor.
     */
    virtual ~WellFormedContent() {}

    /**
     * Returns \c true if the data is well-formed. If checking
     * for well-formed errors has been disabled this will always
     * return true. Also, depending on whether the dialog is
     * enabled, notifies the user about the problem.
     *
     * @return \c true if the data is well-formed or checking is
     * disabled.
     */
    virtual bool IsDataWellFormed() = 0;

    /**
     * Fixes the well-formed errors automatically.
     * This may lead to loss of data.
     */
    virtual void AutoFixWellFormedErrors() = 0;

    /**
     * Scrolls the tab to the specified line.
     *
     * @param line The line to scroll to.
     */
    virtual void ScrollToLine(int line) = 0;

    /**
     * Takes control of the UI, making the GUI component
     * central and taking keyboard focus.
     */
    virtual void TakeControlOfUI() = 0;

    /**
     * Returns the name of the file with the XML data.
     *
     * @return The name of the file.
     */
    virtual QString GetFilename() = 0;

    virtual QString GetShortPathName() = 0;
};

#endif // WELLFORMEDCONTENT_H
