#pragma once

#include <QWidget>

class RubberBandHandle : public QWidget {
   Q_OBJECT
public:
   enum Position { TopLeft, TopRight, BottomLeft, BottomRight };

   RubberBandHandle(Position pos, QWidget* parent = nullptr);
   void updatePosition();

protected:
   void mousePressEvent(QMouseEvent* event) override;
   void mouseMoveEvent(QMouseEvent* event) override;
   void enterEvent(QEnterEvent* event) override;
   void paintEvent(QPaintEvent* event) override;

private:
   Position m_position;
   QPoint m_dragStartPos;
   const int m_size = 8; // Size of the handle square
};
