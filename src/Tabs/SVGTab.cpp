/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin B. Hendricks, Stratford, Ontario Canada 
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

#include "ResourceObjects/SVGResource.h"
#include "Tabs/SVGTab.h"

SVGTab::SVGTab(SVGResource *resource, int line_to_scroll_to, int position_to_scroll_to, QWidget *parent)
    :
    TextTab(resource, CodeViewEditor::Highlight_XHTML, line_to_scroll_to, position_to_scroll_to, parent)
{
    connect(m_wCodeView, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)), this, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)));
    connect(m_wCodeView, SIGNAL(ViewImage(const QUrl &)), this, SLOT(HandleViewImage(const QUrl &)));
    connect(m_wCodeView, SIGNAL(PageUpdated()), this, SLOT(EmitTabUpdated()));
}

void SVGTab::HandleViewImage(const QUrl &url)
{
    if (url.toString().isEmpty()) {
        return;
    }
    
    if (!url.isRelative()) return;

    // we have a relative url, so build an internal
    // book: scheme url book:///bookpath
    
    if (url.path().isEmpty()) return;
    
    QString url_string;
    Resource * resource = GetLoadedResource();

    // handle indicator that user wants to view this tab's svg image
    if (url.toString() == "./SVGTab.svg") {
        url_string = "book:///" + Utility::URLEncodePath(resource->GetRelativePath());
    } else {
        QString startdir = resource->GetFolder();
        QString dest_bookpath = Utility::buildBookPath(url.path(), startdir);
        url_string = "book:///" + Utility::URLEncodePath(dest_bookpath);
    }
    emit ViewImageRequest(QUrl(url_string));
}

void SVGTab::EmitTabUpdated()
{
    emit TabUpdated();
}
