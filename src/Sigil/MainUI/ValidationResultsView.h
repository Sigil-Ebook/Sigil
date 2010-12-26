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
#ifndef VALIDATIONRESULTSVIEW_H
#define VALIDATIONRESULTSVIEW_H

#include <QDockWidget>
#include <vector>

namespace FlightCrew { class Result; };
namespace fc = FlightCrew;

class QTableWidget;

/**
 * Represents the pane in which all the validation results are displayed.
 */
class ValidationResultsView : public QDockWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The QObject's parent.
     */
    ValidationResultsView( QWidget *parent = 0 );

    /**
     * Validates the epub file given and displays the results.
     */
    void ValidateEpub( const QString &filepath );

    /**
     * Clears the result table.
     */
    void ClearResults();

private:

    /**
     * Sets up the table widget to our liking.
     */
    void SetUpTable();

    /**
     * Displays the given results in the widget's table.
     *
     * @param results A list of FlightCrew validation results.
     */
    void DisplayResults( const std::vector< fc::Result > &results );

    /**
     * Informs the user that no problems were found.
     */
    void DisplayNoProblemsMessage();

    /**
     * Configures the table for presenting validation results.
     */
    void ConfigureTableForResults();

    /**
     * Removes the epub filename prefix in the path, if any.
     *
     * @param path The relative path to clean.
     * @return The cleaned path.
     */
    static QString RemoveEpubPathPrefix( const QString &path );

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The table that holds all the validation results.
     */
    QTableWidget &m_ResultTable;
};

#endif // VALIDATIONRESULTSVIEW_H


