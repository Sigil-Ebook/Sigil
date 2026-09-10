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
#include <QImageWriter>
#include <QInputDialog>
#include <QKeySequence>
#include "Misc/SettingsStore.h"
#include "Dialogs/ImageResizeDialog.h"
#include "Widgets/AdjustImage.h"
#include "ui_AdjustImage.h"

static const QString SETTINGS_GROUP = "adjust_image";
static QStringList SAVE_QUALITY_MEDIATYPES = QStringList() << "image/jpeg" << "image/webp" << "image/avif" << "image/jxl";

CornerHandle::CornerHandle(CornerPosition pos, QWidget* parent)
    : QWidget(parent),
      m_position(pos)
{
    setFixedSize(10, 10);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_Hover, true);
    switch (m_position) {
        case CornerTopLeft:
        case CornerBottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case CornerTopRight:
        case CornerBottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        default:
            break;
    }
    hide();
}

void CornerHandle::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    bool hovered = underMouse();
    p.setPen(QPen(QColor(0, 120, 215), 1.5));
    p.setBrush(hovered ? QColor(200, 230, 255) : QColor(255, 255, 255));
    p.drawEllipse(1, 1, width() - 2, height() - 2);
}

QIcon AdjustImage::createReloadIcon()
{
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);

    QColor orange(255, 106, 0);
    p.setPen(QPen(orange, 3.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);

    QRectF arcRect(5.5, 5.5, 21, 21);

    // Two circular arcs in orange
    p.drawArc(arcRect, int(30 * 16), int(120 * 16));
    p.drawArc(arcRect, int(210 * 16), int(120 * 16));

    // Arrow heads
    p.setPen(Qt::NoPen);
    p.setBrush(orange);

    QPolygonF arrow1;
    arrow1 << QPointF(21, 5) << QPointF(27.5, 12.5) << QPointF(20, 14.5);
    p.drawPolygon(arrow1);

    QPolygonF arrow2;
    arrow2 << QPointF(11, 27) << QPointF(4.5, 19.5) << QPointF(12, 17.5);
    p.drawPolygon(arrow2);

    p.end();
    return QIcon(pix);
}

AdjustImage::AdjustImage(const QString filepath, const QString& mediatype,  QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdjustImage),
    m_mediatype(mediatype),
    m_selectingCrop(false),
    m_hasCropSelection(false),
    m_cropRect(QRect()),
    m_scaleFactor(1.0),
    m_handleTL(nullptr),
    m_handleTR(nullptr),
    m_handleBL(nullptr),
    m_handleBR(nullptr),
    m_draggingHandle(CornerNone),
    m_dragAnchor(QPoint())
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
    m_imageLabel->installEventFilter(this);
    // rubber band must be a child of the m_imageLabel
    // otherwise there is a coordinate nightmare
    m_rb = new QRubberBand(QRubberBand::Rectangle, m_imageLabel);
    m_rb->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_rb->hide();

    m_handleTL = new CornerHandle(CornerTopLeft, m_imageLabel);
    m_handleTR = new CornerHandle(CornerTopRight, m_imageLabel);
    m_handleBL = new CornerHandle(CornerBottomLeft, m_imageLabel);
    m_handleBR = new CornerHandle(CornerBottomRight, m_imageLabel);

    m_handleTL->installEventFilter(this);
    m_handleTR->installEventFilter(this);
    m_handleBL->installEventFilter(this);
    m_handleBR->installEventFilter(this);

    m_scrollArea = new QScrollArea;
    m_scrollArea->setBackgroundRole(QPalette::Dark);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_imageLabel);
    m_scrollArea->installEventFilter(this);
    m_scrollArea->viewport()->installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);

    m_description = new QLabel;
    m_statusBar->addPermanentWidget(m_description);
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
    extendToolTip(ui->actionReload,      "Ctrl+Shift+R");
    ui->actionReload->setIcon(createReloadIcon());
        
    vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(2,2,2,2);
    vlayout->addWidget(m_mainToolBar);
    vlayout->addWidget(m_scrollArea);
    vlayout->addWidget(m_statusBar);
    setLayout(vlayout);

    setWindowTitle(tr("Adjust Image"));
    if (!filepath.isEmpty()) {
        m_fileName = filepath;
        m_image = QImage(m_fileName);
        if (m_image.isNull()) {
             QMessageBox::information(this,
                                      tr("Adjust Image"),
                                      tr("Cannot load %1.").arg(m_fileName));
             return;
        }
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
    QString description = QString("(%1px × %2px) %3%4").arg(m_image.width()).arg(m_image.height()).arg(grayscale_color).arg(colorsInfo);
    m_description->setText(description);
}

