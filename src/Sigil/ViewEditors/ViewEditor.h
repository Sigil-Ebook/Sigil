/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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

#include <QString>
#include <QList>
#include "Searchable.h"

class Book;

// Interface for ViewEditors.
// It would be lovely if we could make this a QObject
// subclass, but multiple inheritance with multiple
// QObject subclasses is apparently a bad idea.
class ViewEditor : public Searchable
{

public:

    //   Represents an element in the XHTML document tree
    // and the index of its child that selects the
    // next element in the chain.
    //   By constructing a list of these items, one can
    // navigate the tree by selecting the element,
    // its child with the specified index, its child
    // with its index and so on until reaching
    // the element ultimately identified by this chain.
    //   Because of webkit limitations, this hierarchy
    // does not really look at all child nodes, but only
    // at element child nodes. The text nodes are considered
    // children only for the last element... and even then,
    // it depends on the specific ViewEditor... BookView 
    // does this, CodeView doesn't.
    struct ElementIndex
    {
        QString name;
        int index;
    };

    // Destructor
    virtual ~ViewEditor() {}

    // Sets the content of the View to the specified book
    virtual void SetBook( const Book &book ) = 0;

    // Returns a list of elements representing a "chain"
    // or "walk" through the XHTML document with which one
    // can identify a single element in the document.
    // This list identifies the element in which the 
    // keyboard caret is currently located.
    virtual QList< ElementIndex > GetCaretLocation() = 0;

    // Accepts a list returned by a view's GetCaretLocation
    // and creates and stores an update that sends the caret
    // in this view to the specified element.
    virtual void StoreCaretLocationUpdate( const QList< ElementIndex > &hierarchy ) = 0;

    // Sets a zoom factor for the view,
    // thus zooming in (factor > 1.0) or out (factor < 1.0). 
    virtual void SetZoomFactor( float factor ) = 0;

    // Returns the View's current zoom factor
    virtual float GetZoomFactor() const = 0;    
};

#endif // VIEWEDITOR_H


