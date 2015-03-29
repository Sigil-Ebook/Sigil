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

#include "Tabs/CSSTab.h"
#include "ResourceObjects/CSSResource.h"

CSSTab::CSSTab(CSSResource &resource, int line_to_scroll_to, QWidget *parent)
    :
    TextTab(resource, CodeViewEditor::Highlight_CSS, line_to_scroll_to, parent)
{
    m_wCodeView.SetReformatCSSEnabled(true);
    connect(&m_wCodeView, SIGNAL(PageUpdated()), this, SLOT(EmitCSSUpdated()));
}

void CSSTab::EmitCSSUpdated()
{
    emit CSSUpdated();
}

void CSSTab::Bold()
{
    m_wCodeView.FormatCSSStyle("font-weight", "bold");
}

void CSSTab::Italic()
{
    m_wCodeView.FormatCSSStyle("font-style", "italic");
}

void CSSTab::Underline()
{
    m_wCodeView.FormatCSSStyle("text-decoration", "underline");
}

void CSSTab::Strikethrough()
{
    m_wCodeView.FormatCSSStyle("text-decoration", "line-through");
}

void CSSTab::AlignLeft()
{
    m_wCodeView.FormatCSSStyle("text-align", "left");
}

void CSSTab::AlignCenter()
{
    m_wCodeView.FormatCSSStyle("text-align", "center");
}

void CSSTab::AlignRight()
{
    m_wCodeView.FormatCSSStyle("text-align", "right");
}

void CSSTab::AlignJustify()
{
    m_wCodeView.FormatCSSStyle("text-align", "justify");
}

void CSSTab::TextDirectionLeftToRight()
{
    m_wCodeView.FormatCSSStyle("direction", "ltr");
}

void CSSTab::TextDirectionRightToLeft()
{
    m_wCodeView.FormatCSSStyle("direction", "rtl");
}

void CSSTab::TextDirectionDefault()
{
    m_wCodeView.FormatCSSStyle("direction", "inherit");
}
