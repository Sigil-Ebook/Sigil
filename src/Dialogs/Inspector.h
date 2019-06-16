/************************************************************************
 **
 **  Copyright (C) 2019  Kevin B. Hendricks, Stratford Ontario Canada
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

#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <QDialog>
#include <QEvent>
#include <QCloseEvent>
#include <QSize>
#include <QVBoxLayout>
#include <QtWebEngineWidgets/QWebEngineView>

class QWebEnginePage;

class Inspector : public QDialog
{
    Q_OBJECT

public:
    Inspector(QWidget *parent = nullptr);
    ~Inspector();

    void closeEvent(QCloseEvent* event);

    bool IsLoadingFinished() { return m_LoadingFinished; }
    bool WasLoadOkay() { return m_LoadOkay; }

    QSize sizeHint();
    void  SaveSettings();
    void  LoadSettings();

public slots:
    void InspectPageofView(QWebEngineView * view);
    void StopInspection();

protected slots:
    void UpdateFinishedState(bool okay);
    void LoadingStarted();

private:
    QVBoxLayout* m_Layout;
    QWebEngineView* m_inspectView;
    QWebEngineView* m_view;
    bool m_LoadingFinished;
    bool m_LoadOkay;
};

#endif // INSPECTOR_H
