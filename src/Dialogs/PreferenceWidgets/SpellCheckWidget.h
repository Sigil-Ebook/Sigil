/************************************************************************
**
**  Copyright (C) 2013 Dave Heiland
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
#include <QtGui/QStandardItemModel>

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
    void copyUserDict();
    void removeUserDict();

    void addUserWords();
    void editWord();
    void removeWord();
    void removeAll();

    void userWordChanged(QListWidgetItem *item);

    void loadUserDictionaryWordList(QString dict_name = "");
    void saveUserDictionaryWordList(QString dict_name = "");

    void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void dictionariesCurrentIndexChanged(int index);
    void checkBoxChanged(int state);
    void ItemChanged(QStandardItem *item);
    void tabChanged(int tab);

private:
    void setUpTable();
    void setDefaultUserDictionary(QString dict_name = QString());
    bool createUserDict(const QStringList dict_name);
    bool copyDicWithSubdics(const QString &source, const QStringList &destination);

    QStringList EnabledDictionaries();

    void addNewItem(bool enabled, QString dict_name);
    void addNewMLItem(bool enabled, QStringList dicts);

    QStandardItem * getParent(const QModelIndex selected);
    QStandardItem * getSelected(const QModelIndex selected);
    QString getSelectedDictName(const QModelIndex selected);
    QString getSelectedParentDic(const QModelIndex selected);
    QStringList getChildrenDics(const QModelIndex selected);
    void dictionaryDirty(const QString userDict, const bool force=false);

    void readSettings();
    void connectSignalsToSlots();

    Ui::SpellCheckWidget ui;
    bool m_isDirty;

    QByteArray m_hash;
    QStringList m_dirtyDicts;
    QStandardItemModel *m_Model;
};


#endif // SPELLCHECKWIDGET_H
