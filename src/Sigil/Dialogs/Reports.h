/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Dave Heiland
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
#ifndef REPORTS_H
#define REPORTS_H

#include <QtGui/QDialog>
#include <QHash>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h" 
#include "ReportsWidgets/ReportsWidget.h"

#include "ui_Reports.h"

/**
 *
 * The reports are exposed are instances of ReportsWidget. They are loaded
 * and dynamically displayed based upon which one is seleted.
 */
class Reports : public QDialog
{
    Q_OBJECT

public:
    Reports( QList<Resource*> html_resources,
             QList<Resource*> image_resources,
             QList<Resource*> css_resources,
             QSharedPointer<Book> book,
             QWidget *parent = 0);

    QString SelectedFile();

    int SelectedFileLine();

    QStringList FilesToDelete();
    QHash< QString, QList<CSSInfo::CSSSelector*> > StylesToDelete();

private slots:
    /**
     * Load the ReportsWidget that the user has selected.
     */
    void selectPWidget(QListWidgetItem *current, QListWidgetItem *previous=0);
    /**
     * Saves settings the user has selected.
     *
     * Saves the state of the dialog.
     * Also, calls saveSettings for each loaded ReportsWidget.
     */
    void saveSettings();

private:
    /**
     * Read settings for the dialog.
     *
     * Each ReportsWidget upon creation will load it's own settings.
     */
    void readSettings();
    /**
     * Adds the given items to the dialog.
     *
     * The widget is added to the list of available widgets and when the
     * entry in the list is selected the widget it shown in the widget display
     * area to the right of the avaliable widget list.
     *
     * @param widget The ReportsWidget to add to the dialog.
     */
    void appendReportsWidget(ReportsWidget *widget);
    /**
     * Connect signals to slots used by this dialog.
     */
    void connectSignalsSlots();

    QList<Resource*> m_HTMLResources;
    QList<Resource*> m_ImageResources;
    QList<Resource*> m_CSSResources;

    QSharedPointer< Book > m_Book;

    QString m_SelectedFile;

    int m_SelectedFileLine;

    QStringList m_FilesToDelete;
    QHash<QString, QList<CSSInfo::CSSSelector *> > m_StylesToDelete;

    Ui::Reports ui;
};

#endif // REPORTS_H
