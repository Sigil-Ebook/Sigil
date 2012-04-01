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

#include "ResourceObjects/Resource.h"
#include "ResourceObjects/CSSResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/XPGTResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/ImageResource.h"
#include "Tabs/CSSTab.h"
#include "Tabs/FlowTab.h"
#include "Tabs/ImageTab.h"
#include "Tabs/NCXTab.h"
#include "Tabs/OPFTab.h"
#include "Tabs/TabManager.h"
#include "Tabs/WellFormedContent.h"
#include "Tabs/XPGTTab.h"

TabManager::TabManager( QWidget *parent )
    : 
    QTabWidget( parent )
{
    connect( this, SIGNAL( currentChanged( int ) ),    this, SLOT( EmitTabChanged() ) );
    connect( this, SIGNAL( tabCloseRequested( int ) ), this, SLOT( CloseTab( int ) ) );

    connect( this, SIGNAL( TabChanged(              ContentTab*, ContentTab* ) ),
             this, SLOT(   UpdateTabStatesOnSwitch( ContentTab*, ContentTab* ) ) );

    setDocumentMode( true );
    setMovable( true );
    setTabsClosable( true );
    setUsesScrollButtons( true );
}


ContentTab& TabManager::GetCurrentContentTab()
{
    QWidget* widget = currentWidget();

    // TODO: turn on this assert after you make sure a tab
    // is created before this is called in MainWindow constructor
    //Q_ASSERT( widget != NULL );

    return *qobject_cast< ContentTab* >( widget );
}


bool TabManager::TryCloseAllTabs()
{
    bool status;
    while (count() > 0) {
        status = TryCloseTab(0);
        if (!status) {
            return false;
        }
    }

    return true;
}


bool TabManager::TryCloseTab(int tab_index)
{
    Q_ASSERT(tab_index >= 0);

    WellFormedContent *content = GetWellFormedContent();
    if (content && !content->IsDataWellFormed()) {
        return false;
    }

    ContentTab *tab = qobject_cast<ContentTab *>(widget(tab_index));
    tab->Close();

    return true;
}

bool TabManager::CloseTabForResource(const Resource &resouce)
{
    int index = ResourceTabIndex(resouce);

    if (index != -1) {
        return TryCloseTab(index);
    }

    // Tab for resource so it's not open.
    return true;
}


bool TabManager::TabDataIsWellFormed()
{
    WellFormedContent *content = GetWellFormedContent();

    if ( content )
        
        return content->IsDataWellFormed(); 

    return true;
}

bool TabManager::TabDataIsWellFormed(const Resource &resouce)
{
    int index = ResourceTabIndex(resouce);
    if (index != -1) {
        WellFormedContent *content = dynamic_cast<WellFormedContent *>(widget(index));
        if (content) {
            return content->IsDataWellFormed();
        }
    }

    return true;
}

void TabManager::ReloadTabData()
{
    for (int i = count() - 1; i >= 0; --i) {
        FlowTab *flow_tab = qobject_cast<FlowTab *>(widget(i));
        if (flow_tab) {
            flow_tab->LoadTabContent();
        }
    }
}

void TabManager::WellFormedDialogsEnabled( bool enabled )
{
    WellFormedContent *content = GetWellFormedContent();

    if ( content )
        
        content->SetWellFormedDialogsEnabledState( enabled );  
}


void TabManager::SetCheckWellFormedErrors( bool enabled )
{
    m_CheckWellFormedErrors = enabled;

    for ( int i = 0; i < count(); ++i )
    {
        WellFormedContent *tab = dynamic_cast< WellFormedContent* >( widget( i ) );

        if ( tab )
        {
            tab->SetCheckWellFormedErrorsState( enabled );
        }
    }
}


void TabManager::SaveTabData()
{
    for ( int i = 0; i < count(); ++i )
    {
        ContentTab *tab = qobject_cast< ContentTab* >( widget( i ) );
        
        if ( tab )

            tab->SaveTabContent();
    }
}

void TabManager::OpenResource( Resource& resource, 
                               bool precede_current_tab,
                               const QUrl &fragment,
                               MainWindow::ViewState view_state,
                               int line_to_scroll_to )
{
    if ( SwitchedToExistingTab( resource, fragment, view_state, line_to_scroll_to ) )

        return;

    AddNewContentTab( CreateTabForResource( resource, fragment, view_state, line_to_scroll_to ),
                      precede_current_tab );

    // TODO: loading bar update    
}


