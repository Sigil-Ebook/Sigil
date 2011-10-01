/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
**  Copyright (C) 2011 Gary Jonston  <gary.jonston@gmail.com>
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
#include "KeyboardShortcutsWidget.h"
#include "Misc/KeyboardShortcutManager.h"

const int SEQ_NAME_INDEX = 0;
const int SEQ_DESCRIPTION_INDEX = 1;
const int SEQ_SEQUENCE_INDEX = 2;
const int SEQ_DEFAULT_SEQUENCE_INDEX = 3;

KeyboardShortcutsWidget::KeyboardShortcutsWidget()
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;

    ui.setupUi(this);
    connectSignalsSlots();

    readSettings();
}

void KeyboardShortcutsWidget::saveSettings()
{
     KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

     for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
         QTreeWidgetItem * item = ui.commandList->topLevelItem(i);

         const QString id = item->text(SEQ_NAME_INDEX);
         const QKeySequence keySequence = item->text(SEQ_SEQUENCE_INDEX);

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
        // for shortcut overrides, we need to accept as well
        event->accept();
        return true;
    }

    return false;
}

void KeyboardShortcutsWidget::showEvent(QShowEvent *event)
{
    //fill out the tree view
    readSettings();
    ui.commandList->setColumnWidth(0, ui.commandList->width() * .30);
    ui.commandList->setColumnWidth(1, ui.commandList->width() * .40);
    ui.commandList->setColumnWidth(2, ui.commandList->width() * .15);
    ui.commandList->setColumnWidth(3, ui.commandList->width() * .15);
    QWidget::showEvent(event);
}

void KeyboardShortcutsWidget::treeWidgetItemActivatedSlot(QTreeWidgetItem *item, int column)
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    ui.targetEdit->setText(ui.commandList->currentItem()->text(SEQ_SEQUENCE_INDEX));
}

void KeyboardShortcutsWidget::clearButtonClicked()
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    ui.targetEdit->setText("");
    ui.commandList->currentItem()->setText(SEQ_SEQUENCE_INDEX, "");
    ui.targetEdit->setFocus();
}

void KeyboardShortcutsWidget::filterEditTextChangedSlot(const QString &text)
{
    const QString newText = text.toUpper();
    //if text is empty - show all items.
    if (text.isEmpty()) {
        for(int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
             ui.commandList->topLevelItem(i)->setHidden(false);
        }
    }
    else {
        for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
            const QString name = ui.commandList->topLevelItem(i)->text(SEQ_NAME_INDEX).toUpper();
            const QString description = ui.commandList->topLevelItem(i)->text(SEQ_DESCRIPTION_INDEX).toUpper();
            const QString sequence = ui.commandList->topLevelItem(i)->text(SEQ_SEQUENCE_INDEX).toUpper();
            const QString defaultSequence = ui.commandList->topLevelItem(i)->text(SEQ_DEFAULT_SEQUENCE_INDEX).toUpper();

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

        //get 'current' sequence value from CommandManger
        QKeySequence seq = sm->keyboardShortcut(ui.commandList->currentItem()->text(SEQ_NAME_INDEX)).keySequence();
        //assign sequence value from CommandManager to the currentItem
        ui.commandList->currentItem()->setText(SEQ_SEQUENCE_INDEX, seq);
        ui.targetEdit->setText(seq);
        ui.targetEdit->setFocus();
        updateItemView(ui.commandList->currentItem());
    }
}

