/************************************************************************
**
**  Copyright (C) 2019-2025 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2024-2025 Doug Massay
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

#define DBG if(0)

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv),
      m_isDark(false),
      m_accumulatedQss(QString()),
      m_PaletteChangeTimer(new QTimer()),
      m_AlwaysUseNFC(true)
{

    // Do this only once early on in the Sigil startup
    if (qEnvironmentVariableIsSet("SIGIL_DISABLE_NFC_NORMALIZATION")) m_AlwaysUseNFC = false;
        
    m_isDark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    DBG qDebug() << "initial state based on palette itself with our test is_dark: " << m_isDark;

    // Determine how to best detect dark vs light theme changes based on Qt Version, platform,
    // with an env variable override for Linux and other platforms
    m_UseAppPaletteEvent=true;
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
    m_UseAppPaletteEvent=false;
#else 
    if (qEnvironmentVariableIsSet("SIGIL_USE_COLORSCHEME_CHANGED")) m_UseAppPaletteEvent=false;
#endif
#endif

    if (m_UseAppPaletteEvent) {
        // Set up PaletteChangeTimer to absorb multiple QEvents
        m_PaletteChangeTimer->setSingleShot(true);
        m_PaletteChangeTimer->setInterval(50);
        connect(m_PaletteChangeTimer, SIGNAL(timeout()),this, SLOT(systemColorChanged()));
        m_PaletteChangeTimer->stop();
    } else {
      
// gcc compiler is not smart enough to optimize the else clause away based on qt version < 6.5.0
// so we still need this ifdef
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)

        // Connect Qt system color scheme change signal to reporting mechanism
        DBG qDebug() << "initial styleHints colorScheme: " << styleHints()->colorScheme();
        if (styleHints()->colorScheme() == Qt::ColorScheme::Unknown) {
            m_isDark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
        } else {
            m_isDark = styleHints()->colorScheme() == Qt::ColorScheme::Dark;
        }
        connect(styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
                    MainApplication::systemColorChanged();
            });

#endif  // Qt 6.5.0 or greater

    }
}

void MainApplication::saveInPreviewCache(const QString &key, const QString& xhtml)
{
    m_PreviewCache[key] = xhtml;
}

QString MainApplication::loadFromPreviewCache(const QString &key)
{
    return m_PreviewCache.value(key, ""); 
}

bool MainApplication::event(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::ApplicationActivate) {
        emit applicationActivated();
    } else if (pEvent->type() == QEvent::ApplicationDeactivate) {
        emit applicationDeactivated();
    }
    if (pEvent->type() == QEvent::ApplicationPaletteChange) {
        if (m_UseAppPaletteEvent) {
            DBG qDebug() << "Application Palette Changed Event";
            // Note: can be generated multiple times in a row so use timer to absorb them
            if (m_PaletteChangeTimer->isActive()) m_PaletteChangeTimer->stop();
            m_PaletteChangeTimer->start();
        }
    }
    return QApplication::event(pEvent);
}

void MainApplication::EmitPaletteChanged()
{
    DBG qDebug() << "emitting Sigil's applicationPaletteChanged signal";
    emit applicationPaletteChanged();
}

void MainApplication::systemColorChanged()
{
    DBG qDebug() << "reached systemColorChanged";
    bool isdark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    if (m_UseAppPaletteEvent) {
        m_PaletteChangeTimer->stop();
        DBG qDebug() << "    via App PaletteEvent timer signal with m_isDark: " << m_isDark;
        DBG qDebug() << "        and current palette test isdark: " << isdark;
        // in reality we really should not care if light or dark, just that palette changed
        // but this is where we are at now
        m_isDark = isdark;
    } else {
      
// gcc compiler is not smart enough to optimize the else clause away based on qt version < 6.5.0
// so we still need this ifdef
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)

        DBG qDebug() << "    via Qt ColorSDchemeChanged Signal";
        switch (styleHints()->colorScheme())
        {
            case Qt::ColorScheme::Light:
                DBG qDebug() << "Color Scheme Changed to Light Theme";
                m_isDark = false;
#ifdef Q_OS_WIN32
                windowsLightThemeChange();
#endif
                break;

            case Qt::ColorScheme::Unknown:
                DBG qDebug() << "Color Scheme Changed to Unknown Theme";
                m_isDark = isdark;
                break;

            case Qt::ColorScheme::Dark:
                DBG qDebug() << "Color Scheme Changed to Dark Theme";
                m_isDark = true;
#ifdef Q_OS_WIN32
                windowsDarkThemeChange();
#endif
                break;
        }

#endif // Qt 6.5.0 or greater

    }
    QTimer::singleShot(0, this, SLOT(EmitPaletteChanged()));
}

void MainApplication::windowsDarkThemeChange()
{
    SettingsStore settings;
    if (settings.uiUseCustomSigilDarkTheme()) {
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
            DBG qDebug() << styleSheet();
        }
    }
}

void MainApplication::windowsLightThemeChange()
{
    SettingsStore settings;
    if (settings.uiUseCustomSigilDarkTheme()) {
        // Windows Fusion light mode
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
            DBG qDebug() << styleSheet();
        } else {
            setStyleSheet("");
        }
    }
}

void MainApplication::updateAccumulatedQss(QString &qss) const
{
    m_accumulatedQss = qss;
}
