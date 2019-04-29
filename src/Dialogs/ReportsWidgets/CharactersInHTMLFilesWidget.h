/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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
#ifndef CHARACTERSINHTMLFILESWIDGET_H
#define CHARACTERSINHTMLFILESWIDGET_H

#include <QtCore/QHash>
#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSharedPointer>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"
#include "BookManipulation/BookReports.h"
#include "Dialogs/ReportsWidgets/ReportsWidget.h"

#include "ui_ReportsCharactersInHTMLFilesWidget.h"

class QString;

class CharactersInHTMLFilesWidget : public ReportsWidget
{
    Q_OBJECT

public:
    CharactersInHTMLFilesWidget();
    ~CharactersInHTMLFilesWidget();

    void CreateReport(QSharedPointer<Book> m_Book);

signals:
    void CloseDialog();
    void OpenFileRequest(QString, int);
    void FindText(QString);

private slots:
    void FilterEditTextChangedSlot(const QString &text);

    void Save();
    void DoubleClick();

private:
    void ReadSettings();
    void WriteSettings();

    void connectSignalsSlots();

    void SetupTable();
    void AddTableData();

    QList <QChar> GetDisplayedCharacters(QList<HTMLResource *> resources);

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_ItemModel;

    QString m_LastDirSaved;
    QString m_LastFileSaved;

    bool m_PageLoaded;

    Ui::CharactersInHTMLFilesWidget ui;
};

#endif // CHARACTERSINHTMLFILESWIDGET_H
