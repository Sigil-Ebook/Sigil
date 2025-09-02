/************************************************************************
**
**  Copyright (C) 2015-2024 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2015-2023 Doug Massay
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

#include <QAction>
#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEngineView>
#include <QtWidgets/QDockWidget>
#include <QStringList>
#include <ViewEditors/Viewer.h>
#include <Dialogs/Inspector.h>

class ViewPreview;
class Inspector;
class QWebEngineView;
class QVBoxLayout;
class QHBoxLayout;
class QProgressBar;
class OverlayHelperWidget;
class WebViewPrinter;
class QToolButton;
class QWidget;
class QFocusFrame;

class PreviewWindow : public QDockWidget
{
    Q_OBJECT

public:
    PreviewWindow(QWidget *parent = 0);
    ~PreviewWindow();
    QList<ElementIndex> GetCaretLocation();
    void SetCaretLocation(const QList<ElementIndex> &loc);
    bool IsVisible();
    bool HasFocus();
    float GetZoomFactor();
    bool eventFilter(QObject *object, QEvent *event);
    void setMathJaxURL(QString mathjaxurl) { m_mathjaxurl = mathjaxurl; };
    void setUserCSSURLs(const QStringList&  usercssurls);
    void ForceFullWebCacheClear();

public slots:
    bool UpdatePage(QString filename, QString text, QList<ElementIndex> location);
    void UpdatePageDone();
    void DelayedScrollTo();
    void ScrollTo(QList<ElementIndex> location);
    void SetZoomFactor(float factor);
    void LinkClicked(const QUrl &url);
    void EmitGoToPreviewLocationRequest();
    void InspectPreviewPage();
    void SelectAllPreview();
    void CopyPreview();
    void ReloadPreview();
    void InspectorClosed(int);
    void setProgress(int);
    
    /**
     * Set DockWidget titlebar text independently of tab text (when tabbed)
     * @param text The title to use.
     */
    void setTitleText(const QString &text);
    void previewFloated(bool wasFloated);
    void CycleCustomCSS();
    void PrintRendered();
    void PrintStarted();
    void PrintEnded();
    void SetFocusOnPreview();
    
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
    virtual void resizeEvent(QResizeEvent * event);

    
    /**
     * Reimplemented from QDockWidget to enable setTitleText()
     * @param event The underlying QPaintEvent.
     */
    virtual void paintEvent(QPaintEvent* event);

private:
    void SetupView();
    void LoadSettings();
    void ConnectSignalsToSlots();
    void UpdateWindowTitle();
    bool fixup_fullscreen_svg_images(const QString &text);
    
    const QString titleText();

    QWidget *m_MainWidget;
    QToolButton * m_binspect;
    QToolButton * m_bselect;
    QToolButton * m_bcopy;
    QToolButton * m_breload;
    QToolButton * m_bcycle;
    QToolButton * m_bprint;
    QFrame *m_wrapper;
    QVBoxLayout *m_Layout;
    QHBoxLayout *m_buttons;
    OverlayHelperWidget *m_overlayBase;

    ViewPreview *m_Preview;
    Inspector *m_Inspector;
    QProgressBar* m_progress;

    QString m_Filepath;
    QString m_titleText;

    QString m_mathjaxurl;
    QStringList m_usercssurls;

    QAction * m_inspectAction;
    QAction * m_selectAction;
    QAction * m_copyAction;
    QAction * m_reloadAction;
    QAction * m_cycleCSSAction;
    QAction * m_webviewPrint;

    QList<ElementIndex> m_location;
    
    bool m_updatingPage;
    bool m_usingMathML;
    int m_cycleCSSLevel;

    bool m_skipPrintPreview;
    WebViewPrinter *m_WebViewPrinter;
    bool m_use_focus_highlight;

};

#endif // PREVIEWWINDOW_H
