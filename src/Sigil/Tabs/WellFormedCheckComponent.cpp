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

#include <stdafx.h>
#include "ContentTab.h"
#include "WellFormedCheckComponent.h"
#include "WellFormedContent.h"


WellFormedCheckComponent::WellFormedCheckComponent( WellFormedContent &content )
    :
    QObject(),
    m_Message(),
    m_Content( content ),
    m_MessageBox( new QMessageBox() ),
    m_AutoFixButton( NULL ),
    m_ManualFixButton( NULL ),
    m_LastError(),
    m_DemandingAttention( false ),
    m_WellFormedDialogsEnabled( true )
{
    m_Message = tr( 
        "<p>The operation you requested cannot be performed "
        "because <b>%1</b> is not a well-formed XML document.</p>"
        "<p>An error was found on <b>line %2, column %3: %4.</b></p>"
        "<p>The <i>Fix Manually</i> option will let you fix the problem by hand.</p>"
        "<p>The <i>Fix Automatically</i> option will instruct Sigil to try to "
        "repair the document. <b>This option may lead to loss of data!</b></p>" );

    m_MessageBox->setWindowTitle( "Sigil" );
    m_MessageBox->setIcon( QMessageBox::Critical );

    m_AutoFixButton =
        m_MessageBox->addButton( tr( "Fix &Automatically" ), QMessageBox::DestructiveRole );
    m_ManualFixButton = 
        m_MessageBox->addButton( tr( "Fix &Manually" ),      QMessageBox::RejectRole      );

    m_MessageBox->setDefaultButton( m_AutoFixButton );
}


WellFormedCheckComponent::~WellFormedCheckComponent()
{
    m_MessageBox->deleteLater();
}

    
void WellFormedCheckComponent::SetWellFormedDialogsEnabledState( bool enabled )
{
    m_WellFormedDialogsEnabled = enabled;
}


void WellFormedCheckComponent::DemandAttentionIfAllowed( const XhtmlDoc::WellFormedError &error )
{   
    m_LastError = error;

    // We schedule a request to make the OPF tab the central
    // tab of the UI. We do this async to make sure there are 
    // no infinite loops.
    if ( m_WellFormedDialogsEnabled )

        QTimer::singleShot( 0, this, SLOT( DemandAttention() ) );
}


void WellFormedCheckComponent::DemandAttention()
{
    // This prevents multiple calls at the same time
    if ( m_DemandingAttention )

        return;

    m_DemandingAttention = true;

    m_Content.TakeControlOfUI();
    DisplayErrorMessage();   

    m_LastError = XhtmlDoc::WellFormedError();
    m_DemandingAttention = false;
}


void WellFormedCheckComponent::DisplayErrorMessage()
{
    QString error_line = m_LastError.line != -1              ?
                         QString::number( m_LastError.line ) :
                         "N/A";
    QString error_column = m_LastError.column != -1              ?
                           QString::number( m_LastError.column ) :
                           "N/A";

    QString full_message = m_Message
        .arg( m_Content.GetFilename(), error_line, error_column, m_LastError.message );

    m_MessageBox->setText( full_message );
    m_MessageBox->exec();
         
    if ( m_MessageBox->clickedButton() == m_AutoFixButton ) 
    
        m_Content.AutoFixWellFormedErrors();
     
    else // manual_fix_button
    
        m_Content.ScrollToLine( m_LastError.line );
}








