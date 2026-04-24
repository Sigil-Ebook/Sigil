#include <QTransform>
#include <QDebug>
#include <QFileInfo>
#include <QImageWriter>
#include "Dialogs/ImageResizeDialog.h"
#include "Widgets/AdjustImage.h"
#include "ui_AdjustImage.h"

AdjustImage::AdjustImage(const QString filepath, const QString& mediatype,  QWidget *parent) :
    QWidget(parent),
    m_mediatype(mediatype),
    ui(new Ui::AdjustImage)
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
    m_rb->hide();

    m_scrollArea = new QScrollArea;
    m_scrollArea->setBackgroundRole(QPalette::Dark);
    m_scrollArea->setWidget(m_imageLabel);

    m_description = new QLabel;
    m_statusBar->addPermanentWidget(m_description);
    
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
        m_scaleFactor = 1.0;
        m_croppingState = false;
        setCursor(Qt::ArrowCursor);
        updateActions(true);
        refreshLabel();
    }
    ConnectSignalsToSlots();
}

AdjustImage::~AdjustImage()
{
    m_history.clear();
    m_reverseHistory.clear();
    delete ui;
}

bool AdjustImage::isCropEnabled() { return ui->actionCrop->isEnabled(); }  
bool AdjustImage::isUndoEnabled() { return ui->actionUndo->isEnabled(); }
bool AdjustImage::isRedoEnabled() { return ui->actionRedo->isEnabled(); }


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

void AdjustImage::changeCroppingState(bool changeTo)
{
    m_croppingState = changeTo;
    ui->actionCrop->setDisabled(changeTo);

    if (changeTo)
        setCursor(Qt::CrossCursor);
    else
        setCursor(Qt::ArrowCursor);
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
            m_croppingStart = me->pos() / m_scaleFactor;
            // QRubberBand scales with m_imageLabel scaling
            m_rbstart = me->pos();
            m_rb->setGeometry(QRect(m_rbstart, QSize()));
            m_rb->show();
            break;
        }

        case QEvent::MouseButtonRelease:
        {
            if (!m_croppingState) break;
            saveToHistoryWithClear(m_image);
            const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
            m_croppingEnd = me->pos() / m_scaleFactor;
            m_rbend = me->pos();
            m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
            m_rb->hide();
            QRect rect = BuildRect(m_croppingStart, m_croppingEnd);
            m_image = m_image.copy(rect);
            refreshLabel();
            changeCroppingState(false);
            break;
        }

        case QEvent::MouseMove:
        {
            const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
            const QPoint position = me->pos();
            QString sf = QString::number(m_scaleFactor, 'f', 4);
            m_statusBar->showMessage(QString("(x,y) coordinates: (%1,%2) Zoom (%3)").arg(position.x()).arg(position.y()).arg(sf));
            if (m_croppingState) {
                m_rbend = position;
                m_rb->setGeometry(BuildRect(m_rbstart, m_rbend));
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
    changeCroppingState(true);
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
    bool success = false;
    // if an unknown format just default to let QImage decide based on filename
    if (format.isEmpty()) {
        success = m_image.save(m_fileName);
    } else {
        int quality = -1;
        if (m_mediatype == "image/jpeg") quality = 90;
        if (m_mediatype == "image/webp") quality = 80;
        QImageWriter writer(m_fileName, format.toUtf8().data());
        if (quality != -1) writer.setQuality(quality);
        success = writer.write(m_image);
    }
    if (success) {
        m_statusBar->showMessage(tr("Image successfully saved."));
    } else {
        m_statusBar->showMessage(tr("Image save failed."));
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
    m_image = m_history.last();
    refreshLabel();
    m_history.pop_back();
    if (m_history.size() == 0)
        ui->actionUndo->setEnabled(false);
}

void AdjustImage::doRedo()
{
    saveToHistory(m_image);
    m_image = m_reverseHistory.last();
    refreshLabel();
    m_reverseHistory.pop_back();
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
