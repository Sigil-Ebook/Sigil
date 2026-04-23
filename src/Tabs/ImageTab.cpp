/************************************************************************
**
**  Copyright (C) 2015-2026 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#include <QGuiApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QSignalMapper>
#include <QString>
#include <QUrl>
#include <QClipboard>
#include <QLayout>
#include <QMenu>
#include <QVariant>
#include <QDebug>

#include "MainUI/MainWindow.h"
#include "ViewEditors/SimplePage.h"
#include "Misc/OpenExternally.h"
#include "Misc/SettingsStore.h"
#include "Misc/WebProfileMgr.h"
#include "Misc/Utility.h"
#include "Misc/webviewprinter.h"
#include "ResourceObjects/ImageResource.h"
#include "sigil_constants.h"
#include "Widgets/AdjustImage.h"
#include "Tabs/ImageTab.h"

#include <QMetaType>
static const QVariant QVINVALID = QVariant(QMetaType(QMetaType::UnknownType));

const QString IMAGE_HTML_BASE =
    "<html>"
    "<head>"
    "<style type=\"text/css\">"
    "body { -webkit-user-select: none; }"
    "img { display: block; margin-left: auto; margin-right: auto; border-style: solid; border-width: 1px; }"
    "hr { width: 75%; }"
    "div { text-align: center; }"
    "</style>"
    "</head>"
    "<body>"
    "<div>%2&times;%3px | %4 KB | %5%6</div>"
    "<hr />"
    "<p><img src=\"%1\" /></p>"
    "</body>"
    "</html>";

ImageTab::ImageTab(ImageResource *resource, QWidget *parent)
    :
    ContentTab(resource, parent),
    m_ContextMenu(new QMenu(this)),
    m_OpenWithContextMenu(new QMenu(this)),
    m_AdjImg(nullptr),
    m_openWithMapper(new QSignalMapper(this)),
    m_WebViewPrinter(new WebViewPrinter(this))
{
    const QString path = m_Resource->GetFullPath();
    const QString mediatype = m_Resource->GetMediaType();
    m_AdjImg = new AdjustImage(path, mediatype, this);
    m_AdjImg->setContextMenuPolicy(Qt::CustomContextMenu);
    m_Layout->setContentsMargins(2,2,2,2);
    m_Layout->addWidget(m_AdjImg);
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
    delete m_ContextMenu;
    delete m_OpenWithContextMenu;
    delete m_OpenWith;
    delete m_OpenWithEditor0;
    delete m_OpenWithEditor1;
    delete m_OpenWithEditor2;
    delete m_OpenWithEditor3;
    delete m_OpenWithEditor4;
    delete m_SaveAs;
    delete m_CopyImage;
    delete m_SaveChanges;
    delete m_ZoomIn;
    delete m_ZoomOut;
    delete m_ZoomToFit;
    delete m_Undo;
    delete m_Redo;
    delete m_RotateLeft;
    delete m_RotateRight;
    delete m_CropImage;
    delete m_ResizeImage;
}

// map context menu actions into AdjustImage routines
void ImageTab::SaveChanges() { m_AdjImg->doSave();        }
void ImageTab::ZoomIn()      { m_AdjImg->doZoomIn();      }
void ImageTab::ZoomOut()     { m_AdjImg->doZoomOut();     }
void ImageTab::ZoomToFit()   { m_AdjImg->doZoomToFit();   }
void ImageTab::Undo()        { m_AdjImg->doUndo();        }
void ImageTab::Redo()        { m_AdjImg->doRedo();        }
void ImageTab::RotateLeft()  { m_AdjImg->doRotateLeft();  }
void ImageTab::RotateRight() { m_AdjImg->doRotateRight(); }
void ImageTab::CropImage()   { m_AdjImg->doCrop();        }
void ImageTab::ResizeImage() { m_AdjImg->doResizeImage(); }

float ImageTab::GetZoomFactor() const
{
    float zf = (float) m_AdjImg->getZoomFactor();
    return zf;
    // return m_CurrentZoomFactor();
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

void ImageTab::HandleInternalImageZoomChange(double factor)
{
    float zf = (float) factor;
    SetZoomFactor(zf);
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

void ImageTab::ThemeChangeRefresh()
{
    RefreshContent();
}

void ImageTab::RefreshContent()
{
    const QString path = m_Resource->GetFullPath();
    const QString mediatype = m_Resource->GetMediaType();
    if (!m_AdjImg) {
        m_AdjImg = new AdjustImage(path, mediatype, this);
    }
    m_filepath = path;
    m_AdjImg->hide();
    m_AdjImg->show();
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
        const QString &editorPath = OpenExternally::selectEditorForResourceType((Resource::ResourceType) resourceUrl.port(), this);

        if (!editorPath.isEmpty()) {
            if (OpenExternally::openFile(resourceUrl.toLocalFile(), editorPath)) {
                const QString bookpath= GetLoadedResource()->GetRelativePath();
                emit InsertedFileOpenedExternally(bookpath);
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
                const QString bookpath = GetLoadedResource()->GetRelativePath();
                emit InsertedFileOpenedExternally(bookpath);
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
    if (!m_ContextMenu.isNull()) {
        m_ContextMenu->clear();
    }
}

bool ImageTab::SuccessfullySetupContextMenu(const QPoint &point)
{
    QUrl imageUrl = QUrl::fromLocalFile(m_Resource->GetFullPath());

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
            m_OpenWithEditor0->setData(QVINVALID);
            m_OpenWithEditor1->setData(QVINVALID);
            m_OpenWithEditor2->setData(QVINVALID);
            m_OpenWithEditor3->setData(QVINVALID);
            m_OpenWithEditor4->setData(QVINVALID);
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
                    oeaction->setData(QVINVALID);
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
                    oeaction->setEnabled(true);
                    oeaction->setVisible(true);
                }
                i = i + 1;
            }
            m_OpenWith->setText(tr("Other Application") + "...");
            m_OpenWith->setData(imageUrl);
            m_ContextMenu->addMenu(m_OpenWithContextMenu);
        }

        // handle conditional menu items
        m_CropImage->setEnabled(m_AdjImg->isCropEnabled());
        m_Undo->setEnabled(m_AdjImg->isUndoEnabled());
        m_Redo->setEnabled(m_AdjImg->isRedoEnabled());

        // Save As
        m_SaveAs->setData(imageUrl);
        m_ContextMenu->addAction(m_SaveAs);
        m_ContextMenu->addAction(m_CopyImage);
        m_ContextMenu->addSeparator();
        m_ContextMenu->addAction(m_SaveChanges);
        m_ContextMenu->addSeparator();
        m_ContextMenu->addAction(m_ZoomIn);
        m_ContextMenu->addAction(m_ZoomOut);
        m_ContextMenu->addAction(m_ZoomToFit);
        m_ContextMenu->addSeparator();
        m_ContextMenu->addAction(m_Undo);
        m_ContextMenu->addAction(m_Redo);
        m_ContextMenu->addSeparator();
        m_ContextMenu->addAction(m_RotateLeft);
        m_ContextMenu->addAction(m_RotateRight);
        m_ContextMenu->addSeparator();
        m_ContextMenu->addAction(m_CropImage);
        m_ContextMenu->addAction(m_ResizeImage);
    }

    return true;
}

void ImageTab::CreateContextMenuActions()
{
    m_OpenWithEditor0 = new QAction("", this);
    m_OpenWithEditor1 = new QAction("", this);
    m_OpenWithEditor2 = new QAction("", this);
    m_OpenWithEditor3 = new QAction("", this);
    m_OpenWithEditor4 = new QAction("", this);
    m_OpenWith        = new QAction(tr("Open With") + "...",  this);
    m_OpenWithContextMenu->setTitle(tr("Open With"));
    m_OpenWithContextMenu->addAction(m_OpenWithEditor0);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor1);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor2);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor3);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor4);
    m_OpenWithContextMenu->addAction(m_OpenWith);
    m_SaveAs          = new QAction(tr("Save As") + "...",  this);
    m_CopyImage       = new QAction(tr("Copy Image"),  this);
    
    m_SaveChanges     = new QAction(tr("Save Changes"), this);
    m_ZoomIn          = new QAction(tr("Zoom In"),      this);
    m_ZoomOut         = new QAction(tr("Zoom Out"),     this);
    m_ZoomToFit       = new QAction(tr("Zoom to Fit"),  this);
    m_Undo            = new QAction(tr("Undo Change"),  this);
    m_Redo            = new QAction(tr("Redo Change"),  this);
    m_RotateLeft      = new QAction(tr("Rotate Left"),  this);
    m_RotateRight     = new QAction(tr("Rotate Right"), this);
    m_CropImage       = new QAction(tr("Crop Image"),   this);
    m_ResizeImage     = new QAction(tr("Resize Image"), this);
}

void ImageTab::ConnectSignalsToSlots()
{
    connect(m_Resource, SIGNAL(ResourceUpdatedOnDisk()),   this, SLOT(RefreshContent()));
    connect(m_Resource, SIGNAL(Deleted(const Resource *)), this, SLOT(Close()));

    connect(m_AdjImg, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(OpenContextMenu(const QPoint &)));
    connect(m_AdjImg, SIGNAL(InternalZoomFactorChanged(double)),  this, SLOT(HandleInternalImageZoomChange(double)));

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

    connect(m_openWithMapper, SIGNAL(mappedInt(int)), this, SLOT(openWithEditor(int)));
    connect(m_SaveAs,         SIGNAL(triggered()),    this, SLOT(saveAs()));
    connect(m_CopyImage,      SIGNAL(triggered()),    this, SLOT(copyImage()));
    connect(m_SaveChanges,    SIGNAL(triggered()),    this, SLOT(SaveChanges()));
    connect(m_ZoomIn,         SIGNAL(triggered()),    this, SLOT(ZoomIn()));
    connect(m_ZoomOut,        SIGNAL(triggered()),    this, SLOT(ZoomOut()));
    connect(m_ZoomToFit,      SIGNAL(triggered()),    this, SLOT(ZoomToFit()));
    connect(m_Undo,           SIGNAL(triggered()),    this, SLOT(Undo()));
    connect(m_Redo,           SIGNAL(triggered()),    this, SLOT(Redo()));
    connect(m_RotateLeft,     SIGNAL(triggered()),    this, SLOT(RotateLeft()));
    connect(m_RotateRight,    SIGNAL(triggered()),    this, SLOT(RotateRight()));
    connect(m_CropImage,      SIGNAL(triggered()),    this, SLOT(CropImage()));
    connect(m_ResizeImage,    SIGNAL(triggered()),    this, SLOT(ResizeImage()));
}

void ImageTab::Zoom()
{
    m_AdjImg->scaleImageUsing(m_CurrentZoomFactor);
}

QString ImageTab::PageSourceForPrinting(QString& file_path)
{
    const QString path = m_Resource->GetFullPath();
    const QString folderpath = m_Resource->GetFullFolderPath();
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

    QString html = IMAGE_HTML_BASE.arg(imgUrl.toString()).arg(img.width()).arg(img.height()).arg(fsize)
                         .arg(grayscale_color).arg(colorsInfo);
    file_path = folderpath + "/temp.xhtml";
    return html;
}

void ImageTab::PrintPreview()
{
    QString file_path;
    QString page_source = PageSourceForPrinting(file_path);
    m_WebViewPrinter->setContent(file_path, page_source, false);
}

void ImageTab::Print()
{
    QString file_path;
    QString page_source = PageSourceForPrinting(file_path);
    m_WebViewPrinter->setContent(file_path, page_source, true);
}
