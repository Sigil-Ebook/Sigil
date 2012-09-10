/************************************************************************
**
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
#ifndef VIEWCLASSES_H
#define VIEWCLASSES_H

#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSharedPointer>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"

#include "ui_ViewClasses.h"

class QString;
class QStringList;

class ViewClasses : public QDialog
{
    Q_OBJECT

public:
    ViewClasses(QList<Resource *> css_resources, QList<Resource *> html_resources, QSharedPointer<Book> book, QWidget *parent = 0);

    QString SelectedFile();

private slots:
    void FilterEditTextChangedSlot(const QString &text);

    void WriteSettings();

private:
    struct Selector {
        int css_position;
        QString css_selector_text;
        QString html_filename;
    };

    void SetSelectedFile();

    void ReadSettings();
    void connectSignalsSlots();

    void SetupTable();
    QHash< QString, QList<ViewClasses::Selector *> > CheckHTMLFiles();
    void CheckCSSFiles(QHash< QString, QList<ViewClasses::Selector *> > css_selectors);

    QList<Resource *> m_CSSResources;
    QList<Resource *> m_HTMLResources;

    QSharedPointer< Book > m_Book;

    QStandardItemModel *m_ViewClassesModel;

    QString m_SelectedFile;

    Ui::ViewClasses ui;
};

#endif // VIEWCLASSES_H
