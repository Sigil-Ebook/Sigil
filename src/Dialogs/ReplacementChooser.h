/************************************************************************
**
**  Copyright (C) 2022 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#ifndef REPLACEMENTCHOOSER_H
#define REPLACEMENTCHOOSER_H

#include <QAction>
#include <QMenu>
#include <QPointer>
#include <QDialog>
#include <QHash>
#include <QStandardItemModel>
#include "Dialogs/StyledTextDelegate.h"
#include "ui_ReplacementChooser.h"

class QString;
class QCloseEvent;
class Resource;
class FindReplace;

class ReplacementChooser: public QDialog
{
    Q_OBJECT

public:
    ReplacementChooser(QWidget* parent=NULL);
    ~ReplacementChooser();

    int GetReplacementCount() { return m_replacement_count; };

public slots:

    void CreateTable();
    void closeEvent(QCloseEvent *e);
    void reject();

private slots:
    void ChangeContext();
    // void FilterEditTextChangedSlot(const QString &text);
    void DeleteSelectedRows();
    void ApplyReplacements();
    void CreateContextMenuActions();
    void SetupContextMenu(const QPoint &point);
    void OpenContextMenu(const QPoint &point);

private:
    QString GetPriorContext(int start, const QString &text, int amt);
    QString GetPostContext(int end, const QString &text, int amt);

    void ReadSettings();
    void WriteSettings();

    void SetContextCB(int val);
    
    void connectSignalsSlots();

    QStandardItemModel *m_ItemModel;

    StyledTextDelegate* m_TextDelegate;

    FindReplace * m_FindReplace;

    int m_context_amt;

    QAction *m_Delete;

    QPointer<QMenu> m_ContextMenu;

    QHash<QString, Resource*> m_Resources;

    int m_replacement_count;

    int m_current_count;
    
    Ui::ReplacementChooser ui;
};

#endif // REPLACEMENTCHOOSER_H
