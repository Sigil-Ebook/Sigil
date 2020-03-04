/************************************************************************
**
**  Copyright (C) 2020      Kevin B. Hendricks, Stratford Ontario Canada
**
**  Based on LineNumberArea.h which was:
**    Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>, Nokia Corporation
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
#ifndef TVLINENUMBERAREA_H
#define TVLINENUMBERAREA_H

class TextView;

class TVLineNumberArea : public QWidget
{

public:
    // The parameter is the TextView to
    // which this line number area belongs to
    TVLineNumberArea(TextView *viewer);

    // Implements QWidget::sizeHint();
    // Asks the TextView which width should it take
    QSize sizeHint() const;

protected:

    // The line number area delegates its rendering
    // to the TextView that owns it.
    void paintEvent(QPaintEvent *event);

private:

    // The TextView to which this line number area belongs to
    TextView *m_TextViewer;
};

#endif // TVLINENUMBERAREA_H

