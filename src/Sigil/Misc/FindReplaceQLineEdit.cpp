/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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

#include <QtCore/QFile>
#include <QtCore/QSignalMapper>
#include <QtGui/QAbstractItemView>
#include <QtGui/QAction>
#include <QtGui/QCompleter>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>

#include "Misc/FindReplaceQLineEdit.h"
#include "MiscEditors/SearchEditorModel.h"

FindReplaceQLineEdit::FindReplaceQLineEdit(QWidget *parent)
    : QLineEdit(parent),
      m_FindReplace(parent),
      m_searchMapper(new QSignalMapper(this)),
      m_tokeniseEnabled(true)
{
    connect(m_searchMapper, SIGNAL(mapped(const QString&)), m_FindReplace, SLOT(LoadSearchByName(const QString&)));
}
 
FindReplaceQLineEdit::~FindReplaceQLineEdit()
{
}

void FindReplaceQLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();

    QAction *topAction = 0;
    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    if (m_tokeniseEnabled) {
        QAction *tokeniseAction = new QAction(tr("Tokenise Selection"), menu);
        connect(tokeniseAction, SIGNAL(triggered()), m_FindReplace, SLOT(TokeniseSelection()));
        if (topAction) {
            menu->insertAction(topAction, tokeniseAction);
            menu->insertSeparator(topAction);
        }
        else {
            menu->addAction(tokeniseAction);
        }
        topAction = tokeniseAction;
    }

    QAction *saveSearchAction = new QAction(tr("Save Search") % "...", menu);
    connect(saveSearchAction, SIGNAL(triggered()), m_FindReplace, SLOT(SaveSearchAction()));
    menu->insertAction(topAction, saveSearchAction);
    menu->insertSeparator(topAction);
    topAction = saveSearchAction;

    if (CreateMenuEntries(menu, topAction, SearchEditorModel::instance()->invisibleRootItem())) {
        menu->insertSeparator(topAction);
    }

    menu->exec(mapToGlobal(event->pos()));

    delete menu;
}

bool FindReplaceQLineEdit::CreateMenuEntries(QMenu *parent_menu, QAction *topAction, QStandardItem *item)
{
    QAction *searchAction = 0;
    QMenu *group_menu = parent_menu;

    if (!item) {
        return false;
    }

    if (!item->text().isEmpty()) {
        // If item has no children, add entry to the menu, else create menu
        if (!item->data().toBool()) {
            searchAction = new QAction(item->text(), this);
            connect(searchAction, SIGNAL(triggered()), m_searchMapper, SLOT(map()));
            m_searchMapper->setMapping(searchAction, SearchEditorModel::instance()->GetFullName(item));
            if (!topAction) {
                parent_menu->addAction(searchAction);
            }
            else {
                parent_menu->insertAction(topAction, searchAction);
            }
        }
        else {
            group_menu = new QMenu(this);
            group_menu->setTitle(item->text());

            if (topAction) {
                parent_menu->insertMenu(topAction, group_menu);
            }
            else {
                parent_menu->addMenu(group_menu);
            }
            topAction = 0;
        }
    }

    // Recursively add entries for children
    for (int row = 0; row < item->rowCount(); row++) {
        CreateMenuEntries(group_menu, topAction, item->child(row,0));
    }
    return item->rowCount() > 0;
}

bool FindReplaceQLineEdit::isTokeniseEnabled()
{
    return m_tokeniseEnabled;
}

void FindReplaceQLineEdit::setTokeniseEnabled(bool value)
{
    m_tokeniseEnabled = value;
}

bool FindReplaceQLineEdit::event(QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (completer()->popup()->isVisible()) {
            if ( (ke->modifiers() & Qt::AltModifier) || (ke->modifiers() & Qt::ControlModifier) ) {
                // Alt/Control modifier keys while the autocompletion popup is down are swallowed
                // which prevents any actions with keyboard shortcuts from working.
                completer()->popup()->hide();
            }
        }
    }
    return QLineEdit::event(e);
}
