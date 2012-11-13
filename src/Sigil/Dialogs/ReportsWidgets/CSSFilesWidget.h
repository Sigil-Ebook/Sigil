/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#ifndef CSSFILESWIDGET_H
#define CSSFILESWIDGET_H

#include <QtCore/QSharedPointer>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "ResourceObjects/Resource.h"
#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include "BookManipulation/Book.h"
#include "Dialogs/ReportsWidgets/ReportsWidget.h"

#include "ui_ReportsCSSFilesWidget.h"

class QString;
class QStringList;

class CSSFilesWidget : public ReportsWidget
{
    Q_OBJECT

public:
    CSSFilesWidget();

    void CreateReport(QSharedPointer< Book > book);

    void SetupTable(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

signals:
    void DeleteFilesRequest(QStringList);
    void OpenFileRequest(QString, int);

private slots:
    void ReadSettings();
    void WriteSettings();

    void OpenContextMenu(const QPoint &point);

    void Sort(int logicalindex, Qt::SortOrder order);

    void FilterEditTextChangedSlot(const QString &text);

    void Delete();
    void Save();
    void DoubleClick();

private:
    void CreateContextMenuActions();
    void SetupContextMenu(const QPoint &point);

    void connectSignalsSlots();

    QList<HTMLResource *> m_HTMLResources;
    QList<CSSResource *> m_CSSResources;

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_ItemModel;

    QMenu *m_ContextMenu;

    QAction *m_Delete;

    QString m_LastDirSaved;
    QString m_LastFileSaved;

    Ui::CSSFilesWidget ui;
};

#endif // CSSFILESWIDGET_H
