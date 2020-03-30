/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks, Stratford, Ontario Canada
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
#ifndef VIEWFONT_H
#define VIEWFONT_H

#include <QSize>
#include <QString>
#include <QWidget>
#include <QToolButton>
#include <QDialog>

class QVBoxLayout;
class FontView;

class ViewFont : public QDialog
{
    Q_OBJECT

 public:
    ViewFont(QWidget *parent=0);
    ~ViewFont();

    void ShowFont(QString path);
    QSize sizeHint();

 public slots:
    void ReloadViewer();

 private slots:
    void WriteSettings();

 private:
    void ReadSettings();
    void ConnectSignalsToSlots();

    FontView* m_fv;
    QToolButton* m_bp;
    QVBoxLayout* m_layout;
};

#endif // VIEWFONT_H
