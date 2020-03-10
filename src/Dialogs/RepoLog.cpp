/************************************************************************
 **
 **  Copyright (C) 2020 Kevin B. Hendricks, Stratford Ontario Canada
 **  Copyright (C) 2020 Doug Massay
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
#include <Qt>
#include <QString>
#include <QMessageBox>
#include <QPlainTextEdit>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Dialogs/RepoLog.h"


static const QString SETTINGS_GROUP = "repo_log";

RepoLog::RepoLog(const QString& logdata, const QString& title, QWidget *parent)
    : QDialog(parent),
      m_LogData(logdata)
{
    setAttribute(Qt::WA_DeleteOnClose,true);
    ui.setupUi(this);
    // need fixed width font for diff stats bar graphs to show properly
    QFont font = ui.textEdit->font();
    font.setFamily("Courier New");
    font.setStyleHint(QFont::TypeWriter);
    ui.textEdit->setFont(font);
    ui.label->setText(title);
    ReadSettings();
    connectSignalsToSlots();
}

RepoLog::~RepoLog()
{
    WriteSettings();
}

void RepoLog::ReadSettings()
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

void RepoLog::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void RepoLog::show()
{
    ui.okButton->setEnabled(true);
    ui.textEdit->clear();
    ui.textEdit->setOverwriteMode(true);
    ui.textEdit->setVisible(true);
    ui.textEdit->setPlainText(m_LogData);
    return QDialog::show();
}

int RepoLog::exec()
{
    ui.okButton->setEnabled(true);
    ui.textEdit->clear();
    ui.textEdit->setOverwriteMode(true);
    ui.textEdit->setVisible(true);
    ui.textEdit->setPlainText(m_LogData);
    return QDialog::exec();
}

// should cover both escape key use and using x to close the runner dialog
void RepoLog::reject()
{
    QDialog::reject();
}

void RepoLog::connectSignalsToSlots()
{
    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
}
