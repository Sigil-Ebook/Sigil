/************************************************************************
**
**  Copyright (C) 2013 John Schember <john@nachtimwald.com>
**  Copyright (C) 2013 Dave Heiland
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
#ifndef LINKSWIDGET_H
#define LINKSWIDGET_H

#include <QtCore/QHash>
#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSharedPointer>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"
#include "Dialogs/ReportsWidgets/ReportsWidget.h"

#include "ui_ReportsLinksWidget.h"

class QString;
class QStringList;

class LinksWidget : public ReportsWidget

{
    Q_OBJECT

public:
    LinksWidget();

    void CreateReport(QSharedPointer< Book > book);

    void SetupTable(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

signals:
    void CloseDialog();
    void DeleteFilesRequest(QStringList);
    void OpenFileRequest(QString, int);

private slots:

    void FilterEditTextChangedSlot(const QString &text);

    void DoubleClick();

    void Save();

private:
    void ReadSettings();
    void WriteSettings();

    void connectSignalsSlots();

    QList<HTMLResource *> m_HTMLResources;

    QSharedPointer< Book > m_Book;

    QStandardItemModel *m_ItemModel;

    QString m_LastDirSaved;
    QString m_LastFileSaved;

    Ui::LinksWidget ui;
};

#endif // LINKSWIDGET_H
