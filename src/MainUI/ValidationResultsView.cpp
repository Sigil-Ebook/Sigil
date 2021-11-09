/************************************************************************
**
**  Copyright (C) 2015-2021 Kevin B. Hendricks, Stratford Ontario Canada
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

#include "EmbedPython/EmbeddedPython.h"

#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QRegularExpression>
#include <QVariant>
#include <QFileDialog>

#include "BookManipulation/Book.h"
#include "BookManipulation/FolderKeeper.h"
#include "MainUI/ValidationResultsView.h"
#include "Misc/Utility.h"
#include "sigil_exception.h"

#if(0)
static const QBrush INFO_BRUSH    = QBrush(QColor(224, 255, 255));
static const QBrush WARNING_BRUSH = QBrush(QColor(255, 255, 230));
static const QBrush ERROR_BRUSH   = QBrush(QColor(255, 230, 230));
#endif

const QString ValidationResultsView::SEP = QString(QChar(31));

static const QString SETTINGS_GROUP = "validation_results";


ValidationResultsView::ValidationResultsView(QWidget *parent)
    :
    QDockWidget(tr("Validation Results"), parent),
    m_ResultTable(new QTableWidget(this)),
    m_NoProblems(false),
    m_ContextMenu(new QMenu(this))
{
    setWidget(m_ResultTable);
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    SetUpTable();
    m_ResultTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ExportAll = new QAction(tr("Export All") + "...", this);
    ReadSettings();
    connect(m_ResultTable, SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
            this, SLOT(ResultDoubleClicked(QTableWidgetItem *)));
    connect(m_ResultTable, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(OpenContextMenu(const QPoint &)));
    connect(m_ExportAll,   SIGNAL(triggered()), this, SLOT(ExportAll()));
}

void ValidationResultsView::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    m_LastFolderOpen = settings.value("last_folder_open").toString();
    settings.endGroup();
}

void ValidationResultsView::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("last_folder_open", m_LastFolderOpen);
    settings.endGroup();
}


void ValidationResultsView::OpenContextMenu(const QPoint &point)
{
    m_ContextMenu->addAction(m_ExportAll);
    m_ContextMenu->exec(m_ResultTable->viewport()->mapToGlobal(point));
    if (!m_ContextMenu.isNull()) {
        m_ContextMenu->clear();
        m_ExportAll->setEnabled(true);
    }
}


void ValidationResultsView::ExportAll()
{
    if (m_NoProblems || m_ResultTable->rowCount() == 0) return;

    // Get the filename to use
    QMap<QString,QString> file_filters;
    file_filters[ "csv" ] = tr("CSV files (*.csv)");
    file_filters[ "txt" ] = tr("Text files (*.txt)");
    QStringList filters = file_filters.values();
    QString filter_string = "";
    foreach(QString filter, filters) {
        filter_string += filter + ";;";
    }
    QString default_filter = file_filters.value("csv");

    QFileDialog::Options options = QFileDialog::Options();
#ifdef Q_OS_MAC
    options = options | QFileDialog::DontUseNativeDialog;
#endif

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Export Validation Results"),
                                                    m_LastFolderOpen,
                                                    filter_string,
                                                    &default_filter,
                                                    options);
    if (filename.isEmpty()) return;

    QString ext = QFileInfo(filename).suffix().toLower();
    QChar sep = QChar(',');
    if (ext == "txt") sep = QChar(9);

    QStringList res;
    for (int i = 0; i < m_ResultTable->rowCount(); i++) {
        QStringList data;
        QTableWidgetItem *path_item = m_ResultTable->item(i, 0);
        data << path_item->data(Qt::UserRole+1).toString();
        data << m_ResultTable->item(i,1)->text();
        data << m_ResultTable->item(i,2)->text();
        data << m_ResultTable->item(i,3)->text();
        if (sep == ',') {
            res << Utility::createCSVLine(data);
        } else {
            res << data.join(sep);
        }
    }
    QString text = res.join('\n');
    QString message;
    try {
        Utility::WriteUnicodeTextFile(text, filename);
        m_LastFolderOpen = QFileInfo(filename).absolutePath();
        WriteSettings();
    } catch (CannotOpenFile& e) {
        message = QString(e.what());
        Utility::DisplayStdWarningDialog(tr("Export of Validation Results failed: "), message);
    }
}


void ValidationResultsView::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    raise();
}


QStringList ValidationResultsView::ValidateFile(QString &apath)
{
    int rv = 0;
    QString error_traceback;
    QStringList results;

    QList<QVariant> args;
    args.append(QVariant(apath));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("sanitycheck"),
                                         QString("perform_sanity_check"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in sanitycheck perform_sanity_check: ") + QString::number(rv), 
                                         error_traceback);
        // an error happened - make no changes
        return results;
    }
    return res.toStringList();
}


void ValidationResultsView::ValidateCurrentBook()
{
    ClearResults();
    QList<ValidationResult> results;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_Book->SaveAllResourcesToDisk();

    QList<Resource *> resources = m_Book->GetFolderKeeper()->GetResourceList();
    foreach (Resource * resource, resources) {
        if (resource->Type() == Resource::HTMLResourceType) {
            QString apath = resource->GetFullPath();
            QString bookpath = resource->GetRelativePath();
            QStringList reslst = ValidateFile(apath);
            if (!reslst.isEmpty()) {
                foreach (QString res, reslst) {
                    QStringList details = res.split(SEP);
                    ValidationResult::ResType vtype;
                    QString etype = details[0];
                    if (etype == "info") {
                        vtype = ValidationResult::ResType_Info;
                    } else if (etype == "warning") {
                        vtype = ValidationResult::ResType_Warn;
                    } else if (etype == "error") {
                        vtype = ValidationResult::ResType_Error;
                    } else {
                        continue;
                    }
                    QString filename = details[1];
                    int lineno = details[2].toInt();
                    int charoffset = details[3].toInt();
                    QString msg = details[4];
                    results.append(ValidationResult(vtype,bookpath,lineno,charoffset,msg));
                }
            }
        }
    }
    QApplication::restoreOverrideCursor();
    DisplayResults(results);
    show();
    raise();
}


void ValidationResultsView::LoadResults(const QList<ValidationResult> &results)
{
    ClearResults();
    DisplayResults(results);
    show();
    raise();
}


void ValidationResultsView::ClearResults()
{
    m_ResultTable->clearContents();
    m_ResultTable->setRowCount(0);
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
    QTableWidgetItem *path_item = m_ResultTable->item(row, 0);

    if (!path_item) {
        return;
    }

    QString shortname = path_item->text();
    QString bookpath = path_item->data(Qt::UserRole+1).toString();
    QTableWidgetItem *line_item = m_ResultTable->item(row, 1);
    QTableWidgetItem *offset_item = m_ResultTable->item(row, 2);

    if (!line_item || !offset_item) {
        return;
    }


    int line = line_item->text() != "N/A" ? line_item->text().toInt(): -1;
    int charoffset = offset_item->text() != "N/A" ? offset_item->text().toInt(): -1;

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(bookpath);
        // if character offset info exists, use it in preference to just the line number
        if (charoffset != -1) {
            emit OpenResourceRequest(resource, line, charoffset, QString());
        } else {
            emit OpenResourceRequest(resource, line, -1, QString());
        }
    } catch (ResourceDoesNotExist&) {
        return;
    }
}


void ValidationResultsView::SetUpTable()
{
    m_ResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ResultTable->setTabKeyNavigation(false);
    m_ResultTable->setDropIndicatorShown(false);
    m_ResultTable->horizontalHeader()->setStretchLastSection(true);
    m_ResultTable->verticalHeader()->setVisible(false);
}


void ValidationResultsView::DisplayResults(const QList<ValidationResult> &results)
{
    m_ResultTable->clear();
    m_NoProblems = false;

    if (results.empty()) {
        m_NoProblems = true;
        DisplayNoProblemsMessage();
        return;
    }

    ConfigureTableForResults();

    Q_FOREACH(ValidationResult result, results) {
        int rownum = m_ResultTable->rowCount();
        QTableWidgetItem *item = NULL;

        QBrush row_brush = Utility::ValidationResultBrush(Utility::INFO_BRUSH);
        if (result.Type() == ValidationResult::ResType_Warn) {
            row_brush = Utility::ValidationResultBrush(Utility::WARNING_BRUSH);
        } else if (result.Type() == ValidationResult::ResType_Error) {
            row_brush = Utility::ValidationResultBrush(Utility::ERROR_BRUSH);
        }

        m_ResultTable->insertRow(rownum);
 
        QString path;
        QString bookpath = result.BookPath();
        try {
            Resource * resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(bookpath);
            path = resource->ShortPathName();
        } catch (ResourceDoesNotExist&) {
            if (bookpath.isEmpty()) {
                path = "***Invalid Book Path Provided ***";
            } else {
                path = bookpath;
            }
        }

        item = new QTableWidgetItem(RemoveEpubPathPrefix(path));
        item->setData(Qt::UserRole+1, bookpath);
        SetItemPalette(item, row_brush);
        m_ResultTable->setItem(rownum, 0, item);

        item = result.LineNumber() > 0 ? new QTableWidgetItem(QString::number(result.LineNumber())) : new QTableWidgetItem("N/A");
        SetItemPalette(item, row_brush);
        m_ResultTable->setItem(rownum, 1, item);

        item = result.CharOffset() >= 0 ? new QTableWidgetItem(QString::number(result.CharOffset())) : new QTableWidgetItem("N/A");
        SetItemPalette(item, row_brush);
        m_ResultTable->setItem(rownum, 2, item);

        item = new QTableWidgetItem(result.Message());
        SetItemPalette(item, row_brush);
        m_ResultTable->setItem(rownum, 3, item);
    }

    // Make Line and Offset columns as small as possible
    // Ditto for Filename
    m_ResultTable->resizeColumnToContents(0);
    m_ResultTable->resizeColumnToContents(1);
    m_ResultTable->resizeColumnToContents(2);
    //m_ResultTable->resizeColumnsToContents();
}

int ValidationResultsView::ResultCount()
{
    if (m_NoProblems) return 0;
    return m_ResultTable->rowCount();
}

void ValidationResultsView::DisplayNoProblemsMessage()
{
    m_ResultTable->setRowCount(1);
    m_ResultTable->setColumnCount(1);
    m_ResultTable->setHorizontalHeaderLabels(
        QStringList() << tr("Message"));
    QTableWidgetItem *item = new QTableWidgetItem(tr("No problems found!"));
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QFont font = item->font();
    font.setPointSize(16);
    item->setFont(font);
    m_ResultTable->setItem(0, 0, item);
    m_ResultTable->resizeRowToContents(0);
}


void ValidationResultsView::ConfigureTableForResults()
{
    m_ResultTable->setRowCount(0);
    m_ResultTable->setColumnCount(4);
    m_ResultTable->setHorizontalHeaderLabels(
    QStringList() << tr("File") << tr("Line") << tr("Offset") << tr("Message"));
    m_ResultTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_ResultTable->setSortingEnabled(true);
    m_ResultTable->horizontalHeader()->setSortIndicatorShown(true);

}


QString ValidationResultsView::RemoveEpubPathPrefix(const QString &path)
{
    return QString(path).remove(QRegularExpression("^[\\w-]+\\.epub/?"));
}

void ValidationResultsView::SetItemPalette(QTableWidgetItem * item, QBrush &row_brush)
{
    if (Utility::IsDarkMode()) {
        item->setForeground(row_brush);
    } else {
        item->setBackground(row_brush);
    }   
}

