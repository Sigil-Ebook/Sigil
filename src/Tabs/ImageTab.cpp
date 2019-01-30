/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QApplication>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QSignalMapper>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMenu>
#include <QtWebKitWidgets/QWebView>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>


#include "MainUI/MainWindow.h"
#include "Misc/OpenExternally.h"
#include "Misc/SettingsStore.h"
#include "ResourceObjects/ImageResource.h"
#include "Tabs/ImageTab.h"

const QString IMAGE_HTML_BASE =
    "<html>"
    "<head>"
    "<style type=\"text/css\">"
    "body { -webkit-user-select: none; }"
    "img { display: block; margin-left: auto; margin-right: auto; border-style: solid; border-width: 1px; }"
    "hr { width: 75%; }"
    "div { text-align: center; }"
    "</style>"
    "<body>"
    "<p><img src=\"%1\" /></p>"
    "<hr />"
    "<div>%2&times;%3px | %4 KB | %5%6</div>"
    "</body>"
    "</html>";

ImageTab::ImageTab(ImageResource *resource, QWidget *parent)
    :
    ContentTab(resource, parent),
    m_WebView(new QWebView(this)),
    m_ContextMenu(new QMenu(this)),
    m_OpenWithContextMenu(new QMenu(this)),
    m_openWithMapper(new QSignalMapper(this))
{
    m_WebView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_WebView->setFocusPolicy(Qt::NoFocus);
    m_WebView->setAcceptDrops(false);
    m_Layout->addWidget(m_WebView);
    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore settings;
    m_CurrentZoomFactor = settings.zoomImage();
    Zoom();
    CreateContextMenuActions();
    ConnectSignalsToSlots();
    RefreshContent();
}

ImageTab::~ImageTab()
{
    if (m_WebView) {
        delete m_WebView;
        m_WebView = 0;
    }

    if (m_ContextMenu) {
        delete m_ContextMenu;
        m_ContextMenu = 0;
    }

    if (m_OpenWithContextMenu) {
        delete m_OpenWithContextMenu;
        m_OpenWithContextMenu = 0;
    }

    if (m_OpenWith) {
        delete m_OpenWith;
        m_OpenWith = 0;
    }

    if (m_OpenWithEditor0) {
        delete m_OpenWithEditor0;
        m_OpenWithEditor0 = NULL;
    }

    if (m_OpenWithEditor1) {
        delete m_OpenWithEditor1;
        m_OpenWithEditor1 = NULL;
    }

    if (m_OpenWithEditor2) {
        delete m_OpenWithEditor2;
        m_OpenWithEditor2 = NULL;
    }

    if (m_OpenWithEditor3) {
        delete m_OpenWithEditor3;
        m_OpenWithEditor3 = NULL;
    }

    if (m_OpenWithEditor4) {
        delete m_OpenWithEditor4;
        m_OpenWithEditor4 = NULL;
    }

    if (m_SaveAs) {
        delete m_SaveAs;
        m_SaveAs = 0;
    }

    if (m_CopyImage) {
        delete m_CopyImage;
        m_CopyImage = 0;
    }
}


float ImageTab::GetZoomFactor() const
{
    return m_CurrentZoomFactor;
}

void ImageTab::SetZoomFactor(float new_zoom_factor)
{
    // Save the zoom for this type.
    SettingsStore settings;
    settings.setZoomImage(new_zoom_factor);
    m_CurrentZoomFactor = new_zoom_factor;
    Zoom();
    emit ZoomFactorChanged(m_CurrentZoomFactor);
}


void ImageTab::UpdateDisplay()
{
    SettingsStore settings;
    float stored_factor = settings.zoomImage();

    if (stored_factor != m_CurrentZoomFactor) {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
    }
}

void ImageTab::RefreshContent()
{
    MainWindow::clearMemoryCaches();
    const QString path = m_Resource->GetFullPath();
    const QFileInfo fileInfo = QFileInfo(path);
    const double ffsize = fileInfo.size() / 1024.0;
    const QString fsize = QLocale().toString(ffsize, 'f', 2);
    const QImage img(path);
    const QUrl imgUrl = QUrl::fromLocalFile(path);
    QString colors_shades = img.isGrayscale() ? tr("shades") : tr("colors");
    QString grayscale_color = img.isGrayscale() ? tr("Grayscale") : tr("Color");
    QString colorsInfo = "";

    if (img.depth() == 32) {
        colorsInfo = QString(" %1bpp").arg(img.bitPlaneCount());
    } else if (img.depth() > 0) {
        colorsInfo = QString(" %1bpp (%2 %3)").arg(img.bitPlaneCount()).arg(img.colorCount()).arg(colors_shades);
    }

    const QString html = IMAGE_HTML_BASE.arg(imgUrl.toString()).arg(img.width()).arg(img.height()).arg(fsize)
                         .arg(grayscale_color).arg(colorsInfo);
    m_WebView->setHtml(html, imgUrl);
}

