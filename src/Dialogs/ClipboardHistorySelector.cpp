/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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

#include <QtCore/QSignalMapper>
#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QMimeData>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidgetItem>
#include <QTimer>

#include "Dialogs/ClipboardHistorySelector.h"
#include "MainUI/MainApplication.h"
#include "Misc/Utility.h"

#include "sigil_constants.h"

static const QString SETTINGS_GROUP = "clipboard_history";
static const QString KEY_SELECTORS = "0123456789abcdefghij";
const int MAX_DISPLAY_LENGTH = 500;

ClipboardHistorySelector::ClipboardHistorySelector(QWidget *parent)
    :
    QDialog(parent),
    m_ClipboardHistoryItems(new QStringList()),
    m_PreviousClipboardHistoryItems(new QStringList()),
    m_savedHistory(false)
{
    ui.setupUi(this);
    ExtendUI();
    ReadSettings();
    ConnectSignalsToSlots();
}

void ClipboardHistorySelector::showEvent(QShowEvent *event)
{
    // Make a copy of the clipboard history in case user clicks Cancel
    m_PreviousClipboardHistoryItems->clear();
    for (int i=0; i < m_ClipboardHistoryItems->count(); i++) {
        m_PreviousClipboardHistoryItems->append(m_ClipboardHistoryItems->at(i));
    }
    SetupClipboardHistoryTable();
    ui.clipboardItemsTable->setFocus();
}

void ClipboardHistorySelector::ApplicationActivated()
{
    // Turned on when Sigil is activated, put the latest text if any at the top of the history
    ClipboardChanged();
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(ClipboardChanged()));

    // If we are currently showing this dialog, refresh the display
    if (isVisible()) {
        SetupClipboardHistoryTable();
    }
}

void ClipboardHistorySelector::ApplicationDeactivated()
{
    SaveClipboardState();
}

void ClipboardHistorySelector::LoadClipboardHistory(const QStringList &clipboardHistoryItems)
{
    m_ClipboardHistoryItems->append(clipboardHistoryItems);
}

QStringList ClipboardHistorySelector::GetClipboardHistory(int limit) const
{
    QStringList clipboardHistoryItems;
    clipboardHistoryItems.append(*m_ClipboardHistoryItems);
    // Reduce the history to comply with user limits
    while (clipboardHistoryItems.size() > limit) {
        clipboardHistoryItems.removeLast();
    }
    return clipboardHistoryItems;
}

void ClipboardHistorySelector::SaveClipboardState()
{
    disconnect(QApplication::clipboard(), 0, this, 0);
}

void ClipboardHistorySelector::RestoreClipboardState()
{
    if (m_ClipboardHistoryItems->count() > 0) {
        QApplication::clipboard()->setText(m_ClipboardHistoryItems->at(0));
    }

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(ClipboardChanged()));
}

void ClipboardHistorySelector::SetupClipboardHistoryTable()
{
    ui.clipboardItemsTable->setRowCount(0);
    ui.clipboardItemsTable->setRowCount(m_ClipboardHistoryItems->count());

    for (int row = 0; row < m_ClipboardHistoryItems->count(); row++) {
        QTableWidgetItem *selector = new QTableWidgetItem();
        // Keyboard shortcuts are 1-9 then 0 then a-j
        selector->setText(KEY_SELECTORS.at(row));
        selector->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui.clipboardItemsTable->setItem(row, 0, selector);

        QString text = m_ClipboardHistoryItems->at(row);
        // In case the user has an enormous amount of data restrict what is displayed.
        if (text.length() > MAX_DISPLAY_LENGTH) {
            text.truncate(MAX_DISPLAY_LENGTH);
            text.append("...");
        }
        QString display_text(text);
        // Replace certain non-printable characters with spaces (to avoid
        // drawing boxes when using fonts that don't have glyphs for such
        // characters)
        QChar *uc = display_text.data();

        for (int i = 0; i < (int)text.length(); ++i) {
            if ((uc[i] < 0x20 && uc[i] != 0x09)
                || uc[i] == QChar::LineSeparator
                || uc[i] == QChar::ParagraphSeparator
                || uc[i] == QChar::ObjectReplacementCharacter) {
                uc[i] = QChar(0x0020);
            }
        }

        // Also replace any tab characters with an arrow glyph
        display_text = display_text.replace(QChar('\t'), QChar(0x2192));
        QTableWidgetItem *clip = new QTableWidgetItem();
        clip->setText(display_text);
        clip->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        clip->setData(Qt::UserRole, QVariant(m_ClipboardHistoryItems->at(row)));
        clip->setToolTip(text);
        ui.clipboardItemsTable->setItem(row, 1, clip);
    }

    ui.clipboardItemsTable->setColumnWidth(0, 20);
    ui.clipboardItemsTable->resizeRowsToContents();

    if (m_ClipboardHistoryItems->count() > 0) {
        ui.clipboardItemsTable->selectRow(0);
    }
}

// This is only needed for Linux
// See:  https://bugreports.qt.io/browse/QTBUG-44849
void ClipboardHistorySelector::TakeOwnershipOfClip()
{
    QApplication::clipboard()->setMimeData(copyMimeData(m_lastclip), QClipboard::Clipboard);
}

