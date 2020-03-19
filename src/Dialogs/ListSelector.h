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
#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <QWidget>
#include <QListWidget>
#include <QToolButton>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "Misc/Utility.h"

class ListSelector : public QWidget
{
    Q_OBJECT

public:
    ListSelector(const QString& lbl, const QString& btext, const QStringList& alist, QWidget* parent = 0)
        : QWidget(parent),
	m_lbl(new QLabel(lbl, this)),
	m_bp(new QToolButton(this)),
	m_lw(new QListWidget(this)),
	m_layout(new QVBoxLayout(this))
    {
	m_bp->setToolTip(tr("View selected"));
	m_bp->setText(btext);
	m_bp->setToolButtonStyle(Qt::ToolButtonTextOnly);
	QStringList sortedlist = Utility::LocaleAwareSort(alist);
	foreach(QString aitem, sortedlist) {
	    m_lw->addItem(aitem);
	}
	m_lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_lw->setUniformItemSizes(true);
	m_layout->addWidget(m_lbl);
	m_layout->addWidget(m_lw);
	QHBoxLayout* hl = new QHBoxLayout();
	hl->addStretch(0);
	hl->addWidget(m_bp);
	m_layout->addLayout(hl);
	connect(m_bp, SIGNAL(clicked()), this, SLOT(view_request()));
	connect(m_lw, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(view_request()));
    }

    QStringList get_selections() 
    {
	QStringList selected;
	foreach(QListWidgetItem* wi, m_lw->selectedItems()) {
	    QString apath = wi->text();
	    if (!apath.isEmpty()) {
	        selected << apath;
	    }
	}
	return selected;
    }

 signals:
    void ViewRequest();

 public slots:
    void view_request() { emit ViewRequest(); }

  private:
    QLabel*       m_lbl;
    QToolButton*  m_bp;
    QListWidget*  m_lw;
    QVBoxLayout*  m_layout;
};

#endif // LISTSELECTOR_H
