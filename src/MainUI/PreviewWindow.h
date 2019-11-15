/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012      Dave Heiland, John Schember
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

#include <QPushButton>
#include <QAction>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWidgets/QDockWidget>
#include <ViewEditors/Viewer.h>
#include <Dialogs/Inspector.h>

class ViewPreview;
class Inspector;
class QWebEngineView;
class QVBoxLayout;
class QHBoxLayout;

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
    void setMathJaxURL(QString mathjaxurl) { m_mathjaxurl = mathjaxurl; };
    void setUserCSSURL(QString usercssurl) { m_usercssurl = usercssurl; }

public slots:
    bool UpdatePage(QString filename, QString text, QList<ElementIndex> location);
    void ScrollTo(QList<ElementIndex> location);
    void SetZoomFactor(float factor);
    void LinkClicked(const QUrl &url);
    void EmitGoToPreviewLocationRequest();
    void InspectPreviewPage();
    void SelectAllPreview();
    void CopyPreview();
    void ReloadPreview();
    void InspectorClosed(int);

signals:
    void Shown();
    void ZoomFactorChanged(float factor);
    void GoToPreviewLocationRequest();
    void RequestPreviewReload();

    /**
     * Emitted whenever Preview wants to open an URL.
     * @param url The URL to open.
     */
    void OpenUrlRequest(const QUrl &url);

    /**
     * Emitted whenever Preview wants current CV tab to scroll
     * to fragment
     * @param url The URL to open.
     */
    void ScrollToFragmentRequest(const QString &fragment);
 

protected:
    virtual void hideEvent(QHideEvent* event);
    virtual void showEvent(QShowEvent* event);
    void resizeEvent(QResizeEvent * event);

private:
    void SetupView();
    void LoadSettings();
    void ConnectSignalsToSlots();
    void UpdateWindowTitle();
    bool fixup_fullscreen_svg_images(const QString &text);

    QWidget *m_MainWidget;
    QVBoxLayout *m_Layout;
    QHBoxLayout *m_buttons;

    ViewPreview *m_Preview;
    Inspector *m_Inspector;
    QString m_Filepath;

    QString m_mathjaxurl;
    QString m_usercssurl;

    QAction * m_inspectAction;
    QAction * m_selectAction;
    QAction * m_copyAction;
    QAction * m_reloadAction;

    bool m_updatingPage;
};

#endif // PREVIEWWINDOW_H
