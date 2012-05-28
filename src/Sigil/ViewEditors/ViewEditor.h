/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef VIEWEDITOR_H
#define VIEWEDITOR_H

#include <QtCore/QList>
#include <QtCore/QString>

#include "ViewEditors/Searchable.h"
#include "ViewEditors/Zoomable.h"

class QUrl;


/**
 * Interface for ViewEditors.
 * It would be lovely if we could make this a QObject
 * subclass, but multiple inheritance with multiple
 * QObject subclasses is apparently a bad idea.
 */
class ViewEditor : public Searchable, public Zoomable
{

public:
    /**
     * Destructor.
     */
    virtual ~ViewEditor() {}

    /**
     * Returns the state of the loading procedure.
     *
     * @return \c true if loading is finished.
     */
    virtual bool IsLoadingFinished() = 0;

    virtual int GetCursorLine() const { return -1; }
    virtual int GetCursorColumn() const { return -1; }
};

#endif // VIEWEDITOR_H