void AdjustImage::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    int newValue = factor * scrollBar->value() + (factor - 1) * scrollBar->pageStep() / 2;
    scrollBar->setValue(newValue);
}

void AdjustImage::clearCropSelection()
{
    m_selectingCrop = false;
    m_draggingHandle = CornerNone;
    m_hasCropSelection = false;
    m_cropRect = QRect();
    if (m_rb) {
        m_rb->hide();
    }
    if (m_handleTL) m_handleTL->hide();
    if (m_handleTR) m_handleTR->hide();
    if (m_handleBL) m_handleBL->hide();
    if (m_handleBR) m_handleBR->hide();

    if (ui && ui->actionCrop) {
        ui->actionCrop->setEnabled(false);
    }
    m_imageLabel->setCursor(Qt::ArrowCursor);
    m_scrollArea->viewport()->setCursor(Qt::ArrowCursor);
    setCursor(Qt::ArrowCursor);
}

void AdjustImage::updateCropHandles()
{
    if (!m_hasCropSelection || !m_handleTL || !m_handleTR || !m_handleBL || !m_handleBR) {
        return;
    }
    QRect rbRect = m_rb->geometry();
    int left = rbRect.left();
    int top = rbRect.top();
    int right = rbRect.right();
    int bottom = rbRect.bottom();

    int w = m_handleTL->width();
    int h = m_handleTL->height();

    int maxLabelX = qMax(0, m_imageLabel->width() - w);
    int maxLabelY = qMax(0, m_imageLabel->height() - h);

    m_handleTL->move(qBound(0, left - w / 2, maxLabelX),   qBound(0, top - h / 2, maxLabelY));
    m_handleTR->move(qBound(0, right - w / 2, maxLabelX),  qBound(0, top - h / 2, maxLabelY));
    m_handleBL->move(qBound(0, left - w / 2, maxLabelX),   qBound(0, bottom - h / 2, maxLabelY));
    m_handleBR->move(qBound(0, right - w / 2, maxLabelX),  qBound(0, bottom - h / 2, maxLabelY));

    m_handleTL->show();
    m_handleTR->show();
    m_handleBL->show();
    m_handleBR->show();

    m_handleTL->raise();
    m_handleTR->raise();
    m_handleBL->raise();
    m_handleBR->raise();
}

void AdjustImage::refreshLabel()
{
    m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
    UpdateImageDescription();
    scaleImageBy(1.0);
}

void AdjustImage::rotateImage(int angle)
{
    clearCropSelection();
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
    clearCropSelection();
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
    if (m_hasCropSelection && !m_cropRect.isEmpty()) {
        QRect newRb(std::round(m_cropRect.x() * m_scaleFactor),
                    std::round(m_cropRect.y() * m_scaleFactor),
                    std::round(m_cropRect.width() * m_scaleFactor),
                    std::round(m_cropRect.height() * m_scaleFactor));
        m_rb->setGeometry(newRb);
        updateCropHandles();
    }
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
    if (m_hasCropSelection && !m_cropRect.isEmpty()) {
        QRect newRb(std::round(m_cropRect.x() * m_scaleFactor),
                    std::round(m_cropRect.y() * m_scaleFactor),
                    std::round(m_cropRect.width() * m_scaleFactor),
                    std::round(m_cropRect.height() * m_scaleFactor));
        m_rb->setGeometry(newRb);
        updateCropHandles();
    }
}


