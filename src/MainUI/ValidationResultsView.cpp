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

#include "Misc/EmbeddedPython.h"

#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QRegularExpression>
#include <QVariant>

#include "BookManipulation/Book.h"
#include "BookManipulation/FolderKeeper.h"
#include "MainUI/ValidationResultsView.h"
#include "Misc/Utility.h"
#include "sigil_exception.h"

static const QBrush INFO_BRUSH    = QBrush(QColor(224, 255, 255));
static const QBrush WARNING_BRUSH = QBrush(QColor(255, 255, 230));
static const QBrush ERROR_BRUSH   = QBrush(QColor(255, 230, 230));
const QString ValidationResultsView::SEP = QString(QChar(31));

ValidationResultsView::ValidationResultsView(QWidget *parent)
    :
    QDockWidget(tr("Validation Results"), parent),
    m_ResultTable(new QTableWidget(this))
{
    setWidget(m_ResultTable);
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    SetUpTable();
    connect(m_ResultTable, SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
            this,           SLOT(ResultDoubleClicked(QTableWidgetItem *)));
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
    QTableWidgetItem *line_item = m_ResultTable->item(row, 1);
    QTableWidgetItem *offset_item = m_ResultTable->item(row, 2);

    if (!line_item || !offset_item) {
        return;
    }


    int line = line_item->text().toInt();
    int charoffset = offset_item->text().toInt();

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByShortPathName(shortname);
        // if character offset info exists, use it in preference to just the line number
        if (charoffset != -1) {
            emit OpenResourceRequest(resource, line, charoffset, QString());
        } else {
            emit OpenResourceRequest(resource, line, -1, QString());
        }
    } catch (ResourceDoesNotExist) {
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

    if (results.empty()) {
        DisplayNoProblemsMessage();
        return;
    }

    ConfigureTableForResults();

    Q_FOREACH(ValidationResult result, results) {
        int rownum = m_ResultTable->rowCount();
        QTableWidgetItem *item = NULL;

        QBrush row_brush = INFO_BRUSH;
        if (result.Type() == ValidationResult::ResType_Warn) {
            row_brush = WARNING_BRUSH;
        } else if (result.Type() == ValidationResult::ResType_Error) {
            row_brush = ERROR_BRUSH;
        }

        m_ResultTable->insertRow(rownum);

	Resource * resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(result.BookPath());
	QString path = resource->ShortPathName();
	
        item = new QTableWidgetItem(RemoveEpubPathPrefix(path));
        item->setBackground(row_brush);
        m_ResultTable->setItem(rownum, 0, item);

        item = result.LineNumber() > 0 ? new QTableWidgetItem(QString::number(result.LineNumber())) : new QTableWidgetItem(tr("N/A"));
        item->setBackground(row_brush);
        m_ResultTable->setItem(rownum, 1, item);

        item = result.CharOffset() > 0 ? new QTableWidgetItem(QString::number(result.CharOffset())) : new QTableWidgetItem(tr("N/A"));
        item->setBackground(row_brush);
        m_ResultTable->setItem(rownum, 2, item);

        item = new QTableWidgetItem(result.Message());
        item->setBackground(row_brush);
        m_ResultTable->setItem(rownum, 3, item);
    }

    // Make Line and Offset columns as small as possible
    // Ditto for Filename
    m_ResultTable->resizeColumnToContents(0);
    m_ResultTable->resizeColumnToContents(1);
    m_ResultTable->resizeColumnToContents(2);
    //m_ResultTable->resizeColumnsToContents();
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
}


QString ValidationResultsView::RemoveEpubPathPrefix(const QString &path)
{
    return QString(path).remove(QRegularExpression("^[\\w-]+\\.epub/?"));
}

