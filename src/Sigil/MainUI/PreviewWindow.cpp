/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland, John Schember
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

#include "MainUI/PreviewWindow.h"
#include "ResourceObjects/HTMLResource.h"
#include "ViewEditors/BookViewEditor.h"

PreviewWindow::PreviewWindow(QWidget *parent)
    :
    QDockWidget(tr("Preview"), parent),
    m_MainWidget(*new QWidget(this)),
    m_Layout(*new QVBoxLayout(&m_MainWidget)),
    m_Preview(new BookViewPreview(this)),
    m_OldText(QString())
{
    m_Layout.setContentsMargins(0, 0, 0, 0);
#ifdef Q_WS_MAC
    m_Layout.setSpacing(4);
#endif
    m_Layout.addWidget(m_Preview);
    m_MainWidget.setLayout(&m_Layout);
    setWidget(&m_MainWidget);

    m_MainWidget.setToolTip(
        "<p>" + 
        tr("Preview shows a read-only view of your page when you are in Code View.  The page is refreshed whenever the cursor is moved.") +
        "</p><p>" + 
        tr("You can move the Preview window by clicking on the title bar of the window and dragging it to your desktop or to another part of Sigil.  Dropping it on the TOC or Book Browser windows will create a tabbed window.") +
        "</p>");
}

PreviewWindow::~PreviewWindow()
{
    if (m_Preview) {
        delete m_Preview;
        m_Preview = 0;
    }
}

void PreviewWindow::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    raise();
    emit Shown();
}

void PreviewWindow::UpdatePage(QString filename, QString text, QList< ViewEditor::ElementIndex > location)
{
    if (!m_Preview->isVisible()) {
        return;
    }

    if (text != m_OldText) {
        m_Preview->CustomSetDocument(filename, text);
    }
    m_OldText = text;

    m_Preview->StoreCaretLocationUpdate(location);
    m_Preview->ExecuteCaretUpdate();
    m_Preview->HighlightPosition();
}

void PreviewWindow::ClearPage()
{
    m_Preview->CustomSetDocument("", "");
    m_OldText.clear();
}

void PreviewWindow::SetZoomFactor(float factor) {
    m_Preview->SetZoomFactor(factor);
}
