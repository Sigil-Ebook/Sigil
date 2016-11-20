/************************************************************************
**
**  Copyright (C) 2012, 2013 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012, 2013 Dave Heiland
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
#ifndef SPELLCHECKEDITOR_H
#define SPELLCHECKEDITOR_H

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QShortcut>
#include <QtCore/QSharedPointer>

#include "Misc/SettingsStore.h"
#include "BookManipulation/Book.h"

#include "ui_SpellcheckEditor.h"

class QPoint;
class SpellCheck;

/**
 * The editor used to create and modify index entries
 */
class SpellcheckEditor : public QDialog
{
    Q_OBJECT

public:
    SpellcheckEditor(QWidget *parent);
    ~SpellcheckEditor();

    void SetBook(QSharedPointer <Book> book);
    void ForceClose();

public slots:
    void Refresh(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

signals:
    void ShowStatusMessageRequest(const QString &message);
    void SpellingHighlightRefreshRequest();
    void FindWordRequest(QString word);
    void UpdateWordRequest(QString old_word, QString new_word);


protected:
    bool eventFilter(QObject *obj, QEvent *ev);

protected slots:
    void showEvent(QShowEvent *event);

private slots:
    void FindSelectedWord();
    void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void UpdateSuggestions();

    void DictionaryChanged(QString dictionary);
    void ChangeState(int state);

    void Ignore();
    void Add();
    void SelectAll();
    void ChangeAll();

    void toggleShowAllWords();
    void toggleCaseInsensitiveSort();

    void FilterEditTextChangedSlot(const QString &text);

    void OpenContextMenu(const QPoint &point);

    void Sort(int logicalindex, Qt::SortOrder order);

//******varlogs
    void IgnoreML();
    const QString choseDictionary();
    void loadDictionary();
    void loadDictionary(const QString lang);
    void unloadDictionary();
    void getDictionary();
    void changeDictionary();
    void update_cbCAToolTipp(const QString &word);
    void changeCodeOrAlias(const int row, const int column);

//******varlogs end

private:
    void CreateModel(int logicalindex, Qt::SortOrder order);
    void UpdateDictionaries();
    void SetupSpellcheckEditorTree();
    void MarkSpelledOkay(int row);
    QString GetSelectedWord();
    int GetSelectedRow();

    int SelectedRowsCount();

    void SelectRow(int row);

    QList<QStandardItem *> GetSelectedItems(const int column);

    void ReadSettings();
    void WriteSettings();

    void CreateContextMenuActions();
    void SetupContextMenu(const QPoint &point);

    void ConnectSignalsSlots();

    //***varlogs
    void setupMultiLanguageUi();
    void setupLoadedDicsTable();
    void populateLoadedDicsTable();
    const QString getSelectedWordLanguage();
    QString toCodeName(QString code);
    QString toDicName(QString code);
    QAction *m_loadDic;
    QAction *m_changeDic;
    //***varlogs end

    QAction *m_Ignore;
    QAction *m_Add;
    QAction *m_Find;
    QAction *m_SelectAll;

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_SpellcheckEditorModel;

    QMenu *m_ContextMenu;

    bool m_MultipleSelection;

    int m_SelectRow;

    QShortcut * m_FilterSC;
    QShortcut * m_ShowAllSC;
    QShortcut * m_NoCaseSC;
    QShortcut * m_RefreshSC;

    Ui::SpellcheckEditor ui;

    SpellCheck * m_SpellCheck;
    QList<QVariant> m_bookDCLanguages;

};

#endif // SPELLCHECKEDITOR_H
