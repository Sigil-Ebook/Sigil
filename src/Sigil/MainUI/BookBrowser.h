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
#ifndef BOOKBROWSER_H
#define BOOKBROWSER_H

#include <QDockWidget>
#include <QSharedPointer>
#include "BookManipulation/Book.h"
#include "ResourceObjects/Resource.h"
#include "BookManipulation/GuideSemantics.h"

class QTreeView;
class OPFModel;
class HTMLResource;
class ImageResource;
class Resource;
class QModelIndex;
class QUrl;
class QPoint;
class QMenu;
class QAction;
class QSignalMapper;
class QToolButton;
class QVBoxLayout;


/**
 * Represents the pane with which the user can manipulate the book's files.
 * The user uses this browser to open the files in tabs. Some files can then
 * be edited, others cannot.
 */
class BookBrowser : public QDockWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The QObject's parent.
     */
    BookBrowser( QWidget *parent = 0 );

    // Context menu actions visible to main

    QAction *m_Merge;



    /**
     * Destructor.
     */
    ~BookBrowser();

public slots:

    /**
     * Sets the book whose contents the browser should display.
     *
     * @param book The book to be displayed.
     */
    void SetBook( QSharedPointer< Book > book );

    /**
     * Refreshes the display of the book.
     * This is done by clearing the current content
     * and reloading the file list from the book.
     */
    void Refresh();

    /**
     * Updates the selection in the book display
     */
    void UpdateSelection( Resource &resource );

    /**
     * Returns the previous resource in the book display
     */
    void OpenPreviousResource();

    /**
     * Returns the next resource in the book display
     */
    void OpenNextResource();


    /**
     * Opens the HTML resource referenced by the specified URL.
     *
     * @param url The URL to open.
     */
    void OpenUrlResource( const QUrl &url );

    void AddNewHTML();
    void AddNewCSS();
    /**
     * Implements the Add Existing context menu action functionality.
     */
    void AddExisting();

    /*
     * Sorts the HTML book entries alphanumerically
     */
    void SortHTML();

signals:

    /**
     * Emitted when the user activates a resource in the browser.
     *
     * @param resource The selected resource.
     */
    void ResourceActivated( Resource &resource );

    /**
     * Emitted when merging to force open tabs to close
     *
     * @param resource The resource whose tab needs to close.
     */
    void RemoveTabRequest();

    /**
     * Emitted when the browser wants a resource to be opened.
     *
     * @param resource The resource that should be opened.
     * @param precede_current_tab Should the new tab precede the currently opened one.
     * @param fragment The fragment ID to which the new tab should be scrolled.
     */
    void OpenResourceRequest( Resource &resource, bool precede_current_tab, const QUrl &fragment );

    /**
     * Emitted when the book's content is modified through the Book Browser.
     */
    void BookContentModified();

    /**
     * Emitted when the user clicks on a guide semantic type
     * in the Add Semantics sub-menu. Note that this is also emitted
     * when the user tries to un-check an added type in the menu. 
     *
     * @param html_resource The resource for which the type is being added.
     * @param type The guide semantic type.
     */
    void GuideSemanticTypeAdded( const HTMLResource &html_resource, GuideSemantics::GuideSemanticType type );

    /**
     * Emitted when the user selects an image as a cover.
     * Note that this is also emitted when the user tries 
     * to un-check an image as a cover in the menu. 
     *
     * @param image_resource The resource being set as the cover.
     */
    void CoverImageSet( const ImageResource &image_resource );

    /**
     * Wired to the current MainWindow::UpdateBrowserSelectionToTab signal.
     */
    void UpdateBrowserSelection();

private slots:

    /**
     * Emits the ResourceActivated signal.
     *
     * @param The selected model index in the Tree View.
     */
    void EmitResourceActivated( const QModelIndex &index );

    /**
     * Opens the context menu at the requested point.
     *
     * @param point The point at which the menu should be opened.
     */
    void OpenContextMenu( const QPoint &point );

    /**
     * Implements the Add New context menu action functionality.
     */
    void AddNew();

    /**
     * Implements the Rename context menu action functionality.
     */
    void Rename();

    /**
     * Implements the Rename selected context menu action functionality.
     */
    void RenameSelected();

    /**
     * Implements the Add New context menu action functionality.
     */
    void Remove();

    /**
     * Implements the Cover Image semantic context menu action functionality.
     */
    void SetCoverImage();

    /**
     * Adds the semantic type information to the currently
     * selected resource.
     *
     * @param type The integer value of the GuideSemantics::GuideSemanticType
     *             to be added to the resource.
     */
    void AddGuideSemanticType( int type );

    /**
     * Implements the Merge context menu action functionality.
     */
    void Merge();

    /**
     * Sets the use of Adobe's obfuscation method for the current resource.
     */
    void AdobesObfuscationMethod();

    /**
     * Sets the use of the IDPF's obfuscation method for the current resource.
     */
    void IdpfsObfuscationMethod();

