/************************************************************************
**
**  Copyright (C) 2021 Doug Massay
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#ifndef OVERLAY_H
#define OVERLAY_H

#include <QtGui>
#include <QtWidgets>

class OverlayWidget : public QWidget {
   void newParent() {
      if (!parent()) return;
      parent()->installEventFilter(this);
      raise();
   }
public:
   explicit OverlayWidget(QWidget *parent = {}) : QWidget(parent) {
      setAttribute(Qt::WA_NoSystemBackground);
      setAttribute(Qt::WA_TransparentForMouseEvents);
      newParent();
   }
protected:
   //! Catches resize and child events from the parent widget
   bool eventFilter(QObject *obj, QEvent *ev) override {
      if (obj == parent()) {
         if (ev->type() == QEvent::Resize)
            resize(static_cast<QResizeEvent*>(ev)->size());
         else if (ev->type() == QEvent::ChildAdded)
            raise();
      }
      return QWidget::eventFilter(obj, ev);
   }
   //! Tracks parent widget changes
   bool event(QEvent *ev) override {
      if (ev->type() == QEvent::ParentAboutToChange) {
         if (parent()) parent()->removeEventFilter(this);
      }
      else if (ev->type() == QEvent::ParentChange)
         newParent();
      return QWidget::event(ev);
   }
};

class OverlayHelperWidget : public QWidget
{
public:
   explicit OverlayHelperWidget(QWidget *parent = {}) : QWidget(parent) {}
   void setSize(QObject *obj) {
      if (obj->isWidgetType()) static_cast<QWidget*>(obj)->setGeometry(rect());
   }
protected:
   //! Resizes children to fill the extent of this widget
   bool event(QEvent *ev) override {
      if (ev->type() == QEvent::ChildAdded) {
         setSize(static_cast<QChildEvent*>(ev)->child());
      }
      return QWidget::event(ev);
   }
   //! Keeps the children appropriately sized
   void resizeEvent(QResizeEvent *) override {
      for(auto obj : children()) setSize(obj);
   }
};

class LoadingOverlay : public OverlayWidget
{
    Q_OBJECT

public:
   LoadingOverlay(QWidget *parent = {}) : OverlayWidget{parent} {
      setAttribute(Qt::WA_TranslucentBackground);
   }
protected:
   void paintEvent(QPaintEvent *) override {
      QPainter p{this};
      p.fillRect(rect(), {100, 100, 100, 128});
      p.setPen({200, 200, 255});
      p.setFont({"arial,helvetica", 48});
      p.drawText(rect(), tr("Loading..."), Qt::AlignHCenter | Qt::AlignVCenter);
   }
};

#endif // OVERLAY_H
