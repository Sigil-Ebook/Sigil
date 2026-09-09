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

#ifndef ADJUSTIMAGE_H
#define ADJUSTIMAGE_H

#include <algorithm>
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QMessageBox>
#include <QScrollBar>
#include <QMouseEvent>
#include <QStatusBar>
#include <QRect>
#include <QVector>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QRubberBand>
#include <QDebug>
#include <QVBoxLayout>
#include <QToolBar>
#include <QIcon>

class QAction;

namespace Ui {
class AdjustImage;
}

enum CornerPosition {
    CornerNone = 0,
    CornerTopLeft,
    CornerTopRight,
    CornerBottomLeft,
    CornerBottomRight
};

class CornerHandle : public QWidget
{
public:
    CornerHandle(CornerPosition pos, QWidget* parent = nullptr);
    CornerPosition position() const { return m_position; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    CornerPosition m_position;
};

class AdjustImage : public QWidget
{
    Q_OBJECT

public:
    explicit AdjustImage(const QString filepath, const QString& mediatype, QWidget *parent = 0);
    ~AdjustImage();

    double getZoomFactor() { return m_scaleFactor; };
    void scaleImageUsing(double factor);

    bool isCropEnabled();
    bool isUndoEnabled();
    bool isRedoEnabled();
                          
public slots:
    void doSave();
    void doZoomIn();
    void doZoomOut();
    void doZoomToFit();
    void doUndo();
    void doRedo();
    void doRotateLeft();
    void doRotateRight();
    void doCrop();
    void doResizeImage();
    void doReload();

signals:
    void InternalZoomFactorChanged(double factor);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void toggleShowToolbar(bool checked);

  
private:
    void ReadSettings();
    void WriteSettings();
    void ConnectSignalsToSlots();
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void clearCropSelection();
    void updateCropHandles();
    void refreshLabel();
    void rotateImage(int angle);
    void resizeImage(int tgtw, int tgth); 
    void saveToHistory(QImage imageToSave);
    void saveToHistoryWithClear(QImage imageToSave);
    void saveToReverseHistory(QImage imageToSave);
    void scaleImageBy(double factor);
    void updateActions(bool updateTo);
    void UpdateImageDescription();
    QRect BuildRect(const QPoint& p1, const QPoint& p2);
    void extendToolTip(QAction* m, const QString sc);
    QIcon createReloadIcon();
    
    Ui::AdjustImage *ui;
    QVBoxLayout* vlayout;
    QStatusBar * m_statusBar;
    QToolBar * m_mainToolBar;
    QScrollArea * m_scrollArea;
    QLabel * m_imageLabel;
    QImage m_image;
    QLabel * m_description;

    bool m_selectingCrop;
    bool m_hasCropSelection;
    QRect m_cropRect;
    QPoint m_croppingStart;
    QPoint m_croppingEnd;
    QPoint m_rbstart;
    QPoint m_rbend;
    QRubberBand*  m_rb;

    CornerHandle* m_handleTL;
    CornerHandle* m_handleTR;
    CornerHandle* m_handleBL;
    CornerHandle* m_handleBR;
    CornerPosition m_draggingHandle;
    QPoint m_dragAnchor;

    QString m_fileName;
    QString m_mediatype;

    QVector<QImage> m_history;
    QVector<QImage> m_reverseHistory;

    double m_scaleFactor;
    int m_jpeg_quality;
    int m_webp_quality;
    int m_jxl_quality;
    int m_avif_quality;

};

#endif // ADJUSTIMAGE_H
