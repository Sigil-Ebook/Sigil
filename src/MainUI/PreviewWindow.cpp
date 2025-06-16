/************************************************************************
**
**  Copyright (C) 2015-2024  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2019-2023  Doug Massay
**  Copyright (C) 2012       Dave Heiland, John Schember
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

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QTimer>
#include <QProgressBar>
#include <QApplication>
#include <QToolButton>
#include <QDebug>
#include <QFrame>
#include <QWidget>

#include "MainUI/PreviewWindow.h"
#include "Dialogs/Inspector.h"
#include "Parsers/GumboInterface.h"
#include "Misc/SleepFunctions.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/webviewprinter.h"
#include "ViewEditors/ViewPreview.h"
#include "ViewEditors/Overlay.h"
#include "sigil_constants.h"

static const QStringList HEADERTAGS = QStringList() << "h1" << "h2" << "h3" << "h4" << "h5" << "h6";

static const QString SETTINGS_GROUP = "previewwindow";

static const QString MATHJAX3_CONFIG =
"  <script> "
"    MathJax = { "
"      loader: { "
"        load: [ '[mml]/mml3', 'core', 'input/mml', 'output/svg' ] "
"      } "
"    }; "
" </script> ";


#define DBG if(0)

PreviewWindow::PreviewWindow(QWidget *parent)
    :
    QDockWidget(tr("Preview"), parent),
    m_MainWidget(new QWidget(this)),
    m_binspect(new QToolButton(m_MainWidget)),
    m_bselect(new QToolButton(m_MainWidget)),
    m_bcopy(new QToolButton(m_MainWidget)),
    m_breload(new QToolButton(m_MainWidget)),
    m_bcycle(new QToolButton(m_MainWidget)),
    m_bprint(new QToolButton(m_MainWidget)),
    m_wrapper(new QFrame(m_MainWidget)),
    m_Layout(new QVBoxLayout()),
    m_buttons(new QHBoxLayout()),
    m_overlayBase(new OverlayHelperWidget(this)),
    m_Preview(new ViewPreview(m_overlayBase)),
    m_Inspector(new Inspector(this)),
    m_progress(new QProgressBar(this)),
    m_Filepath(QString()),
    m_titleText(QString()),
    m_updatingPage(false),
    m_usingMathML(false),
    m_cycleCSSLevel(0),
    m_skipPrintPreview(false),
    m_WebViewPrinter(new WebViewPrinter(this))
{
    m_progress->reset();
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);
    setWindowTitle(tr("Preview"));
    SetupView();
    LoadSettings();
    ConnectSignalsToSlots();
    setFocusPolicy(Qt::NoFocus);
    setFocusProxy(m_MainWidget);
    m_MainWidget->setObjectName("pwmain");
    m_MainWidget->setFocusPolicy(Qt::NoFocus);
    m_MainWidget->setFocusProxy(m_binspect);

    m_wrapper->setObjectName("previewframe");
    m_wrapper->setFocusPolicy(Qt::NoFocus);
    m_wrapper->setFocusProxy(m_Preview);

    m_Preview->setObjectName("webpreview");
    m_Preview->setFocusPolicy(Qt::StrongFocus);

    m_binspect->setFocusPolicy(Qt::TabFocus);
    m_bselect->setFocusPolicy(Qt::TabFocus);
    m_bcopy->setFocusPolicy(Qt::TabFocus);
    m_breload->setFocusPolicy(Qt::TabFocus);
    m_bcycle->setFocusPolicy(Qt::TabFocus);
    m_bprint->setFocusPolicy(Qt::TabFocus);

    setTabOrder(m_binspect, m_bselect);
    setTabOrder(m_bselect, m_bcopy);
    setTabOrder(m_bcopy, m_breload);
    setTabOrder(m_breload, m_bcycle);
    setTabOrder(m_bcycle, m_bprint);
    setTabOrder(m_bprint, m_Preview);
}

PreviewWindow::~PreviewWindow()
{
    // BookViewPreview must be deleted before QWebInspector.
    // BookViewPreview's QWebPage is linked to the QWebInspector
    // and when deleted it will send a message to the linked QWebInspector
    // to remove the association. If QWebInspector is deleted before
    // BookViewPreview, BookViewPreview will try to access the deleted
    // QWebInspector and the application will SegFault. This is an issue
    // with how QWebPages interface with QWebInspector.

    if (m_Preview) {
        delete m_Preview;
        m_Preview = nullptr;
    }

    if (m_Inspector) {
        if (m_Inspector->isVisible()) {
            m_Inspector->StopInspection();
            m_Inspector->close();
        }
        delete m_Inspector;
        m_Inspector = nullptr;
    }

    if (m_progress) {
        m_progress->reset();
    }
}

void PreviewWindow::setUserCSSURLs(const QStringList& usercssurls)
{
    m_usercssurls = usercssurls;
    m_cycleCSSAction->setEnabled(!m_usercssurls.isEmpty());
    if (!m_usercssurls.isEmpty()) m_cycleCSSLevel = 1;
}


void PreviewWindow::resizeEvent(QResizeEvent *event)
{
    // Update self normally
    QDockWidget::resizeEvent(event);
    UpdateWindowTitle();
}


void PreviewWindow::hideEvent(QHideEvent * event)
{
    if (m_Inspector) {
        m_Inspector->StopInspection();
        m_Inspector->close();
    }

    if ((m_Preview) && m_Preview->isVisible()) {
        m_Preview->hide();
    }
}


void PreviewWindow::showEvent(QShowEvent * event)
{
    // perform the show for all children of this widget
    if ((m_Preview) && !m_Preview->isVisible()) {
        m_Preview->show();
    }

    QDockWidget::showEvent(event);
    raise();
    emit Shown();
}


void PreviewWindow::paintEvent(QPaintEvent *event)
{
    // Allow title text to be set independently of tab text
    // (when QDockWidget is tabified).
    QStylePainter painter(this);

    if (isFloating()) {
        QStyleOptionFrame options;
        options.initFrom(this);

#ifdef Q_OS_MAC
        // This is needed for Qt6 but works on Qt5 as well
        options.palette = QApplication::palette();
#endif

        painter.drawPrimitive(QStyle::PE_FrameDockWidget, options);
    }
    QStyleOptionDockWidget options;
    initStyleOption(&options);
    options.title = titleText();

#ifdef Q_OS_MAC
    // This is needed for Qt6 but works on Qt5 as well
    options.palette = QApplication::palette();
#endif

    painter.drawControl(QStyle::CE_DockWidgetTitle, options);
}

bool PreviewWindow::IsVisible()
{
    return m_Preview->isVisible();
}

bool PreviewWindow::HasFocus()
{
    if (!m_Preview->isVisible()) {
        return false;
    }
    return m_Preview->hasFocus();
}

void PreviewWindow::SetFocusOnPreview()
{
    if (m_Preview) m_Preview->setFocus();
}

float PreviewWindow::GetZoomFactor()
{
    return m_Preview->GetZoomFactor();
}

void PreviewWindow::SetupView()
{
    // QWebEngineView events are routed to their parent
    m_Preview->installEventFilter(this);

    QWidget * fp = m_Preview->focusProxy();
    if (fp) fp->installEventFilter(this);

    m_Layout->setContentsMargins(0, 0, 0, 0);

    // use a QWidget wrapper around the QWebEnginePreview
    // with a single pixel margin to use as focus indicator
    QVBoxLayout * wl = new QVBoxLayout(m_wrapper);
    wl->setContentsMargins(0,0,0,0);
    wl->addWidget(m_Preview);
    m_Layout->addWidget(m_wrapper);

    m_inspectAction = new QAction(QIcon(":/main/inspect.svg"),"", this);
    m_inspectAction ->setEnabled(true);
    m_inspectAction->setToolTip(tr("Inspect Page"));
    m_binspect->setDefaultAction(m_inspectAction);
    m_binspect->setAutoRaise(true);

    m_selectAction  = new QAction(QIcon(":/main/edit-select-all.svg"),"", this);
    m_selectAction ->setEnabled(true);
    m_selectAction->setToolTip(tr("Select-All"));
    m_bselect->setDefaultAction(m_selectAction);
    m_bselect->setAutoRaise(true);

    m_copyAction    = new QAction(QIcon(":/main/edit-copy.svg"),"", this);
    m_copyAction ->setEnabled(true);
    m_copyAction->setToolTip(tr("Copy Selection To ClipBoard"));
    m_bcopy->setDefaultAction(m_copyAction);
    m_bcopy->setAutoRaise(true);

    m_reloadAction  = new QAction(QIcon(":/main/reload-page.svg"),"", this);
    m_reloadAction ->setEnabled(true);
    m_reloadAction->setToolTip(tr("Update Preview Window"));
    m_breload->setDefaultAction(m_reloadAction);
    m_breload->setAutoRaise(true);

    m_cycleCSSAction = new QAction(QIcon(":/main/cycle-css.svg"),"", this);
    m_cycleCSSAction ->setEnabled(false);
    m_cycleCSSAction->setToolTip(tr("Cycle Custom CSS Files"));
    m_bcycle->setDefaultAction(m_cycleCSSAction);
    m_bcycle->setAutoRaise(true);

    QIcon pricon;
    pricon.addFile(":/main/document-print.svg", QSize(), QIcon::Normal);
    pricon.addFile(":/main/busy-working.svg", QSize(), QIcon::Disabled);
    m_webviewPrint = new QAction(pricon, "", this);
    m_webviewPrint ->setEnabled(true);
    m_webviewPrint->setToolTip(tr("Print Preview View"));
    m_bprint->setDefaultAction(m_webviewPrint);
    m_bprint->setAutoRaise(true);

    m_buttons->setContentsMargins(1,1,1,1);
    m_buttons->addWidget(m_binspect);
    m_buttons->addWidget(m_bselect);
    m_buttons->addWidget(m_bcopy);
    m_buttons->addWidget(m_breload);
    m_buttons->addWidget(m_bcycle);
    m_buttons->addWidget(m_bprint);
    m_buttons->addWidget(m_progress);

    m_Layout->addLayout(m_buttons);

    m_MainWidget->setLayout(m_Layout);

    setWidget(m_MainWidget);
}


void PreviewWindow::PrintStarted()
{
    m_webviewPrint ->setEnabled(false);
}

void PreviewWindow::PrintEnded()
{
    m_webviewPrint ->setEnabled(true);
}


void PreviewWindow::CycleCustomCSS()
{
    if (m_usercssurls.isEmpty()) return;
    m_cycleCSSLevel++;
    if (m_cycleCSSLevel > m_usercssurls.size()) m_cycleCSSLevel = 0;
    ReloadPreview();
}

// Note every call to Update Page needs to be followed by a call
// to Zoom() as it is not properly zooming after loading
// But Zoom() is not done synchronously so after zooming
// you must delay before trying to update Preview to a specific location
bool PreviewWindow::UpdatePage(QString filename_url, QString text, QList<ElementIndex> location)
{
    DBG qDebug() << "Entered PV UpdatePage with filename: " << filename_url;

    if (!m_Preview->isVisible()) {
        DBG qDebug() << "ignoring PV UpdatePage since PV is not visible";
        return true;
    }

    if (m_updatingPage) {
        DBG qDebug() << "delaying PV UpdatePage request as currently loading a page: ";
        return false;
    }

    m_updatingPage = true;
    SetCaretLocation(location);
    m_progress->setRange(0,100);
    m_progress->setValue(0);

    QRegularExpression mathused("<\\s*math [^>]*>");
    QRegularExpressionMatch mo = mathused.match(text);
    m_usingMathML = mo.hasMatch();

    DBG qDebug() << "PV UpdatePage " << filename_url;
    DBG { foreach(ElementIndex ei, location) qDebug()<< "PV name: " << ei.name << " index: " << ei.index; }


    //if isDarkMode is set, inject a local style in head
    SettingsStore settings;
    if (Utility::IsDarkMode() && settings.previewDark()) {
        text = Utility::AddDarkCSS(text);
        DBG qDebug() << "Preview injecting dark style: ";
    }
    m_Preview->page()->setBackgroundColor(Utility::WebViewBackgroundColor(true));

    // If the user has set a default stylesheet inject it last
    // so it can override anything above it
    if (!m_usercssurls.isEmpty() && (m_cycleCSSLevel > 0)) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_userstyles = 
              "<link rel=\"stylesheet\" type=\"text/css\" "
                "href=\"" + m_usercssurls.at(m_cycleCSSLevel - 1) + "\" />\n";
            DBG qDebug() << "Preview injecting stylesheet: " << inject_userstyles;
            text.insert(endheadpos, inject_userstyles);
        }
    }

    // If this page uses mathml tags, inject a polyfill
    // MathJax.js so that the mathml appears in the Preview Window
    if (m_usingMathML) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_mathjax;
            if (m_mathjaxurl.endsWith("startup.js")) {
                inject_mathjax = MATHJAX3_CONFIG + "<script type=\"text/javascript\" async=\"async\" id=\"MathJax-script\" src=\"" + m_mathjaxurl + "\"></script>\n";
            } else {
                inject_mathjax = "<script type=\"text/javascript\" async=\"async\" id=\"MathJax-script\" src=\"" + m_mathjaxurl + "\"></script>\n";
            }
            text.insert(endheadpos, inject_mathjax);
        }
    }

    m_Filepath = filename_url;
    m_Preview->CustomSetDocument(filename_url, text);

    m_progress->setValue(10);
    return true;
}

void PreviewWindow::UpdatePageDone()
{
    // ignore spurious page DocumentLoaded signals from ViewPreview
    if (!m_updatingPage) return;

    if (!m_Preview->WasLoadOkay()) qDebug() << "PV loadFinished with okay set to false!";
 
    DBG qDebug() << "PreviewWindow UpdatePage load is Finished";
    DBG qDebug() << "PreviewWindow UpdatePage final step scroll to location";

    // Zoom is handled internally to mPreview just before this is called
    UpdateWindowTitle();
    m_progress->setValue(100);
    m_progress->reset();
    // keep this hear in case things get out of sync
    m_Preview->HideOverlay();
    // need to delay long enough for Zoom changes to be reflected in View widget
    // before trying to center it on a location.
    QTimer::singleShot(30, this, SLOT(DelayedScrollTo()));
    m_updatingPage = false;
}

void PreviewWindow::DelayedScrollTo()
{
    m_Preview->StoreCaretLocationUpdate(m_location);
    m_Preview->ExecuteCaretUpdate();
}

void PreviewWindow::ScrollTo(QList<ElementIndex> location)
{
    DBG qDebug() << "received a PreviewWindow ScrollTo event";
    if (!m_Preview->isVisible()) {
        return;
    }
    DBG { foreach(ElementIndex ei, location) qDebug() << "name: " << ei.name << " index: " << ei.index; }
    SetCaretLocation(location);
    if (!m_updatingPage) {
        m_Preview->StoreCaretLocationUpdate(m_location);
        m_Preview->ExecuteCaretUpdate();
    }
}

void PreviewWindow::UpdateWindowTitle()
{
    if ((m_Preview) && m_Preview->isVisible()) {
        int height = m_Preview->height();
        int width = m_Preview->width();
        QString filename;
        if (!m_Filepath.isEmpty()) {
            filename = QFileInfo(m_Filepath).fileName();
        }
        setTitleText(tr("Preview") + 
                        " (" + QString::number(width) + "x" + QString::number(height) + ") " +
                        filename);
    }
    // qDebug() << "QDockWidget" << isFloating() << isVisible();
    if (isFloating()) {
        setWindowTitle(titleText());
    } else {
        setWindowTitle(tr("Preview"));
    }
}

// Set DockWidget titlebar text independently of tab text
// (when QDockWidget is tabified)
void PreviewWindow::setTitleText(const QString &text)
{
    m_titleText = text;
    // qDebug() << "In setTitleText: " << text;
    repaint();
}

const QString PreviewWindow::titleText()
{
    if (m_titleText.isEmpty()) {
        return windowTitle();
    }
    return m_titleText;
}

// Needed to update Preview's title when undocked on some platforms
void PreviewWindow::previewFloated(bool wasFloated) {
    // qDebug() << "In previewFloated (pre-if): " << wasFloated;
    if (wasFloated) {
        // qDebug() << "In previewFloated: (post-if)" << wasFloated;
        UpdateWindowTitle();
    }
}

QList<ElementIndex> PreviewWindow::GetCaretLocation()
{
    DBG qDebug() << "PreviewWindow in GetCaretLocation";
    QList<ElementIndex> hierarchy = m_Preview->GetCaretLocation();
    for (int i = 0; i < hierarchy.length(); i++) {
        DBG qDebug() << "name: " << hierarchy[i].name << " index: " << hierarchy[i].index;
    }
    return hierarchy;
}


void PreviewWindow::SetCaretLocation(const QList<ElementIndex> &loc)
{
    DBG qDebug() << "PreviewWindow in SetCaretLocation";
    QList<ElementIndex> hierarchy;
    foreach(ElementIndex ei, loc) {
        hierarchy << ei;
        DBG qDebug() << "name: " << ei.name << " index: " << ei.index;
    }
    m_location = hierarchy;
    // Any Zoom must come *before* we do any caret updating
    // *BUT* Zoom() does not complete instantaneously/synchronously
}

void PreviewWindow::SetZoomFactor(float factor)
{
    m_Preview->SetZoomFactor(factor);
}

void PreviewWindow::EmitGoToPreviewLocationRequest()
{
    DBG qDebug() << "EmitGoToPreviewLocationRequest request";
    emit GoToPreviewLocationRequest();
}

bool PreviewWindow::eventFilter(QObject *object, QEvent *event)
{
  switch (event->type()) {
    case QEvent::FocusIn:
      // track focus in change events of m_Preview (or one of its child focus proxies)
      if (m_use_focus_highlight) {

#if defined(Q_OS_MAC)
          QString focus_qss = "border: 3px solid HIGHLIGHT_COLOR;";
#elif defined(Q_OS_WIN32)
          QString focus_qss = "border: 1px solid HIGHLIGHT_COLOR;";
#else // Linux
          QString focus_qss = "border: 1px solid HIGHLIGHT_COLOR;";
#endif
          QString hcolor = palette().color(QPalette::Highlight).name();
          QString user_color = Utility::GetEnvironmentVar("SIGIL_FOCUS_HIGHLIGHT_COLOR");
          if (!user_color.isEmpty() && user_color.startsWith("#") && user_color.length() == 7) {
              if (QColor::isValidColorName(user_color)) {
                  hcolor = user_color;
              }
          }
	  focus_qss.replace("HIGHLIGHT_COLOR", hcolor);
	  m_wrapper->setStyleSheet(focus_qss);
      }
      DBG qDebug() << "focus in event: " << object;
      break;
    case QEvent::FocusOut:
      // track focus out change events of m_Preview (or one of its child focus proxies)
      if (m_use_focus_highlight) {
          m_wrapper->setStyleSheet("border: hidden;");
      }
      DBG qDebug() << "focus out event: " << object;
      break;
    case QEvent::ChildAdded:
      if (object == m_Preview) {
          const QChildEvent *childEvent(static_cast<QChildEvent*>(event));
          if (childEvent->child()) {
              DBG qDebug() << "added child: " << childEvent->child(); 
              childEvent->child()->installEventFilter(this);
          }
      }
      break;
    case QEvent::ChildRemoved:
      if (object == m_Preview) {
          const QChildEvent *childEvent(static_cast<QChildEvent*>(event));
          if (childEvent->child()) {
              DBG qDebug() << "removed child: " << childEvent->child(); 
              childEvent->child()->removeEventFilter(this);
          }
      }
      break;
    case QEvent::MouseButtonPress:
      {
          DBG qDebug() << "Preview mouse button press event " << object;
          const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
          if (mouseEvent) {
              if (mouseEvent->button() == Qt::LeftButton) {
                  DBG qDebug() << "Detected Left Mouse Button Press Event";
                  QString hoverurl = m_Preview->GetHoverUrl();
                  if (hoverurl.isEmpty()) {
                       DBG qDebug() << "emitting GoToPreviewLocationRequest";
                      QTimer::singleShot(50, this, SLOT(EmitGoToPreviewLocationRequest()));
                  } else {
                      QUrl link2url(hoverurl);
                      QUrl currenturl(m_Preview->url());
                      DBG qDebug() << "mouse press with : " << link2url.toString();
                      DBG qDebug() << "  with current url: " << currenturl.toString();
                      QString fragment;
                      if (link2url.hasFragment()) {
                          fragment = link2url.fragment();
                          link2url.setFragment(QString());
                      }
                      if (currenturl.hasFragment()) {
                          currenturl.setFragment(QString());
                      }
                      // test for local in-page link
                      // otherwise do nothing and acceptNavigationRequest will handle it
                      if (link2url == currenturl) {
                          DBG qDebug() << "we have a local link to fragment: " << fragment;
                          // tell current CV tab to scroll to fragment or top
                          emit ScrollToFragmentRequest(fragment);  
                       }
                  }
              } else if (mouseEvent->button() == Qt::RightButton) {
                  QString hoverurl = m_Preview->GetHoverUrl();
                  if (!hoverurl.isEmpty()) {
                      QApplication::clipboard()->setText(hoverurl);
                  }
              }
          }
      }
      break;
    case QEvent::MouseButtonRelease:
      {
          DBG qDebug() << "Preview mouse button release event " << object;
          const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
          if (mouseEvent) {
              if (mouseEvent->button() == Qt::LeftButton) {
                  DBG qDebug() << "Detected Left Mouse Button Release Event";
              }
          }
      }
      break;
    default:
      break;
  }
  return QObject::eventFilter(object, event);
}

void PreviewWindow::LinkClicked(const QUrl &url)
{
    DBG qDebug() << "in PreviewWindow LinkClicked with url :" << url.toString();

    if (url.toString().isEmpty()) {
        return;
    }

    QFileInfo finfo(m_Filepath);
    QString url_string = url.toString();

    // Convert fragments to full filename/fragments
    if (url_string.startsWith("#")) {
        url_string.prepend(finfo.fileName());
    } else if (url.scheme() == "file") {
        if (url_string.contains("/#")) {
            url_string.insert(url_string.indexOf("/#") + 1, finfo.fileName());
        }
    }
    emit OpenUrlRequest(QUrl(url_string));
}

void PreviewWindow::InspectorClosed(int i)
{
    DBG qDebug() << "received finished with argument: " << i;
}

void PreviewWindow::InspectPreviewPage()
{
    // non-modal dialog
    if (!m_Inspector->isVisible()) {
        DBG qDebug() << "inspecting";
        m_Inspector->InspectPageofView(m_Preview);
        m_Inspector->show();
        m_Inspector->raise();
        m_Inspector->activateWindow();
        return;
    }
    m_Inspector->StopInspection();
    m_Inspector->close();
}


void PreviewWindow::PrintRendered()
{
    // Refresh skipflags from Prefs
    SettingsStore settings;
    m_skipPrintPreview = settings.skipPrintPreview();
    m_WebViewPrinter->setContent(m_Filepath, m_Preview->GetHTML(), m_skipPrintPreview);

}

void PreviewWindow::SelectAllPreview()
{
    m_Preview->triggerPageAction(QWebEnginePage::SelectAll);
}

void PreviewWindow::CopyPreview()
{
    m_Preview->triggerPageAction(QWebEnginePage::Copy);
}

void PreviewWindow::ReloadPreview()
{
    // m_Preview->triggerPageAction(QWebEnginePage::ReloadAndBypassCache);
    // m_Preview->triggerPageAction(QWebEnginePage::Reload);
    //force reset m_updatingPage in case a signal is lost
    m_progress->reset();
    m_updatingPage = false;
    // m_Preview->ClearWebCache();
    emit RequestPreviewReload();
}

void PreviewWindow::setProgress(int val)
{
    if (val > 10 && val < 100) {
      m_progress->setValue(val);
    }
}

void PreviewWindow::LoadSettings()
{
    SettingsStore settings;
    m_use_focus_highlight = settings.uiHighlightFocusWidgetEnabled();
    // settings.beginGroup(SETTINGS_GROUP);
    // m_Layout->restoreState(settings.value("layout").toByteArray());
    // settings.endGroup();
}

void PreviewWindow::ConnectSignalsToSlots()
{
    connect(m_Preview,   SIGNAL(ZoomFactorChanged(float)),  this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_Preview,   SIGNAL(LinkClicked(const QUrl &)), this, SLOT(LinkClicked(const QUrl &)));
    connect(m_Preview,   SIGNAL(DocumentLoaded()),          this, SLOT(UpdatePageDone()));
    connect(m_Preview,   SIGNAL(ViewProgress(int)),         this, SLOT(setProgress(int)));
    connect(m_inspectAction,  SIGNAL(triggered()),          this, SLOT(InspectPreviewPage()));
    connect(m_selectAction,   SIGNAL(triggered()),          this, SLOT(SelectAllPreview()));
    connect(m_copyAction,     SIGNAL(triggered()),          this, SLOT(CopyPreview()));
    connect(m_reloadAction,   SIGNAL(triggered()),          this, SLOT(ReloadPreview()));
    connect(m_cycleCSSAction, SIGNAL(triggered()),          this, SLOT(CycleCustomCSS()));
    connect(m_webviewPrint,   SIGNAL(triggered()),          this, SLOT(PrintRendered()));
    connect(m_WebViewPrinter, SIGNAL(printStarted()),       this, SLOT(PrintStarted()));
    connect(m_WebViewPrinter, SIGNAL(printEnded()),         this, SLOT(PrintEnded()));
    connect(m_Inspector,      SIGNAL(finished(int)),        this, SLOT(InspectorClosed(int)));
    connect(this,     SIGNAL(topLevelChanged(bool)),        this, SLOT(previewFloated(bool)));
}

// Note: You can not use gumbo to perform the replacement as being
// a repair parser, it will fix all kinds of mistakes hiding the errors
bool PreviewWindow::fixup_fullscreen_svg_images(const QString &text) 
{
    GumboInterface gi = GumboInterface(text, "any_version");

    QList<GumboNode*> image_tags = gi.get_all_nodes_with_tag(GUMBO_TAG_IMAGE);
    if (image_tags.count() != 1) return false;

    QList<GumboNode*> svg_tags = gi.get_all_nodes_with_tag(GUMBO_TAG_SVG);
    if (svg_tags.count() != 1) return false;

    QList<GumboNode*> body_tags = gi.get_all_nodes_with_tag(GUMBO_TAG_BODY);
    if (body_tags.count() != 1) return false;
    
    GumboNode* image_node = image_tags.at(0);
    GumboNode* svg_node   = svg_tags.at(0);
    GumboNode* body_node  = body_tags.at(0);

    // loop through immediate children of body ignore script and style tags
    // and empty headers to make sure the div or svg is only child of body
    QStringList child_names;
    int elcount = 0;
    GumboVector* children = &body_node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*>(children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT) {
            QString name = QString::fromStdString(gi.get_tag_name(child));
            bool ignore_tag = (name == "script") || (name == "style");
            if (HEADERTAGS.contains(name)) {
                QString contents = gi.get_local_text_of_node(child);
                ignore_tag = ignore_tag || contents.isEmpty();
            }
            if (!ignore_tag) {
                child_names << name;
                elcount++;
            }
            if (elcount > 1) break;
        }
    }
    const QStringList allowed_tags = QStringList() << "div" << "svg"; 
    if ((elcount != 1) || !allowed_tags.contains(child_names.at(0))) return false;
    
    // verify either body->div->svg->image or body->svg->image 
    // structure exists (ignoring script and style tags)
    GumboNode* anode = image_node;
    QStringList path_pieces = QStringList() << QString::fromStdString(gi.get_tag_name(anode));
    while (anode && !((anode->type == GUMBO_NODE_ELEMENT) && (anode->v.element.tag == GUMBO_TAG_BODY))) {
        GumboNode* myparent = anode->parent;
        QString parent_name = QString::fromStdString(gi.get_tag_name(myparent));
        if ((parent_name != "script") && (parent_name != "style")) {
            path_pieces.prepend(parent_name);
        }
        anode = myparent;
    }
    const QString apath = path_pieces.join(",");
    if ((apath != "body,div,svg,image") && (apath != "body,svg,image")) return false;
    
    // finally check if svg height and width attributes are both "100%"
    // and if so change them to 100vh and 100vw respectively
    QHash<QString,QString> svgatts = gi.get_attributes_of_node(svg_node);
    if ((svgatts.value("width","") == "100%") && (svgatts.value("height","") == "100%")) {
        return true;
    }
    return false;
}
