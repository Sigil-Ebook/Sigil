#include "Widgets/RubberBandHandle.h"
#include <QMouseEvent>
#include <QPainter>

RubberBandHandle::RubberBandHandle(Position pos, QWidget* parent)
   : QWidget(parent), m_position(pos)
{
   resize(m_size, m_size);
   // Ensure the handle paints on top of the rubber band line
   raise();
}

void RubberBandHandle::updatePosition() {
   QWidget* parent = parentWidget();
   if (!parent) return;

   int w = parent->width();
   int h = parent->height();

   // Align handles perfectly to the rubber band boundaries
   switch (m_position) {
       case TopLeft:     move(0, 0); break;
       case TopRight:    move(w - m_size, 0); break;
       case BottomLeft:  move(0, h - m_size); break;
       case BottomRight: move(w - m_size, h - m_size); break;
   }
}

void RubberBandHandle::enterEvent(QEnterEvent* event) {
   if (m_position == TopLeft || m_position == BottomRight) {
       setCursor(Qt::SizeFDiagCursor);
   } else {
       setCursor(Qt::SizeBDiagCursor);
   }
   QWidget::enterEvent(event);
}

void RubberBandHandle::mousePressEvent(QMouseEvent* event) {
   if (event->button() == Qt::LeftButton) {
       m_dragStartPos = event->globalPosition().toPoint();
   }
}

void RubberBandHandle::mouseMoveEvent(QMouseEvent* event) {
   QWidget* parent = parentWidget();
   if (!parent || !(event->buttons() & Qt::LeftButton)) return;

   QPoint currentGlobalPos = event->globalPosition().toPoint();
   QPoint delta = currentGlobalPos - m_dragStartPos;
   m_dragStartPos = currentGlobalPos;

   QRect geom = parent->geometry();

   switch (m_position) {
       case TopLeft:     geom.setTopLeft(geom.topLeft() + delta); break;
       case TopRight:    geom.setTopRight(geom.topRight() + delta); break;
       case BottomLeft:  geom.setBottomLeft(geom.bottomLeft() + delta); break;
       case BottomRight: geom.setBottomRight(geom.bottomRight() + delta); break;
   }

   if (geom.width() > 30 && geom.height() > 30) {
       parent->setGeometry(geom);
       // Force the custom rubber band to refresh sister handles
       QMetaObject::invokeMethod(parent, "updateHandles", Qt::DirectConnection);
   }
}

void RubberBandHandle::paintEvent(QPaintEvent* event) {
   Q_UNUSED(event);
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing);

   // Draw solid square handle anchor
   painter.setBrush(Qt::white);
   painter.setPen(QPen(Qt::darkBlue, 1.5));
   painter.drawRect(0, 0, width() - 1, height() - 1);
}
