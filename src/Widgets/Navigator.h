/************************************************************************
 **
 **  Copyright (C) 2020-2021 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QTimer>
#include <QWidget>
#include <QIcon>
#include <QToolButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QLayout>
#include <QGridLayout>

class Navigator : public QWidget
{
    Q_OBJECT

public:
    Navigator(QWidget* parent = 0)
        : QWidget(parent),
        m_layout(new QGridLayout(this)),
        m_bp(new QToolButton(this)),
        m_bn(new QToolButton(this)),
        m_search(new QLineEdit(this)),
        m_sn(new QToolButton(this)),
        m_sp(new QToolButton(this)),
        m_lb(new QRadioButton(tr("Left"), this)),
        m_rb(new QRadioButton(tr("Right"), this)),
        m_done(new QToolButton(this))
    {
        int r = m_layout->rowCount();
        // previous change
        m_bp->setIcon(QIcon(":/main/back.svg"));
        m_bp->setToolTip(tr("Go to previous change - [p]"));
        m_bp->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_layout->addWidget(m_bp, r, 0);

        // next change
        m_bn->setIcon(QIcon(":/main/forward.svg"));
        m_bn->setToolTip(tr("Go to next change - [n]"));
        m_bn->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_layout->addWidget(m_bn, r, 1);

        // search field
        m_layout->addWidget(m_search, r, 2);
        m_search->setPlaceholderText(tr("Search ..."));

        // find next
        m_sn->setIcon(QIcon(":/main/arrow-down.svg"));
        m_sn->setToolTip(tr("Find Next"));
        m_sn->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_layout->addWidget(m_sn, r, 3);

        // find previous
        m_sp->setIcon(QIcon(":/main/arrow-up.svg"));
        m_sp->setToolTip(tr("Find Previous"));
        m_sp->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_layout->addWidget(m_sp, r, 4);

        // left panel
        m_lb->setToolTip(tr("Use Left Panel"));
        m_layout->addWidget(m_lb, r, 5);
        // right panel
        m_rb->setToolTip(tr("Use Right Panel"));
        m_layout->addWidget(m_rb, r, 6);
        m_rb->setChecked(true);

        // done button
        m_done->setToolTip(tr("Close this window"));
        m_done->setText(tr("Done"));
        m_done->setToolButtonStyle(Qt::ToolButtonTextOnly);
        m_layout->addWidget(m_done, r, 7);

        connect(m_bp, SIGNAL(clicked()), this, SLOT(do_prev_change()));
        connect(m_bn, SIGNAL(clicked()), this, SLOT(do_next_change()));
        connect(m_search, SIGNAL(returnPressed()), this, SLOT(do_search()));
        connect(m_sn, SIGNAL(clicked()), this, SLOT(do_find_next()));
        connect(m_sp, SIGNAL(clicked()), this, SLOT(do_find_prev()));
        connect(m_done, SIGNAL(clicked()), this, SLOT(do_done()));
    }

    bool use_left_panel() { return m_lb->isChecked(); }

    QString get_search_text() { return m_search->text(); }

    void set_focus_on_search() { m_search->setFocus(Qt::OtherFocusReason); }

 signals:
    void NextChange(int dir);
    void DoSearch(bool reverse);
    void DoDone();

 public slots:

    void do_prev_change() { emit NextChange(-1); }
    void do_next_change() { emit NextChange(1); }

    void do_find_next() { do_search(false); }
    void do_find_prev() { do_search(true); }

    void do_search(bool reverse=false) {
        emit DoSearch(reverse);
    }

    void do_done() { emit DoDone(); }

private:
    QGridLayout*  m_layout;
    QToolButton*  m_bp;
    QToolButton*  m_bn;
    QLineEdit*    m_search;
    QToolButton*  m_sn;
    QToolButton*  m_sp;
    QRadioButton* m_lb;
    QRadioButton* m_rb;
    QToolButton * m_done;
};

#endif // NAVIGATOR_H
