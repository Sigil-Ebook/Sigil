/************************************************************************
**
**  Copyright (C) 2020-2021 Kevin B. Hendricks, Stratford, Ontario Canada
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
#ifndef AVVIEW_H
#define AVVIEW_H

#include <QString>
#include <QWidget>

class QVBoxLayout;
class QWebEngineView;

class AVView : public QWidget
{
    Q_OBJECT

 public:
    AVView(QWidget *parent=0);
    ~AVView();

 public slots:
    void ShowAV(QString path);
    void ReloadViewer();

 private:

    QString m_path;
    QWebEngineView *m_WebView;
    QVBoxLayout* m_layout;
};

#endif // AVVIEW_H
