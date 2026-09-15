#pragma once

#include <QRubberBand>

class RubberBandHandle;

class BetterRubberBand : public QRubberBand {
   Q_OBJECT
public:
   explicit BetterRubberBand(Shape shape, QWidget* parent = nullptr);

    QPoint getTopLeftPos() { return geometry().topLeft(); };
    QPoint getBottomRightPos() { return geometry().bottomRight(); };

public slots:
   void updateHandles();


protected:
   void resizeEvent(QResizeEvent* event) override;
   void mousePressEvent(QMouseEvent* event) override;
   void mouseMoveEvent(QMouseEvent* event) override;

private:
   RubberBandHandle* m_topLeft;
   RubberBandHandle* m_topRight;
   RubberBandHandle* m_bottomLeft;
   RubberBandHandle* m_bottomRight;
   QPoint m_dragStartPos;
};
