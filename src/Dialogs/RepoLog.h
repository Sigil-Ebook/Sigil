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
#include <QStringList>
#include <QList>
#include <QLabel>
#include <QDialog>

class Navigator2;
class TextView;
class QVBoxLayout;

class RepoLog : public QDialog

{
    Q_OBJECT

public:

    RepoLog(const QString& lbl, const QString& data, QWidget *parent);
    ~RepoLog();

    void LoadViewer();

public slots:
    int exec();
    void reject();
    void next_page(int dir);
    void do_search(bool reverse=false);

protected:
    void keyPressEvent(QKeyEvent * ev);

private:
    void ReadSettings();
    void WriteSettings();
    void connectSignalsToSlots();

    QStringList   m_blockmap;
    TextView*     m_view;
    Navigator2*   m_nav;
    QLabel*       m_lbl;
    QString       m_data;
    QVBoxLayout*  m_layout;
};
#endif
