/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Grant Drake
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

#pragma once
#ifndef APPEARANCEWIDGET_H
#define APPEARANCEWIDGET_H

#include "MainUI/MainWindow.h"
#include "Misc/SettingsStore.h"
#include "PreferencesWidget.h"
#include "ui_PAppearanceWidget.h"

/**
 * Preferences widget for font/appearance related preferences.
 */
class AppearanceWidget : public PreferencesWidget
{
    Q_OBJECT

public:
    AppearanceWidget();
    PreferencesWidget::ResultAction saveSettings();

private slots:
    void customColorButtonClicked();
    void resetAllButtonClicked();
    void newSliderValue(int value);

private:
    SettingsStore::CodeViewAppearance readSettings();
    void loadComboValueOrDefault(QFontComboBox *fontComboBox, const QString &value, const QString &defaultValue);
    void loadCodeViewColorsList(SettingsStore::CodeViewAppearance);
    void addColorItem(const QString &text, const QColor &color);
    QColor getListItemColor(const int &row = -1);
    void connectSignalsToSlots();

    SettingsStore::CodeViewAppearance m_codeViewAppearance;
    QColor m_currentColor;

    Ui::AppearanceWidget ui;
};

#endif // APPEARANCEWIDGET_H
