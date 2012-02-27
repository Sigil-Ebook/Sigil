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
#ifndef KEYBOARDSHORTCUTPRIVATE_H
#define KEYBOARDSHORTCUTPRIVATE_H

#include <QtCore/QSharedData>

class QAction;
class QKeySequence;
class QShortcut;
class QString;

class KeyboardShortcutPrivate : public QSharedData
{
public:
    KeyboardShortcutPrivate();
    KeyboardShortcutPrivate(const KeyboardShortcutPrivate &other);

    // The shortcut that that is associated with the data below.
    QAction *action;
    QShortcut *shortcut;
    // If this is an action this is the text that will display when the
    // action is part of a menu.
    QString name;
    // Short text desciription what command 'does'.
    QString description;
    // KeySequnce that is assigned.
    QKeySequence keySequence;
    // Default KeySequence used when "resetting" the shortcut.
    QKeySequence defaultKeySequence;
};

#endif // KEYBOARDSHORTCUTPRIVATE_H
