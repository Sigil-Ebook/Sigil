/************************************************************************
**
**  Copyright (C) 2019-2024 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2024      Doug Massay
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Grant Drake
**  Copyright (C) 2012      Dave Heiland
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
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    #include <QStyleHints>
#endif
#include <QPalette>
#include <QDebug>

#include "MainUI/MainApplication.h"
#include "Misc/SigilDarkStyle.h"
#include "Misc/SettingsStore.h"
#include "Widgets/CaretStyle.h"

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv),
      m_Style(nullptr),
      m_isDark(false),
      m_accumulatedQss(QString())
{
#ifdef Q_OS_MAC
    // on macOS the application palette actual text colors never seem to change when DarkMode is enabled
    // so use a mac style standardPalette
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_Style = QStyleFactory::create("macintosh");
#else
    m_Style = QStyleFactory::create("macOS");
#endif
    QPalette app_palette = m_Style->standardPalette();
    m_isDark = app_palette.color(QPalette::Active,QPalette::WindowText).lightness() > 128;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // overwriting the app palette in Qt 6 is bad
    // set the initial app palette
    fixMacDarkModePalette(app_palette);
    setPalette(app_palette);
#endif
#endif

// Connect system color scheme change signal to reporting mechanism
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0) && defined(Q_OS_WIN32)
    connect(styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
                MainApplication::systemColorChanged();
        });
#endif
}

void MainApplication::saveInPreviewCache(const QString &key, const QString& xhtml)
{
#if 0
    if (m_CacheKeys.size() > 10) {
        QString oldest_key = m_CacheKeys.takeFirst();
        m_PreviewCache.remove(oldest_key);
    }
    m_CacheKeys.append(key);
#endif
    m_PreviewCache[key] = xhtml;
}

QString MainApplication::loadFromPreviewCache(const QString &key)
{
#if 0
    if (m_CacheKeys.contains(key)) {
        // move to end of list as newest accessed key
        m_CacheKeys.removeOne(key);
        m_CacheKeys.append(key);
        return m_PreviewCache[key];
    }
    return QString();
#endif
    return m_PreviewCache.take(key);
}

void MainApplication::fixMacDarkModePalette(QPalette &pal)
{
# ifdef Q_OS_MAC
    // See QTBUG-75321 and follow Kovid's workaround for broken ButtonText always being dark
    pal.setColor(QPalette::ButtonText, pal.color(QPalette::WindowText));
    if (m_isDark) {
        // make alternating base color change not so sharp
        pal.setColor(QPalette::AlternateBase, pal.color(QPalette::Base).lighter(150));
        // make link color better for dark mode (try to match calibre for consistency)
        pal.setColor(QPalette::Link, QColor("#6cb4ee"));
    }
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
    // on macOS the application palette actual colors never seem to change after launch 
    // even when DarkMode is enabled. So we use a mac style standardPalette to determine
    // if a drak vs light mode transition has been made and then use it to set the 
    // Application palette
    QPalette app_palette = m_Style->standardPalette();
    bool isdark = app_palette.color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    if (m_isDark != isdark) {
        qDebug() << "Theme changed " << "was isDark:" << m_isDark << "now isDark:" << isdark;
        m_isDark = isdark;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // overwriting the app palette in Qt 6 is bad
        fixMacDarkModePalette(app_palette);
        setPalette(app_palette);
#endif
        emit applicationPaletteChanged();
    }
#endif
}

void MainApplication::systemColorChanged()
{
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    switch (styleHints()->colorScheme())
    {
        case Qt::ColorScheme::Light:
            qDebug() << "System Changed to Light Theme";
#ifdef Q_OS_WIN32
            windowsLightThemeChange();
#endif
            break;
        case Qt::ColorScheme::Unknown:
            qDebug() << "System Changed to Uknown Theme";
            break;
        case Qt::ColorScheme::Dark:
            qDebug() << "System Changed to Dark Theme";
#ifdef Q_OS_WIN32
            windowsDarkThemeChange();
#endif
            break;
    }
#endif
}

void MainApplication::windowsDarkThemeChange()
{
    SettingsStore settings;
    QStyle* astyle = QStyleFactory::create("Fusion");
    setStyle(astyle);
    //Handle the new CaretStyle (double width cursor)
    bool isbstyle = false;
    QStyle* bstyle;
    if (settings.uiDoubleWidthTextCursor()) {
        bstyle = new CaretStyle(astyle);
        setStyle(bstyle);
        isbstyle = true;
    }
    // modify windows sigil palette to our dark
    QStyle* cstyle;
    if (isbstyle) {
        cstyle = new SigilDarkStyle(bstyle);
    } else {
        cstyle = new SigilDarkStyle(astyle);
    }
    setStyle(cstyle);
    setPalette(style()->standardPalette());

    // Add back stylesheet changes added after MainApplication started
    if (!m_accumulatedQss.isEmpty()) {
        setStyleSheet(styleSheet().append(m_accumulatedQss));
        qDebug() << styleSheet();
    }
    QTimer::singleShot(0, this, SIGNAL(applicationPaletteChanged()));
}

void MainApplication::windowsLightThemeChange()
{
    // Windows Fusion light mode
    SettingsStore settings;
    QStyle* astyle = QStyleFactory::create("Fusion");
    setStyle(astyle);
    // Handle the new CaretStyle (double width cursor)
    if (settings.uiDoubleWidthTextCursor()) {
        QStyle* bstyle = new CaretStyle(astyle);
        setStyle(bstyle);
    }
    setPalette(style()->standardPalette());
    // Add back stylesheet changes added after MainApplication started
    if (!m_accumulatedQss.isEmpty()) {
        setStyleSheet(m_accumulatedQss);
        qDebug() << styleSheet();
    }
    QTimer::singleShot(0, this, SIGNAL(applicationPaletteChanged()));
}

void MainApplication::updateAccumulatedQss(QString &qss) const
{
    m_accumulatedQss = qss;
}

