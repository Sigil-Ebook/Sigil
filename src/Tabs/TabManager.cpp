/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, ON Canada
**  Copyright (C) 2009-2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QDebug>
#include "BookManipulation/CleanSource.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/CSSResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/ImageResource.h"
#include "ResourceObjects/MiscTextResource.h"
#include "ResourceObjects/SVGResource.h"
#include "Tabs/AVTab.h"
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

TabManager::TabManager(QWidget *parent)
    :
    QTabWidget(parent),
    m_LastContentTab(NULL)
{
    QTabBar *tab_bar = new TabBar(this);
    setTabBar(tab_bar);
    connect(tab_bar, SIGNAL(TabBarClicked()),            this, SLOT(SetFocusInTab()));
    connect(tab_bar, SIGNAL(CloseOtherTabsRequest(int)), this, SLOT(CloseOtherTabs(int)));
    connect(this, SIGNAL(currentChanged(int)),         this, SLOT(EmitTabChanged(int)));
    connect(this, SIGNAL(tabCloseRequested(int)),      this, SLOT(CloseTab(int)));
    setDocumentMode(true);

    // re-enable tab drag and drop as a test to see if last commit helped
#if 0    
    // QTabBar has a bug when a user "presses and flicks" on a non current tab it will cause it 
    // to setCurrentIndex() but during EmitTabChanged(int) it then may allows the resulting mouseMoveEvent
    // be processed on the same index that is being set in setCurrentIndex which causes a crash.
    // This bug makes it dangerous to enable dragging and dropping to move tabs in the QTabBar
    // So default to non-movable but let a environment variable override this decision
    if (qEnvironmentVariableIsSet("SIGIL_ALLOW_TAB_MOVEMENT")) {
        setMovable(true);
    } else {
        setMovable(false);
    }
#else
    setMovable(true);
#endif

    setTabsClosable(true);
    // setElideMode(Qt::ElideRight); this is the default after qt-5.6
    setUsesScrollButtons(true);
}


ContentTab *TabManager::GetCurrentContentTab()
{
    QWidget *widget = currentWidget();
    // TODO: turn on this assert after you make sure a tab
    // is created before this is called in MainWindow constructor
    //Q_ASSERT( widget != NULL );
    return qobject_cast<ContentTab *>(widget);
}

QList<ContentTab *> TabManager::GetContentTabs()
{
    QList <ContentTab *> tabs;

    for (int i = 0; i < count(); ++i) {
        ContentTab *tab = qobject_cast<ContentTab *>(widget(i));
        tabs.append(tab);
    }

    return tabs;
}

