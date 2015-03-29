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

ImageTab::ImageTab(ImageResource &resource, QWidget *parent)
    :
    ContentTab(resource, parent),
    m_WebView(*new QWebView(this)),
    m_ContextMenu(*new QMenu(this)),
    m_OpenWithContextMenu(*new QMenu(this))
{
    m_WebView.setContextMenuPolicy(Qt::CustomContextMenu);
    m_WebView.setFocusPolicy(Qt::NoFocus);
    m_WebView.setAcceptDrops(false);
    m_Layout.addWidget(&m_WebView);
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
    if (m_OpenWith) {
        delete m_OpenWith;
        m_OpenWith = 0;
    }

    if (m_OpenWithEditor) {
        delete m_OpenWithEditor;
        m_OpenWithEditor = 0;
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
    const QString path = m_Resource.GetFullPath();
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
    m_WebView.setHtml(html, imgUrl);
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
    const QImage img(m_Resource.GetFullPath());
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

void ImageTab::openWithEditor()
{
    const QVariant &data = m_OpenWithEditor->data();

    if (data.isValid()) {
        const QUrl &resourceUrl = data.toUrl();
        const QString &editorPath = OpenExternally::editorForResourceType((Resource::ResourceType) resourceUrl.port());

        if (OpenExternally::openFile(resourceUrl.toLocalFile(), editorPath)) {
            const QString &pathname = resourceUrl.toString();
            emit InsertedFileOpenedExternally(pathname);
        }
    }
}


void ImageTab::OpenContextMenu(const QPoint &point)
{
    if (!SuccessfullySetupContextMenu(point)) {
        return;
    }

    m_ContextMenu.exec(mapToGlobal(point));
    m_ContextMenu.clear();
}

bool ImageTab::SuccessfullySetupContextMenu(const QPoint &point)
{
    QUrl imageUrl("file:///" % m_Resource.GetFullPath());

    if (imageUrl.isValid() && imageUrl.isLocalFile()) {
        // Open With
        Resource::ResourceType imageType = Resource::ImageResourceType;

        if (imageUrl.path().toLower().endsWith(".svg")) {
            imageType = Resource::SVGResourceType;
        }

        imageUrl.setPort(imageType);   // "somewhat" ugly, but cheaper than using a QList<QVariant>
        const QString &editorPath = OpenExternally::editorForResourceType(imageType);

        if (editorPath.isEmpty()) {
            m_OpenWithEditor->setData(QVariant::Invalid);
            m_OpenWith->setText(tr("Open With") + "...");
            m_OpenWith->setData(imageUrl);
            m_ContextMenu.addAction(m_OpenWith);
        } else {
            const QString &editorDescription = OpenExternally::editorDescriptionForResourceType(imageType);
            m_OpenWithEditor->setText(editorDescription);
            m_OpenWithEditor->setData(imageUrl);
            m_OpenWith->setText(tr("Other Application") + "...");
            m_OpenWith->setData(imageUrl);
            m_ContextMenu.addMenu(&m_OpenWithContextMenu);
        }

        // Save As
        m_SaveAs->setData(imageUrl);
        m_ContextMenu.addAction(m_SaveAs);
        m_ContextMenu.addSeparator();
        m_ContextMenu.addAction(m_CopyImage);
    }

    return true;
}

void ImageTab::CreateContextMenuActions()
{
    m_OpenWithEditor = new QAction("",          this);
    m_OpenWith       = new QAction(tr("Open With") + "...",  this);
    m_SaveAs         = new QAction(tr("Save As") + "...",  this);
    m_CopyImage      = new QAction(tr("Copy Image"),  this);
    m_OpenWithContextMenu.setTitle(tr("Open With"));
    m_OpenWithContextMenu.addAction(m_OpenWithEditor);
    m_OpenWithContextMenu.addAction(m_OpenWith);
}

void ImageTab::ConnectSignalsToSlots()
{
    connect(&m_Resource, SIGNAL(ResourceUpdatedOnDisk()), this, SLOT(RefreshContent()));
    connect(&m_Resource, SIGNAL(Deleted(Resource)), this, SLOT(Close()));
    connect(&m_WebView, SIGNAL(customContextMenuRequested(const QPoint &)),  this, SLOT(OpenContextMenu(const QPoint &)));
    connect(m_OpenWith,       SIGNAL(triggered()),  this, SLOT(openWith()));
    connect(m_OpenWithEditor, SIGNAL(triggered()),  this, SLOT(openWithEditor()));
    connect(m_SaveAs,         SIGNAL(triggered()),  this, SLOT(saveAs()));
    connect(m_CopyImage,      SIGNAL(triggered()),  this, SLOT(copyImage()));
}

void ImageTab::Zoom()
{
    m_WebView.setZoomFactor(m_CurrentZoomFactor);
}

void ImageTab::PrintPreview()
{
    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog(this);
    connect(print_preview, SIGNAL(paintRequested(QPrinter *)), &m_WebView, SLOT(print(QPrinter *)));
    print_preview->exec();
    print_preview->deleteLater();
}

void ImageTab::Print()
{
    QPrinter printer;
    QPrintDialog print_dialog(&printer, this);
    print_dialog.setWindowTitle(tr("Print %1").arg(GetFilename()));

    if (print_dialog.exec() == QDialog::Accepted) {
        m_WebView.print(&printer);
    }
}


