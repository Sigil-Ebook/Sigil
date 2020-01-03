/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Grant Drake
**  Copyright (C) 2012 Dave Heiland
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

#include <QApplication>
#include <QTimer>
#include <QStyleFactory>
#include <QStyle>
#include <QPalette>
#include <QDebug>

#include "MainUI/MainApplication.h"

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv),
      m_Style(nullptr),
      m_isDark(false)
{
#ifdef Q_OS_MAC
    // on macOS the application palette actual text colors never seem to change when DarkMode is enabled
    // so use a mac style standardPalette
    m_Style = QStyleFactory::create("macintosh");
    QPalette app_palette = m_Style->standardPalette();
    m_isDark = app_palette.color(QPalette::Active,QPalette::WindowText).lightness() > 128;
#endif
}

bool MainApplication::event(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::ApplicationActivate) {
        emit applicationActivated();
    } else if (pEvent->type() == QEvent::ApplicationDeactivate) {
        emit applicationDeactivated();
    }
#ifdef Q_OS_MAC
    if (pEvent->type() == QEvent::ApplicationPaletteChange) {
        // qDebug() << "Application Palette Changed";
	QTimer::singleShot(0, this, SLOT(EmitPaletteChanged()));
    }
#endif
    return QApplication::event(pEvent);
}

void MainApplication::EmitPaletteChanged()
{
#ifdef Q_OS_MAC
    QPalette app_palette = m_Style->standardPalette();
    bool isdark = app_palette.color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    if (m_isDark != isdark) {
        qDebug() << "Theme changed " << "was isDark:" << m_isDark << "now isDark:" << isdark;
        m_isDark = isdark;
        setPalette(app_palette);
        emit applicationPaletteChanged();
    }
#endif
}
