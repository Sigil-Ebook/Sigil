/************************************************************************
**
**  Copyright (C) 2014 Marek Gibek
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

#include "Dialogs/CleanContent.h"

CleanContent::CleanContent(QWidget *parent)
    :
    QDialog(parent)
{
    ui.setupUi(this);
}

CleanContentParams CleanContent::GetParams()
{
    CleanContentParams params;

    params.remove_page_numbers = ui.checkBoxRemovePageNumbers->isChecked();
    params.page_number_format = ui.lineEditPageNumberFormat->text();

    params.remove_empty_paragraphs = ui.checkBoxRemoveEmptyParagraphs->isChecked();

    params.join_paragraphs = ui.checkBoxJoinParagraphs->isChecked();

    return params;
}
