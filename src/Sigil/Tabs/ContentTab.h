/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#ifndef CONTENTTAB_H
#define CONTENTTAB_H

#include <QWidget>
#include <QString>
#include "ViewEditors/Zoomable.h"

class QLayout;
class Searchable;
class Resource;


/**
 * A generic tab widget for editing/viewing a resource.
 */
class ContentTab : public QWidget, public Zoomable
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * 
     * @param resource The resource this tab will be displaying.
     * @param parent The parent of this QObject.
     */
    ContentTab( Resource& resource, QWidget *parent = 0 );

    /**
     * Destructor.
     */
    virtual ~ContentTab() {}

    /**
     * Returns the filename of the displayed resource.
     *
     * @return The filename of the displayed resource.
     */
    QString GetFilename();

    /**
     * Returns the icon appropriate for the displayed resource.
     *
     * @return The icon appropriate for the displayed resource.
     */
    QIcon GetIcon();

    /**
     * The resource currently loaded in the tab.
     *
     * @return A reference to the resource.
     */
    Resource& GetLoadedResource();

    /**
     * The modification state of the resource.
     *
     * @return \c true if the tab has modified the resource.
     */
    virtual bool IsModified()           { return false; }

    /**
     * Can the user perform the Cut clipboard operation.
     *
     * @return \c true if the user can Cut.
     */
    virtual bool CutEnabled()           { return false; }

    /**
     * Can the user perform the Copy clipboard operation.
     *
     * @return \c true if the user can Copy.
     */
    virtual bool CopyEnabled()          { return false; }

    /**
     * Can the user perform the Paste clipboard operation.
     *
     * @return \c true if the user can Paste.
     */
    virtual bool PasteEnabled()         { return false; }

    /**
     * Checked state of the Bold action.
     *
     * @return \c true if the Bold action should be checked.
     */
    virtual bool BoldChecked()          { return false; }

    /**
     * Checked state of the Italic action.
     *
     * @return \c true if the Italic action should be checked.
     */
    virtual bool ItalicChecked()        { return false; }
    
    /**
     * Checked state of the Underline action.
     *
     * @return \c true if the Underline action should be checked.
     */
    virtual bool UnderlineChecked()     { return false; }
    
    /**
     * Checked state of the Strikethrough action.
     *
     * @return \c true if the Strikethrough action should be checked.
     */
    virtual bool StrikethroughChecked() { return false; }
    
    /**
     * Checked state of the BulletList action.
     *
     * @return \c true if the BulletList action should be checked.
     */
    virtual bool BulletListChecked()    { return false; }
   
    /**
     * Checked state of the NumberList action.
     *
     * @return \c true if the NumberList action should be checked.
     */
    virtual bool NumberListChecked()    { return false; }

    /**
     * Checked state of the BookView action.
     *
     * @return \c true if the BookView action should be checked.
     */
    virtual bool BookViewChecked()      { return false; }
    
    /**
     * Checked state of the SplitView action.
     *
     * @return \c true if the SplitView action should be checked.
     */
    virtual bool SplitViewChecked()     { return false; }
    
    /**
     * Checked state of the CodeView action.
     *
     * @return \c true if the CodeView action should be checked.
     */
    virtual bool CodeViewChecked()      { return false; }

    /**
     * Returns the name of the element the caret is located in.
     *
     * @return The name of the element the caret is located in.
     */
    virtual QString GetCaretElementName() { return "";  }

    virtual float GetZoomFactor() const { return 1.0;   }
    virtual void SetZoomFactor( float new_zoom_factor ) { }

    /**
     * Returns a pointer to searchable content in the tab.
     * Returns pointer instead of reference because we need
     * to return NULL for tabs with no searchable content.
     *
     * @return The searchable content.
     */
    virtual Searchable* GetSearchableContent();

    /**
     * Describes the type of the View mode
     * currently used in the tab.
     */
    enum ViewState
    {
        ViewState_BookView,   /**< The WYSIWYG view. */
        ViewState_CodeView,   /**< The XHTML code editing view. */
        ViewState_RawView,    /**< The view for editing non-XHTML related resources. */
        ViewState_StaticView, /**< The static view for non-editable content. */
        ViewState_AnyView,    /**< Specifies that no state is preferred or set. */
    };

    /**
     * Returns the current view state.
     *
     * @return The current view state.
     */
    virtual ViewState GetViewState() { return ViewState_StaticView; };

    virtual void SetViewState( ViewState new_view_state ) {};

    virtual bool IsLoadingFinished() { return true; } 

public slots:

    /**
     * Saves the tab data, then schedules tab for deletion.
     */
    void Close();

    /**
     * Saves the changed content when the user leaves the tab.
     */
    virtual void SaveTabContent();

    /**
     * Loads the resource content when the user enters the tab.
     */
    virtual void LoadTabContent();

signals:

    /**
     * Emitted when the tab wants to be deleted.
     *
     * @param tab_to_delete A pointer to this tab.
     */
    void DeleteMe( ContentTab *tab_to_delete );

    /**
     * Emitted when the zoom factor changes.
     *
     * @param factor The new zoom factor.
     */
    void ZoomFactorChanged( float factor );

    /**
     * Emitted when the content of the tab changes.
     */
    void ContentChanged();

    /**
     * Emitted when tab header text has been changed.
     *
     * @param renamed_tab Pointer to this tab.
     */
    void TabRenamed( ContentTab *renamed_tab );

protected slots:

    /**
     * Emits the DeleteMe signal.
     */
    void EmitDeleteMe();

    /**
     * Emits the TabRenamed signal.
     */
    void EmitTabRenamed();

protected:

    /**
     * A custom focusIn handler.
     * By default, calls the LoadTabContent function.
     *
     * @warning Will \b not be called for child tabs
     *          that set a focus proxy (like FlowTab, TextTab etc.)
     */
    virtual void focusInEvent( QFocusEvent *event );

    /**
     * A custom focusOut handler.
     * By default, calls the SaveTabContent function.
     *
     * @warning Will \b not be called for child tabs
     *          that set a focus proxy (like FlowTab, TextTab etc.)
     */
    virtual void focusOutEvent( QFocusEvent *event );


    ///////////////////////////////
    // PROTECTED MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The resource being displayed.
     */
    Resource &m_Resource;

    /**
     * The main layout of the widget.
     */
    QLayout &m_Layout;
};

#endif // CONTENTTAB_H