void TabManager::NextTab()
{
    int current_index = currentIndex();

    // No tabs present
    if ( current_index == -1 )

        return;

    // Wrap around
    int next_index = current_index != count() - 1 ? current_index + 1 : 0;

    if ( widget( next_index ) != 0 && 
         current_index != next_index )
    {
        setCurrentIndex( next_index );
    }
}


void TabManager::PreviousTab()
{
    int current_index = currentIndex();

    // No tabs present
    if ( current_index == -1 )

        return;

    // Wrap around
    int previous_index = current_index != 0 ? current_index - 1 : count() - 1;

    if ( widget( previous_index ) != 0 &&
         current_index != previous_index )
    {
        setCurrentIndex( previous_index );
    }
}


void TabManager::RemoveTab()
{
    // Can leave window with no tabs, so re-open a tab asap
    removeTab( currentIndex() );
}


void TabManager::CloseTab()
{
    CloseTab( currentIndex() );
}


void TabManager::CloseOtherTabs()
{
    if ( count() <= 1 )
    {
        return;
    }

    int current_index = currentIndex();
    int max_index = count() - 1;

    // Close all tabs after the current one.
    for ( int i = current_index + 1; i <= max_index; i++ )
    {
        CloseTab( current_index + 1 );
    }

    // Close all tabs before the current one.
    for ( int i = 0; i < current_index; i++ )
    {
        CloseTab( 0 );
    }
}


void TabManager::MakeCentralTab( ContentTab *tab )
{
    Q_ASSERT( tab );
    setCurrentIndex( indexOf( tab ) );
}


void TabManager::EmitTabChanged()
{
    ContentTab *current_tab = qobject_cast< ContentTab* >( currentWidget() );

    emit TabChanged( m_LastContentTab.data(), current_tab );

    m_LastContentTab = QWeakPointer< ContentTab >( current_tab );
}


void TabManager::UpdateTabStatesOnSwitch( ContentTab* old_tab, ContentTab* new_tab )
{
    // These Save and Load operations are probably unneeded as they will be handled by the focus change.
    // But the modification checks ensure that they don't entail much of a performance penalty, so they
    // are worth retaining as a safety mechanism.
    if (old_tab) {
        old_tab->SaveTabContent();
    }

    if (new_tab) {
        new_tab->LoadTabContent();
        UpdateTabDisplay(new_tab);
    }
}


void TabManager::DeleteTab( ContentTab *tab_to_delete )
{
    Q_ASSERT( tab_to_delete );

    removeTab( indexOf( tab_to_delete ) );

    tab_to_delete->deleteLater();
}


void TabManager::CloseTab(int tab_index)
{   
    if (count() <= 1) {
        return;
    }

    TryCloseTab(tab_index);
}


void TabManager::UpdateTabName( ContentTab *renamed_tab )
{
    Q_ASSERT( renamed_tab );

    setTabText( indexOf( renamed_tab ), renamed_tab->GetFilename() );
}


WellFormedContent* TabManager::GetWellFormedContent()
{
    return dynamic_cast< WellFormedContent* >( currentWidget() );
}



// Returns the index of the tab the index is loaded in, -1 if it isn't
int TabManager::ResourceTabIndex( const Resource& resource ) const
{
    QString identifier( resource.GetIdentifier() );

    int index = -1;

    for ( int i = 0; i < count(); ++i )
    {
        ContentTab *tab = qobject_cast< ContentTab* >( widget( i ) );

        if ( tab && tab->GetLoadedResource().GetIdentifier() == identifier )
        {
            index = i;
            break;
        }
    }

    return index;
}


bool TabManager::SwitchedToExistingTab( Resource& resource, 
                                        const QUrl &fragment, 
                                        MainWindow::ViewState view_state,
                                        int line_to_scroll_to )
{
    Q_UNUSED(view_state)

    int resource_index = ResourceTabIndex( resource );

    // If the resource is already opened in
    // some tab, then we just switch to it
    if ( resource_index != -1 )
    {
        setCurrentIndex( resource_index );
        QWidget *tab = widget( resource_index );
        Q_ASSERT( tab );
        tab->setFocus();

        FlowTab *flow_tab = qobject_cast< FlowTab* >( tab );

        if ( flow_tab != NULL )
        {
            // Restore cursor position if there is one
            flow_tab->RestoreCaret();

            if ( fragment.toString() != "" )
            {
                flow_tab->ScrollToFragment( fragment.toString() );
            }
            if ( line_to_scroll_to > 0 )
            {
                flow_tab->ScrollToLine( line_to_scroll_to );
            }

            return true;
        }

        TextTab *text_tab = qobject_cast< TextTab* >( tab );

        if ( text_tab != NULL )
        {
            text_tab->ScrollToLine( line_to_scroll_to );
            return true;
        }

        ImageTab *image_tab = qobject_cast< ImageTab* >( tab );

        if ( image_tab != NULL )
        {
            return true;
        }
    }

    return false;
}


