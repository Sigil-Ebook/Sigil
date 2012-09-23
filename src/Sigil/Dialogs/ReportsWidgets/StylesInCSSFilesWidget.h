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
#ifndef STYLESINCSSFILESWIDGET_H
#define STYLESINCSSFILESWIDGET_H

#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSharedPointer>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"
#include "ReportsWidget.h"

#include "ui_ReportsStylesInCSSFilesWidget.h"

class QString;
class QStringList;

class StylesInCSSFilesWidget : public ReportsWidget
{
    Q_OBJECT

public:
    StylesInCSSFilesWidget(QList<Resource *> html_resources, QList<Resource *> css_resources, QSharedPointer<Book> book);

    ReportsWidget::Results saveSettings();

signals:
    void Done();

private slots:
    void OpenContextMenu(const QPoint &point);

    void FilterEditTextChangedSlot(const QString &text);

    void Delete();

private:
    void CreateContextMenuActions();
    void SetupContextMenu(const QPoint &point);

    struct Selector {
        int css_line;
        int css_position;
        QString css_selector_text;
        QString html_filename;
    };

    void connectSignalsSlots();

    void SetupTable();
    QHash< QString, QList<StylesInCSSFilesWidget::Selector *> > CheckHTMLFiles();
    void CheckCSSFiles(QHash< QString, QList<StylesInCSSFilesWidget::Selector *> > css_selectors);

    QList<Resource *> m_HTMLResources;
    QList<Resource *> m_CSSResources;

    QSharedPointer< Book > m_Book;

    QStandardItemModel *m_ItemModel;

    QMenu *m_ContextMenu;

    QAction *m_Delete;

    bool m_DeleteStyles;

    Ui::StylesInCSSFilesWidget ui;
};

#endif // STYLESINCSSFILESWIDGET_H