void ImageTab::saveAs()
{
    const QVariant &data = m_SaveAs->data();

    if (data.isValid()) {
        const QUrl &url = data.toUrl();
        emit InsertedFileSaveAs(url);
    }
}

void ImageTab::copyImage()
{
    const QImage img(m_Resource->GetFullPath());
    QApplication::clipboard()->setImage(img);
}

void ImageTab::openWith()
{
    const QVariant &data = m_OpenWith->data();

    if (data.isValid()) {
        const QUrl &resourceUrl = data.toUrl();
        const QString &editorPath = OpenExternally::selectEditorForResourceType((Resource::ResourceType) resourceUrl.port());

        if (!editorPath.isEmpty()) {
            if (OpenExternally::openFile(resourceUrl.toLocalFile(), editorPath)) {
                const QString &pathname = resourceUrl.toString();
                emit InsertedFileOpenedExternally(pathname);
            }
        }
    }
}

void ImageTab::openWithEditor(int slotnum)
{
    QAction * oeaction = NULL;
    if (slotnum == 0) oeaction = m_OpenWithEditor0;
    if (slotnum == 1) oeaction = m_OpenWithEditor1;
    if (slotnum == 2) oeaction = m_OpenWithEditor2;
    if (slotnum == 3) oeaction = m_OpenWithEditor3;
    if (slotnum == 4) oeaction = m_OpenWithEditor4;
    if (oeaction) {
        const QVariant &data = oeaction->data();
	const QUrl resourceUrl = data.toUrl();
	const QString action_name = oeaction->text();
        const QStringList editor_paths = OpenExternally::editorsForResourceType(
                                            (Resource::ResourceType) resourceUrl.port());
        const QStringList editor_names = OpenExternally::editorDescriptionsForResourceType(
                                            (Resource::ResourceType) resourceUrl.port());
	QString editor_path = QString();
	int i = 0;
	foreach(QString ename, editor_names) {
	    if (ename == action_name) {
	        editor_path = editor_paths[i];
		break;
	    }
	    i = i + 1;
	}
        if (data.isValid() && !editor_path.isEmpty()) {
            const QUrl &resourceUrl = data.toUrl();
            if (OpenExternally::openFile(resourceUrl.toLocalFile(), editor_path)) {
                const QString &pathname = resourceUrl.toString();
                emit InsertedFileOpenedExternally(pathname);
            }
        }
    }
}


void ImageTab::OpenContextMenu(const QPoint &point)
{
    if (!SuccessfullySetupContextMenu(point)) {
        return;
    }

    m_ContextMenu->exec(mapToGlobal(point));
    m_ContextMenu->clear();
}

