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
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/ImageResource.h"
#include "ResourceObjects/MiscTextResource.h"
#include "ResourceObjects/SVGResource.h"
#include "Tabs/CSSTab.h"
#include "Tabs/FlowTab.h"
#include "Tabs/ImageTab.h"
#include "Tabs/MiscTextTab.h"
#include "Tabs/SVGTab.h"
#include "Tabs/NCXTab.h"
#include "Tabs/OPFTab.h"
#include "Tabs/TabManager.h"
#include "Tabs/WellFormedContent.h"
#include "Tabs/TabBar.h"

TabManager::TabManager( QWidget *parent )
    :
    QTabWidget( parent )
{
    QTabBar *tab_bar = new TabBar(this);
    setTabBar(tab_bar);

    connect( tab_bar, SIGNAL( TabBarDoubleClicked() ),      this, SIGNAL( ToggleViewStateRequest() ) );
    connect( tab_bar, SIGNAL( TabBarClicked() ),            this, SLOT( SetFocusInTab() ) );
    connect( tab_bar, SIGNAL( CloseOtherTabsRequest(int) ), this, SLOT( CloseOtherTabs(int) ) );

    connect( this, SIGNAL( currentChanged( int ) ),         this, SLOT( EmitTabChanged() ) );
    connect( this, SIGNAL( currentChanged( int ) ),         this, SLOT( UpdateTab( int ) ) );
    connect( this, SIGNAL( tabCloseRequested( int ) ),      this, SLOT( CloseTab( int ) ) );

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

QList<ContentTab*> TabManager::GetContentTabs()
{
    QList <ContentTab*> tabs;

    for (int i = 0; i < count(); ++i) {
        ContentTab *tab = qobject_cast<ContentTab*>(widget(i));
        tabs.append(tab);
    }

    return tabs;
}

QList<Resource*> TabManager::GetTabResources()
{
    QList <ContentTab*> tabs = GetContentTabs();
    QList <Resource*> tab_resources;

    foreach (ContentTab *tab, tabs) {
        tab_resources.append(&tab->GetLoadedResource());
    }

    return tab_resources;
}

void TabManager::tabInserted(int index)
{
    emit TabCountChanged();
}

int TabManager::GetTabCount()
{
    return count();
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

    WellFormedContent *content = GetWellFormedContent(tab_index);
    if (content && !content->IsDataWellFormed()) {
        return false;
    }

    ContentTab *tab = qobject_cast<ContentTab *>(widget(tab_index));
    tab->Close();

    emit TabCountChanged();

    return true;
}

bool TabManager::CloseTabForResource(const Resource &resource)
{
    int index = ResourceTabIndex(resource);

    if (index != -1) {
        return TryCloseTab(index);
    }

    // Tab for resource so it's not open.
    return true;
}

bool TabManager::IsAllTabDataWellFormed()
{
    QList<Resource*> resources = GetTabResources();

    foreach(Resource *resource, resources) {
        int index = ResourceTabIndex(*resource);
        WellFormedContent *content = dynamic_cast<WellFormedContent *>(widget(index));
        if (content) {
            if (!content->IsDataWellFormed()) {
                return false;
            }
        }
    }
    return true;
}

void TabManager::ReloadTabDataForResources( const QList<Resource*> &resources )
{
    foreach (Resource *resource, resources) {
        int index = ResourceTabIndex(*resource);
        if ( index != -1 ) {
            FlowTab *flow_tab = qobject_cast<FlowTab *>(widget(index));
            if (flow_tab) {
                flow_tab->LoadTabContent();
            }
        }
    }
}

void TabManager::ReopenTabs()
{
    ContentTab &currentTab = GetCurrentContentTab();

    QList<Resource*> resources = GetTabResources();
    foreach(Resource *resource, resources) {
        CloseTabForResource(*resource);
    }

    foreach(Resource *resource, resources) {
        OpenResource(*resource);
    }

    OpenResource(currentTab.GetLoadedResource());
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

void TabManager::LinkClicked(const QUrl &url)
{
    if (url.toString().isEmpty()) {
        return;
    }

    ContentTab &tab = GetCurrentContentTab();

    QString url_string = url.toString();

    // Convert fragments to full filename/fragments
    if (url_string.startsWith("#")) {
        url_string.prepend(tab.GetFilename());
    }
    else if (url.scheme() == "file") {
        if (url_string.contains("/#")) {
            url_string.insert(url_string.indexOf( "/#") + 1, tab.GetFilename());
        }
    }

    emit OpenUrlRequest(QUrl(url_string));
}

void TabManager::OpenResource( Resource& resource,
                               bool precede_current_tab,
                               const QUrl &fragment,
                               MainWindow::ViewState view_state,
                               int line_to_scroll_to,
                               int position_to_scroll_to,
                               QString caret_location_to_scroll_to,
                               bool grab_focus )
{
    if ( SwitchedToExistingTab( resource, fragment, line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to ) ) {
        return;
    }
    ContentTab *new_tab = CreateTabForResource( resource, fragment, view_state, 
                                                line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to, grab_focus);

    if (new_tab) {
        AddNewContentTab( new_tab, precede_current_tab);
        emit ShowStatusMessageRequest("");
    }
    else {
        QString message = tr("Cannot edit file") + ": " + resource.Filename();
        emit ShowStatusMessageRequest(message);
    }
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
    CloseOtherTabs(currentIndex());
}

void TabManager::CloseOtherTabs(int index)
{
    if (count() <= 1 || index < 0 || index >= count()) {
        return;
    }

    int max_index = count() - 1;

    // Close all tabs after the tab
    for (int i = index + 1; i <= max_index; i++) {
        CloseTab(index + 1);
    }

    // Close all tabs before the tab
    for (int i = 0; i < index; i++) {
        CloseTab(0);
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

    if ( m_LastContentTab.data() != current_tab ) {
        emit TabChanged( m_LastContentTab.data(), current_tab );
        m_LastContentTab = QWeakPointer< ContentTab >( current_tab );
    }
}

void TabManager::UpdateTab(int index)
{
    if (index == -1) {
        return;
    }

    ContentTab *content_tab = qobject_cast<ContentTab *>(widget(index));
    content_tab->UpdateDisplay();
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

void TabManager::SetFocusInTab()
{
    ContentTab &tab = GetCurrentContentTab();
    if (&tab != NULL) {
        tab.setFocus();
    }
}


WellFormedContent* TabManager::GetWellFormedContent(int index)
{
    return dynamic_cast< WellFormedContent* >( widget(index) );
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
                                        int line_to_scroll_to,
                                        int position_to_scroll_to,
                                        QString caret_location_to_scroll_to )
{
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
            if ( !caret_location_to_scroll_to.isEmpty() )
            {
                flow_tab->ScrollToCaretLocation( caret_location_to_scroll_to );
            }
            else if ( position_to_scroll_to > 0 )
            {
                flow_tab->ScrollToPosition( position_to_scroll_to );
            }
            else if ( fragment.toString() != "" )
            {
                flow_tab->ScrollToFragment( fragment.toString() );
            }
            else if ( line_to_scroll_to > 0 )
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
                                              int line_to_scroll_to,
                                              int position_to_scroll_to,
                                              QString caret_location_to_scroll_to,
                                              bool grab_focus )
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
                position_to_scroll_to,
                caret_location_to_scroll_to,
                grab_focus,
                this );

            connect( tab,  SIGNAL( LinkClicked( const QUrl& ) ), this, SLOT( LinkClicked( const QUrl& ) ) );
            connect( tab,  SIGNAL( OldTabRequest( QString, HTMLResource& ) ),
                this, SIGNAL( OldTabRequest( QString, HTMLResource& ) ) );
            break;
        }

    case Resource::CSSResourceType:
        {
            tab = new CSSTab( *( qobject_cast< CSSResource* >( &resource ) ), line_to_scroll_to, this );
            break;
        }

    case Resource::ImageResourceType:
        {
            tab = new ImageTab( *( qobject_cast< ImageResource* >( &resource ) ), this );
            break;
        }

    case Resource::MiscTextResourceType:
        {
            tab = new MiscTextTab( *( qobject_cast< MiscTextResource* >( &resource ) ), line_to_scroll_to, this );
            break;
        }


    case Resource::SVGResourceType:
        {
            tab = new SVGTab( *( qobject_cast< SVGResource* >( &resource ) ), line_to_scroll_to, this );
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
            tab = new TextTab( *( qobject_cast< TextResource* >( &resource ) ), CodeViewEditor::Highlight_NONE, line_to_scroll_to, this );
            break;
        }
    default:
        break;
    }

    // Set whether to inform or auto correct well-formed errors.
    WellFormedContent *wtab = dynamic_cast< WellFormedContent* >( tab );
    if ( wtab )
    {
        // In case of well-formed errors we want the tab to be focused.
        connect( tab,  SIGNAL( CentralTabRequest( ContentTab* ) ),
                this, SLOT( MakeCentralTab( ContentTab* ) ) );//, Qt::QueuedConnection );
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

void TabManager::UpdateTabDisplay()
{
    for (int i = 0; i < count(); ++i) {
        ContentTab *tab = dynamic_cast<ContentTab *>(widget(i));

        if (tab) {
            tab->UpdateDisplay();
        }
    }
}
