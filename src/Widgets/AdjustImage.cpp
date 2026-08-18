/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Kevin B. Hendricks, Stratford, ON, Canada
 *   
 *  Based on wojtodzio/ImageViewer from github with lots of bug fixes
 *      and improvements added, and modified to be a QWidget to work
 *      inside Sigil.
 * 
 *      Original code was: Copyright (c) 2016 Wojciech Wrona 
 *                         with this MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cmath>
#include <QTransform>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QLocale>
#include <QImageWriter>
#include <QInputDialog>
#include <QKeySequence>
#include <QPushButton>
#include "Misc/SettingsStore.h"
#include "Dialogs/ImageResizeDialog.h"
#include "Widgets/AdjustImage.h"
#include "ui_AdjustImage.h"

static const QString SETTINGS_GROUP = "adjust_image";
static QStringList SAVE_QUALITY_MEDIATYPES = QStringList() << "image/jpeg" << "image/webp" << "image/avif" << "image/jxl";
static const int HANDLE_SIZE = 10;
static const int MIN_CROP_SIZE = 2;

AdjustImage::AdjustImage(const QString filepath, const QString& mediatype,  QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdjustImage),
    m_mediatype(mediatype),
    m_draggingHandle(-1),
    m_lastMousePos(0, 0),
    m_cropConfirmBtn(nullptr),
    m_cropCancelBtn(nullptr)
{
    ui->setupUi(this);
    m_mainToolBar = ui->mainToolBar;
    m_statusBar = ui->statusBar;

    updateActions(false);
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    m_imageLabel = new QLabel;
    m_imageLabel->resize(0, 0);
    m_imageLabel->setMouseTracking(true);
    m_imageLabel->setBackgroundRole(QPalette::Base);
    m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_imageLabel->setScaledContents(true);
    // Center align the image
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->installEventFilter(this);
    // rubber band must be a child of the m_imageLabel
    // otherwise there is a coordinate nightmare
    m_rb = new QRubberBand(QRubberBand::Rectangle, m_imageLabel);
    m_rb->hide();

    m_scrollArea = new QScrollArea;
    m_scrollArea->setBackgroundRole(QPalette::Dark);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_imageLabel);
    m_scrollArea->setWidgetResizable(true);

    m_description = new QLabel;
    m_statusBar->addPermanentWidget(m_description);
    
    // Create crop confirmation buttons
    CreateCropButtons();
    
    // update tooltips on toolbar icons to include shortcut in a platform specific manner
    extendToolTip(ui->actionSave,        "Ctrl+S");    
    extendToolTip(ui->actionZoomIn,      "Ctrl++");
    extendToolTip(ui->actionZoomOut,     "Ctrl+-");
    extendToolTip(ui->actionZoomToFit,   "Ctrl+F");
    extendToolTip(ui->actionUndo,        "Ctrl+Z");
    extendToolTip(ui->actionRedo,        "Ctrl+Y");
    extendToolTip(ui->actionRotateLeft,  "Ctrl+L");
    extendToolTip(ui->actionRotateRight, "Ctrl+R");
    extendToolTip(ui->actionCrop,        "Ctrl+K");
    extendToolTip(ui->actionResizeImage, "Ctrl+E");
        
    vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(2,2,2,2);
    vlayout->addWidget(m_mainToolBar);
    vlayout->addWidget(m_scrollArea);
    vlayout->addWidget(m_statusBar);
    setLayout(vlayout);

    setWindowTitle(tr("Adjust Image"));
    if (!filepath.isEmpty()) {
        m_fileName = filepath;
        m_ffsize = QFile(m_fileName).size() / 1024.0;
        m_fsize =  QLocale().toString(m_ffsize, 'f', 2);
        m_image = QImage(m_fileName);
        if (m_image.isNull()) {
             QMessageBox::information(this,
                                      tr("Adjust Image"),
                                      tr("Cannot load %1.").arg(m_fileName));
             return;
        }
        m_scaleFactor = 1.0;
        m_croppingState = false;
        m_croppingRegionSelected = false;
        m_draggingHandle = -1;
        setCursor(Qt::ArrowCursor);
        updateActions(true);
        refreshLabel();
    }
    ConnectSignalsToSlots();
    ReadSettings();
}

