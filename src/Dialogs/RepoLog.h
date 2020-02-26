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

#ifndef REPOLOG_H
#define REPOLOG_H

#include <QString>
#include <QDialog>
// #include <QDebug>

#include "ui_RepoLog.h"

class RepoLog : public QDialog

{
    Q_OBJECT

public:
    RepoLog(const QString& localRepo, const QString& bookid, QWidget *parent);
    ~RepoLog();


public slots:
    int exec();
    void reject();

private:
    void ReadSettings();
    void WriteSettings();
    void showConsole();
    void connectSignalsToSlots();

    QString m_bookid;
    QString m_localRepo;
    QString m_LogData;
    bool m_ready;

    Ui::RepoLog ui;
};
#endif
