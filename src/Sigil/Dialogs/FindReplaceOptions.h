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
#ifndef FINDREPLACEOPTIONS_H
#define FINDREPLACEOPTIONS_H

#include "ui_FindReplaceOptions.h"
#include "MainUI/FindReplace.h"

class FindReplaceOptions : public QDialog
{
public:

    FindReplaceOptions( QWidget *parent );

    FindReplace::LookWhere GetLookWhere();
    FindReplace::SearchMode GetSearchMode();

    bool GetMatchWholeWord();
    bool GetMatchCase();
    bool GetMatchMinimal();

    void SetLookWhere( FindReplace::LookWhere look_where);
    void SetSearchMode( FindReplace::SearchMode search_mode );

    void SetMatchWholeWord( bool enabled );
    void SetMatchCase( bool enabled );
    void SetMatchMinimal( bool enabled );

private:

    void ExtendUI();

    Ui::FindReplaceOptions ui;
};

#endif // FINDREPLACEOPTIONS_H
