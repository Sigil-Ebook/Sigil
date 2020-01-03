/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, Ontario
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QUrl>
#include <QtWidgets/QTabWidget>

#include "MainUI/MainWindow.h"
#include "Tabs/ContentTab.h"

class Resource;
class HTMLResource;
class WellFormedContent;

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
    TabManager(QWidget *parent = 0);

    /**
     * Returns a reference to the current content tab.
     *
     * @return A reference to the current content tab.
     */
    ContentTab *GetCurrentContentTab();

    QList<ContentTab *> GetContentTabs();

    QList<Resource *> GetTabResources();

    int GetTabCount();

    void CloseAllTabs(bool all=false);
    void CloseTabForResource(const Resource *resource, bool force=false);

    /**
     * Returns \c true if all open tabs data is well-formed.
     * If an issue occurs, user is notified of the problem with a
     * dialog and the offending tab is switched to automatically.
     *
     * @return \c true if the tab data is well-formed.
     */
    bool IsAllTabDataWellFormed();

    void ReloadTabDataForResources(const QList<Resource *> &resources);

    /**
     * Close and reopen all tabs
     */
    void ReopenTabs();

    void UpdateTabDisplay();

    /**
     * Close the OPF tab if it's currently open.
     * Returns true if the OPF had to be closed.
     */
    bool CloseOPFTabIfOpen();

public slots:

    /**
     * Saves any unsaved data in the all the open tabs.
     */
    void SaveTabData();

    /**
     * Opens the specified resource in a new tab.
     * If the resource is already opened, it becomes the current one.
     *
     * @param resource - The resource that should be opened.
     * @param line_to_scroll_to - To which line should the resource scroll (CV).
     * @param position_to_scroll_to - To which position should the resource scroll (CV).
     * @param caret_location_to_scroll_to - To which stored caret location should the resource scroll (BV/PV).
     * @param fragment - The fragment ID to which the new tab should be scrolled to.
     * @param precede_current_tab - Should the new tab precede the currently opened one.
     */
    void OpenResource(Resource *resource,
                      int line_to_scroll_to = -1,
                      int position_to_scroll_to = -1,
                      const QString &caret_location_to_scroll_to = QString(),
                      const QUrl &fragment = QUrl(),
                      bool precede_current_tab = false);

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
     * If there is only one tab opened, the command is ignored.
     */
    void CloseTab();

    /**
     * Removes the current tab, may leave no tabs in windows
     */
    void RemoveTab();

    /**
     * Closes all tabs except the current tab.
     * If there is only one tab opened, the command is ignored.
     */
    void CloseOtherTabs();

    void CloseOtherTabs(int index);

    /**
     * Makes the tab the central (shown) tab of the UI.
     *
     * @param tab The tab to make central.
     */
    void MakeCentralTab(ContentTab *tab);

    void LinkClicked(const QUrl &url);

signals:
    /**
     * Emitted whenever the user switches from one tab to the next.
     *
     * @param old_tab The tab \e from which the user is switching.
     * @param new_tab The tab \e to which the user is switching.
     */
    void TabChanged(ContentTab *old_tab, ContentTab *new_tab);

    void TabCountChanged();

    /**
     * Emitted whenever one of the tabs wants to open an URL.
     *
     * @param url The URL to open.
     */
    void OpenUrlRequest(const QUrl &url);

    /**
     * Wired to the current FlowTab::OldTabRequest signal.
     */
    void OldTabRequest(QString content, HTMLResource *originating_resource);

    void ShowStatusMessageRequest(const QString &message, int duration = 5000);

protected:
    virtual void tabInserted(int index);

private slots:

    /**
     * Emits the TabChanged signal.
     */
    void EmitTabChanged(int new_index);

    /**
     * Deletes the specified tab.
     *
     * @param tab_to_delete The tab to delete.
     */
    void DeleteTab(ContentTab *tab_to_delete);

    /**
     * Closes the tab at the specified index.
     * If there is only one tab left, the command is ignored.
     *
     * @param tab_index The index of the tab to close.
     * @param force Ignore checks that would prevent a tab from closing.
     */
    void CloseTab(int tab_index, bool force=false);

    /**
     * Updates the name/header text of the specified tab.
     *
     * @param renamed_tab The renamed tab.
     */
    void UpdateTabName(ContentTab *renamed_tab);

    void SetFocusInTab();

private:

    /**
     * Returns the element of the UI that houses well-formed XML data.
     *
     * @note CAN BE NULL! This means the main tab has no XML data.
     * @param index The index of the tab to retrieve.
     * @return The element with XML data.
     */
    WellFormedContent *GetWellFormedContent(int index);

    /**
     * Returns the index of tab in which the resource is loaded.
     * If the resource is not currently loaded, -1 is returned.
     *
     * @param resource The resource whose tab index we want.
     * @return The index of the resource.
     */
    int ResourceTabIndex(const Resource *resource) const;

    /**
     * Returns true if we have succeeded in switching to the tab of the provided resource.
     *
     * @param resource The resource we want to switch to.
     * @param fragment The fragment ID to which the tab should scroll.
     * @param line_to_scroll_to To which line should the resource scroll.
     * @return \c true if we succeeded in switching.
     */
    bool SwitchedToExistingTab(const Resource *resource,
                               int line_to_scroll_to,
                               int position_to_scroll_to,
                               const QString &caret_location_to_scroll_to,
                               const QUrl &fragment);

    /**
     * Creates a tab for the specified resource.
     *
     * @param resource The resource for which we want to create a tab.
     * @param line_to_scroll_to To which line should the resource scroll.
     * @param fragment The fragment ID to which the tab should scroll after load.
     * @return The newly created tab.
     */
    ContentTab *CreateTabForResource(Resource *resource,
                                     int line_to_scroll_to,
                                     int position_to_scroll_to,
                                     const QString &caret_location_to_scroll_to,
                                     const QUrl &fragment,
                                     bool grab_focus = true);

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
    bool AddNewContentTab(ContentTab *new_tab, bool precede_current_tab);

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * Stores a reference to the tab used before the current one.
     * Needed for the TabChanged signal.
     */
    ContentTab* m_LastContentTab;

    bool m_CheckWellFormedErrors;

    QList<ContentTab*> m_TabsToDelete;
    bool m_tabs_deletion_in_use;
};

#endif // TABMANAGER_H


