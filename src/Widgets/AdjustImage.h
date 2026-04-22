#ifndef ADJUSTIMAGE_H
#define ADJUSTIMAGE_H

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
#include <QMenuBar>
#include <QToolBar>

namespace Ui {
class AdjustImage;
}

class AdjustImage : public QWidget
{
    Q_OBJECT

public:
    explicit AdjustImage(const QString filepath, QWidget *parent = 0);
    ~AdjustImage();

private slots:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void doCrop();
    void doResizeImage();
    void doRotateLeft();
    void doRotateRight();
    void doSave();
    void doUndo();
    void doRedo();
    void doZoomIn();
    void doZoomOut();
    void doZoomToFit();
    void toggleShowToolbar(bool checked);
    void toggleFullscreen();
  
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
    void scaleImage(double factor);
    void updateActions(bool updateTo);
    void UpdateImageDescription();

    Ui::AdjustImage *ui;
    QVBoxLayout* vlayout;
    QMenuBar * m_menuBar;
    QStatusBar * m_statusBar;
    QToolBar * m_mainToolBar;
    QScrollArea * m_scrollArea;
    QLabel * m_imageLabel;
    QImage m_image;
    QLabel * m_description;

    bool m_croppingState;
    QPoint m_croppingStart;
    QPoint m_croppingEnd;
    QRubberBand*  m_rb;

    QString m_fileName;

    QVector<QImage> m_history;
    QVector<QImage> m_reverseHistory;

    double m_scaleFactor;

};

#endif // ADJUSTIMAGE_H
