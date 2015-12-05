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

#include <QtCore/QTimer>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "Tabs/ContentTab.h"
#include "Tabs/WellFormedCheckComponent.h"
#include "Tabs/WellFormedContent.h"

WellFormedCheckComponent::WellFormedCheckComponent(WellFormedContent *content, QWidget *parent)
    :
    QObject(),
    m_Content(content),
    m_Message(),
    m_AutoFixMessage(),
    m_AutoFixButton(NULL),
    m_ManualFixButton(NULL),
    m_LastError(),
    m_DemandingAttention(false)
{
    m_Message = tr(
                    "<p>The operation you requested cannot be performed "
                    "because <b>%1</b> is not a well-formed XML document.</p>"
                    "<p>An error was found <b>at or above line %2: %3.</b></p>"
                    "<p>The <i>Fix Manually</i> option will let you fix the problem by hand.</p>");
    m_AutoFixMessage = tr(
                           "<p>The <i>Fix Automatically</i> option will instruct Sigil to try to "
                           "repair the document. <b>This option may lead to loss of data!</b></p>");
    m_MessageBox = new QMessageBox(parent);
    m_MessageBox->setWindowTitle("Sigil");
    m_MessageBox->setIcon(QMessageBox::Critical);
    m_AutoFixButton =
        m_MessageBox->addButton(tr("Fix &Automatically"), QMessageBox::DestructiveRole);
    m_ManualFixButton =
        m_MessageBox->addButton(tr("Fix &Manually"),      QMessageBox::RejectRole);
    m_MessageBox->setDefaultButton(m_ManualFixButton);
}


WellFormedCheckComponent::~WellFormedCheckComponent()
{
    m_MessageBox->deleteLater();
}

void WellFormedCheckComponent::deleteLater()
{
    // Clear the parent to ensure the object disposal for the parent tab
    // will not dispose of the messagebox before the destructor gets called.
    // Quite probably the destructor and call to deleteLater() from FlowTab
    // and XMLTab destructors are completely redundant by parenting this
    // messagebox, but not going to risk memory leak at this point by changing.
    m_MessageBox->setParent(0);
    QObject::deleteLater();
}

void WellFormedCheckComponent::DemandAttentionIfAllowed(const XhtmlDoc::WellFormedError &error)
{
    m_LastError = error;
    // We schedule a request to make the OPF tab the central
    // tab of the UI. We do this async to make sure there are
    // no infinite loops.
    QTimer::singleShot(0, this, SLOT(DemandAttention()));
}


void WellFormedCheckComponent::DemandAttention()
{
    // This prevents multiple calls at the same time
    if (m_DemandingAttention) {
        return;
    }

    m_DemandingAttention = true;
    m_Content->TakeControlOfUI();
    DisplayErrorMessage();
    m_LastError = XhtmlDoc::WellFormedError();
    m_DemandingAttention = false;
}


void WellFormedCheckComponent::DisplayErrorMessage()
{
    const QString error_line = m_LastError.line != -1 ?
                               QString::number(m_LastError.line) :
                               "N/A";
    QString full_message = m_Message
                           .arg(m_Content->GetFilename(), error_line, m_LastError.message);

    m_AutoFixButton->setVisible(true);
    full_message.append(m_AutoFixMessage);

    m_MessageBox->setText(full_message);
    m_MessageBox->exec();

    if (m_MessageBox->clickedButton() == m_AutoFixButton) {
        m_Content->AutoFixWellFormedErrors();
    } else {
        if (m_LastError.line != -1) {
            m_Content->ScrollToLine(m_LastError.line);
        }
    }
}