QList<Resource *> TabManager::GetTabResources()
{
    QList <ContentTab *> tabs = GetContentTabs();
    QList <Resource *> tab_resources;
    foreach(ContentTab *tab, tabs) {
        tab_resources.append(tab->GetLoadedResource());
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

void TabManager::CloseAllTabs(bool all)
{
    while (count() > 0) {
        CloseTab(0, all);
    }
}

void TabManager::CloseTabForResource(const Resource *resource)
{
    int index = ResourceTabIndex(resource);

    if (index != -1) {
        CloseTab(index);
    }
}

bool TabManager::IsAllTabDataWellFormed()
{
    QList<Resource *> resources = GetTabResources();
    foreach(Resource *resource, resources) {
        int index = ResourceTabIndex(resource);
        WellFormedContent *content = dynamic_cast<WellFormedContent *>(widget(index));

        // Only check Xhtml for now.
        if (content && resource->Type() == Resource::HTMLResourceType) {
            if (!content->IsDataWellFormed()) {
                return false;
            }
        }
    }
    return true;
}

void TabManager::ReloadTabDataForResources(const QList<Resource *> &resources)
{
    foreach(Resource *resource, resources) {
        int index = ResourceTabIndex(resource);

        if (index != -1) {
            FlowTab *flow_tab = qobject_cast<FlowTab *>(widget(index));

            if (flow_tab) {
                flow_tab->LoadTabContent();
            }
        }
    }
}

void TabManager::ReopenTabs()
{
    ContentTab *currentTab = GetCurrentContentTab();
    QList<Resource *> resources = GetTabResources();
    foreach(Resource *resource, resources) {
        CloseTabForResource(resource);
    }
    foreach(Resource *resource, resources) {
        OpenResource(resource, -1, -1, QString());
    }
    OpenResource(currentTab->GetLoadedResource(), -1, -1, QString());
}


void TabManager::SaveTabData()
{
    for (int i = 0; i < count(); ++i) {
        ContentTab *tab = qobject_cast<ContentTab *>(widget(i));

        if (tab) {
            tab->SaveTabContent();
        }
    }
}

void TabManager::LinkClicked(const QUrl &url)
{
    QString url_string = url.toString();

    if (url_string.isEmpty()) {
        return;
    }
    
    ContentTab *tab = GetCurrentContentTab();

    if (url_string.indexOf(':') == -1) {

        // we have a relative url, so build an internal
        // book: scheme url book:///bookpath#fragment
        QString attpath = Utility::URLDecodePath(url_string);
	QString dest_bookpath;
	int fragpos = attpath.lastIndexOf('#');
	bool has_fragment = fragpos != -1;
	QString fragment = "";
	if (has_fragment) {
	    fragment = url_string.mid(fragpos+1, -1);
	    attpath = attpath.mid(0, fragpos);
	}
	if (attpath.isEmpty()) {
	    dest_bookpath = tab->GetLoadedResource()->GetRelativePath();
	} else {
	    QString startdir = tab->GetLoadedResource()->GetFolder();
	    dest_bookpath = Utility::buildBookPath(attpath, startdir);
	}
	if (!fragment.isEmpty()) {
	    dest_bookpath = dest_bookpath + "#" + fragment;
	}
	url_string = "book:///" + dest_bookpath;
	// QUrl will take care of encoding the url path
    } else {
        // we have a scheme and are absolute
        if (url.scheme() == "file") {
            if (url_string.contains("/#")) {
                url_string.insert(url_string.indexOf("/#") + 1, tab->GetFilename());
            }
	}
    }

    emit OpenUrlRequest(QUrl(url_string));
}

void TabManager::OpenResource(Resource *resource,
                              int line_to_scroll_to,
                              int position_to_scroll_to,
                              const QString &caret_location_to_scroll_to,
                              const QUrl &fragment,
                              bool precede_current_tab)
{
    if (SwitchedToExistingTab(resource, line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to, fragment)) {
        return;
    }

    bool grab_focus = !precede_current_tab;
    ContentTab *new_tab = CreateTabForResource(resource, line_to_scroll_to, position_to_scroll_to,
                          caret_location_to_scroll_to, fragment, grab_focus);

    if (new_tab) {
        AddNewContentTab(new_tab, precede_current_tab);
        emit ShowStatusMessageRequest("");
    } else {
        QString message = tr("Cannot edit file") + ": " + resource->ShortPathName();
        emit ShowStatusMessageRequest(message);
    }

    // do not Scroll the Preview in response as new Flow Tabs have
    // delayed initialization.  Instead FlowTab itself will handle this
}


void TabManager::NextTab()
{
    int current_index = currentIndex();

    // No tabs present
    if (current_index == -1) {
        return;
    }

    // Wrap around
    int next_index = current_index != count() - 1 ? current_index + 1 : 0;

    if (widget(next_index) != 0 &&
        current_index != next_index) {
        setCurrentIndex(next_index);
    }
}


void TabManager::PreviousTab()
{
    int current_index = currentIndex();

    // No tabs present
    if (current_index == -1) {
        return;
    }

    // Wrap around
    int previous_index = current_index != 0 ? current_index - 1 : count() - 1;

    if (widget(previous_index) != 0 &&
        current_index != previous_index) {
        setCurrentIndex(previous_index);
    }
}


void TabManager::RemoveTab()
{
    // Can leave window with no tabs, so re-open a tab asap
    removeTab(currentIndex());
}


void TabManager::CloseTab()
{
    CloseTab(currentIndex());
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


void TabManager::MakeCentralTab(ContentTab *tab)
{
    Q_ASSERT(tab);
    setCurrentIndex(indexOf(tab));
}

// Note: m_LastContentTab was previously declared as follows:
//
//     QPointer<ContentTab> m_LastContentTab;
//
// instead of just a simple:
//
//     ContentTab * m_LastContentTab;
//
// but this caused many issues with fast deleting and updating of 
// this special pointer.
//
// This pointer only ever records the last curent ContentTab and it is 
// never shared outside the TabManager and it remains valid until the TabManager
// itself is destroyed.  So there was no rhyme or reason for using or needing the 
// QPointer type here especially with its associated problems with fast/recursive updates. 

void TabManager::EmitTabChanged(int new_index)
{
    ContentTab *current_tab = qobject_cast<ContentTab *>(currentWidget());
    if (current_tab) {
        if (m_LastContentTab != current_tab) {
            emit TabChanged(m_LastContentTab, current_tab);
            m_LastContentTab = current_tab;
        }
    }
}


void TabManager::DeleteTab(ContentTab *tab_to_delete)
{
    Q_ASSERT(tab_to_delete);
    // to prevent segfaults, disconnect and reconnect the currentChanged()
    // signal and invoke EmitTabChanged() manually after QTabBar::removeTab(int) 
    // completes because QTabBar::setCurrentIndex(int) somehow invokes processEvents()
    // ***BEFORE*** properly setting the current index
    disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(EmitTabChanged(int)));
    removeTab(indexOf(tab_to_delete));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(EmitTabChanged(int)));
    // now do the short version of EmitTabChanged()
    ContentTab *new_tab = qobject_cast<ContentTab *>(currentWidget());
    if (new_tab) {
        emit TabChanged(tab_to_delete, new_tab);
        m_LastContentTab = new_tab;
    }
    tab_to_delete->deleteLater();
}


void TabManager::CloseTab(int tab_index, bool force)
{
    if (count() == 0) {
        return;
    }
    if (!force && count() <= 1) {
        return;
    }

    ContentTab *tab = qobject_cast<ContentTab *>(widget(tab_index));
    if (tab) tab->Close();
    emit TabCountChanged();
}


void TabManager::UpdateTabName(ContentTab *renamed_tab)
{
    Q_ASSERT(renamed_tab);
    setTabText(indexOf(renamed_tab), renamed_tab->GetShortPathName());
}

void TabManager::SetFocusInTab()
{
    ContentTab *tab = GetCurrentContentTab();

    if (tab != NULL) {
        tab->setFocus();
    }
}


WellFormedContent *TabManager::GetWellFormedContent(int index)
{
    return dynamic_cast<WellFormedContent *>(widget(index));
}



// Returns the index of the tab the index is loaded in, -1 if it isn't
int TabManager::ResourceTabIndex(const Resource *resource) const
{
    QString identifier(resource->GetIdentifier());
    int index = -1;

    for (int i = 0; i < count(); ++i) {
        ContentTab *tab = qobject_cast<ContentTab *>(widget(i));

        if (tab && tab->GetLoadedResource()->GetIdentifier() == identifier) {
            index = i;
            break;
        }
    }

    return index;
}


bool TabManager::SwitchedToExistingTab(const Resource *resource,
                                       int line_to_scroll_to,
                                       int position_to_scroll_to,
                                       const QString &caret_location_to_scroll_to,
                                       const QUrl &fragment)
{
    int resource_index = ResourceTabIndex(resource);

    // If the resource is already opened in
    // some tab, then we just switch to it
    if (resource_index != -1) {
        // the next line will cause TabChanged to be emitted which will update Preview
        // but to whatever location this tab has now now after scrolling
        setCurrentIndex(resource_index);
        QWidget *tab = widget(resource_index);
        Q_ASSERT(tab);
        tab->setFocus();

        FlowTab *flow_tab = qobject_cast<FlowTab *>(tab);

        if (flow_tab != NULL) {
            if (!caret_location_to_scroll_to.isEmpty()) {
                flow_tab->ScrollToCaretLocation(caret_location_to_scroll_to);
            } else if (position_to_scroll_to > 0) {
                flow_tab->ScrollToPosition(position_to_scroll_to);
            } else if (!fragment.toString().isEmpty()) {
                flow_tab->ScrollToFragment(fragment.toString());
            } else if (line_to_scroll_to > 0) {
                flow_tab->ScrollToLine(line_to_scroll_to);
            }


            // manually update the Preview Location
	    // flow_tab->EmitScrollPreviewImmediately();

            return true;
        }

        TextTab *text_tab = qobject_cast<TextTab *>(tab);

        if (text_tab != NULL) {
            if (position_to_scroll_to > 0) {
                text_tab->ScrollToPosition(position_to_scroll_to);
            } else {
                text_tab->ScrollToLine(line_to_scroll_to);
            }
            return true;
        }

        ImageTab *image_tab = qobject_cast<ImageTab *>(tab);

        if (image_tab != NULL) {
            return true;
        }

        AVTab *av_tab = qobject_cast<AVTab *>(tab);

        if (av_tab != NULL) {
            return true;
        }

    }

    return false;
}


ContentTab *TabManager::CreateTabForResource(Resource *resource,
        int line_to_scroll_to,
        int position_to_scroll_to,
        const QString &caret_location_to_scroll_to,
        const QUrl &fragment,
        bool grab_focus)
{
    ContentTab *tab = NULL;

    switch (resource->Type()) {
        case Resource::HTMLResourceType: {
            HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
            if (!html_resource) {
                break;
            }
            tab = new FlowTab(html_resource,
                              fragment,
                              line_to_scroll_to,
                              position_to_scroll_to,
                              caret_location_to_scroll_to,
                              grab_focus,
                              this);
            connect(tab,  SIGNAL(LinkClicked(const QUrl &)), this, SLOT(LinkClicked(const QUrl &)));
            connect(tab,  SIGNAL(OldTabRequest(QString, HTMLResource *)),
                    this, SIGNAL(OldTabRequest(QString, HTMLResource *)));
            break;
        }

        case Resource::CSSResourceType: {
            tab = new CSSTab(qobject_cast<CSSResource *>(resource), line_to_scroll_to, position_to_scroll_to, this);
            break;
        }

        case Resource::ImageResourceType: {
            tab = new ImageTab(qobject_cast<ImageResource *>(resource), this);
            break;
        }

        case Resource::MiscTextResourceType: {
            tab = new MiscTextTab(qobject_cast<MiscTextResource *>(resource), line_to_scroll_to, position_to_scroll_to, this);
            break;
        }

        case Resource::SVGResourceType: {
            tab = new SVGTab(qobject_cast<SVGResource *>(resource), line_to_scroll_to, position_to_scroll_to, this);
            break;
        }

        case Resource::OPFResourceType: {
            tab = new OPFTab(qobject_cast<OPFResource *>(resource), line_to_scroll_to, position_to_scroll_to, this);
            break;
        }

        case Resource::NCXResourceType: {
            tab = new NCXTab(qobject_cast<NCXResource *>(resource), line_to_scroll_to, position_to_scroll_to, this);
            break;
        }

        case Resource::XMLResourceType: {
            tab = new XMLTab(qobject_cast<XMLResource *>(resource), line_to_scroll_to, position_to_scroll_to, this);
            break;
        }

        case Resource::TextResourceType: {
            tab = new TextTab(qobject_cast<TextResource *>(resource), CodeViewEditor::Highlight_NONE, 
                              line_to_scroll_to, position_to_scroll_to, this);
            break;
        }

        case Resource::AudioResourceType:
        case Resource::VideoResourceType: {
            tab = new AVTab(qobject_cast<Resource *>(resource), this);
            break;
        }

        default:
            break;
    }

    // Set whether to inform or auto correct well-formed errors.
    WellFormedContent *wtab = dynamic_cast<WellFormedContent *>(tab);

    if (wtab) {
        // In case of well-formed errors we want the tab to be focused.
        connect(tab,  SIGNAL(CentralTabRequest(ContentTab *)),
                this, SLOT(MakeCentralTab(ContentTab *)));    //, Qt::QueuedConnection );
    }

    return tab;
}


bool TabManager::AddNewContentTab(ContentTab *new_tab, bool precede_current_tab)
{
    if (new_tab == NULL) {
        return false;
    }
    int idx = -1;
    if (!precede_current_tab) {

#if defined(Q_OS_MAC)
        // drop use of icons to workaround Qt Bugs: QTBUG-61235, QTBUG-61742, QTBUG-63445, QTBUG-64630
        // This is still broken in Qt 5.12.2 
        // elided text is wrong when icon image present (hides most of elided text)
        // icon image still overlaps with name of tab if ElideNone is used
        idx = addTab(new_tab, new_tab->GetShortPathName());
#else
        idx = addTab(new_tab, new_tab->GetIcon(), new_tab->GetShortPathName());
#endif

        setCurrentWidget(new_tab);
        new_tab->setFocus();
    } else {

#if defined(Q_OS_MAC)
        // drop use of icons to workaround Qt Bugs: QTBUG-61235, QTBUG-61742, QTBUG-63445, QTBUG-64630
        // This is still broken in Qt 5.12.2
        // elided text is wrong when icon image present image (hides most of elided text)
        // icon image still overlaps with name of tab if ElideNone is used
        idx = insertTab(currentIndex(), new_tab, new_tab->GetShortPathName());
#else
        idx = insertTab(currentIndex(), new_tab, new_tab->GetIcon(), new_tab->GetShortPathName());
#endif

    }
    setTabToolTip(idx, new_tab->GetShortPathName());

    connect(new_tab, SIGNAL(DeleteMe(ContentTab *)), this, SLOT(DeleteTab(ContentTab *)));
    connect(new_tab, SIGNAL(TabRenamed(ContentTab *)), this, SLOT(UpdateTabName(ContentTab *)));
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

bool TabManager::CloseOPFTabIfOpen()
{
    QList<Resource *> resources = GetTabResources();
    foreach(Resource *resource, resources) {
        int index = ResourceTabIndex(resource);
        if (resource->Type() == Resource::OPFResourceType) {
            CloseTab(index);
            return true;
        }
    }
    return false;
}
