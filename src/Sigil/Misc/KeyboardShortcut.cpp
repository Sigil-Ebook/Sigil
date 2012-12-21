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

#include <QtCore/QString>
#include <QtWidgets/QAction>
#include <QtGui/QKeySequence>
#include <QtWidgets/QShortcut>

#include "Misc/KeyboardShortcut.h"
#include "Misc/KeyboardShortcut_p.h"

KeyboardShortcutPrivate::KeyboardShortcutPrivate()
    : action(0),
      shortcut(0)
{
}

KeyboardShortcutPrivate::KeyboardShortcutPrivate(const KeyboardShortcutPrivate &other)
    : QSharedData(other),
      action(other.action),
      shortcut(other.shortcut),
      description(other.description),
      toolTip(other.toolTip),
      keySequence(other.keySequence),
      defaultKeySequence(other.defaultKeySequence)
{
}

KeyboardShortcut::KeyboardShortcut()
{
    d = new KeyboardShortcutPrivate;
}

KeyboardShortcut::KeyboardShortcut(const KeyboardShortcut &other) :
    d(other.d)
{
}

bool KeyboardShortcut::isEmpty()
{
    if (d->action == 0 && d->shortcut == 0) {
        return true;
    }

    return false;
}

void KeyboardShortcut::setAction(QAction *action)
{
    d->action = action;
}

void KeyboardShortcut::setShortcut(QShortcut *shortcut)
{
    d->shortcut = shortcut;
}

void KeyboardShortcut::setName(const QString &name)
{
    d->name = name;
}

void KeyboardShortcut::setDescription(const QString &description)
{
    d->description = description;
}

void KeyboardShortcut::setToolTip(const QString &toolTip)
{
    d->toolTip = toolTip;
}

void KeyboardShortcut::setKeySequence(const QKeySequence &keySequence)
{
    d->keySequence = keySequence;
}

void KeyboardShortcut::setDefaultKeySequence(const QKeySequence &defaultKeySequence)
{
    d->defaultKeySequence = defaultKeySequence;
}

QAction *KeyboardShortcut::action()
{
    return d->action;
}

QShortcut *KeyboardShortcut::shortcut()
{
    return d->shortcut;
}

QString KeyboardShortcut::name()
{
    return d->name;
}

QString KeyboardShortcut::description()
{
    return d->description;
}

QString KeyboardShortcut::toolTip()
{
    return d->toolTip;
}

QKeySequence KeyboardShortcut::keySequence()
{
    return d->keySequence;
}

QKeySequence KeyboardShortcut::defaultKeySequence()
{
    return d->defaultKeySequence;
}
