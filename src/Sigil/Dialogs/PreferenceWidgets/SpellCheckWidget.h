/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef SPELLCHECKWIDGET_H
#define SPELLCHECKWIDGET_H

#include "PreferencesWidget.h"
#include "ui_PSpellCheckWidget.h"

class SpellCheckWidget : public PreferencesWidget
{
    Q_OBJECT

public:
    SpellCheckWidget();
    PreferencesWidget::ResultAction saveSettings();

private slots:
    void addUserDict();
    void renameUserDict();
    void removeUserDict();

    void addWord();
    void editWord();
    void removeWord();
    void removeAll();

    void userWordChanged(QListWidgetItem *item);

    void openDictionaryDirectory();

    void loadUserDictionaryWordList(QListWidgetItem *item=0);
    void saveUserDictionaryWordList(QListWidgetItem *item=0);

    void userDictionaryChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void dictionariesCurrentIndexChanged(int index);

private:
    void readSettings();
    void connectSignalsToSlots();

    Ui::SpellCheckWidget ui;
    bool m_isDirty;
};

#endif // SPELLCHECKWIDGET_H
