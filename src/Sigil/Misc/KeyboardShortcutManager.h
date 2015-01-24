/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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
#ifndef KEYBOARDSHORTCUTMANAHER_H
#define KEYBOARDSHORTCUTMANAHER_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtGui/QKeySequence>

#include "Misc/KeyboardShortcut.h"

class QAction;
class QShortcut;
class QStringlist;

/**
 * Manages keyboard shortcuts for the application.
 *
 * Example:
 * KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
 * sm->registerAction(ui.actionAbout, "main.actionAbout", tr("Show the about dialog."));
 * sm->setKeySequence("main.actionAbout", QKeySequence("Ctrl+1"));
 * sm->registerAction(ui.actionFind, "main.actionFind", tr("Find matching text."));
 * sm->setKeySequence("main.actionFind", QKeySequence("Ctrl+2));
 *
 * The manager will save and load previously set KeyboardShortcut's associated
 * with an id.
 */
class KeyboardShortcutManager
{
public:
    /**
     * The accessor function to access the manager.
     */
    static KeyboardShortcutManager *instance();
    ~KeyboardShortcutManager();

    /**
     * Register an action with an id.
     *
     * This will set create a KeyboardShortcut if necessary and set its keySequence and
     * defaultKeySequence to the actions shortcut if it is not already set. Existing
     * KeyboardShortcuts will not have key sequences changed.
     *
     * @param action QAction to register with the manager.
     * @param id Id used to reference the action for manipulation such as changing
     * the key sequence.
     * @param description A human readable description of what this action represents.
     */
    void registerAction(QWidget* win, QAction *action, const QString &id, const QString &description = QString());
    /**
     * Register a shortcut with an id.
     *
     * This will set create a KeyboardShortcut if necessary and set its keySequence and
     * defaultKeySequence to the actions shortcut if it is not already set. Existing
     * KeyboardShortcuts will not have key sequences changed.
     *
     * @param shortcut QShortcut to register with the manager.
     * @param id Id used to reference the action for manipulation such as changing
     * the key sequence.
     * @param description A human readable description of what this shortcut represents.
     */
    void registerShortcut(QShortcut *shortcut, const QString &id, const QString &description = QString());

    /**
     * Associate a key sequence with a given id.
     *
     * Sets the action or shortcut associated with id to the given key sequence if
     * it is not already in use. If it is a default key sequence then this is
     * guaranteed to succeed.
     *
     * @param id Id that references the action or shortcut.
     * @param keySequence The key sequence to set.
     * @param isDefault Whether this is a default key sequence.
     *
     * @return ture if the key sequence was set. False if it is already in use
     * and was not set.
     */
    bool setKeySequence(const QString &id, const QKeySequence &keySequence, bool isDefault = false);
    /**
     * Associate a default key sequence with a given id.
     *
     * Sets the action or shortcut associated with id to a given default key sequence
     * if it is not already in use.
     *
     * @param id Id that references the action or shortcut.
     * @param keysSequence The key sequence to set.
     *
     * @return true if the key sequence is not in use and is set to the default
     * for id. Otherwise false.
     */
    bool setDefaultKeySequence(const QString &id, const QKeySequence &keySequence);
    /**
     * Removes the key sequence for id and sets the key sequence to the default
     * key sequence associated with id.
     *
     * @param id The id to reset.
     */
    void resetKeySequence(const QString &id);

    /**
     * Set the human readable description for id.
     */
    void setDescription(const QString &id, const QString &description);

    /**
     * removes all actions in all KeyboardShortcuts
     * associated with this parent QWidget *.
     */
    void removeActionsOf(QWidget* win);

    /**
     * Unregisters a given id from the manager.
     *
     * @param id The id to stop managing.
     */
    void unregisterId(const QString &id);
    /**
     * Unregisters a given action from the manager.
     *
     * @param action The action to stop managing.
     */
    void unregisterAction(QAction *action);
    /**
     * Unregisters a given shortcut from the manager.
     *
     * @param shortcut The shortcut to stop managing.
     */
    void unregisterShortcut(QShortcut *shortcut);
    /**
     * Unregisters everything and stops managing everything previously registered
     * with the manager.
     */
    void unregisterAll();

    /**
     * Get the KeyboardShortcut associated with a given id.
     *
     * @return The shortcut associated with id. If there is no shortcut
     * for id an empty shortcut is created.
     */
    KeyboardShortcut keyboardShortcut(const QString &id);
    /**
     * Get a list of all ids managed by the manager.
     *
     * @return A list of ids.
     */
    QStringList ids();

    /**
     * Check if a given key sequence is already in use.
     *
     * @param keySequence The key sequence to check.
     *
     * @return true if the key sequence is in use otherwise false.
     */
    bool keySequenceInUse(const QKeySequence &keySequence);
    /**
     * Check if a given default key sequence is already in use.
     *
     * @param keySequence The key sequence to check.
     *
     * @return true if the default key sequence is in use otherwise false.
     */
    bool defaultKeySequenceInUse(const QKeySequence &keySequence);

    /**
     * Store all managed information presistantly.
     */
    void writeSettings();

private:
    /**
     * Private constructor
     */
    KeyboardShortcutManager();

    /**
     * Create a generic KeyboardShortcut.
     */
    KeyboardShortcut createShortcut(const QKeySequence &keySequence, const QKeySequence &defaultKeySequence = QKeySequence(), const QString &description = QString());

    /**
     * Loads settings from persistent storage for use.
     */
    void readSettings();

    /**
     * If stored as a previously saved setting, copy to our current settings.
     */
    void restoreSavedShortcutForId(const QString &id);

    // Tracks the KeyboardShortcuts we are managing.
    QHash<QString, KeyboardShortcut> m_shortcuts;

    // Tracks the last saved shortcuts. The ini file may contain shortcuts from
    // older versions of Sigil which we will want removed to avoid issues.
    // A saved shortcut will be ignored until its id has been registered by Sigil
    // application code.
    QHash<QString, KeyboardShortcut> m_savedShortcuts;

    static KeyboardShortcutManager *m_instance;
};

#endif // KEYBOARDSHORTCUTMANAHER_H
