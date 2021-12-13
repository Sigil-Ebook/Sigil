/************************************************************************
 **
 **  Copyright (C) 2020-2021 Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QString>
#include <QList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QApplication>
#include <QtConcurrent>
#include <QFuture>

#include <QDebug>

#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "EmbedPython/PythonRoutines.h"
#include "Dialogs/RepoLog.h"
#include "Dialogs/ManageRepos.h"

static const QString SETTINGS_GROUP = "manage_repos";

ManageRepos::ManageRepos(QWidget *parent)
    : QDialog(parent),
      m_repoList(QStringList())
{
    ui.setupUi(this);
    ReadSettings();
    InitializeTable();
    ConnectSignalsToSlots();
}

ManageRepos::~ManageRepos()
{
    WriteSettings();
}

void ManageRepos::SetRepoList()
{
    QDir  d(Utility::DefinePrefsDir() + "/repo");
    if (!d.exists()) {
        return;
    }
    // get the directory names in this directory that start with "epub_"
    m_repoList = d.entryList(QStringList("epub_*"), QDir::Dirs|QDir::NoDotAndDotDot);
}

QStringList ManageRepos::GetBookInfo(const QString& reponame)
{
    QStringList bookinfo;
    QString infopath = Utility::DefinePrefsDir() + "/repo/" + reponame + "/.bookinfo";
    QFileInfo fi(infopath);
    if (fi.exists() && fi.isFile() && fi.isReadable()) {
        QString data = Utility::ReadUnicodeTextFile(infopath);
        bookinfo = data.split("\n");
    }
    return bookinfo;
}

void ManageRepos::InitializeTable()
{
    m_repoList.clear();
    SetRepoList();
    int nrows = 0;
    // clear out the table but do NOT clear out column headings
    while (ui.repoTable->rowCount() > 0) {
        ui.repoTable->removeRow(0);
    }
    foreach(QString rp, m_repoList) {
        QStringList fields = GetBookInfo(rp);
        if (!fields.isEmpty() && (fields.count() >= 5)) {
            ui.repoTable->insertRow(nrows);
            SetRepoTableRow(fields,nrows);
            nrows++;
        }
    }
    ui.repoTable->resizeColumnsToContents();
    ui.repoTable->setSortingEnabled(true);
}

void ManageRepos::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void ManageRepos::SetRepoTableRow(const QStringList &fields, int row)
{
    bool sortingOn = ui.repoTable->isSortingEnabled();
    ui.repoTable->setSortingEnabled(false);
    ui.repoTable->setItem(row,ManageRepos::FileField, new QTableWidgetItem(fields.at(ManageRepos::FileField)));
    ui.repoTable->setItem(row,ManageRepos::TitleField, new QTableWidgetItem(fields.at(ManageRepos::TitleField)));
    ui.repoTable->setItem(row,ManageRepos::ModifiedField, new QTableWidgetItem(fields.at(ManageRepos::ModifiedField)));
    ui.repoTable->setItem(row,ManageRepos::VersionField, new QTableWidgetItem(fields.at(ManageRepos::VersionField)));
    ui.repoTable->setItem(row,ManageRepos::UUIDField, new QTableWidgetItem(fields.at(ManageRepos::UUIDField)));
    ui.repoTable->setSortingEnabled(sortingOn);
}

void ManageRepos::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}

void ManageRepos::RepoSelected(int row, int col)
{
    ui.repoTable->setCurrentCell(row, col);
}

void ManageRepos::ShowLog()
{
    // limited to work with one selection at a time to prevent row mixup upon removal
    QList<QTableWidgetItem *> itemlist = ui.repoTable->selectedItems();
    if (itemlist.isEmpty()) {
        Utility::DisplayStdWarningDialog(tr("Nothing is Selected."));
        return;
    }
    int row = ui.repoTable->row(itemlist.at(0));
    QString bookid = ui.repoTable->item(row, ManageRepos::UUIDField)->text();
    QString localRepo = Utility::DefinePrefsDir() + "/repo/";

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // generate the repo log using python in a separate thread since this
    // may take a while depending on the speed of the filesystem
    PythonRoutines pr;
    QFuture<QString> future =
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QtConcurrent::run(&pr,
                          &PythonRoutines::GenerateRepoLogSummaryInPython,
                          localRepo,
                          bookid);
#else
        QtConcurrent::run(&PythonRoutines::GenerateRepoLogSummaryInPython,
                          &pr,
                          localRepo,
                          bookid);
#endif
    future.waitForFinished();
    QString logData = future.result();

    QApplication::restoreOverrideCursor();

    RepoLog log(tr("Repository Log"), logData, this);
    log.exec();
}


void ManageRepos::RemoveRepo()
{
    // limited to work with one selection at a time to prevent row mixup upon removal
    QList<QTableWidgetItem *> itemlist = ui.repoTable->selectedItems();
    if (itemlist.isEmpty()) {
        Utility::DisplayStdWarningDialog(tr("Nothing is Selected."));
        return;
    }
    ui.repoTable->setSortingEnabled(false);
    int row = ui.repoTable->row(itemlist.at(0));
    QString bookid = ui.repoTable->item(row, ManageRepos::UUIDField)->text();
    QString reponame = "epub_" + bookid;
    QString repopath = Utility::DefinePrefsDir() + "/repo/" + reponame;
    bool success = Utility::removeDir(repopath);
    if (success) {
        m_repoList.removeOne(reponame);
        ui.repoTable->removeRow(row);
    } else {
        qDebug() << "Error removing Repo: " << repopath;
    }
    ui.repoTable->resizeColumnsToContents();
    ui.repoTable->setSortingEnabled(true);
}

void ManageRepos::RemoveAllRepos()
{
    QMessageBox msgBox;

    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    msgBox.setWindowTitle(tr("Remove All Repositories"));
    msgBox.setText(tr("Are you sure sure you want to remove all checkpoint repositories?"));
    QPushButton *yesButton = msgBox.addButton(QMessageBox::Yes);
    QPushButton *noButton  = msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(noButton);
    msgBox.exec();
    if (msgBox.clickedButton() == yesButton) {
        foreach(QString reponame, m_repoList) {
            QString repopath = Utility::DefinePrefsDir() + "/repo/" + reponame;
            Utility::removeDir(repopath);
        }
        InitializeTable();
    }
}

void ManageRepos::ConnectSignalsToSlots()
{
    connect(ui.logButton,       SIGNAL(clicked()),                  this, SLOT(ShowLog()));
    connect(ui.removeButton,    SIGNAL(clicked()),                  this, SLOT(RemoveRepo()));
    connect(ui.removeAllButton, SIGNAL(clicked()),                  this, SLOT(RemoveAllRepos()));
    connect(ui.repoTable,       SIGNAL(cellDoubleClicked(int,int)), this, SLOT(RepoSelected(int,int)));
}
