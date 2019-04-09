/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2012 Dave Heiland, John Schember
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
#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWidgets/QDockWidget>
#include <ViewEditors/Viewer.h>

class ViewPreview;
class QSplitter;
class QStackedWidget;
class QWebEngineView;
class QVBoxLayout;

class PreviewWindow : public QDockWidget
{
    Q_OBJECT

public:
    PreviewWindow(QWidget *parent = 0);
    ~PreviewWindow();
    QList<ElementIndex> GetCaretLocation();
    bool IsVisible();
    bool HasFocus();
    float GetZoomFactor();
    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void UpdatePage(QString filename, QString text, QList<ElementIndex> location);
    void SetZoomFactor(float factor);
    void SplitterMoved(int pos, int index);
    void LinkClicked(const QUrl &url);
    void EmitGoToPreviewLocationRequest();

signals:
    void Shown();
    void ZoomFactorChanged(float factor);
    void GoToPreviewLocationRequest();
    /**
     * Emitted whenever Preview wants to open an URL.
     * @param url The URL to open.
     */
    void OpenUrlRequest(const QUrl &url);


protected:
    virtual void hideEvent(QHideEvent* event);
    virtual void showEvent(QShowEvent* event);
    void resizeEvent(QResizeEvent * event);

private:
    void SetupView();
    void LoadSettings();
    void ConnectSignalsToSlots();
    void UpdateWindowTitle();

    QWidget *m_MainWidget;
    QVBoxLayout *m_Layout;

    ViewPreview *m_Preview;
    QWebEngineView *m_Inspector;
    QSplitter *m_Splitter;
    QStackedWidget *m_StackedViews;
    QString m_Filepath;
    bool m_GoToRequestPending;
};

#endif // PREVIEWWINDOW_H
