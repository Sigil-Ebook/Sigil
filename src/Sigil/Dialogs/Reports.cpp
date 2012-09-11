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

#include "QtGui/QMessageBox"
#include <QtGui/QScrollArea>
#include <QtGui/QProgressDialog>

#include "Dialogs/Reports.h"
#include "Misc/SettingsStore.h"
#include "ReportsWidgets/HTMLFilesWidget.h"
#include "ReportsWidgets/ImageFilesWidget.h"
#include "ReportsWidgets/CSSFilesWidget.h"
#include "ReportsWidgets/ClassesInHTMLFilesWidget.h"
#include "ReportsWidgets/StylesInCSSFilesWidget.h"

static const QString SETTINGS_GROUP = "reports_dialog";
static const int NUMBER_OF_REPORTS = 5;

Reports::Reports(QList<Resource*> html_resources,
                 QList<Resource*> image_resources,
                 QList<Resource*> css_resources,
                 QSharedPointer< Book > book,
                 QWidget *parent)
    :
    QDialog(parent),
    m_HTMLResources(html_resources),
    m_ImageResources(image_resources),
    m_CSSResources(css_resources),
    m_Book(book)
{
    // Display progress dialog
    QProgressDialog progress(QObject::tr("Creating reports..."), tr("Cancel"), 0, NUMBER_OF_REPORTS, this);
    progress.setMinimumDuration(1500);
    int progress_value = 0;
    progress.setValue(progress_value++);

    ui.setupUi(this);

    // Create and load all of our report widgets
    appendReportsWidget(new HTMLFilesWidget(m_HTMLResources, m_Book));
    progress.setValue(progress_value++);

    appendReportsWidget(new ImageFilesWidget(m_ImageResources, m_Book));
    progress.setValue(progress_value++);

    appendReportsWidget(new CSSFilesWidget(m_HTMLResources, m_CSSResources, m_Book));
    progress.setValue(progress_value++);

    appendReportsWidget(new StylesInCSSFilesWidget(m_HTMLResources, m_CSSResources, m_Book));
    progress.setValue(progress_value++);

    appendReportsWidget(new ClassesInHTMLFilesWidget(m_HTMLResources, m_CSSResources, m_Book));
    progress.setValue(progress_value++);

    connectSignalsSlots();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    readSettings();
    QApplication::restoreOverrideCursor();
}

void Reports::selectPWidget(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    int index = ui.availableWidgets->row(current);
    ui.pWidget->setCurrentIndex(index);
}

QString Reports::SelectedFile()
{
    return m_SelectedFile;
}

void Reports::saveSettings()
{
    QString widgetResult;

    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    QApplication::setOverrideCursor(Qt::WaitCursor);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("lastreport", ui.availableWidgets->currentRow());

    // Get the selected filename from the currently opened widget
    ReportsWidget *rw = qobject_cast<ReportsWidget*>(ui.pWidget->widget(ui.pWidget->currentIndex()));
    if (rw) {
        widgetResult = rw->saveSettings();
    }

    m_SelectedFile = widgetResult;

    QApplication::restoreOverrideCursor();
}

void Reports::readSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // Ensure the previous item selected in the available reports widgets list
    // is highlighted.
    int last_report_index = settings.value( "lastreport", 0 ).toInt();
    if ( last_report_index > ui.availableWidgets->count() - 1 ) {
        last_report_index = 0;
    }
    ui.availableWidgets->setCurrentRow(last_report_index);

    settings.endGroup();
}

void Reports::appendReportsWidget(ReportsWidget *widget)
{
    // Add the ReportsWidget to the stack view area.
    ui.pWidget->addWidget(widget);

    // Add an entry to the list of available reports widgets.
    ui.availableWidgets->addItem(widget->windowTitle());
}

void Reports::connectSignalsSlots()
{
    connect(ui.availableWidgets, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selectPWidget(QListWidgetItem*, QListWidgetItem*)));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveSettings()));
}
