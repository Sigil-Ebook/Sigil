/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2013 Dave Heiland
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
#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>

class QVBoxLayout;
class QWebEngineView;
class QPrinter;

class ImageView : public QWidget
{
    Q_OBJECT

public:
    ImageView(QWidget *parent = 0);
    ~ImageView();

public slots:
    void ShowImage(QString path);
    void ReloadViewer();

private:
    QString m_path;
    QWebEngineView *m_WebView;
    QVBoxLayout* m_layout;
};

#endif // IMAGEVIEW_H
