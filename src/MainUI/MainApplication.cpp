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

#define DBG if(1)

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv),
      m_isDark(false),
      m_accumulatedQss(QString()),
      m_PaletteChangeTimer(new QTimer()),
      m_AlwaysUseNFC(true)
{

    // Do this only once early on in the Sigil startup
    if (qEnvironmentVariableIsSet("SIGIL_DISABLE_NFC_NORMALIZATION")) m_AlwaysUseNFC = false;
        
    // Keep track on our own of dark or light
    m_isDark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    
    // Set up PaletteChangeTimer to absorb multiple QEvents
    // We need this for older < Qt 6.5 and as a backup mechanism for Linux
    // Whose ability to detect theme changes is not in a good state
    m_PaletteChangeTimer->setSingleShot(true);
    m_PaletteChangeTimer->setInterval(50);
    connect(m_PaletteChangeTimer, SIGNAL(timeout()),this, SLOT(systemColorChanged()));
    m_PaletteChangeTimer->stop();

// Connect system color scheme change signal to reporting mechanism
// Note: This mechanism is very very unreliable on Linux (across many distributions and desktops)
// So fall back to the QApplication:Palette change event instead for all of Linux for now
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    DBG qDebug() << "initial styleHints colorScheme: " << styleHints()->colorScheme();
    if (styleHints()->colorScheme() == Qt::ColorScheme::Unknown) {
        m_isDark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    } else {
        m_isDark = styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    }
    connect(styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
                MainApplication::systemColorChanged();
        });
#endif
}

void MainApplication::saveInPreviewCache(const QString &key, const QString& xhtml)
{
    m_PreviewCache[key] = xhtml;
}

QString MainApplication::loadFromPreviewCache(const QString &key)
{
    return m_PreviewCache.take(key);
}

bool MainApplication::event(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::ApplicationActivate) {
        emit applicationActivated();
    } else if (pEvent->type() == QEvent::ApplicationDeactivate) {
        emit applicationDeactivated();
    }
    if (pEvent->type() == QEvent::ApplicationPaletteChange) {
        // can be generated multiple times
        DBG qDebug() << "Application Palette Changed";

#if QT_VERSION < QT_VERSION_CHECK(6,5,0) ||  (!defined(Q_OS_WIN32) && !defined(Q_OS_MAC))
	// Use this approach as a backup for Linux currently
        if (m_PaletteChangeTimer->isActive()) m_PaletteChangeTimer->stop();
        m_PaletteChangeTimer->start();
#endif

    }
    return QApplication::event(pEvent);
}

void MainApplication::EmitPaletteChanged()
{
    DBG qDebug() << "emitting applicationPaletteChanged";
    emit applicationPaletteChanged();
}

void MainApplication::systemColorChanged()
{
    // intential race on Linux so that both approaches work (typically won by styleHint() signal
    // if it ever gets properly generated
    m_PaletteChangeTimer->stop();
    bool theme_changed = false;
    bool isdark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    DBG qDebug() << "reached systemColorChanged";
    
#if QT_VERSION < QT_VERSION_CHECK(6,5,0)
    // in reality we really should not care if light or dark, just that theme changed
    // but this is where we are at now
    if (isdark != m_isDark) {
        m_isDark = isdark;
        theme_changed = true;
    }

#else  // Qt >= 6.5 and not Linux till it gets more robust

    switch (styleHints()->colorScheme())
    {
        case Qt::ColorScheme::Light:
            DBG qDebug() << "System Changed to Light Theme";
            m_isDark = false;
            theme_changed = true;
            
#ifdef Q_OS_WIN32
            windowsLightThemeChange();
#endif // Q_OS_WIN32
            
            break;

        case Qt::ColorScheme::Unknown:
            DBG qDebug() << "System Changed to Unknown Theme";
	    // This is typical needed for all linux where Qt can not properly
	    // identify the theme being used
            if (isdark != m_isDark) {
                m_isDark = isdark;
                theme_changed = true;
            }
            break;

        case Qt::ColorScheme::Dark:
            DBG qDebug() << "System Changed to Dark Theme";
            m_isDark = true;
            theme_changed = true;

#ifdef Q_OS_WIN32
            windowsDarkThemeChange();
#endif // Q_OS_WIN32

            break;
    }
    
#endif // Qt Version Check
    
    if (theme_changed) QTimer::singleShot(0, this, SLOT(EmitPaletteChanged()));

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