AdjustImage::~AdjustImage()
{
    WriteSettings();
    m_history.clear();
    m_reverseHistory.clear();
    delete ui;
}

bool AdjustImage::isCropEnabled() { return ui->actionCrop->isEnabled(); }  
bool AdjustImage::isUndoEnabled() { return ui->actionUndo->isEnabled(); }
bool AdjustImage::isRedoEnabled() { return ui->actionRedo->isEnabled(); }


void AdjustImage::CreateCropButtons()
{
    // Create Confirm button
    m_cropConfirmBtn = new QPushButton(tr("Confirm"));
    m_cropConfirmBtn->setToolTip(tr("Confirm crop (Enter)"));
    m_cropConfirmBtn->setMaximumWidth(80);
    m_cropConfirmBtn->hide();
    m_statusBar->addPermanentWidget(m_cropConfirmBtn);
    
    // Create Cancel button
    m_cropCancelBtn = new QPushButton(tr("Cancel"));
    m_cropCancelBtn->setToolTip(tr("Cancel crop (Esc)"));
    m_cropCancelBtn->setMaximumWidth(80);
    m_cropCancelBtn->hide();
    m_statusBar->addPermanentWidget(m_cropCancelBtn);
    
    // Connect buttons
    connect(m_cropConfirmBtn, SIGNAL(clicked()), this, SLOT(doCropConfirm()));
    connect(m_cropCancelBtn, SIGNAL(clicked()), this, SLOT(doCropCancel()));
}

void AdjustImage::extendToolTip(QAction*m, const QString sc)
{
    QString shct = QKeySequence(sc).toString(QKeySequence::NativeText);
    QString current_tip = m->toolTip();
    m->setToolTip(current_tip + " (" + shct + ")");
}

void AdjustImage::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    m_jpeg_quality = settings.value("jpeg_quality", QVariant(93)).toInt();
    m_webp_quality = settings.value("webp_quality", QVariant(90)).toInt();
    m_jxl_quality =  settings.value("jxl_quality", QVariant(93)).toInt();
    m_avif_quality = settings.value("avif_quality", QVariant(90)).toInt();
    settings.endGroup();
}


void AdjustImage::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("jpeg_quality", m_jpeg_quality);
    settings.setValue("webp_quality", m_webp_quality);
    settings.setValue("jxl_quality",  m_jxl_quality);
    settings.setValue("avif_quality", m_avif_quality);
    settings.endGroup();
}

QRect AdjustImage::BuildRect(const QPoint& p1, const QPoint& p2)
{
    QRect arect = QRect(p1, p2).normalized();
    if ((arect.x() < 2) && (arect.y() < 2)) {
        arect.setX(0);
        arect.setY(0);
    }
    return arect;
}

int AdjustImage::GetHandleAtPosition(const QPoint& pos)
{
    QRect cropRect = BuildRect(m_rbstart, m_rbend);
    int handle = -1;
    
    // Check which handle is being clicked (top-left, top-right, bottom-left, bottom-right, or edges)
    if (QRect(cropRect.topLeft().x() - HANDLE_SIZE, cropRect.topLeft().y() - HANDLE_SIZE, HANDLE_SIZE * 2, HANDLE_SIZE * 2).contains(pos)) {
        handle = 0; // top-left
    } else if (QRect(cropRect.topRight().x() - HANDLE_SIZE, cropRect.topRight().y() - HANDLE_SIZE, HANDLE_SIZE * 2, HANDLE_SIZE * 2).contains(pos)) {
        handle = 1; // top-right
    } else if (QRect(cropRect.bottomLeft().x() - HANDLE_SIZE, cropRect.bottomLeft().y() - HANDLE_SIZE, HANDLE_SIZE * 2, HANDLE_SIZE * 2).contains(pos)) {
        handle = 2; // bottom-left
    } else if (QRect(cropRect.bottomRight().x() - HANDLE_SIZE, cropRect.bottomRight().y() - HANDLE_SIZE, HANDLE_SIZE * 2, HANDLE_SIZE * 2).contains(pos)) {
        handle = 3; // bottom-right
    } else if (cropRect.contains(pos)) {
        handle = 4; // move entire rectangle
    }
    
    return handle;
}

