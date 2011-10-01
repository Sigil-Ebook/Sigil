/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
**  Copyright (C) 2011 Grzegorz Wolszczak <grzechu81@gmail.com>
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
#ifndef KEYBOARDSHORTCUTSWIDGET_H
#define KEYBOARDSHORTCUTSWIDGET_H

#include "PreferencesWidget.h"
#include "ui_PKeyboardShortcutsWidget.h"

// For now QShortcut can only have 1 key (+modifier) e.g. (ALT+F), although code supports up to 4.
// It would be strange if user wanted to define shortcuts like (ALT+F,ALT+D) to do some action,
// but if for some reason it would be needed in code to extend key values to more than key in shortcuts
// ...just change below define
#define MAX_KEYS_IN_SHORTCUT 1

class QString;

/**
 * Preferences widget for KeyboardShortcut related preferences.
 */
class KeyboardShortcutsWidget : public PreferencesWidget
{
    Q_OBJECT

public:
    KeyboardShortcutsWidget();
    void saveSettings();

    /**
     * Filter events.
     *
     * @param object Object that is being watched.
     * @param event Event that has taken place.
     *
     * @return true if the event should not be handled further.
     */
    bool eventFilter(QObject *object, QEvent *event);

protected:
    /**
     * When the widget is shown.
     *
     * @param event Information about showing the widget.
     */
    void showEvent(QShowEvent *event);

private slots:
    /**
     * Handles activation of an item in the treeview that lists the actions.
     *
     * @param item The item that was activated.
     * @param column The column that was activated.
     */
    void treeWidgetItemActivatedSlot(QTreeWidgetItem *item, int column);
    /**
     * Clear the keysequence associated with the selected item.
     */
    void clearButtonClicked();

    /**
     * Displays item in the list that match the user's entered text.
     *
     * @param text Text that was entered into the filter edit.
     */
    void filterEditTextChangedSlot(const QString &text);
    /**
     * Associate the shortcut that the user entered with the id.
     *
     * @param text The text that was entered.
     */
    void targetEditTextChangedSlot(const QString &text);

    /**
     * Replace the keysequence for the selected item with its default.
     */
    void resetButtonClickedSlot();
    /**
     * Replace the keysequences for every selected item with its default.
     */
    void resetAllButtonClickedSlot();

private:
    void readSettings();
    void connectSignalsSlots();
    /**
     * Handles which characters are allowed to be used as shortcuts.
     *
     * @param event Information associated with this function call.
     */
    void handleKeyEvent(QKeyEvent *event);
    /**
     * Turns modifier keys into a displayable character.
     *
     * @param state Which modifier keys are in use.
     * @param text The text that was entered.
     */
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    /**
     * Changes how an item in the list is displayed. Modified items are shown
     * as italic.
     *
     * @param item The item in the list.
     */
    void updateItemView(QTreeWidgetItem *item);
    /**
     * Store the shortcut for the currently selected item.
     */
    void updateCurrentItemShortcut();
    /**
     * Displays all items that are in use by another item in red.
     */
    void markSequencesAsDuplicatedIfNeeded();

    Ui::KeyboardShortcutsWidget ui;
    int m_key[MAX_KEYS_IN_SHORTCUT];
    int m_keyNum;
};

#endif // KEYBOARDSHORTCUTSWIDGET_H
