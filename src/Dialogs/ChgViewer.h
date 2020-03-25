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

#ifndef CHGVIEWER_H
#define CHGVIEWER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QLabel>
#include <QDialog>

#include "Misc/DiffRec.h"

class Navigator;
class TextView;
class QTextCursor;

class ChgViewer : public QDialog

{
    Q_OBJECT

public:

    ChgViewer(const QList<DiffRecord::DiffRec>& diffinfo, const QString& file1,
	      const QString& file2, QWidget *parent);
    ~ChgViewer();

    void insert_with_background(QTextCursor&  tc, const QString& sval, const QString& cval);

    void LoadViewers(const QList<DiffRecord::DiffRec>& diffinfo);

public slots:
    int exec();
    void reject();
    void slideraction();
    void next_change(int dir);
    void do_search(bool reverse=false);

protected:
    void cross_link_scrollbars(bool link=true);
    void synchronize_viewers();
    void keyPressEvent(QKeyEvent * ev);

private:
    void ReadSettings();
    void WriteSettings();
    void connectSignalsToSlots();

    QString       m_filepath1;
    QString       m_filepath2;
    TextView*     m_view1;
    TextView*     m_view2;
    Navigator*    m_nav;
    QLabel*       m_lbl1;
    QLabel*       m_lbl2;
    QVBoxLayout*  m_layout;

    QStringList    m_leftno;
    QStringList    m_rightno;
    QList<int>     m_changelst;

};
#endif
