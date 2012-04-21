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
#ifndef ABOUT_H
#define ABOUT_H

#include <QtCore/QDateTime>
#include <QtGui/QDialog>

#include "ui_About.h"

/**
 * Shows general information about Sigil.
 * Information includes things like author, license and website,
 * but also useful debugging information like loaded Qt version,
 * the build time and Sigil version.
 */
class About : public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The dialog's parent.
     */
    About( QWidget *parent = 0 );

private:

    /**
     * Returns the time that Sigil was built in UTC.
     * 
     * @return The build time in UTC.
     */
    static QDateTime GetUTCBuildTime();

    /**
     * Converts a three letter string like "Jun"
     * into that month's index.
     *
     * @param three_letter_string The month string.
     * @return The index of that month, 1-12.
     */
    static int MonthIndexFromString( const QString& three_letter_string );

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::About ui;
};

#endif // ABOUT_H


