/************************************************************************
**
**  Copyright (C) 2011 John Schember <john@nachtimwald.com>
**  Copyright (C) 2011 Grzegorz Wolszczak <grzechu81@gmail.com>
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

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>

#include "KeyboardShortcutsWidget.h"
#include "Misc/KeyboardShortcutManager.h"

const int COL_NAME = 0;
const int COL_DESCRIPTION = 1;
const int COL_SHORTCUT = 2;
const int COL_DEFAULT_SHORTCUT = 3;
// This column is not displayed but we still need it so we can reference
// The short cut in the shortcut manager.
const int COL_ID = 4;

KeyboardShortcutsWidget::KeyboardShortcutsWidget()
{
    ui.setupUi(this);
    connectSignalsSlots();
    readSettings();
}

void KeyboardShortcutsWidget::saveSettings()
{
     KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

     for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
         QTreeWidgetItem *item = ui.commandList->topLevelItem(i);

         const QString id = item->text(COL_ID);
         const QKeySequence keySequence = item->text(COL_SHORTCUT);

         sm->setKeySequence(id, keySequence);

         updateItemView(item);
     }
}

bool KeyboardShortcutsWidget::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent*>(event);
        handleKeyEvent(k);
        return true;
    }

    if (event->type() == QEvent::Shortcut || event->type() == QEvent::KeyRelease) {
        return true;
    }

    if (event->type() == QEvent::ShortcutOverride) {
        // For shortcut overrides, we need to accept as well
        event->accept();
        return true;
    }

    return false;
}

void KeyboardShortcutsWidget::showEvent(QShowEvent *event)
{
    // Fill out the tree view
    readSettings();
    ui.commandList->setColumnWidth(COL_NAME, ui.commandList->width() * .30);
    ui.commandList->setColumnWidth(COL_DESCRIPTION, ui.commandList->width() * .35);
    ui.commandList->setColumnWidth(COL_SHORTCUT, ui.commandList->width() * .15);
    ui.commandList->setColumnWidth(COL_DEFAULT_SHORTCUT, ui.commandList->width() * .15);
    QWidget::showEvent(event);
}

void KeyboardShortcutsWidget::treeWidgetItemActivatedSlot(QTreeWidgetItem *item, int column)
{
    ui.targetEdit->setText(ui.commandList->currentItem()->text(COL_SHORTCUT));
}

void KeyboardShortcutsWidget::clearButtonClicked()
{
    ui.targetEdit->setText("");
    ui.commandList->currentItem()->setText(COL_SHORTCUT, "");
    ui.targetEdit->setFocus();
}

void KeyboardShortcutsWidget::filterEditTextChangedSlot(const QString &text)
{
    const QString newText = text.toUpper();
    // If text is empty - show all items.
    if (text.isEmpty()) {
        for(int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
             ui.commandList->topLevelItem(i)->setHidden(false);
        }
    }
    else {
        for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
            const QString name = ui.commandList->topLevelItem(i)->text(COL_NAME).toUpper();
            const QString description = ui.commandList->topLevelItem(i)->text(COL_DESCRIPTION).toUpper();
            const QString sequence = ui.commandList->topLevelItem(i)->text(COL_SHORTCUT).toUpper();
            const QString defaultSequence = ui.commandList->topLevelItem(i)->text(COL_DEFAULT_SHORTCUT).toUpper();

            if (name.contains(newText) ||
                description.contains(newText) ||
                sequence.contains(newText) ||
                defaultSequence.contains(newText)
              )
            {
                 ui.commandList->topLevelItem(i)->setHidden(false);
            }
            else {
                 ui.commandList->topLevelItem(i)->setHidden(true);
            }
        }
    }
}

void KeyboardShortcutsWidget::targetEditTextChangedSlot(const QString &text)
{
    updateCurrentItemShortcut();
    markSequencesAsDuplicatedIfNeeded();
}

void KeyboardShortcutsWidget::resetButtonClickedSlot()
{
    if(ui.commandList->currentItem() != 0) {
        KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

        // Get 'current' shortcut value from KeyboardShortcutManager
        QKeySequence seq = sm->keyboardShortcut(ui.commandList->currentItem()->text(COL_ID)).defaultKeySequence();
        // Assign shortcut value from KeyboardShortcutManager to the currentItem
        ui.commandList->currentItem()->setText(COL_SHORTCUT, seq);
        ui.targetEdit->setText(seq);
        ui.targetEdit->setFocus();
        updateItemView(ui.commandList->currentItem());
    }
}

void KeyboardShortcutsWidget::resetAllButtonClickedSlot()
{
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

    // Go through all items
    for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui.commandList->topLevelItem(i);
        QKeySequence seq = sm->keyboardShortcut(item->text(COL_ID)).defaultKeySequence();
        item->setText(COL_SHORTCUT, seq);
        updateItemView(item);
    }

    markSequencesAsDuplicatedIfNeeded();
}

void KeyboardShortcutsWidget::readSettings()
{
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

    ui.commandList->clear();

    QStringList ids = sm->ids();
    foreach (QString id, ids)
    {
        KeyboardShortcut shortcut = sm->keyboardShortcut(id);
        if (!shortcut.isEmpty()) {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(COL_NAME, shortcut.name());
            item->setText(COL_DESCRIPTION, shortcut.description());
            item->setText(COL_SHORTCUT, shortcut.keySequence());
            item->setText(COL_DEFAULT_SHORTCUT, shortcut.defaultKeySequence());
            item->setText(COL_ID, id);
            ui.commandList->addTopLevelItem(item);
        }
    }
    ui.commandList->sortItems( 0, Qt::AscendingOrder );

    markSequencesAsDuplicatedIfNeeded();
}

void KeyboardShortcutsWidget::connectSignalsSlots()
{
    connect(ui.commandList, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(treeWidgetItemActivatedSlot(QTreeWidgetItem*,int)));
    connect(ui.clearButton, SIGNAL(clicked()), this, SLOT(clearButtonClicked()));
    connect(ui.filterEdit, SIGNAL(textChanged(QString)), this, SLOT(filterEditTextChangedSlot(QString)));
    connect(ui.targetEdit, SIGNAL( textChanged(QString)), this, SLOT(targetEditTextChangedSlot(QString)));
    connect(ui.resetButton, SIGNAL(clicked()), this, SLOT(resetButtonClickedSlot()));
    connect(ui.resetAllButton, SIGNAL(clicked()), this, SLOT(resetAllButtonClickedSlot()));

    ui.targetEdit->installEventFilter(this);

}

void KeyboardShortcutsWidget::handleKeyEvent(QKeyEvent *event)
{
    int nextKey = event->key();
    if (nextKey == Qt::Key_Control ||
         nextKey == Qt::Key_Shift   ||
         nextKey == Qt::Key_Meta    ||
         nextKey == Qt::Key_Alt     ||
         nextKey == Qt::Key_Backspace || // This button cannot be assigned, because we want to 'clear' shortcut after backspace push
         ui.commandList->currentItem() == 0 // Do not allow writting in shortcut line edit if no item is selected
        )
    {
        // If key was Qt::Key_Backspace additionaly clear sequence dialog
        if(nextKey == Qt::Key_Backspace) {
            clearButtonClicked();
        }
        return;
    }

    nextKey |= translateModifiers(event->modifiers(), event->text());
    ui.targetEdit->setText(QKeySequence(nextKey));
    event->accept();
}

int KeyboardShortcutsWidget::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;

    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) && (text.size() == 0
                                        || !text.at(0).isPrint()
                                        || text.at(0).isLetterOrNumber()
                                        || text.at(0).isSpace()
                                        )
       ) {
        result |= Qt::SHIFT;
    }
    if (state & Qt::ControlModifier) {
        result |= Qt::CTRL;
    }
    if (state & Qt::MetaModifier) {
        result |= Qt::META;
    }
    if (state & Qt::AltModifier) {
        result |= Qt::ALT;
    }

    return result;
}

void KeyboardShortcutsWidget::updateItemView(QTreeWidgetItem *item)
{
    if (item == 0) {
        return;
    }
    const QString seqName = item->text(COL_ID);
    const QKeySequence seqValue = item->text(COL_SHORTCUT);
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

    QKeySequence seq = sm->keyboardShortcut(seqName).keySequence();
    QFont font = item->font(COL_SHORTCUT);
    // If item is 'modified' show it in 'italics'
    if ( seq != seqValue ) {
        font.setItalic( true );
    }
    else {
        font.setItalic( false );
    }
    item->setFont(COL_SHORTCUT, font);
}

void KeyboardShortcutsWidget::updateCurrentItemShortcut()
{
    if (ui.commandList->currentItem() != 0) {
        ui.commandList->currentItem()->setText(COL_SHORTCUT, ui.targetEdit->text());
        updateItemView(ui.commandList->currentItem());
    }
}

void KeyboardShortcutsWidget::markSequencesAsDuplicatedIfNeeded()
{
    // This is not optized , but since this will be called rather seldom
    // effort for optimization may not be worth it

    QMap<QKeySequence, QSet<QTreeWidgetItem*> > seqMap;

    // Go through all items
    for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui.commandList->topLevelItem(i);

        const QKeySequence keySequence = item->text(COL_SHORTCUT);
        seqMap[keySequence].insert(item);
    }

    // Now, mark all conflicting sequences
    foreach (QKeySequence sequence, seqMap.keys()) {
        QSet<QTreeWidgetItem *> itemSet = seqMap[sequence];
        if (itemSet.size() > 1) {
            //mark items as conflicted items
            foreach(QTreeWidgetItem *item, itemSet.values()) {
                QFont font = item->font(COL_SHORTCUT);
                font.setBold(true);
                item->setForeground(COL_SHORTCUT, QColor(Qt::red));
                item->setFont(COL_SHORTCUT, font);
            }
        }
        else {
            // Mark as non-confilicted
            foreach(QTreeWidgetItem *item, itemSet.values())
            {
                QFont font = item->font(COL_SHORTCUT);
                font.setBold(false);
                item->setForeground(COL_SHORTCUT, QColor(Qt::black));
                item->setFont(COL_SHORTCUT, font);
            }
        }
    }
}
