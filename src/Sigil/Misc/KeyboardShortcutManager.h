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

#include <QHash>
#include <QKeySequence>
#include <QString>
#include "KeyboardShortcut.h"

class QAction;
class QShortcut;

/**
 * Managers keyboard short cuts for the application.
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
    void registerAction(QAction *action, const QString &id, const QString &description=QString());
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
    void registerShortcut(QShortcut *shortcut, const QString &id, const QString &description=QString());

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
    bool setKeySequence(const QString &id, const QKeySequence &keySequence, bool isDefault=false);
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
    KeyboardShortcut createShortcut(const QKeySequence &keySequence, const QKeySequence &defaultKeySequence, const QString &description=QString());

    /**
     * Loads settings form persistant storage for use.
     */
    void readSettings();

    // Tracks the KeyboardShortcuts we are managing.
    QHash<QString, KeyboardShortcut> m_shortcuts;
    static KeyboardShortcutManager *m_instance;
};

#endif // KEYBOARDSHORTCUTMANAHER_H
