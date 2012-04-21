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
#ifndef WELLFORMEDCHECKCOMPONENT_H
#define WELLFORMEDCHECKCOMPONENT_H

#include <QtCore/QObject>

#include "BookManipulation/XhtmlDoc.h"

class WellFormedContent;
class QMessageBox;
class QPushButton;


/**
 * Handles the display of a dialog that the document
 * is not well-formed XML. It would be great if we
 * could make FlowTab and XMLTab subclasses of this,
 * but multiple inheritance from several QObject base
 * classes is not allowed and doesn't work.
 */
class WellFormedCheckComponent : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param content The GUI element containing well-formed content.
     */
    WellFormedCheckComponent( WellFormedContent &content );

    /**
     * Destructor.
     */
    ~WellFormedCheckComponent();

    /**
     * Gets whether checking for well-formed errors is enabled.
     */
    bool GetCheckWellFormedErrors();
    
public slots:

    /**
     * Turns on/off the dialog responsible for notifying the user
     * about well-formed errors.
     *
     * @param enabled If \true, the dialog is enabled.
     */
    void SetWellFormedDialogsEnabledState( bool enabled );

    /**
     * Turns on/off checking for well-formed errors.
     *
     * @param enabled If \true, the content will be checked for
     * well-formed errors.
     */
    void SetCheckWellFormedErrorsState( bool enabled );
    
    /**
     * Displays a dialog informing the user about the well-formed error
     * if it's allowed to show the dialog.
     *
     * @param error The error info to display.
     */
    void DemandAttentionIfAllowed( const XhtmlDoc::WellFormedError &error );

private slots:

    /**
     * Demands attention from the UI.
     */
    void DemandAttention();

private:

    /**
     * Displays the stored error message in a dialog.
     */
    void DisplayErrorMessage();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The GUI element containing well-formed content.
     */
    WellFormedContent& m_Content;

    /**
     * The message used in the dialog (with placeholders).
     */
    QString m_Message;

    /**
     * The message box used to inform the user about an error.
     */
    QMessageBox *m_MessageBox;

    /**
     * The Fix Automatically button.
     */
    QPushButton *m_AutoFixButton;
    
    /**
     * The Fix Manually button.
     */
    QPushButton *m_ManualFixButton;

    /**
     * The last error reported to the DemandAttentionIfAllowed func.
     */
    XhtmlDoc::WellFormedError m_LastError;

    /**
     * \c true if we are currently trying to notify the user
     * about an error. Prevents multiple notifications about the
     * same problem.
     */
    bool m_DemandingAttention;

    /**
     * If \c true, then we are allowed to show a dialog for errors.
     */
    bool m_WellFormedDialogsEnabled;

    /**
     * If \c true, then we are allowed to check for errors.
     */
    bool m_CheckWellFormedErrors;

};

#endif // WELLFORMEDCHECKCOMPONENT_H
