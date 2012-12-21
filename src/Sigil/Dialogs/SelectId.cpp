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

#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>

#include "Dialogs/SelectId.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "select_id";

SelectId::SelectId(QString id, HTMLResource *html_resource, QSharedPointer< Book > book, QWidget *parent)
    :
    QDialog(parent),
    m_SelectedText(id),
    m_HTMLResource(html_resource),
    m_Book(book)
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
    SetList();
}

void SelectId::SetList()
{
    QLineEdit *q = new QLineEdit(this);
    ui.id->setLineEdit(q);
    QCompleter *qc = ui.id->completer();
    qc->setCaseSensitivity(Qt::CaseSensitive);
    ui.id->setCompleter(qc);
    QStringList ids = m_Book->GetIdsInHTMLFile(m_HTMLResource);
    foreach(QString id, ids) {
        ui.id->addItem(id);
    }
    // Set default id name
    ui.id->setEditText(m_SelectedText);
}


QString SelectId::GetId()
{
    return m_SelectedText;
}

void SelectId::SetSelectedText()
{
    m_SelectedText = ui.id->currentText();
}

void SelectId::ReadSettings()
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

void SelectId::WriteSettings()
{
    SetSelectedText();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void SelectId::connectSignalsSlots()
{
    connect(this,         SIGNAL(accepted()),
            this,         SLOT(WriteSettings()));
}
