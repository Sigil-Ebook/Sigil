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

#pragma once
#ifndef PASTETARGET_H
#define PASTETARGET_H

#include <QtCore/QString>

#include "MiscEditors/ClipEditorModel.h"

class PasteTarget
{

public:
    /**
     * Replace the current selected text if any in this control with the specified text.
     */
    virtual void PasteText(const QString &text) = 0;

    /**
     * Replace the current selected text with using the clipEntry data,
     * which may allow for regex substitutions of the selection.
     */
    virtual bool PasteClipEntries(const QList<ClipEditorModel::clipEntry *> &clips) = 0;
};

#endif // PASTETARGET_H

