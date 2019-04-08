/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#ifndef VIEWER_H
#define VIEWER_H

#include <QtCore/QList>
#include <QtCore/QString>

#include "ViewEditors/Zoomable.h"
#include "ViewEditors/ElementIndex.h"

class QUrl;


/**
 * Interface for Preview Viewer.
 * It would be lovely if we could make this a QObject
 * subclass, but multiple inheritance with multiple
 * QObject subclasses is apparently a bad idea.
 */
class Viewer : public Zoomable
{

public:

    /**
     * Destructor.
     */
    virtual ~Viewer() {}

    /**
     * Returns the state of the loading procedure.
     *
     * @return \c true if loading is finished.
     */
    virtual bool IsLoadingFinished() = 0;

#if 0
    virtual int GetCursorLine() const {
        return -1;
    }
    virtual int GetCursorColumn() const {
        return -1;
    }
#endif

    /**
     * Returns an "encoded" location of the caret element.
     * The returned list of elements represents a "chain"
     * or "walk" through the XHTML document with which one
     * can identify a single element in the document.
     * This list identifies the element in which the
     * keyboard caret is currently located.
     *
     * @return The element selecting list.
     */
    virtual QList<ElementIndex> GetCaretLocation() {
        return QList<ElementIndex>();
    }

    /**
     * Accepts a list returned by a view's GetCaretLocation()
     * and creates and stores an update that sends the caret
     * in this view to the specified element.
     *
     * @param hierarchy The element selecting list.
     */
    virtual void StoreCaretLocationUpdate(const QList<ElementIndex> &hierarchy) {}

    virtual QString GetCaretLocationUpdate() {
        return QString();
    }

    /**
     * Perform the relocating of the caret on screen to
     * the location state that has been persisted.
     *
     * @return \c true if the update was performed.
     */
    virtual bool ExecuteCaretUpdate() {
        return false;
    }
};

#endif // VIEWER_H