void AdjustImage::clampCropRectToBounds()
{
    // Calculate the display bounds of the image
    int maxDisplayX = static_cast<int>(m_scaleFactor * m_image.width());
    int maxDisplayY = static_cast<int>(m_scaleFactor * m_image.height());
    
    // Clamp rubber band coordinates to image bounds
    m_rbstart.setX(std::max(0, std::min(m_rbstart.x(), maxDisplayX)));
    m_rbstart.setY(std::max(0, std::min(m_rbstart.y(), maxDisplayY)));
    
    m_rbend.setX(std::max(0, std::min(m_rbend.x(), maxDisplayX)));
    m_rbend.setY(std::max(0, std::min(m_rbend.y(), maxDisplayY)));
}

void AdjustImage::UpdateImageDescription()
{
    QString colors_shades = m_image.isGrayscale() ? tr("shades") : tr("colors");
    QString grayscale_color = m_image.isGrayscale() ? tr("Grayscale") : tr("Color");
    QString colorsInfo = "";
    if (m_image.depth() == 32) {
        colorsInfo = QString(" %1bpp").arg(m_image.bitPlaneCount());
    } else if (m_image.depth() > 0) {
        colorsInfo = QString(" %1bpp (%2 %3)").arg(m_image.bitPlaneCount()).arg(m_image.colorCount()).arg(colors_shades);
    }
    QString description = QString("(%1px × %2px) %3 KB  %4%5").arg(m_image.width()).arg(m_image.height()).arg(m_fsize).arg(grayscale_color).arg(colorsInfo);
    m_description->setText(description);
}

void AdjustImage::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    int newValue = factor * scrollBar->value() + (factor - 1) * scrollBar->pageStep() / 2;
    scrollBar->setValue(newValue);
}

void AdjustImage::changeCroppingState(bool changeTo)
{
    m_croppingState = changeTo;
    ui->actionCrop->setDisabled(changeTo);

    if (changeTo) {
        setCursor(Qt::CrossCursor);
        m_cropConfirmBtn->hide();
        m_cropCancelBtn->hide();
        m_statusBar->showMessage(tr("Click and drag to select crop area, then click Confirm"));
    } else {
        setCursor(Qt::ArrowCursor);
        m_cropConfirmBtn->hide();
        m_cropCancelBtn->hide();
        m_statusBar->clearMessage();
    }
}

void AdjustImage::showCropButtons()
{
    if (m_croppingRegionSelected) {
        m_cropConfirmBtn->show();
        m_cropCancelBtn->show();
    }
}

void AdjustImage::refreshLabel()
{
    m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
    UpdateImageDescription();
    scaleImageBy(1.0);
}

void AdjustImage::rotateImage(int angle)
{
    saveToHistoryWithClear(m_image);
    QPixmap pixmap(m_imageLabel->pixmap());
    QTransform rm;
    rm.rotate(angle);
    pixmap = pixmap.transformed(rm, Qt::SmoothTransformation);
    m_image = pixmap.toImage();
    refreshLabel();
}

void AdjustImage::saveToHistory(QImage imageToSave)
{
    m_history.push_back(imageToSave);
    ui->actionUndo->setEnabled(true);
}

void AdjustImage::saveToHistoryWithClear(QImage imageToSave)
{
    saveToHistory(imageToSave);
    m_reverseHistory.clear();
    ui->actionRedo->setEnabled(false);
}

void AdjustImage::saveToReverseHistory(QImage imageToSave)
{
    m_reverseHistory.push_back(imageToSave);
    ui->actionRedo->setEnabled(true);
}