void AdjustImage::updateActions(bool updateTo)
{
    ui->actionCrop->setEnabled(updateTo && m_hasCropSelection);
    ui->actionResizeImage->setEnabled(updateTo);
    ui->actionReload->setEnabled(updateTo && !m_fileName.isEmpty());
    ui->actionRotateLeft->setEnabled(updateTo);
    ui->actionRotateRight->setEnabled(updateTo);
    ui->actionSave->setEnabled(updateTo);
    ui->actionZoomIn->setEnabled(updateTo);
    ui->actionZoomOut->setEnabled(updateTo);
    ui->actionZoomToFit->setEnabled(updateTo);
}


// Slots

void AdjustImage::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && m_hasCropSelection) {
        clearCropSelection();
        m_statusBar->showMessage(tr("Crop selection deleted."));
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        if (m_hasCropSelection) {
            clearCropSelection();
            m_statusBar->showMessage(tr("Crop cancelled."));
            event->accept();
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

bool AdjustImage::eventFilter(QObject* watched, QEvent* event)
{
    // Global key handling across all watched widgets (imageLabel, corner handles, scrollArea, viewport)
    if (event->type() == QEvent::KeyPress) {
        const QKeyEvent* const ke = static_cast<const QKeyEvent*>(event);
        if ((ke->key() == Qt::Key_Delete || ke->key() == Qt::Key_Backspace) && m_hasCropSelection) {
            clearCropSelection();
            m_statusBar->showMessage(tr("Crop selection deleted."));
            return true;
        }
        if (ke->key() == Qt::Key_Escape) {
            if (m_hasCropSelection) {
                clearCropSelection();
                m_statusBar->showMessage(tr("Crop cancelled."));
                return true;
            }
        }
        if (ke->key() == Qt::Key_Control) {
            m_imageLabel->setCursor(Qt::CrossCursor);
            m_scrollArea->viewport()->setCursor(Qt::CrossCursor);
        }
    }
    else if (event->type() == QEvent::KeyRelease) {
        const QKeyEvent* const ke = static_cast<const QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Control && !m_selectingCrop && m_draggingHandle == CornerNone) {
            m_imageLabel->setCursor(Qt::ArrowCursor);
            m_scrollArea->viewport()->setCursor(Qt::ArrowCursor);
            setCursor(Qt::ArrowCursor);
        }
    }

    // Handle events from CornerHandle widgets
    CornerHandle* handle = nullptr;
    if (watched == m_handleTL) handle = m_handleTL;
    else if (watched == m_handleTR) handle = m_handleTR;
    else if (watched == m_handleBL) handle = m_handleBL;
    else if (watched == m_handleBR) handle = m_handleBR;

    if (handle) {
        switch (event->type())
        {
            case QEvent::MouseButtonPress:
            {
                const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) {
                    m_draggingHandle = handle->position();
                    QRect rbRect = m_rb->geometry();
                    switch (m_draggingHandle) {
                        case CornerTopLeft:
                            m_dragAnchor = QPoint(rbRect.right(), rbRect.bottom());
                            break;
                        case CornerTopRight:
                            m_dragAnchor = QPoint(rbRect.left(), rbRect.bottom());
                            break;
                        case CornerBottomLeft:
                            m_dragAnchor = QPoint(rbRect.right(), rbRect.top());
                            break;
                        case CornerBottomRight:
                            m_dragAnchor = QPoint(rbRect.left(), rbRect.top());
                            break;
                        default:
                            break;
                    }
                    setFocus();
                    return true;
                }
                break;
            }

            case QEvent::MouseMove:
            {
                if (m_draggingHandle != CornerNone) {
                    const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
                    QPoint pos = handle->mapToParent(me->pos());
                    int maxX = qMax(0, m_imageLabel->width() - 1);
                    int maxY = qMax(0, m_imageLabel->height() - 1);
                    pos.setX(qBound(0, pos.x(), maxX));
                    pos.setY(qBound(0, pos.y(), maxY));

                    QRect newRbRect = BuildRect(m_dragAnchor, pos);
                    m_rb->setGeometry(newRbRect);

                    int imgLeft = std::round(newRbRect.left() / m_scaleFactor);
                    int imgTop = std::round(newRbRect.top() / m_scaleFactor);
                    int imgRight = std::round(newRbRect.right() / m_scaleFactor);
                    int imgBottom = std::round(newRbRect.bottom() / m_scaleFactor);
                    m_cropRect = BuildRect(QPoint(imgLeft, imgTop), QPoint(imgRight, imgBottom)).intersected(m_image.rect());

                    updateCropHandles();

                    QString sf = QString::number(m_scaleFactor, 'f', 4);
                    QString msg = tr("(x,y) coordinates:") + " (%1,%2)  " + tr("Zoom") + " (%3)";
                    msg = msg.arg(std::round(pos.x() / m_scaleFactor)).arg(std::round(pos.y() / m_scaleFactor)).arg(sf);
                    m_statusBar->showMessage(msg);
                    return true;
                }
                break;
            }

            case QEvent::MouseButtonRelease:
            {
                if (m_draggingHandle != CornerNone) {
                    const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
                    if (me->button() == Qt::LeftButton) {
                        m_draggingHandle = CornerNone;
                        QRect rbRect = m_rb->geometry();
                        int imgLeft = std::round(rbRect.left() / m_scaleFactor);
                        int imgTop = std::round(rbRect.top() / m_scaleFactor);
                        int imgRight = std::round(rbRect.right() / m_scaleFactor);
                        int imgBottom = std::round(rbRect.bottom() / m_scaleFactor);
                        QRect rect = BuildRect(QPoint(imgLeft, imgTop), QPoint(imgRight, imgBottom)).intersected(m_image.rect());

                        if (rect.width() > 2 && rect.height() > 2) {
                            m_cropRect = rect;
                            m_hasCropSelection = true;
                            ui->actionCrop->setEnabled(true);
                            updateCropHandles();
                            m_statusBar->showMessage(tr("Crop area selected: (%1,%2) %3x%4 px. Click Crop to execute.")
                                                     .arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()));
                        } else {
                            clearCropSelection();
                        }
                        return true;
                    }
                }
                break;
            }

            default:
                break;
        }
        return false;
    }

    if (watched != m_imageLabel)
        return false;

    switch (event->type())
    {
        case QEvent::MouseButtonPress:
        {
            setFocus();
            const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && (me->modifiers() & Qt::ControlModifier)) {
                clearCropSelection();
                m_selectingCrop = true;
                QPoint pos = me->pos();
                int maxX = qMax(0, m_imageLabel->width() - 1);
                int maxY = qMax(0, m_imageLabel->height() - 1);
                pos.setX(qBound(0, pos.x(), maxX));
                pos.setY(qBound(0, pos.y(), maxY));
                m_rbstart = pos;
                m_rbend = pos;
                m_croppingStart = QPoint(std::round(pos.x() / m_scaleFactor), std::round(pos.y() / m_scaleFactor));
                m_rb->setGeometry(QRect(m_rbstart, QSize()));
                m_rb->show();
                m_imageLabel->setCursor(Qt::CrossCursor);
                m_scrollArea->viewport()->setCursor(Qt::CrossCursor);
                return true;
            }
            break;
        }

        case QEvent::MouseButtonRelease:
        {
            const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && m_selectingCrop) {
                m_selectingCrop = false;
                QPoint pos = me->pos();
                int maxX = qMax(0, m_imageLabel->width() - 1);
                int maxY = qMax(0, m_imageLabel->height() - 1);
                pos.setX(qBound(0, pos.x(), maxX));
                pos.setY(qBound(0, pos.y(), maxY));
                m_rbend = pos;
                m_croppingEnd = QPoint(std::round(pos.x() / m_scaleFactor), std::round(pos.y() / m_scaleFactor));

                QRect rect = BuildRect(m_croppingStart, m_croppingEnd).intersected(m_image.rect());
                if (rect.width() > 2 && rect.height() > 2) {
                    m_cropRect = rect;
                    m_hasCropSelection = true;
                    m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
                    m_rb->show();
                    ui->actionCrop->setEnabled(true);
                    updateCropHandles();
                    m_statusBar->showMessage(tr("Crop area selected: (%1,%2) %3x%4 px. Click Crop to execute.")
                                             .arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()));
                } else {
                    clearCropSelection();
                }
                if (me->modifiers() & Qt::ControlModifier) {
                    m_imageLabel->setCursor(Qt::CrossCursor);
                    m_scrollArea->viewport()->setCursor(Qt::CrossCursor);
                } else {
                    m_imageLabel->setCursor(Qt::ArrowCursor);
                    m_scrollArea->viewport()->setCursor(Qt::ArrowCursor);
                    setCursor(Qt::ArrowCursor);
                }
                return true;
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
            int y_pos = std::round(position.y() / m_scaleFactor);
            msg = msg.arg(x_pos).arg(y_pos).arg(sf);
            m_statusBar->showMessage(msg);

            if (m_selectingCrop) {
                QPoint pos = position;
                int maxX = qMax(0, m_imageLabel->width() - 1);
                int maxY = qMax(0, m_imageLabel->height() - 1);
                pos.setX(qBound(0, pos.x(), maxX));
                pos.setY(qBound(0, pos.y(), maxY));
                m_rbend = pos;
                m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
                m_imageLabel->setCursor(Qt::CrossCursor);
                m_scrollArea->viewport()->setCursor(Qt::CrossCursor);
            } else if (me->modifiers() & Qt::ControlModifier) {
                m_imageLabel->setCursor(Qt::CrossCursor);
                m_scrollArea->viewport()->setCursor(Qt::CrossCursor);
            } else if (m_imageLabel->cursor().shape() != Qt::ArrowCursor) {
                m_imageLabel->setCursor(Qt::ArrowCursor);
                m_scrollArea->viewport()->setCursor(Qt::ArrowCursor);
                setCursor(Qt::ArrowCursor);
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
    if (!m_hasCropSelection || m_cropRect.isEmpty()) {
        return;
    }
    saveToHistoryWithClear(m_image);
    m_image = m_image.copy(m_cropRect);
    clearCropSelection();
    refreshLabel();
    m_statusBar->showMessage(tr("Image cropped."));
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
    saveToHistoryWithClear(m_image);
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
    clearCropSelection();
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
    clearCropSelection();
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
    connect(ui->actionReload,      SIGNAL(triggered()), this, SLOT(doReload()));
    // connect(ui->actionFullscreen,  SIGNAL(triggered()), this, SLOT(toggleFullscreen()));
    connect(ui->actionShowToolbar, SIGNAL(triggered(bool)), this, SLOT(toggleShowToolbar(bool)));
}

void AdjustImage::doReload()
{
    if (!m_history.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this,
            tr("Reload Image"),
            tr("The image has unsaved modifications.\nDo you want to discard all changes and reload from disk?"),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel
        );
        if (reply != QMessageBox::Ok) {
            return;
        }
    }

    if (m_fileName.isEmpty()) {
        return;
    }

    QImage reloaded(m_fileName);
    if (reloaded.isNull()) {
        QMessageBox::warning(this, tr("Reload Image"), tr("Cannot reload %1.").arg(m_fileName));
        return;
    }

    m_image = reloaded;
    m_history.clear();
    m_reverseHistory.clear();
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    clearCropSelection();
    refreshLabel();

    m_statusBar->showMessage(tr("Image reloaded from disk."));
}
