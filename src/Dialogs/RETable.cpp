/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford Ontario Canada
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

#include "Dialogs/RETable.h"
#include "ResourceObjects/Resource.h"
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "re_table";

RETable::RETable(const QList<Resource*> &resources, QString &retext, QString &replacetext, QWidget *parent)
    :
    QDialog(parent),
    m_REText(retext),
    m_ReplaceText(replacetext),
    m_NewNames(QStringList())
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
    SetTable(resources);
}

void RETable::SetTable(const QList<Resource*> &resources)
{
    QStringList bookpaths;
    QStringList newbookpaths;
    QRegularExpression retext = QRegularExpression(m_REText);
    foreach(Resource * resource, resources) {
        QString bkpath = resource->GetRelativePath();
        QString beforefn = bkpath.split('/').last();
	QString afterfn =  beforefn.replace(retext,m_ReplaceText);
	QString basedir = Utility::startingDir(bkpath);
        QString afterpath = afterfn;
        if (!basedir.isEmpty()) afterpath = basedir + "/" + afterfn;
        newbookpaths << afterpath;
        bookpaths << bkpath;
        newnames << afterfn;
    }
    int rows = resources.count();
    ui.tableWidget->setRowCount(rows);
    int r = 0;
    foreach(QString bpath, bookpaths) {
        QString apath = newbookpaths.at(r);
	ui.tableWidget->setItem(r, 0, new QTableWidgetItem(bpath));
        ui.tableWidget->setItem(r, 1, new QTableWidgetItem(apath));
        r++;
    }
    ui.tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui.tableWidget->resizeColumnsToContents();
    ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QStringList RETable::GetNewNames()
{
    return m_NewNames;
}

void RETable::SetResults()
{
    m_NewNames = newnames;
}

void RETable::ReadSettings()
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

void RETable::WriteSettings()
{
    SetResults();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void RETable::connectSignalsSlots()
{
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
}
