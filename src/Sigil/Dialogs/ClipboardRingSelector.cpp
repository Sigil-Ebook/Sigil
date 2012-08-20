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

#include "Dialogs/ClipboardRingSelector.h"
#include "Misc/Utility.h"

static const QString SETTINGS_GROUP = "clipboard_ring";
static const QString KEY_SELECTORS = "0123456789abcdefghij";

ClipboardRingSelector::ClipboardRingSelector(QWidget *parent)
    : 
    QDialog(parent),
    m_ClipboardRingHistory(new QStringList())
{
    ui.setupUi(this);
    ExtendUI();

    ReadSettings();

    ConnectSignalsToSlots();
} 

void ClipboardRingSelector::showEvent(QShowEvent *event)
{
    SetupClipboardRingTable();
}

void ClipboardRingSelector::LoadClipboardHistory(const QStringList &clipboardHistory)
{
    m_ClipboardRingHistory->append(clipboardHistory);
}

QStringList ClipboardRingSelector::GetClipboardHistory() const
{
    QStringList clipboardRingHistory;
    clipboardRingHistory.append(*m_ClipboardRingHistory);
    return clipboardRingHistory;
}

void ClipboardRingSelector::SetupClipboardRingTable()
{
    ui.clipboardItemsTable->setRowCount(0);
    ui.clipboardItemsTable->setRowCount(m_ClipboardRingHistory->count());

    for (int row = 0; row < m_ClipboardRingHistory->count(); row++)
    {
        QTableWidgetItem *selector = new QTableWidgetItem();
        // Keyboard shortcuts are 1-9 then 0 then a-j
        selector->setText(KEY_SELECTORS.at(row));
        selector->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui.clipboardItemsTable->setItem(row, 0, selector);

        QString text = m_ClipboardRingHistory->at(row); 
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
    if ( m_ClipboardRingHistory->count() > 0 )
        ui.clipboardItemsTable->selectRow(0);
}

void ClipboardRingSelector::ClipboardChanged( QClipboard::Mode mode )
{
    if ( mode != QClipboard::Clipboard )
        return;
    QString text = QApplication::clipboard()->text();
    if ( text.isNull() || text.isEmpty() ) {
        return;
    }
    int existing_index = m_ClipboardRingHistory->indexOf(text);
    if (existing_index > 0 ) {
        m_ClipboardRingHistory->move(existing_index, 0);
    }
    else if (existing_index < 0 ) {
        m_ClipboardRingHistory->insert(0, text);
        while (m_ClipboardRingHistory->size() > 20) {
            m_ClipboardRingHistory->removeLast();
        }
    }
}

void ClipboardRingSelector::ReadSettings()
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

void ClipboardRingSelector::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    settings.endGroup();
}

bool ClipboardRingSelector::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.clipboardItemsTable) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            int key = keyEvent->key();
            int row = -1;

            if (key == Qt::Key_Delete) {
                int current_row = ui.clipboardItemsTable->currentRow();
                if (current_row >= 0) {
                    m_ClipboardRingHistory->removeAt(current_row);
                    SetupClipboardRingTable();
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

void ClipboardRingSelector::ClipboardItemDoubleClicked(QTableWidgetItem *item)
{
    accept();
}

void ClipboardRingSelector::accept()
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

void ClipboardRingSelector::reject()
{
    WriteSettings();
    QDialog::reject();
}

void ClipboardRingSelector::ExtendUI()
{
    QPushButton *paste_button = ui.buttonBox->button(QDialogButtonBox::Ok);
    paste_button->setText( tr("Paste") );

    ui.clipboardItemsTable->installEventFilter(this);
    ui.clipboardItemsTable->setWordWrap(false);
    ui.clipboardItemsTable->horizontalHeader()->setStretchLastSection(true);
}

void ClipboardRingSelector::ConnectSignalsToSlots()
{
    connect(ui.clipboardItemsTable,    SIGNAL( itemDoubleClicked(QTableWidgetItem*) ),  this, SLOT( ClipboardItemDoubleClicked(QTableWidgetItem*) ));
    connect(QApplication::clipboard(), SIGNAL( changed(QClipboard::Mode) ),             this, SLOT( ClipboardChanged(QClipboard::Mode) ));
}
