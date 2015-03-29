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
#ifndef KEYBOARDSHORTCUT_H
#define KEYBOARDSHORTCUT_H

#include <QList>
#include <QtCore/QExplicitlySharedDataPointer>

#include "Misc/KeyboardShortcut_p.h"

class QAction;
class QKeySequence;
class QShortcut;
class QString;

class KeyboardShortcut
{
public:
    KeyboardShortcut();
    KeyboardShortcut(const KeyboardShortcut &other);

    bool isEmpty();

    void addAction(QWidget*, QAction *action);
    void removeAction(QWidget*);
    QList<QAction*> getAllActions();
    void setShortcut(QShortcut *shortcut);
    void setName(const QString &name);
    void setDescription(const QString &description);
    void setToolTip(const QString &toolTip);
    void setKeySequence(const QKeySequence &keySequence);
    void setDefaultKeySequence(const QKeySequence &defaultKeySequence);

    QAction *action();
    QShortcut *shortcut();
    QString name();
    QString description();
    QString toolTip();
    QKeySequence keySequence();
    QKeySequence defaultKeySequence();

private:
    QExplicitlySharedDataPointer<KeyboardShortcutPrivate> d;
};

#endif // KEYBOARDSHORTCUT_H
