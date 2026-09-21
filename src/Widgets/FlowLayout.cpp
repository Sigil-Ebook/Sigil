// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Additions and changes: Copyright (C) Kevin B. Hendricks, Stratford, Ontario Canada

#include <QtWidgets>
#include <QDebug>

#include "Widgets/FlowLayout.h"

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}


FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    itemList.append(item);
}

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int FlowLayout::count() const
{
    return itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return { };
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem *item : std::as_const(itemList))
        size = size.expandedTo(item->minimumSize());

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

void FlowLayout::addLineBreak() 
{
    QWidget * lb = new QWidget();
    m_lbcount++;
    QString objn = "LineBreak" + QString::number(m_lbcount);
    lb->setObjectName(objn);
    lb->setVisible(false);  // Keeps it hidden
    addWidget(lb);
}


int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : std::as_const(itemList)) {
        const QWidget *wid = item->widget();

        // check for line break flag
        bool isLineBreak = (wid && wid->objectName().startsWith("LineBreak"));

        if (isLineBreak) {
            // Force wrap: Move cursor to the next line immediately
            x = effectiveRect.x();
            y = y + lineHeight + verticalSpacing();
            lineHeight = 0;
            continue; // Skip layout allocation for this dummy widget
        }

        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1)
            spaceY = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        // standard wrap logic modified to skip is x is already reset (where is this modified?)
        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    int vs = -1;
    if (!parent) {
        return vs;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    } else {
        QLayout* layout = static_cast<QLayout *>(parent);
        vs = layout->spacing();
        if ((vs == -1) && layout->parentWidget()) {
            QWidget *pw = static_cast<QWidget *>(layout->parentWidget());
            vs = pw->style()->pixelMetric(pm, nullptr, pw);
            if (vs == -1) {
                vs = pw->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
            }
        }
    }
    return vs;
}
