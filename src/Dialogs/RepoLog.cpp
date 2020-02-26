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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QtConcurrent>
#include <QFuture>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/PythonRoutines.h"
#include "Dialogs/RepoLog.h"


static const QString SETTINGS_GROUP = "repo_log";

RepoLog::RepoLog(const QString& localRepo, const QString& bookid, QWidget *parent)
    : QDialog(parent),
      m_bookid(bookid),
      m_localRepo(localRepo),
      m_LogData(""),
      m_ready(false)

{
    ui.setupUi(this);
    // need fixed width font
    QFont font = ui.textEdit->font();
    font.setFamily("Courier New");
    font.setStyleHint(QFont::TypeWriter);
    ui.textEdit->setFont(font);
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

int RepoLog::exec()
{
    SettingsStore settings;
    m_ready = false;
    ui.textEdit->clear();
    ui.textEdit->setOverwriteMode(true);
    ui.textEdit->setVisible(false);

    // FIXME: need status info or wait cursor here

    // generate the repo log using python in a separate thread since this
    // may take a while depending on the speed of the filesystem
    PythonRoutines pr;
    QFuture<QString> future = QtConcurrent::run(&pr, 
						&PythonRoutines::GenerateRepoLogSummaryInPython,
						m_localRepo,
						m_bookid);
    future.waitForFinished();
    QString m_LogData = future.result();
    m_ready = true;
    ui.textEdit->setPlainText(m_LogData);
    // ui.textEdit->insertPlainText(newbytedata);
    // ui.textEdit->append(tr("Launcher process crashed"));
    showConsole();
    ui.okButton->setEnabled(true);
    return QDialog::exec();
}

void RepoLog::showConsole()
{
    ui.textEdit->setVisible(true);
    // resize(789, 550);
}


// should cover both escape key use and using x to close the runner dialog
void RepoLog::reject()
{
    // qDebug() << "in reject";
    QDialog::reject();
}

void RepoLog::connectSignalsToSlots()
{
    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
}