bool ImageTab::SuccessfullySetupContextMenu(const QPoint &point)
{
    QUrl imageUrl("file:///" % m_Resource->GetFullPath());

    if (imageUrl.isValid() && imageUrl.isLocalFile()) {
        // Open With
        Resource::ResourceType imageType = Resource::ImageResourceType;

        if (imageUrl.path().toLower().endsWith(".svg")) {
            imageType = Resource::SVGResourceType;
        }

        imageUrl.setPort(imageType);   // "somewhat" ugly, but cheaper than using a QList<QVariant>
        QStringList editors = OpenExternally::editorsForResourceType(imageType);
	QStringList editor_names = OpenExternally::editorDescriptionsForResourceType(imageType);

        if (editors.isEmpty()) {
            m_OpenWithEditor0->setData(QVariant::Invalid);
            m_OpenWithEditor1->setData(QVariant::Invalid);
            m_OpenWithEditor2->setData(QVariant::Invalid);
            m_OpenWithEditor3->setData(QVariant::Invalid);
            m_OpenWithEditor4->setData(QVariant::Invalid);
            m_OpenWith->setText(tr("Open With") + "...");
            m_OpenWith->setData(imageUrl);
            m_ContextMenu->addAction(m_OpenWith);
        } else {
	    // clear previous open with action info                                                                 
	    for (int k = 0; k < 5; k++) {
	        QAction * oeaction = NULL;
	        if (k==0) oeaction = m_OpenWithEditor0;
	        if (k==1) oeaction = m_OpenWithEditor1;
	        if (k==2) oeaction = m_OpenWithEditor2;
	        if (k==3) oeaction = m_OpenWithEditor3;
	        if (k==4) oeaction = m_OpenWithEditor4;
	        if (oeaction) {
	            oeaction->setData(QVariant::Invalid);
	            oeaction->setText("");
	            oeaction->setEnabled(false);
	            oeaction->setVisible(false);
	        }
	    }
	    int i = 0;
	    foreach(QString editor, editors) {
	        const QString aname = editor_names[i];
		QAction * oeaction = NULL;
		if (i == 0) oeaction = m_OpenWithEditor0;
		if (i == 1) oeaction = m_OpenWithEditor1;
		if (i == 2) oeaction = m_OpenWithEditor2;
		if (i == 3) oeaction = m_OpenWithEditor3;
		if (i == 4) oeaction = m_OpenWithEditor4;
		if (oeaction) {
                    oeaction->setText(aname);
                    oeaction->setData(imageUrl);
		    oeaction->setVisible(true);
		}
		i = i + 1;
	    }
            m_OpenWith->setText(tr("Other Application") + "...");
            m_OpenWith->setData(imageUrl);
            m_ContextMenu->addMenu(m_OpenWithContextMenu);
        }

        // Save As
        m_SaveAs->setData(imageUrl);
        m_ContextMenu->addAction(m_SaveAs);
        m_ContextMenu->addSeparator();
        m_ContextMenu->addAction(m_CopyImage);
    }

    return true;
}

void ImageTab::CreateContextMenuActions()
{
    m_OpenWithEditor0 = new QAction("",          this);
    m_OpenWithEditor1 = new QAction("",          this);
    m_OpenWithEditor2 = new QAction("",          this);
    m_OpenWithEditor3 = new QAction("",          this);
    m_OpenWithEditor4 = new QAction("",          this);
    m_OpenWith        = new QAction(tr("Open With") + "...",  this);
    m_SaveAs          = new QAction(tr("Save As") + "...",  this);
    m_CopyImage       = new QAction(tr("Copy Image"),  this);
    m_OpenWithContextMenu->setTitle(tr("Open With"));
    m_OpenWithContextMenu->addAction(m_OpenWithEditor0);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor1);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor2);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor3);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor4);
    m_OpenWithContextMenu->addAction(m_OpenWith);
}

void ImageTab::ConnectSignalsToSlots()
{
    connect(m_Resource, SIGNAL(ResourceUpdatedOnDisk()), this, SLOT(RefreshContent()));
    connect(m_Resource, SIGNAL(Deleted(Resource)), this, SLOT(Close()));
    connect(m_WebView, SIGNAL(customContextMenuRequested(const QPoint &)),  this, SLOT(OpenContextMenu(const QPoint &)));
    connect(m_OpenWith,       SIGNAL(triggered()),   this, SLOT(openWith()));
    connect(m_OpenWithEditor0, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor0, 0);
    connect(m_OpenWithEditor1, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor1, 1);
    connect(m_OpenWithEditor2, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor2, 2);
    connect(m_OpenWithEditor3, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor3, 3);
    connect(m_OpenWithEditor4, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor4, 4);
    connect(m_openWithMapper, SIGNAL(mapped(int)), this, SLOT(openWithEditor(int)));
    connect(m_SaveAs,         SIGNAL(triggered()),   this, SLOT(saveAs()));
    connect(m_CopyImage,      SIGNAL(triggered()),   this, SLOT(copyImage()));
}

void ImageTab::Zoom()
{
    m_WebView->setZoomFactor(m_CurrentZoomFactor);
}

void ImageTab::PrintPreview()
{
    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog(this);
    connect(print_preview, SIGNAL(paintRequested(QPrinter *)), m_WebView, SLOT(print(QPrinter *)));
    print_preview->exec();
    print_preview->deleteLater();
}

void ImageTab::Print()
{
    QPrinter printer;
    QPrintDialog print_dialog(&printer, this);
    print_dialog.setWindowTitle(tr("Print %1").arg(GetFilename()));

    if (print_dialog.exec() == QDialog::Accepted) {
        m_WebView->print(&printer);
    }
}


