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

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QtCore/QSignalMapper>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidgetItem>

#include "Dialogs/ClipboardHistorySelector.h"
#include "Misc/Utility.h"

static const QString SETTINGS_GROUP = "clipboard_history";
static const QString KEY_SELECTORS = "0123456789abcdefghij";

ClipboardHistorySelector::ClipboardHistorySelector(QWidget *parent)
    : 
    QDialog(parent),
    m_ClipboardHistoryItems(new QStringList())
{
    ui.setupUi(this);
    ExtendUI();

    ReadSettings();

    ConnectSignalsToSlots();
} 

void ClipboardHistorySelector::showEvent(QShowEvent *event)
{
    SetupClipboardHistoryTable();
}

void ClipboardHistorySelector::LoadClipboardHistory(const QStringList &clipboardHistoryItems)
{
    m_ClipboardHistoryItems->append(clipboardHistoryItems);
}

QStringList ClipboardHistorySelector::GetClipboardHistory() const
{
    QStringList clipboardHistoryItems;
    clipboardHistoryItems.append(*m_ClipboardHistoryItems);
    return clipboardHistoryItems;
}

void ClipboardHistorySelector::SaveClipboardState()
{
    disconnect(QApplication::clipboard(), 0, this, 0 );
}

void ClipboardHistorySelector::RestoreClipboardState()
{
    if (m_ClipboardHistoryItems->count() > 0) {
        QApplication::clipboard()->setText(m_ClipboardHistoryItems->at(0));
    }
    connect(QApplication::clipboard(), SIGNAL( changed(QClipboard::Mode) ), this, SLOT( ClipboardChanged(QClipboard::Mode) ));
}

void ClipboardHistorySelector::SetupClipboardHistoryTable()
{
    ui.clipboardItemsTable->setRowCount(0);
    ui.clipboardItemsTable->setRowCount(m_ClipboardHistoryItems->count());

    for (int row = 0; row < m_ClipboardHistoryItems->count(); row++)
    {
        QTableWidgetItem *selector = new QTableWidgetItem();
        // Keyboard shortcuts are 1-9 then 0 then a-j
        selector->setText(KEY_SELECTORS.at(row));
        selector->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui.clipboardItemsTable->setItem(row, 0, selector);

        QString text = m_ClipboardHistoryItems->at(row); 
        QString display_text(text);
        
        // Replace certain non-printable characters with spaces (to avoid
        // drawing boxes when using fonts that don't have glyphs for such
        // characters)
        QChar *uc = display_text.data();
        for (int i = 0; i < (int)text.length(); ++i) {
            if ((uc[i] < 0x20 && uc[i] != 0x09)
                || uc[i] == QChar::LineSeparator
                || uc[i] == QChar::ParagraphSeparator
                || uc[i] == QChar::ObjectReplacementCharacter)
                uc[i] = QChar(0x0020);
        }
        // Also replace any tab characters with an arrow glyph
        display_text = display_text.replace(QChar('\t'), QChar(0x2192));

        QTableWidgetItem *clip = new QTableWidgetItem();
        clip->setText(display_text);
        clip->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        clip->setData(Qt::UserRole, QVariant(text));
        clip->setToolTip(text);
        ui.clipboardItemsTable->setItem(row, 1, clip);
    }
    ui.clipboardItemsTable->setColumnWidth(0, 20);
    ui.clipboardItemsTable->resizeRowsToContents();
    if ( m_ClipboardHistoryItems->count() > 0 )
        ui.clipboardItemsTable->selectRow(0);
}

void ClipboardHistorySelector::ClipboardChanged( QClipboard::Mode mode )
{
    if ( mode != QClipboard::Clipboard )
        return;
    QString text = QApplication::clipboard()->text();
    if ( text.isNull() || text.isEmpty() ) {
        return;
    }
    int existing_index = m_ClipboardHistoryItems->indexOf(text);
    if (existing_index > 0 ) {
        m_ClipboardHistoryItems->move(existing_index, 0);
    }
    else if (existing_index < 0 ) {
        m_ClipboardHistoryItems->insert(0, text);
        while (m_ClipboardHistoryItems->size() > 20) {
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
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            int key = keyEvent->key();
            int row = -1;

            if (key == Qt::Key_Delete) {
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
            }
            else if ((key >= Qt::Key_0) && (key <= Qt::Key_9)) {
                row = key - Qt::Key_0;
            }
            else if ((key >= Qt::Key_A) && (key <= Qt::Key_J)) {
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

void ClipboardHistorySelector::accept()
{
    if ( ui.clipboardItemsTable->rowCount() > 0 ) {
        int selected_row = ui.clipboardItemsTable->currentRow();
        if (selected_row > 0) {
            QTableWidgetItem *item = ui.clipboardItemsTable->item(selected_row, 1);
            QString text = item->data(Qt::UserRole).toString();
            QApplication::clipboard()->setText(text);
        }

        emit PasteFromClipboardRequest();
    }

    WriteSettings();
    QDialog::accept();
}

void ClipboardHistorySelector::reject()
{
    WriteSettings();
    QDialog::reject();
}

void ClipboardHistorySelector::ExtendUI()
{
    QPushButton *paste_button = ui.buttonBox->button(QDialogButtonBox::Ok);
    paste_button->setText( tr("Paste") );

    ui.clipboardItemsTable->installEventFilter(this);
    ui.clipboardItemsTable->setWordWrap(false);
    ui.clipboardItemsTable->horizontalHeader()->setStretchLastSection(true);
}

void ClipboardHistorySelector::ConnectSignalsToSlots()
{
    connect(ui.clipboardItemsTable,    SIGNAL( itemDoubleClicked(QTableWidgetItem*) ),  this, SLOT( ClipboardItemDoubleClicked(QTableWidgetItem*) ));
    connect(QApplication::clipboard(), SIGNAL( changed(QClipboard::Mode) ),             this, SLOT( ClipboardChanged(QClipboard::Mode) ));
}
