/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QLineEdit>

#include "Misc/PasteTargetComboBox.h"

PasteTargetComboBox::PasteTargetComboBox( QWidget *parent )
    : QComboBox( parent )
{
}

void PasteTargetComboBox::PasteText(const QString &text)
{
    // I originally tried logic manipulating the selected text. But the problem is that QLineEdit
    // does not support an Undo() if you call setEditText(), only if you call paste()
    // The only downside is that we will want to save/restore the clipboard state around it.
    emit ClipboardSaveRequest();
    
    QApplication::clipboard()->setText(text);
    lineEdit()->paste();
    
    emit ClipboardRestoreRequest();
}

bool PasteTargetComboBox::PasteClipEntries(const QList<ClipEditorModel::clipEntry *> &clips)
{
    bool applied = false;
    foreach(ClipEditorModel::clipEntry *clip, clips) {
        applied = applied || PasteClipEntry(clip);
    }
    return applied;
}

bool PasteTargetComboBox::PasteClipEntry(ClipEditorModel::clipEntry *clip)
{
    if (!clip || clip->text.isEmpty()) {
        return false;
    }

    PasteText(clip->text);
    return true;
}