ContentTab* TabManager::CreateTabForResource( Resource& resource, 
                                              const QUrl &fragment, 
                                              MainWindow::ViewState view_state,
                                              int line_to_scroll_to )
{
    ContentTab *tab = NULL;

    switch( resource.Type() )
    {
    case Resource::HTMLResourceType:
        {
            tab = new FlowTab( *( qobject_cast< HTMLResource* >( &resource ) ), 
                fragment, 
                view_state, 
                line_to_scroll_to, 
                this );

            connect( tab,  SIGNAL( LinkClicked( const QUrl& ) ), this, SIGNAL( OpenUrlRequest( const QUrl& ) ) );
            connect( tab,  SIGNAL( OldTabRequest( QString, HTMLResource& ) ), 
                this, SIGNAL( OldTabRequest( QString, HTMLResource& ) ) );
            connect( tab,  SIGNAL( NewChaptersRequest( QStringList, HTMLResource& ) ),
                this, SIGNAL( NewChaptersRequest( QStringList, HTMLResource& ) ) );
            break;
        }

    case Resource::CSSResourceType:
        {
            tab = new CSSTab( *( qobject_cast< CSSResource* >( &resource ) ), line_to_scroll_to, this );
            break;
        }

    case Resource::XPGTResourceType:
        {
            tab = new XPGTTab( *( qobject_cast< XPGTResource* >( &resource ) ), line_to_scroll_to, this );
            break;
        }

    case Resource::ImageResourceType:
        {
            tab = new ImageTab( *( qobject_cast< ImageResource* >( &resource ) ), this );
            break;
        }

    case Resource::OPFResourceType:
        {
            tab = new OPFTab( *( qobject_cast< OPFResource* >( &resource ) ), line_to_scroll_to, this );
            break;
        }

    case Resource::NCXResourceType:
        {
            tab = new NCXTab( *( qobject_cast< NCXResource* >( &resource ) ), line_to_scroll_to, this );
            break;
        }

    case Resource::XMLResourceType:
        {
            tab = new XMLTab( *( qobject_cast< XMLResource* >( &resource ) ), line_to_scroll_to, this );
            break;
        }

    case Resource::TextResourceType:
        {
            tab = new TextTab( *( qobject_cast< TextResource* >( &resource ) ), CodeViewEditor::Highlight_XHTML, line_to_scroll_to, this );
            break;
        }
    default:
        break;
    }

    // In case of well-formed errors we want the tab to be focused.
    connect( tab,  SIGNAL( CentralTabRequest( ContentTab* ) ),
             this, SLOT( MakeCentralTab( ContentTab* ) ) );//, Qt::QueuedConnection );

    // Wet whether to inform or auto correct well-formed errors.
    WellFormedContent *wtab = dynamic_cast< WellFormedContent* >( tab );
    if ( wtab )
    {
        wtab->SetCheckWellFormedErrorsState( m_CheckWellFormedErrors );
    }

    return tab;    
}


bool TabManager::AddNewContentTab( ContentTab *new_tab, bool precede_current_tab )
{
    if ( new_tab == NULL )

        return false;

    if ( !precede_current_tab )
    {
        addTab( new_tab, new_tab->GetIcon(), new_tab->GetFilename() );
        setCurrentWidget( new_tab );
        new_tab->setFocus();
    }

    else
    {
        insertTab( currentIndex(), new_tab, new_tab->GetIcon(), new_tab->GetFilename() );
    }

    connect( new_tab, SIGNAL( DeleteMe(   ContentTab* ) ), this, SLOT( DeleteTab(     ContentTab* ) ) );
    connect( new_tab, SIGNAL( TabRenamed( ContentTab* ) ), this, SLOT( UpdateTabName( ContentTab* ) ) );

    return true;
}   


void TabManager::UpdateTabDisplay( ContentTab *tab )
{
    tab->UpdateDisplay();
}
