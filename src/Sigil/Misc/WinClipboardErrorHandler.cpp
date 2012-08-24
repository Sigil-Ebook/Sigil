/************************************************************************
**
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Grant Drake
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

#include <QApplication>
#include <QtCore/QTimer>
#include <QtGui/QPlainTextEdit>
#include <QtWebKit/QWebPage>

#include "Misc/WinClipboardErrorHandler.h"
#include "ViewEditors/BookViewPreview.h"

static const int RETRY_DELAY_MS = 5;

// Constructor;
// The argument is the object's parent.
WinClipboardErrorHandler::WinClipboardErrorHandler( QObject *parent )
    : QObject( parent ),
      m_textEdit( NULL ),
      m_webPage( NULL )
{
}

bool WinClipboardErrorHandler::HandleError()
{
    // Try to identify the control with focus, to be able to repeat the copy operation on it
    // to ensure the data does end up on the clipboard.
    QWidget *widget = QApplication::focusWidget();
    if (widget) {
        m_textEdit = dynamic_cast<QPlainTextEdit*>(widget);
        if (!m_textEdit) {
            // BV/PV copying is a little different, in that the focus widget is set to
            // the parent editor (unlike CodeView).
            BookViewPreview *bookViewPreview = dynamic_cast<BookViewPreview*>(widget);
            if (bookViewPreview) {
                m_webPage = bookViewPreview->page();
            }
        }
        if (m_textEdit || m_webPage) {
            // After a short delay reattempt the copy
            QTimer::singleShot(RETRY_DELAY_MS, this, SLOT(AttemptAnotherCopy()));
            return true;
        }
    }
    return false;
}

void WinClipboardErrorHandler::AttemptAnotherCopy()
{
    if (m_textEdit) {
        m_textEdit->copy();
        m_textEdit = NULL;
    }
    else if (m_webPage) {
        m_webPage->triggerAction(QWebPage::Copy);
        m_webPage = NULL;
    }
}
