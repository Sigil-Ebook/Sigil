/************************************************************************
**
**  Copyright (C) 2019      Doug Massay
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

#include "Misc/SigilDarkStyle.h"

SigilDarkStyle::SigilDarkStyle() : SigilDarkStyle(styleBase()) {}

SigilDarkStyle::SigilDarkStyle(QStyle *style) : QProxyStyle(style) {}

QStyle *SigilDarkStyle::styleBase(QStyle *style) const {
    static QStyle *base =
        !style ? QStyleFactory::create(QStringLiteral("Fusion")) : style;
    return base;
}

QStyle *SigilDarkStyle::baseStyle() const { return styleBase(); }

void SigilDarkStyle::polish(QPalette &palette) {
    // modify sigil palette to dark
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::Disabled, QPalette::Window, QColor(80, 80, 80));
    palette.setColor(QPalette::WindowText, QColor(238, 238, 238));
    palette.setColor(QPalette::Disabled, QPalette::WindowText,
                        QColor(127, 127, 127));
    palette.setColor(QPalette::Base, QColor(42, 42, 42));
    palette.setColor(QPalette::Disabled, QPalette::Base, QColor(80, 80, 80));
    palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    palette.setColor(QPalette::ToolTipBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipText, QColor(238, 238, 238));
    palette.setColor(QPalette::Text, QColor(238, 238, 238));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::Dark, QColor(35, 35, 35));
    palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, QColor(238, 238, 238));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                        QColor(127, 127, 127));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(108, 180, 238));
    palette.setColor(QPalette::LinkVisited, QColor(108, 180, 238));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    palette.setColor(QPalette::HighlightedText, QColor(238, 238, 238));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                        QColor(127, 127, 127));
}

void SigilDarkStyle::polish(QApplication *app) {
    if (!app) return;

    // loadstylesheet
    QFile qfDarkstyle(QStringLiteral(":/dark/win-dark-style.qss"));
    if (qfDarkstyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // set stylesheet
        QString qsStylesheet = QString::fromLatin1(qfDarkstyle.readAll());
        app->setStyleSheet(qsStylesheet);
        qfDarkstyle.close();
    }
}