void AdjustImage::resizeImage(int targetW, int targetH)
{
    // no size change so don't record a history step or rescale the image
    if (targetW == m_image.width() && targetH == m_image.height()) {
        return;
    }

    saveToHistoryWithClear(m_image);
    QPixmap pixmap(m_imageLabel->pixmap());
    pixmap = pixmap.scaled(targetW, targetH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_image = pixmap.toImage();
    refreshLabel();
}

void AdjustImage::scaleImageBy(double factor)
{
    m_scaleFactor *= factor;
    m_imageLabel->resize(m_scaleFactor * m_imageLabel->pixmap().size());

    adjustScrollBar(m_scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(m_scrollArea->verticalScrollBar(), factor);

    ui->actionZoomIn->setEnabled(m_scaleFactor < 3.0);
    ui->actionZoomOut->setEnabled(m_scaleFactor > 0.333);
    emit InternalZoomFactorChanged(m_scaleFactor);
}

void AdjustImage::scaleImageUsing(double factor)
{
    m_scaleFactor = factor;
    m_imageLabel->resize(m_scaleFactor * m_imageLabel->pixmap().size());

    adjustScrollBar(m_scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(m_scrollArea->verticalScrollBar(), factor);

    ui->actionZoomIn->setEnabled(m_scaleFactor < 3.0);
    ui->actionZoomOut->setEnabled(m_scaleFactor > 0.333);
}


void AdjustImage::updateActions(bool updateTo)
{
    ui->actionCrop->setEnabled(updateTo);
    ui->actionResizeImage->setEnabled(updateTo);
    ui->actionRotateLeft->setEnabled(updateTo);
    ui->actionRotateRight->setEnabled(updateTo);
    ui->actionSave->setEnabled(updateTo);
    ui->actionZoomIn->setEnabled(updateTo);
    ui->actionZoomOut->setEnabled(updateTo);
    ui->actionZoomToFit->setEnabled(updateTo);
}


// Slots

bool AdjustImage::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_imageLabel)
        return false;

    switch (event->type())
    {
        case QEvent::MouseButtonPress:
        {
            if (!m_croppingState) break;
            const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
            
            if (!m_croppingRegionSelected) {
                // Initial selection mode
                m_croppingStart = me->pos() / m_scaleFactor;
                m_rbstart = me->pos();
                m_rb->setGeometry(QRect(m_rbstart, QSize()));
                m_rb->show();
                m_lastMousePos = me->pos();
            } else {
                // Adjustment mode - check if clicking on a handle
                m_draggingHandle = GetHandleAtPosition(me->pos());
                if (m_draggingHandle == -1) {
                    // Not on handle, restart selection
                    m_croppingRegionSelected = false;
                    m_croppingStart = me->pos() / m_scaleFactor;
                    m_rbstart = me->pos();
                    m_rb->setGeometry(QRect(m_rbstart, QSize()));
                    m_rb->show();
                    m_cropConfirmBtn->hide();
                    m_cropCancelBtn->hide();
                } else if (m_draggingHandle == 4) {
                    // Initialize for move operation
                    m_lastMousePos = me->pos();
                }
            }
            break;
        }

        case QEvent::MouseButtonRelease:
        {
            if (!m_croppingState) break;
            const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
            
            if (!m_croppingRegionSelected && m_draggingHandle == -1) {
                // Finish initial selection
                m_croppingEnd = me->pos() / m_scaleFactor;
                m_rbend = me->pos();
                m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
                m_croppingRegionSelected = true;
                m_statusBar->showMessage(tr("Drag corners/edges to adjust, then click Confirm to crop"));
                showCropButtons();
            } else if (m_draggingHandle >= 0) {
                // Finish handle dragging and clamp to bounds
                clampCropRectToBounds();
                m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
                m_draggingHandle = -1;
            }
            break;
        }

        case QEvent::MouseMove:
        {
            const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
            const QPoint position = me->pos();
            QString sf = QString::number(m_scaleFactor, 'f', 4);
            QString msg = tr("(x,y) coordinates:") + " (%1,%2)  " + tr("Zoom") + " (%3)";
            int x_pos = std::round(position.x() / m_scaleFactor);
            int y_pos = std::round(position.y()/ m_scaleFactor);
            msg = msg.arg(x_pos).arg(y_pos).arg(sf);
            m_statusBar->showMessage(msg);
            
            if (m_croppingState) {
                if (!m_croppingRegionSelected && m_draggingHandle == -1) {
                    // Drawing initial selection
                    m_rbend = position;
                    m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
                } else if (m_croppingRegionSelected && m_draggingHandle >= 0) {
                    // Dragging a handle to adjust crop area
                    QRect currentRect = BuildRect(m_rbstart, m_rbend);
                    
                    if (m_draggingHandle == 0) {
                        // top-left
                        m_rbstart = position;
                    } else if (m_draggingHandle == 1) {
                        // top-right
                        m_rbstart.setY(position.y());
                        m_rbend.setX(position.x());
                    } else if (m_draggingHandle == 2) {
                        // bottom-left
                        m_rbstart.setX(position.x());
                        m_rbend.setY(position.y());
                    } else if (m_draggingHandle == 3) {
                        // bottom-right
                        m_rbend = position;
                    } else if (m_draggingHandle == 4) {
                        // move entire rectangle - use previous position to calculate delta
                        if (!m_lastMousePos.isNull()) {
                            QPoint delta = position - m_lastMousePos;
                            m_rbstart += delta;
                            m_rbend += delta;
                            // Clamp to image bounds during move
                            clampCropRectToBounds();
                        }
                        m_lastMousePos = position;
                    }
                    m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
                }
                
                // Update cursor based on handle
                if (m_croppingRegionSelected) {
                    int handle = GetHandleAtPosition(position);
                    if (handle == 0 || handle == 3) {
                        setCursor(Qt::SizeFDiagCursor);
                    } else if (handle == 1 || handle == 2) {
                        setCursor(Qt::SizeBDiagCursor);
                    } else if (handle == 4) {
                        setCursor(Qt::SizeAllCursor);
                    } else {
                        setCursor(Qt::CrossCursor);
                    }
                }
            }
            break;
        }

        default:
            break;
    }
    return false;
}


void AdjustImage::doCrop()
{
    m_croppingRegionSelected = false;
    m_draggingHandle = -1;
    changeCroppingState(true);
}

void AdjustImage::doCropConfirm()
{
    if (m_croppingRegionSelected) {
        // Convert display coordinates to image coordinates
        m_croppingEnd = m_rbend / m_scaleFactor;
        m_croppingStart = m_rbstart / m_scaleFactor;
        QRect rect = BuildRect(m_croppingStart, m_croppingEnd);
        
        // Validate minimum crop size
        if (rect.width() < MIN_CROP_SIZE || rect.height() < MIN_CROP_SIZE) {
            m_statusBar->showMessage(tr("Crop area too small (minimum 2x2 pixels)."));
            return;
        }
        
        // Validate crop area stays within image bounds
        if (rect.x() < 0 || rect.y() < 0 || 
            rect.right() > m_image.width() || 
            rect.bottom() > m_image.height()) {
            m_statusBar->showMessage(tr("Crop area exceeds image bounds."));
            return;
        }
        
        // Perform the crop
        saveToHistoryWithClear(m_image);
        m_image = m_image.copy(rect);
        refreshLabel();
        m_rb->hide();
        m_croppingRegionSelected = false;
        changeCroppingState(false);
        m_statusBar->showMessage(tr("Image cropped successfully."));
    }
}

void AdjustImage::doCropCancel()
{
    m_rb->hide();
    m_croppingRegionSelected = false;
    m_draggingHandle = -1;
    m_lastMousePos = QPoint(0, 0);
    changeCroppingState(false);
    m_statusBar->showMessage(tr("Crop cancelled."));
}

#if 0
void AdjustImage::toggleFullscreen()
{
    if(isFullScreen()) {
        this->showNormal();
    } else {
        this->showFullScreen();
    }
}
#endif

void AdjustImage::doResizeImage()
{
    // only record history once the resize is actually applied in resizeImage()
    // so canceling the dialog leaves the undo/redo stacks untouched
    int width = m_image.width();
    int height = m_image.height();
    ImageResizeDialog dlg(width, height, this);
    if (dlg.exec() == QDialog::Accepted) {
        int newWidth = dlg.getWidth();
        int newHeight = dlg.getHeight();
        resizeImage(newWidth, newHeight);
    }
}


void AdjustImage::doRotateLeft()
{
    rotateImage(-90);
}

void AdjustImage::doRotateRight()
{
    rotateImage(90);
}

void AdjustImage::doSave()
{
    QString format;
    if (m_mediatype.startsWith("image/")) {
        format = m_mediatype.mid(6,-1).toUpper();
    }
    // if an unknown format just default to let QImage decide based on filename
    if (format.isEmpty()) {
        bool success = m_image.save(m_fileName);
        if (success) {
            m_statusBar->showMessage(tr("Image successfully saved."));
        } else {
            m_statusBar->showMessage(tr("Image save failed."));
        }
    } else {
        int quality = -1;
        // handle lossy image types
        if (SAVE_QUALITY_MEDIATYPES.contains(m_mediatype)) {
            if (m_mediatype == "image/jpeg") quality = m_jpeg_quality;
            if (m_mediatype == "image/webp") quality = m_webp_quality;
            if (m_mediatype == "image/jxl")  quality = m_jxl_quality;
            if (m_mediatype == "image/avif") quality = m_avif_quality;
            bool ok;
            quality = QInputDialog::getInt(nullptr, tr("Image Quality"),
                                           tr("Enter quality level (0-100):"), quality, 0, 100, 1, &ok);
            if (!ok) {
                m_statusBar->showMessage(tr("Image save failed. "));
                return;
            }
            if (m_mediatype == "image/jpeg") m_jpeg_quality = quality;
            if (m_mediatype == "image/webp") m_webp_quality = quality;
            if (m_mediatype == "image/jxl")  m_jxl_quality = quality;
            if (m_mediatype == "image/avif") m_avif_quality = quality;
        }
        QImageWriter writer(m_fileName, format.toUtf8().data());
        if (quality != -1) writer.setQuality(quality);
        writer.setOptimizedWrite(true);
        bool success = writer.write(m_image);
        if (success) {
            m_statusBar->showMessage(tr("Image successfully saved."));
            m_ffsize = QFile(m_fileName).size() / 1024.0;
            m_fsize =  QLocale().toString(m_ffsize, 'f', 2);
        } else {
            m_statusBar->showMessage(tr("Image save failed: ") + writer.errorString() );
        }
    }
    
}


void AdjustImage::toggleShowToolbar(bool checked)
{
    if (checked)
        m_mainToolBar->show();
    else
        m_mainToolBar->hide();
}


void AdjustImage::doUndo()
{
    saveToReverseHistory(m_image);
    if (!m_history.isEmpty()) {
        m_image = m_history.last();
        refreshLabel();
        m_history.pop_back();
    }
    if (m_history.size() == 0)
        ui->actionUndo->setEnabled(false);
}

void AdjustImage::doRedo()
{
    saveToHistory(m_image);
    if (!m_reverseHistory.isEmpty()) {
        m_image = m_reverseHistory.last();
        refreshLabel();
        m_reverseHistory.pop_back();
    }
    if (m_reverseHistory.size() == 0)
        ui->actionRedo->setEnabled(false);
}

void AdjustImage::doZoomIn()
{
    scaleImageBy(1.25);
}

void AdjustImage::doZoomOut()
{
    scaleImageBy(0.80);
}

void AdjustImage::doZoomToFit()
{
    QSize windowSize = m_scrollArea->viewport()->size();
    QSize labelSize = m_imageLabel->pixmap().size();

    double imageRatio = double(labelSize.height()) / labelSize.width();
    double scaleTo;

    if (windowSize.width() * imageRatio > windowSize.height()) {
        scaleTo = double(windowSize.height()) / labelSize.height();
    } else {
        scaleTo = double(windowSize.width()) / labelSize.width();
    }
    double scaleBy = scaleTo / m_scaleFactor;
    scaleImageBy(scaleBy);
}


void AdjustImage::ConnectSignalsToSlots()
{
    connect(ui->actionCrop,        SIGNAL(triggered()), this, SLOT(doCrop()));
    connect(ui->actionResizeImage, SIGNAL(triggered()), this, SLOT(doResizeImage()));
    connect(ui->actionRotateLeft,  SIGNAL(triggered()), this, SLOT(doRotateLeft()));
    connect(ui->actionRotateRight, SIGNAL(triggered()), this, SLOT(doRotateRight()));
    connect(ui->actionSave,        SIGNAL(triggered()), this, SLOT(doSave()));
    connect(ui->actionZoomIn,      SIGNAL(triggered()), this, SLOT(doZoomIn()));
    connect(ui->actionZoomOut,     SIGNAL(triggered()), this, SLOT(doZoomOut()));
    connect(ui->actionZoomToFit,   SIGNAL(triggered()), this, SLOT(doZoomToFit()));
    connect(ui->actionRedo,        SIGNAL(triggered()), this, SLOT(doRedo()));
    connect(ui->actionUndo,        SIGNAL(triggered()), this, SLOT(doUndo()));
    // connect(ui->actionFullscreen,  SIGNAL(triggered()), this, SLOT(toggleFullscreen()));
    connect(ui->actionShowToolbar, SIGNAL(triggered(bool)), this, SLOT(toggleShowToolbar(bool)));
}
