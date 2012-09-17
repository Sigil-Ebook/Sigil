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
#ifndef IMAGETAB_H
#define IMAGETAB_H

#include "Tabs/ContentTab.h"

class ImageResource;
class QWebView;
class QAction;
class QMenu;

class ImageTab : public ContentTab
{
    Q_OBJECT

public:

    ImageTab( ImageResource& resource, QWidget *parent = 0 );
    ~ImageTab();

    // Overrides inherited from ContentTab
    float GetZoomFactor() const;

    void SetZoomFactor( float new_zoom_factor );

    void UpdateDisplay();

public slots:
    void RefreshContent();

    void openWith();
    void openWithEditor();

private slots:

    /**
     * Opens the context menu at the requested point.
     *
     * @param point The point at which the menu should be opened.
     */
    void OpenContextMenu( const QPoint &point );

private:

    /**
     * Creates all the context menu actions.
     */
    void CreateContextMenuActions();

    /**
     * Tries to setup the context menu for the specified point,
     * and returns true on success.
     *
     * @param point The point at which the menu should be opened.
     * @return \c true if the menu could be set up.
     */
    bool SuccessfullySetupContextMenu( const QPoint &point );

    void ConnectSignalsToSlots();

    void Zoom();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    QWebView &m_WebView;

    QMenu &m_ContextMenu;
    QMenu &m_OpenWithContextMenu;

    QAction *m_OpenWith;
    QAction *m_OpenWithEditor;

    float m_CurrentZoomFactor;
};

#endif // IMAGETAB_H