private:

    /**
     * Asks the user to confirm editing the OFP or NCX.
     * If the provided resource is an OPF or an NCX, then the user
     * is warned that editing them is only for experts. The dialog
     * also provides an option to remember the choice in the future.
     * 
     * @param resource The resource to ask permission about. 
     * @return \true if the user wishes to continue or the provided
     *         resource is not an OPF or an NCX.
     */
    bool ShouldContinueOpeningResource( const Resource &resource );

    /**
     * Expands the Text folder so that all HTML files are shown.
     */
    void ExpandTextFolder();    

    /**
     * Reads all the stored application settings like
     * window position, geometry etc.
     */
    void ReadSettings();

    /**
     * Writes all the stored application settings like
     * window position, geometry etc.
     */
    void WriteSettings();

    /**
     * Performs the initial setup for the tree view. 
     */
    void SetupTreeView();

    /**
     * Creates all the context menu actions.
     */
    void CreateContextMenuActions();

    /**
     * Creates all the actions for the <guide>
     * element semantics.
     */
    void CreateGuideSemanticActions();

    /**
     * Tries to setup the context menu for the specified point,
     * and returns true on success.
     *
     * @param point The point at which the menu should be opened.
     * @return \c true if the menu could be set up.
     */
    bool SuccessfullySetupContextMenu( const QPoint &point );

    /**
     * Sets up the sub-menu for adding resource-specific context menu actions.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupResourceSpecificContextMenu( Resource *resource );

    /**
     * Sets up the sub-menu for adding semantic information.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupSemanticContextmenu( Resource *resource );

    /**
     * Sets up the sub-menu for adding semantic information,
     * when the menu is invoked for HTMLResources.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupHTMLSemanticContextMenu( Resource *resource );

    /**
     * Sets up the sub-menu for adding semantic information,
     * when the menu is invoked for ImageResources.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupImageSemanticContextMenu( Resource *resource ); 

    /**
     * Sets the checked state for the HTML semantic actions
     * based on the resource's current state.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetHTMLSemanticActionCheckState( Resource *resource );

    /**
     * Sets up the sub-menu for adding or removing font obfuscation,
     * when the menu is invoked for FontResources.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupFontObfuscationMenu( Resource * esource );

    /**
     * Sets the checked state for the font obfuscation actions
     * based on the resource's current state.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetFontObfuscationActionCheckState( Resource *resource );

    /**
     * Adds the MergeWithPrevious action to the context menu
     * if conditions are met.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void AddMergeAction( Resource *resource );

    /**
     * Returns the currently selected resource in the tree view.
     *
     * @return The currently selected resource in the tree view,
     *         or NULL if no resource is selected. 
     */
    Resource* GetCurrentResource();

    /**
     * Returns the resource for the given index in the tree view.
     *
     * @return the resource for the given index in the tree view,
     *         or NULL if no resource is selected. 
     */
    Resource* GetResourceByIndex( QModelIndex index );


    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();

    /**
     * List of selected resources after validating selection
     */
    QList <Resource *> ValidSelectedResources();

    /**
     * Number of valid items selected
     */
    int ValidSelectedItemCount();

    


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The book currently being displayed.
     */
    QSharedPointer< Book > m_Book;

   /**
     * A container widget for the UI widgets.
     */
    QWidget &m_MainWidget;

    /**
     * A container widget for the prev/next buttons
     * Used to work around a visual glitch on Mac OS X.
     * If we didn't use this, then we would have an ugly
     * margin on the left and right side of the m_TreeView.
     */
    QWidget &m_ButtonHolderWidget;

    /**
     * The layout for the container widget.
     */
    QVBoxLayout &m_Layout;

    /**
     * The tree view used to represent the book's files.
     */
    QTreeView &m_TreeView;

    /**
     * The buttons that initiate previous/next file
     */
    QToolButton &m_PreviousButton;
    QToolButton &m_NextButton;

    
    /**
     * The data model used to feed the tree view.
     */
    OPFModel &m_OPFModel;

    /**
     * The right-click context menu.
     */
    QMenu &m_ContextMenu;

    /**
     * The sub-menu for adding semantic
     * information to resources.
     */
    QMenu &m_SemanticsContextMenu;

    /**
     * The sub-menu for marking fonts
     * for obfuscation.
     */
    QMenu &m_FontObfuscationContextMenu;

    // The context menu actions.

    QAction *m_AddNewHTML;
    QAction *m_AddNewCSS;
    QAction *m_AddExisting;
    QAction *m_Rename;
    QAction *m_RenameSelected;
    QAction *m_Remove;
    QAction *m_MergeWithPrevious;
    QAction *m_CoverImage;
    QAction *m_AdobesObfuscationMethod;
    QAction *m_IdpfsObfuscationMethod;
    QAction *m_SortHTML;
    QAction *m_SortHTMLSelected;

    /**
     * All the semantic actions for the <guide>
     * element. Only present on HTMLResources.
     */
    QList< QAction* > m_GuideSemanticActions;

    /**
     * Used to translate all the triggered() signals from the
     * various guide-related semantic actions into calls to
     * AddGuideSemanticType().
     */
    QSignalMapper &m_GuideSemanticMapper;

    /**
     * The resource type of the last item on which the 
     * the context menu was invoked.
     */
    Resource::ResourceType m_LastContextMenuType;

    /**
     * The last folder from which the user
     * added an existing file.
     */
    QString m_LastFolderOpen;
};

#endif // BOOKBROWSER_H


