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
#ifndef TABMANAGER_H
#define TABMANAGER_H

#include "ContentTab.h"
#include <QTabWidget>
#include <QWeakPointer>
#include <QUrl>

class Resource;
class HTMLResource;

/**
 * Manages the tabs shown in the main UI.
 * Handles open resource requests, tab switching, closing etc.
 */
class TabManager : public QTabWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * 
     * @param parent The QObject's parent.
     */
    TabManager( QWidget *parent = 0 );

    /**
     * Returns a reference to the current content tab.
     *
     * @return A reference to the current content tab.
     */
    ContentTab& GetCurrentContentTab();    

public slots:

    /**
     * Saves any unsaved data in the all the open tabs.
     */
    void SaveTabData();

    /**
     * Opens the specified resource in a new tab.
     * If the resource is already opened, it becomes the current one. 
     *
     * @param resource The resource that should be opened.
     * @param precede_current_tab Should the new tab precede the currently opened one.
     * @param fragment The fragment ID to which the new tab should be scrolled.
     * @param view_state In which View should the resource open or switch to.
     * @param line_to_scroll_to To which line should the resource scroll.
     */
    void OpenResource( Resource& resource, 
                       bool precede_current_tab = false,
                       const QUrl &fragment = QUrl(),
                       ContentTab::ViewState view_state = ContentTab::ViewState_AnyView,
                       int line_to_scroll_to = -1);

    /**
     * Makes the next (right) tab the current one.
     * Wraps around if necessary.
     */
    void NextTab();

    /**
     * Makes the previous (left) tab the current one.
     * Wraps around if necessary.
     */
    void PreviousTab();

    /**
     * Closes the current tab.
     * If there is the only one tab opened, the command is ignored.
     */
    void CloseTab();

signals:

    /**
     * Emitted whenever the user switches from one tab to the next.
     *
     * @param old_tab The tab \e from which the user is switching.
     * @param new_tab The tab \e to which the user is switching.
     */
    void TabChanged( ContentTab* old_tab, ContentTab* new_tab );

    /**
     * Emitted whenever one of the tabs wants to open an URL.
     *
     * @param url The URL to open.
     */
    void OpenUrlRequest( const QUrl &url );

    /**
     * Wired to the current FlowTab::OldTabRequest signal.
     */
    void OldTabRequest( QString content, HTMLResource& originating_resource );

    /**
     * Wired to the current FlowTab::NewChaptersRequest signal.
     */
    void NewChaptersRequest( QStringList chapters );

private slots:

    /**
     * Emits the TabChanged signal.
     */
    void EmitTabChanged();

    /**
     * Makes sure the old tab saves its state, and the new one loads its.
     * This happens whenever the user switches from one tab to different one.
     */
    void UpdateTabStatesOnSwitch( ContentTab* old_tab, ContentTab* new_tab );

    /**
     * Deletes the specified tab.
     *
     * @param tab_to_delete The tab to delete.
     */
    void DeleteTab( ContentTab *tab_to_delete );

    /**
     * Closes the tab at the specified index.
     * If there is only one tab left, the command is ignored.
     *
     * @param tab_index The index of the tab to close.
     */
    void CloseTab( int tab_index );

    /**
     * Updates the name/header text of the specified tab.
     *
     * @param renamed_tab The renamed tab. 
     */
    void UpdateTabName( ContentTab *renamed_tab );

private:

    /**
     * Returns the index of tab in which the resource is loaded.
     * If the resource is not currently loaded, -1 is returned. 
     *
     * @param resource The resource whose tab index we want.
     * @return The index of the resource.
     */
    int ResourceTabIndex( const Resource& resource ) const;

    /**
     * Returns true if we have succeeded in switching to the tab of the provided resource.
     * 
     * @param resource The resource we want to switch to.
     * @param fragment The fragment ID to which the tab should scroll.
     * @param view_state In which View should the resource open or switch to.
     * @param line_to_scroll_to To which line should the resource scroll.
     * @return \c true if we succeeded in switching.
     */
    bool SwitchedToExistingTab( Resource& resource, 
                                const QUrl &fragment, 
                                ContentTab::ViewState view_state,
                                int line_to_scroll_to );

    /**
     * Creates a tab for the specified resource.
     *
     * @param resource The resource for which we want to create a tab.
     * @param fragment The fragment ID to which the tab should scroll after load.
     * @param view_state In which View should the resource open or switch to.
     * @param line_to_scroll_to To which line should the resource scroll.
     * @return The newly created tab.
     */
    ContentTab* CreateTabForResource( Resource& resource, 
                                      const QUrl &fragment, 
                                      ContentTab::ViewState view_state,
                                      int line_to_scroll_to );

    /**
     * Adds a new content tab to the displayed tabs.
     * If precede_current_tab is false, the new tab is appended
     * and becomes the current one. Otherwise, it's added just before
     * the current one and does \e not become the current one.
     *
     * @param new_tab The tab to add.
     * @param precede_current_tab Should the new tab precede the current one.
     * @return \c true if the tab was successfully added.
     */
    bool AddNewContentTab( ContentTab *new_tab, bool precede_current_tab );

    /**
     * Returns the view state that a new tab should take when being created.
     * We want to use the view state of the current tab for the new tab.
     *
     * @return The new view state.
     */
    ContentTab::ViewState GetNewViewState();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * Stores a reference to the tab used before the current one.
     * Needed for the TabChanged signal.
     */
    QWeakPointer< ContentTab > m_LastContentTab;
};

#endif // TABMANAGER_H


