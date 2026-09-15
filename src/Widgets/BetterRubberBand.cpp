#include <QMouseEvent>
#include "Widgets/BetterRubberBand.h"
#include "Widgets/RubberBandHandle.h"

BetterRubberBand::BetterRubberBand(Shape shape, QWidget* parent)
   : QRubberBand(shape, parent)
{
   // Crucial: Allow mouse interactions on the body of the rubber band
   setAttribute(Qt::WA_TransparentForMouseEvents, false);

   // Instantiate and attach handles
   m_topLeft     = new RubberBandHandle(RubberBandHandle::TopLeft, this);
   m_topRight    = new RubberBandHandle(RubberBandHandle::TopRight, this);
   m_bottomLeft  = new RubberBandHandle(RubberBandHandle::BottomLeft, this);
   m_bottomRight = new RubberBandHandle(RubberBandHandle::BottomRight, this);

   updateHandles();
}

void BetterRubberBand::updateHandles() {
   m_topLeft->updatePosition();
   m_topRight->updatePosition();
   m_bottomLeft->updatePosition();
   m_bottomRight->updatePosition();
}

void BetterRubberBand::resizeEvent(QResizeEvent* event) {
   QRubberBand::resizeEvent(event);
   updateHandles(); // Reposition child handles when rubber band expands/shrinks
}

// Allow dragging the entire body of the rubber band
void BetterRubberBand::mousePressEvent(QMouseEvent* event) {
   if (event->button() == Qt::LeftButton) {
       m_dragStartPos = event->globalPosition().toPoint();
   }
}

void BetterRubberBand::mouseMoveEvent(QMouseEvent* event) {
   if (event->buttons() & Qt::LeftButton) {
       QPoint currentGlobalPos = event->globalPosition().toPoint();
       QPoint delta = currentGlobalPos - m_dragStartPos;
       m_dragStartPos = currentGlobalPos;
       move(pos() + delta);
   }
}
