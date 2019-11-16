/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef SELECTHYPERLINK_H
#define SELECTHYPERLINK_H

#include <QtCore/QHash>
#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"

#include "ui_SelectHyperlink.h"

class SelectHyperlink: public QDialog
{
    Q_OBJECT

public:
    SelectHyperlink(QString href, Resource *base_resource, 
                    const QString &restype,  QList<Resource *> resources, 
                    QSharedPointer<Book> book, QWidget *parent = 0);

    void SetList();

    QString GetTarget();

private slots:
    void WriteSettings();

    void FilterEditTextChangedSlot(const QString &text);

    void DoubleClicked(const QModelIndex &);
    void Clicked(const QModelIndex &);

private:
    void SetSelectedText();

    void ReadSettings();
    void connectSignalsSlots();

    void AddEntry(Resource *resource);

    QString GetSelectedText();

    void SelectText(QString &text);

    Resource *m_CurrentResource;
    QString m_restype;

    QString m_DefaultTarget;
    QString m_SavedTarget;

    QHash<QString, QStringList> m_IDNames;

    QList<Resource *> m_Resources;

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_SelectHyperlinkModel;

    Ui::SelectHyperlink ui;
};

#endif // SELECTHYPERLINK_H
