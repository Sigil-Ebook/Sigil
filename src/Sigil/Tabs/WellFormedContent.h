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
#ifndef WELLFORMEDCONTENT_H
#define WELLFORMEDCONTENT_H

#include <QString>


class WellFormedContent 
{

public:

    /**
     * Destructor.
     */ 
    virtual ~WellFormedContent() {}

    /**
     * Gets whether checking for well-formed errors is enabled.
     */
    virtual bool GetCheckWellFormedErrors() = 0;
    
    /**
     * Turns on/off the dialog responsible for notifying the user
     * about well-formed errors.
     *
     * @param enabled If \c true, the dialog is enabled.
     */
    virtual void SetWellFormedDialogsEnabledState( bool enabled ) = 0;

    /**
     * Turns on/off checking for well-formed errors. The state
     * will be used in IsDataWellFormed.
     *
     * @param enabled If \true, the content will be checked for
     * well-formed errors.
     */
    virtual void SetCheckWellFormedErrorsState( bool enabled ) = 0;

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
    virtual void ScrollToLine( int line ) = 0;

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
};

#endif // WELLFORMEDCONTENT_H
