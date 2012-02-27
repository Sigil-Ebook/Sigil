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
#ifndef RENAMETEMPLATE_H
#define RENAMETEMPLATE_H

#include <QtGui/QDialog>

#include "ui_RenameTemplate.h"

/**
 * Shows general information about Sigil.
 * Information includes things like author, license and website,
 * but also useful debugging information like loaded Qt version,
 * the build time and Sigil version.
 */
class RenameTemplate: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The dialog's parent.
     */
    RenameTemplate( QString &startingName, QWidget *parent = 0 );

    QString GetTemplateName();


private:

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::RenameTemplate ui;
};

#endif // RENAMETEMPLATE_H


