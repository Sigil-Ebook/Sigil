/************************************************************************
**
**  Copyright (C) 2024 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QLineEdit>

#include "Dialogs/SemanticTargetID.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "semantic_target_id";

SemanticTargetID::SemanticTargetID(HTMLResource *html_resource, QWidget *parent)
    :
    QDialog(parent),
    m_SelectedText(""),
    m_HTMLResource(html_resource)
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
    SetList();
}

void SemanticTargetID::SetList()
{
    QString BookPath = m_HTMLResource->GetRelativePath();
    QString xhtmlsrc = m_HTMLResource->GetText();
    QStringList ids = XhtmlDoc::GetAllDescendantIDs(xhtmlsrc);
    ui.id->addItem(BookPath);
    foreach(QString id, ids) {
        ui.id->addItem(id);
    }
    ui.id->setEditText(BookPath);
}


QString SemanticTargetID::GetID()
{
    return m_SelectedText;
}

void SemanticTargetID::SetSelectedText()
{
    QString tgt = ui.id->currentText();
    if (tgt == m_HTMLResource->GetRelativePath()) tgt = "";
    m_SelectedText = tgt;
}

void SemanticTargetID::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}

void SemanticTargetID::WriteSettings()
{
    SetSelectedText();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void SemanticTargetID::connectSignalsSlots()
{
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
}
