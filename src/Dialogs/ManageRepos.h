/************************************************************************
 **
 **  Copyright (C) 2020 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef MANAGEREPOS_H
#define MANAGEREPOS_H

#include <QtGui/QStandardItemModel>

#include "ui_ManageRepos.h"

class QComboBox;

class ManageRepos : public QDialog
{
    Q_OBJECT

public:
    ManageRepos(QWidget* parent);
    ~ManageRepos();

private slots:
    void ShowLog();
    void RemoveRepo();
    void RemoveAllRepos();
    void RepoSelected(int row, int col);

private:
    enum PluginFields {
        FileField        = 0,
        TitleField       = 1,
        ModifiedField    = 2,
        VersionField     = 3,
        UUIDField        = 4,
    };

    void WriteSettings();
    void ReadSettings();
    void SetRepoList();
    void InitializeTable();
    QStringList GetBookInfo(const QString &reponame);
    void SetRepoTableRow(const QStringList &fields, int row);
    void ConnectSignalsToSlots();

    Ui::ManageRepos ui;

    QStringList m_repoList;
};

#endif // MANAGEREPOS_H