void KeyboardShortcutsWidget::resetAllButtonClickedSlot()
{
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

    //Go through all items
    for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui.commandList->topLevelItem(i);
        QKeySequence seq = sm->keyboardShortcut(item->text(SEQ_NAME_INDEX)).keySequence();
        item->setText(SEQ_SEQUENCE_INDEX, seq);
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
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(SEQ_NAME_INDEX, id);
        item->setText(SEQ_DESCRIPTION_INDEX, shortcut.description());
        item->setText(SEQ_SEQUENCE_INDEX, shortcut.keySequence());
        item->setText(SEQ_DEFAULT_SEQUENCE_INDEX, shortcut.defaultKeySequence());
        ui.commandList->addTopLevelItem(item);
    }

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
    if (m_keyNum > MAX_KEYS_IN_SHORTCUT - 1 ||
         nextKey == Qt::Key_Control ||
         nextKey == Qt::Key_Shift   ||
         nextKey == Qt::Key_Meta    ||
         nextKey == Qt::Key_Alt     ||
         nextKey == Qt::Key_Backspace || //this button cannot be assigned, because we want to 'clear' shortcut after backspace push
         ui.commandList->currentItem() == 0 //do not allow writting in shortcut line edit if no item is selected
        )
    {
        //if key was Qt::Key_Backspace additionaly clear sequence dialog
        if(nextKey == Qt::Key_Backspace) {
            clearButtonClicked();
        }
        return;
    }

    nextKey |= translateModifiers(event->modifiers(), event->text());
    switch (m_keyNum) {
    case 0:
        m_key[0] = nextKey;
        break;
    case 1:
        m_key[1] = nextKey;
        break;
    case 2:
        m_key[2] = nextKey;
        break;
    case 3:
        m_key[3] = nextKey;
        break;
    default:
        break;
    }
    m_keyNum++;
    QKeySequence ks;
    if(MAX_KEYS_IN_SHORTCUT == 1) {
        ks = QKeySequence(m_key[0]);
    }
    else if (MAX_KEYS_IN_SHORTCUT == 2) {
        ks = QKeySequence(m_key[0], m_key[1]);
    }
    else if (MAX_KEYS_IN_SHORTCUT == 3) {
        ks = QKeySequence(m_key[0], m_key[1], m_key[2]);
    }
    else if (MAX_KEYS_IN_SHORTCUT == 4) {
        ks = QKeySequence(m_key[0], m_key[1], m_key[2], m_key[3]);
    }

    ui.targetEdit->setText(ks);
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
    const QString seqName = item->text(SEQ_NAME_INDEX);
    const QKeySequence seqValue = item->text(SEQ_SEQUENCE_INDEX);
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

    QKeySequence seq = sm->keyboardShortcut(seqName).keySequence();
    QFont font = item->font(SEQ_SEQUENCE_INDEX);
    //If item is 'modified' show it in 'italics'
    if ( seq != seqValue ) {
        font.setItalic( true );
    }
    else {
        font.setItalic( false );
    }
    item->setFont(SEQ_SEQUENCE_INDEX, font);
}

void KeyboardShortcutsWidget::updateCurrentItemShortcut()
{
    if (ui.commandList->currentItem() != 0) {
        ui.commandList->currentItem()->setText(SEQ_SEQUENCE_INDEX, ui.targetEdit->text());
        updateItemView(ui.commandList->currentItem());
    }
}

void KeyboardShortcutsWidget::markSequencesAsDuplicatedIfNeeded()
{
    //this is not optized , but since this will be called rather seldom
    //effort for optimization may not be worth it

    QMap<QKeySequence, QSet<QTreeWidgetItem*> > seqMap;

    //Got through all items
    for (int i = 0; i < ui.commandList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui.commandList->topLevelItem(i);

        const QKeySequence keySequence = item->text(SEQ_SEQUENCE_INDEX);
        seqMap[keySequence].insert(item);
    }

    //now, mark all conflicting sequences
    foreach (QKeySequence sequence, seqMap.keys()) {
        QSet<QTreeWidgetItem *> itemSet = seqMap[sequence];
        if (itemSet.size() > 1) {
            //mark items as conflicted items
            foreach(QTreeWidgetItem *item, itemSet.values()) {
                QFont font = item->font(SEQ_SEQUENCE_INDEX);
                font.setBold(true);
                item->setForeground(SEQ_SEQUENCE_INDEX, QColor(Qt::red));
                item->setFont(SEQ_SEQUENCE_INDEX, font);
            }
        }
        else {
            //mark as non-confilicted
            foreach(QTreeWidgetItem *item, itemSet.values())
            {
                QFont font = item->font(SEQ_SEQUENCE_INDEX);
                font.setBold(false);
                item->setForeground(SEQ_SEQUENCE_INDEX, QColor(Qt::black));
                item->setFont(SEQ_SEQUENCE_INDEX, font);
            }
        }
    }
}
