/****************************************************************************
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

#ifndef EMPTYLAYOUT_H
#define EMPTYLAYOUT_H

#include <QString>
#include <QDialog>
#include <QWidget>
#include <QModelIndex>
#include "Misc/TempFolder.h"

#include "ui_EmptyLayout.h"

class QFileSystemModel;

class EmptyLayout : public QDialog, private Ui::EmptyLayout
{
    Q_OBJECT

public:
    EmptyLayout(const QString &version, QWidget *parent = 0);

    ~EmptyLayout() { };

    static QStringList GetPathsToFilesInFolder(const QString&fullfolderpath, const QString &basepath);

    QStringList GetBookPaths() { return m_BookPaths; };

public slots:
    void fileWasRenamed(const QString& apath, const QString &oldname, const QString &newname);
    void updateActions();

protected slots:
    void reject();

private slots:
    void deleteCurrent();
    void addFolder();
    void addFile(QAction * act);
    void renameCurrent();
    void saveData();

 private:
    void ReadSettings();
    void WriteSettings();
    void setupMarkersMenu();
    QString GetInput(const QString& title, const QString& prompt, const QString& initvalue);

    QFileSystemModel * m_fsmodel;
    QMenu* m_filemenu;
    TempFolder m_TempFolder;
    QString m_MainFolder;
    QString m_EpubVersion;
    QStringList m_BookPaths;
};

#endif // EMPTYLAYOUT_H
