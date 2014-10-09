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

#include <flightcrew.h>

#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QRegularExpression>

#include "BookManipulation/FolderKeeper.h"
#include "MainUI/ValidationResultsView.h"
#include "Misc/Utility.h"
#include "sigil_exception.h"

namespace fc = FlightCrew;

static const QBrush WARNING_BRUSH = QBrush(QColor(255, 255, 230));
static const QBrush ERROR_BRUSH   = QBrush(QColor(255, 230, 230));


ValidationResultsView::ValidationResultsView(QWidget *parent)
    :
    QDockWidget(tr("Validation Results"), parent),
    m_ResultTable(*new QTableWidget(this))
{
    setWidget(&m_ResultTable);
    setAllowedAreas(Qt::BottomDockWidgetArea);
    SetUpTable();
    connect(&m_ResultTable, SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
            this,           SLOT(ResultDoubleClicked(QTableWidgetItem *)));
}

void ValidationResultsView::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    raise();
}

void ValidationResultsView::ValidateCurrentBook()
{
    ClearResults();
    std::vector<fc::Result> results;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_Book->SaveAllResourcesToDisk();

    try {
        results = fc::ValidateEpubRootFolder(
                      m_Book->GetFolderKeeper().GetFullPathToMainFolder().toUtf8().constData());
    } catch (std::exception &exception) {
        // TODO: extract boost exception info
        Utility::DisplayStdErrorDialog(
            tr("An exception occurred during validation: %1.")
            .arg(QString::fromStdString(exception.what()))
        );
        return;
    }

    QApplication::restoreOverrideCursor();
    DisplayResults(results);
    show();
    raise();
}


void ValidationResultsView::ClearResults()
{
    m_ResultTable.clearContents();
    m_ResultTable.setRowCount(0);
}


void ValidationResultsView::SetBook(QSharedPointer<Book> book)
{
    m_Book = book;
    ClearResults();
}


void ValidationResultsView::ResultDoubleClicked(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    int row = item->row();
    QTableWidgetItem *path_item = m_ResultTable.item(row, 0);

    if (!path_item) {
        return;
    }

    QString filename = QFileInfo(path_item->text()).fileName();
    QTableWidgetItem *line_item = m_ResultTable.item(row, 1);

    if (!line_item) {
        return;
    }

    int line = line_item->text().toInt();

    try {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename(filename);
        emit OpenResourceRequest(resource, line, -1, QString(), MainWindow::ViewState_CodeView);
    } catch (ResourceDoesNotExist &) {
        return;
    }
}


void ValidationResultsView::SetUpTable()
{
    m_ResultTable.setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ResultTable.setTabKeyNavigation(false);
    m_ResultTable.setDropIndicatorShown(false);
    m_ResultTable.horizontalHeader()->setStretchLastSection(true);
    m_ResultTable.verticalHeader()->setVisible(false);
}


void ValidationResultsView::DisplayResults(const std::vector<fc::Result> &results)
{
    m_ResultTable.clear();

    if (results.empty()) {
        DisplayNoProblemsMessage();
        return;
    }

    ConfigureTableForResults();

    for (unsigned int i = 0; i < results.size(); ++i) {
        fc::Result result = results[ i ];
        m_ResultTable.insertRow(m_ResultTable.rowCount());
        QBrush row_brush = result.GetResultType() == fc::ResultType_WARNING ?
                           WARNING_BRUSH                                    :
                           ERROR_BRUSH;
        QTableWidgetItem *item = NULL;
        QString path = QString::fromUtf8(result.GetFilepath().c_str());
        item = new QTableWidgetItem(RemoveEpubPathPrefix(path));
        item->setBackground(row_brush);
        m_ResultTable.setItem(i, 0, item);
        item = result.GetErrorLine() > 0                                        ?
               new QTableWidgetItem(QString::number(result.GetErrorLine())) :
               new QTableWidgetItem(tr("N/A"));
        item->setBackground(row_brush);
        m_ResultTable.setItem(i, 1, item);
        item = new QTableWidgetItem(QString::fromUtf8(result.GetMessage().c_str()));
        item->setBackground(row_brush);
        m_ResultTable.setItem(i, 2, item);
    }

    // We first force the line number column
    // to the smallest needed size...
    m_ResultTable.resizeColumnToContents(0);
    // ... and now the file column can be widened.
    m_ResultTable.resizeColumnToContents(1);
}


void ValidationResultsView::DisplayNoProblemsMessage()
{
    m_ResultTable.setRowCount(1);
    m_ResultTable.setColumnCount(1);
    m_ResultTable.setHorizontalHeaderLabels(
        QStringList() << tr("Message"));
    QTableWidgetItem *item = new QTableWidgetItem(tr("No problems found!"));
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QFont font = item->font();
    font.setPointSize(16);
    item->setFont(font);
    m_ResultTable.setItem(0, 0, item);
    m_ResultTable.resizeRowToContents(0);
}


void ValidationResultsView::ConfigureTableForResults()
{
    m_ResultTable.setRowCount(0);
    m_ResultTable.setColumnCount(3);
    m_ResultTable.setHorizontalHeaderLabels(
        QStringList() << tr("File") << tr("Line") << tr("Message"));
    m_ResultTable.verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


QString ValidationResultsView::RemoveEpubPathPrefix(const QString &path)
{
    return QString(path).remove(QRegularExpression("^[\\w-]+\\.epub/?"));
}

