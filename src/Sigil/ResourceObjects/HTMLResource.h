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
#ifndef HTMLRESOURCE_H
#define HTMLRESOURCE_H

#include <QtCore/QHash>

#include "Misc/CSSInfo.h"
#include "BookManipulation/GuideSemantics.h"
#include "ResourceObjects/XMLResource.h"

class QString;


/**
 * Represents an HTML file of the book.
 * Stores several caches of the content for faster access.
 * There's a QWebPage cache that stores the rendered form of
 * the HTML and a QTextDocument cache that stores the syntax
 * colored version.
 */
class HTMLResource : public XMLResource
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param resources The hash of Resources present in the FolderKeeper.
     * @param parent The object's parent.
     */
    HTMLResource(const QString &fullfilepath,
                 const QHash< QString, Resource * > &resources,
                 QObject *parent = NULL);

    /**
     * Sets the guide semantic type information.
     *
     * @param type The new semantic type.
     */
    void SetGuideSemanticType(GuideSemantics::GuideSemanticType type);

    // inherited
    virtual ResourceType Type() const;

    virtual void SetText(const QString &text);

    virtual bool LoadFromDisk();

    void SaveToDisk(bool book_wide_save = false);

    /**
     * Splits the content of the resource into multiple section.
     * The SGF section markers are used as the break points.
     * The first section is set as the content of the resource,
     * and the others are returned.
     *
     * @return The content of all the sections except the first.
     */
    QStringList SplitOnSGFSectionMarkers();

    /**
     * Returns the paths to all the linked resources
     * like images and stylesheets.
     *
     * @return The paths to the linked resources.
     */
    QStringList GetPathsToLinkedResources();

    /**
     * Returns the paths to all the linked stylesheets
     *
     * @return The paths to the linked stylesheets.
     */
    QStringList GetLinkedStylesheets();

    bool DeleteCSStyles(QList<CSSInfo::CSSSelector *> css_selectors);

signals:
    void LinkedResourceUpdated();
    void TextChanging();
    void LoadedFromDisk();

private:
    /**
     * Makes sure the given paths are watched for updates.
     *
     * @param filepaths The paths to resources to watch.
     */
    void TrackNewResources(const QStringList &filepaths);

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The resource list from FolderKeeper.
     * @todo This is ugly as hell. Find a way to remove this.
     */
    const QHash< QString, Resource * > &m_Resources;
};

#endif // HTMLRESOURCE_H
