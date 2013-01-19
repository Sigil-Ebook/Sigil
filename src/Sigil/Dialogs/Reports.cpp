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

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QProgressDialog>

#include "Dialogs/Reports.h"
#include "Misc/SettingsStore.h"
#include "Misc/CSSInfo.h"
#include "ReportsWidgets/AllFilesWidget.h"
#include "ReportsWidgets/HTMLFilesWidget.h"
#include "ReportsWidgets/LinksWidget.h"
#include "ReportsWidgets/ImageFilesWidget.h"
#include "ReportsWidgets/CSSFilesWidget.h"
#include "ReportsWidgets/ClassesInHTMLFilesWidget.h"
#include "ReportsWidgets/StylesInCSSFilesWidget.h"

static const QString SETTINGS_GROUP = "reports_dialog";

Reports::Reports(QWidget *parent)
    : QDialog(parent),
      m_AllFilesWidget(new AllFilesWidget()),
      m_HTMLFilesWidget(new HTMLFilesWidget()),
      m_LinksWidget(new LinksWidget()),
      m_ImageFilesWidget(new ImageFilesWidget()),
      m_CSSFilesWidget(new CSSFilesWidget()),
      m_ClassesInHTMLFilesWidget(new ClassesInHTMLFilesWidget()),
      m_StylesInCSSFilesWidget(new StylesInCSSFilesWidget())
{
    ui.setupUi(this);

    appendReportsWidget(m_AllFilesWidget);
    appendReportsWidget(m_HTMLFilesWidget);
    appendReportsWidget(m_ImageFilesWidget);
    appendReportsWidget(m_CSSFilesWidget);
    appendReportsWidget(m_ClassesInHTMLFilesWidget);
    appendReportsWidget(m_StylesInCSSFilesWidget);
    appendReportsWidget(m_LinksWidget);

    connectSignalsSlots();
    readSettings();
    ui.Refresh->setFocus();
}

Reports::~Reports()
{
    if (m_AllFilesWidget) {
        delete m_AllFilesWidget;
        m_AllFilesWidget = 0;
    }

    if (m_HTMLFilesWidget) {
        delete m_HTMLFilesWidget;
        m_HTMLFilesWidget = 0;
    }

    if (m_LinksWidget) {
        delete m_LinksWidget;
        m_LinksWidget = 0;
    }

    if (m_ImageFilesWidget) {
        delete m_ImageFilesWidget;
        m_ImageFilesWidget = 0;
    }

    if (m_CSSFilesWidget) {
        delete m_CSSFilesWidget;
        m_CSSFilesWidget = 0;
    }

    if (m_ClassesInHTMLFilesWidget) {
        delete m_ClassesInHTMLFilesWidget;
        m_ClassesInHTMLFilesWidget = 0;
    }

    if (m_StylesInCSSFilesWidget) {
        delete m_StylesInCSSFilesWidget;
        m_StylesInCSSFilesWidget = 0;
    }
}

void Reports::CreateReports(QSharedPointer< Book > book)
{
    m_Book = book;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Display progress dialog
    QProgressDialog progress(QObject::tr("Creating reports..."), 0, 0, ui.availableWidgets->count(), this);
    progress.setMinimumDuration(0);
    int progress_value = 0;
    progress.setValue(progress_value++);
    qApp->processEvents();
    // Populate all of our report widgets
    m_AllFilesWidget->CreateReport(m_Book);
    progress.setValue(progress_value++);
    qApp->processEvents();
    m_HTMLFilesWidget->CreateReport(m_Book);
    progress.setValue(progress_value++);
    qApp->processEvents();
    m_LinksWidget->CreateReport(m_Book);
    progress.setValue(progress_value++);
    qApp->processEvents();
    m_ImageFilesWidget->CreateReport(m_Book);
    progress.setValue(progress_value++);
    qApp->processEvents();
    m_CSSFilesWidget->CreateReport(m_Book);
    progress.setValue(progress_value++);
    qApp->processEvents();
    m_ClassesInHTMLFilesWidget->CreateReport(m_Book);
    progress.setValue(progress_value++);
    qApp->processEvents();
    m_StylesInCSSFilesWidget->CreateReport(m_Book);
    progress.setValue(progress_value++);
    qApp->processEvents();
    QApplication::restoreOverrideCursor();
}

void Reports::selectPWidget(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    int index = ui.availableWidgets->row(current);
    ui.pWidget->setCurrentIndex(index);
}

void Reports::saveSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("lastreport", ui.availableWidgets->currentRow());
    QApplication::restoreOverrideCursor();
}

void Reports::readSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // Ensure the previous item selected in the available reports widgets list
    // is highlighted.
    int last_report_index = settings.value("lastreport", 0).toInt();

    if (last_report_index > ui.availableWidgets->count() - 1) {
        last_report_index = 0;
    }

    ui.availableWidgets->setCurrentRow(last_report_index);
    settings.endGroup();
}

void Reports::appendReportsWidget(ReportsWidget *widget)
{
    // Add the ReportsWidget to the stack view area.
    ui.pWidget->addWidget(widget);
    connect(widget, SIGNAL(OpenFileRequest(QString, int)), this, SIGNAL(OpenFileRequest(QString, int)));
    connect(widget, SIGNAL(CloseDialog()), this, SLOT(accept()));
    // Add an entry to the list of available reports widgets.
    ui.availableWidgets->addItem(widget->windowTitle());
}

void Reports::connectSignalsSlots()
{
    connect(m_HTMLFilesWidget, SIGNAL(DeleteFilesRequest(QStringList)), this, SIGNAL(DeleteFilesRequest(QStringList)));
    connect(m_ImageFilesWidget, SIGNAL(DeleteFilesRequest(QStringList)), this, SIGNAL(DeleteFilesRequest(QStringList)));
    connect(m_CSSFilesWidget, SIGNAL(DeleteFilesRequest(QStringList)), this, SIGNAL(DeleteFilesRequest(QStringList)));
    connect(m_StylesInCSSFilesWidget, SIGNAL(DeleteStylesRequest(QList<BookReports::StyleData *>)), this, SIGNAL(DeleteStylesRequest(QList<BookReports::StyleData *>)));

    connect(ui.availableWidgets, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(selectPWidget(QListWidgetItem *, QListWidgetItem *)));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveSettings()));
    connect(ui.Refresh, SIGNAL(clicked()), this, SIGNAL(Refresh()));
}
