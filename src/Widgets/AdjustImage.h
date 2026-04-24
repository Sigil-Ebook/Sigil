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

namespace Ui {
class AdjustImage;
}

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

signals:
    void InternalZoomFactorChanged(double factor);

private slots:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void toggleShowToolbar(bool checked);

  
private:
    void ConnectSignalsToSlots();
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void changeCroppingState(bool changeTo);
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
    
    Ui::AdjustImage *ui;
    QVBoxLayout* vlayout;
    QStatusBar * m_statusBar;
    QToolBar * m_mainToolBar;
    QScrollArea * m_scrollArea;
    QLabel * m_imageLabel;
    QImage m_image;
    QLabel * m_description;

    bool m_croppingState;
    QPoint m_croppingStart;
    QPoint m_croppingEnd;
    QPoint m_rbstart;
    QPoint m_rbend;
    QRubberBand*  m_rb;

    QString m_fileName;
    QString m_mediatype;

    QVector<QImage> m_history;
    QVector<QImage> m_reverseHistory;

    double m_scaleFactor;

};

#endif // ADJUSTIMAGE_H