// This is only needed for Linux
// See:  https://bugreports.qt.io/browse/QTBUG-44849
QMimeData *ClipboardHistorySelector::copyMimeData(const QMimeData *mimeReference)
{
    QMimeData *mimeCopy = new QMimeData();
    foreach(QString format, mimeReference->formats()) {
        // Retrieving data
        QByteArray data = mimeReference->data(format);
        // Checking for custom MIME types
        if(format.startsWith("application/x-qt")) {
            // Retrieving true format name
            int indexBegin = format.indexOf('"') + 1;
            int indexEnd = format.indexOf('"', indexBegin);
            format = format.mid(indexBegin, indexEnd - indexBegin);
        }
        mimeCopy->setData(format, data);
    }
    return mimeCopy;
}

void ClipboardHistorySelector::ClipboardChanged()
{
    const QString text = QApplication::clipboard()->text();

    if (text.isEmpty()) {
        return;
    }

// This is only needed for Linux
// See:  https://bugreports.qt.io/browse/QTBUG-44849
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // if there is something on the clipboard make sure we own it
    if (!QApplication::clipboard()->ownsClipboard()) {
        m_lastclip = QApplication::clipboard()->mimeData(QClipboard::Clipboard);
        QTimer::singleShot(50, this, SLOT(TakeOwnershipOfClip()));
    }
#endif

    int existing_index = m_ClipboardHistoryItems->indexOf(text);

    if (existing_index > 0) {
        m_ClipboardHistoryItems->move(existing_index, 0);
    } else if (existing_index < 0) {
        m_ClipboardHistoryItems->insert(0, text);

        while (m_ClipboardHistoryItems->size() > CLIPBOARD_HISTORY_MAX) {
            m_ClipboardHistoryItems->removeLast();
        }
    }
}

void ClipboardHistorySelector::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}

void ClipboardHistorySelector::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

bool ClipboardHistorySelector::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.clipboardItemsTable) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();
            int row = -1;

            if ((key == Qt::Key_Backspace) || (key == Qt::Key_Delete)) {
                int current_row = ui.clipboardItemsTable->currentRow();

                if (current_row >= 0) {
                    m_ClipboardHistoryItems->removeAt(current_row);
                    SetupClipboardHistoryTable();

                    if (current_row >= ui.clipboardItemsTable->rowCount()) {
                        current_row--;
                    }

                    ui.clipboardItemsTable->selectRow(current_row);
                    return true;
                }
            } else if ((key >= Qt::Key_0) && (key <= Qt::Key_9)) {
                row = key - Qt::Key_0;
            } else if ((key >= Qt::Key_A) && (key <= Qt::Key_J)) {
                row = key - Qt::Key_A + 10;
            }

            if (row >= 0 && row < ui.clipboardItemsTable->rowCount()) {
                ui.clipboardItemsTable->selectRow(row);
                accept();
                return true;
            }
        }
    }

    // pass the event on to the parent class
    return QDialog::eventFilter(obj, event);
}

void ClipboardHistorySelector::ClipboardItemDoubleClicked(QTableWidgetItem *item)
{
    accept();
}


void ClipboardHistorySelector::buttonClicked(QAbstractButton * button)
{
    QDialogButtonBox::StandardButton standardButton = ui.buttonBox->standardButton(button);
    // abort does an abort without cancelloing changes 
    if (standardButton == QDialogButtonBox::Abort) {
        m_savedHistory = true;
    }
}


void ClipboardHistorySelector::accept()
{
    if (ui.clipboardItemsTable->rowCount() > 0) {
        int selected_row = ui.clipboardItemsTable->currentRow();

        if (selected_row >= 0) {
            QTableWidgetItem *item = ui.clipboardItemsTable->item(selected_row, 1);
            const QString text = item->data(Qt::UserRole).toString();
            QApplication::clipboard()->setText(text);
            emit PasteRequest(text);
        } else {
            // user does not have any rows selected - paste text currently on clipboard
            emit PasteRequest(QApplication::clipboard()->text());
        }
    }

    WriteSettings();
    QDialog::accept();
}

void ClipboardHistorySelector::reject()
{
    if (!m_savedHistory) {
        // As user is cancelling, reject any deletions they have done to the history
        m_ClipboardHistoryItems->clear();
        for (int i=0; i < m_PreviousClipboardHistoryItems->count(); i++) {
            m_ClipboardHistoryItems->append(m_PreviousClipboardHistoryItems->at(i));
        }
    }
    m_savedHistory = false;
    WriteSettings();
    QDialog::reject();
}

void ClipboardHistorySelector::ExtendUI()
{
    QPushButton *paste_button = ui.buttonBox->button(QDialogButtonBox::Ok);
    paste_button->setText(tr("Paste"));
    QPushButton *save_button = ui.buttonBox->button(QDialogButtonBox::Abort);
    save_button->setText(tr("Save"));
    ui.clipboardItemsTable->installEventFilter(this);
    ui.clipboardItemsTable->setWordWrap(false);
    ui.clipboardItemsTable->horizontalHeader()->setStretchLastSection(true);
}

void ClipboardHistorySelector::ConnectSignalsToSlots()
{
    connect(ui.clipboardItemsTable,    SIGNAL(itemDoubleClicked(QTableWidgetItem *)),  this, SLOT(ClipboardItemDoubleClicked(QTableWidgetItem *)));
    connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(buttonClicked(QAbstractButton *)));

    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);

    if (mainApplication) {
        connect(mainApplication, SIGNAL(applicationActivated()),   this, SLOT(ApplicationActivated()));
        connect(mainApplication, SIGNAL(applicationDeactivated()), this, SLOT(ApplicationDeactivated()));
    }
}
