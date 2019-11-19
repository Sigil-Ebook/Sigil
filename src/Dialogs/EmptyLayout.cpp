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

#include <QDialog>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QtGui>
#include <QString>
#include <QFileInfo>
#include <QTextStream>
#include <QDate>
#include <QFileSystemModel>
#include <QTreeView>
#include <QModelIndex>
#include <QDesktopWidget>
#include <QDir>
#include <QApplication>
#include <QListWidget>
#include <QObject>
#include <QAbstractButton>
#include <QShortcut>
#include <QKeySequence>
#include <QMessageBox>
#include <QDebug>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

#include "Dialogs/EmptyLayout.h"


static const QString SETTINGS_GROUP = "empty_epub_layout";


EmptyLayout::EmptyLayout(const QString &epubversion, QWidget *parent)
  : QDialog(parent),
    m_MainFolder(m_TempFolder.GetPath()),
    m_EpubVersion(epubversion),
    m_BookPaths(QStringList()),
    m_hasOPF(false),
    m_hasNCX(false),
    m_hasNAV(false)
{
    setupUi(this);
    m_filemenu = new QMenu(this);

    // make target root folder
    QDir folder(m_MainFolder);
    folder.mkdir("EpubRoot");

    // initialize QFileSystemModel to point to our TempFolder
    m_fsmodel = new QFileSystemModel();
    m_fsmodel->setReadOnly(false);
    m_fsmodel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    m_fsmodel->setRootPath(m_MainFolder);

    // initialize QTreeView for our model
    view->setModel(m_fsmodel);
    const QModelIndex rootIndex = m_fsmodel->index(QDir::cleanPath(m_MainFolder));
    if (rootIndex.isValid()) {
        view->setRootIndex(rootIndex);
    }
    view->setAnimated(false);
    view->setIndentation(20);
    view->setSortingEnabled(true);
    const QSize availableSize = QApplication::desktop()->availableGeometry(view).size();
    view->resize(availableSize / 2);
    view->setColumnWidth(0, view->width() / 3);
    view->setWindowTitle(QObject::tr("Custom Epub Layout Designer"));
    view->setRootIsDecorated(true);
    // column 0 is name, 1 is size, 2 is kind, 3 is date modified
    view->hideColumn(1);
    view->hideColumn(3);
    view->setHeaderHidden(false);
    // do not allow inline file folder name editing
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if (!isVisible()) {
        ReadSettings();
    }

    // Set up a popup menu with allowed file types
    setupMarkersMenu();

    // the button takes over management of this qmenu
    addFileButton->setMenu(m_filemenu);
    
    // connect signals to slots
    connect(delButton,     SIGNAL(clicked()),           this, SLOT(deleteCurrent()));
    connect(addButton,     SIGNAL(clicked()),           this, SLOT(addFolder()));
    connect(renameButton,  SIGNAL(clicked()),           this, SLOT(renameCurrent()));
    connect(buttonBox,     SIGNAL(accepted()),          this, SLOT(saveData()));
    connect(buttonBox,     SIGNAL(rejected()),          this, SLOT(reject()));
    connect(m_filemenu,    SIGNAL(triggered(QAction*)), this, SLOT(addFile(QAction*)));

    connect(view->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), 
            this, SLOT(updateActions()));

    connect(m_fsmodel, SIGNAL(fileRenamed(const QString&, const QString&, const QString&)),
	    this, SLOT(fileWasRenamed(const QString&, const QString&, const QString&)));

    // assign basic shortcuts
    delButton->     setShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Delete));
    addButton->     setShortcut(QKeySequence("Ctrl+Shift+D"));
    renameButton->  setShortcut(QKeySequence("Ctrl+Shift+F2"));
    addFileButton-> setShortcut(QKeySequence("Ctrl+Shift+F"));

    view->show();
    view->setCurrentIndex(m_fsmodel->index(m_MainFolder + "/EpubRoot"));
    updateActions();
}


EmptyLayout::~EmptyLayout()
{
    // to prevent errors with Windows fs watchers
    // delete the model first *before* 
    // m_TmpFolder destructor is invoked.
    if (m_fsmodel) delete m_fsmodel;
}


void EmptyLayout::setupMarkersMenu()
{
    // ftypes and fmarks should be kept in sync
    QStringList FTypes = QStringList() << QT_TR_NOOP("Xhtml files") << QT_TR_NOOP("Style files") 
                                    << QT_TR_NOOP("Image files") << QT_TR_NOOP("Font files") 
                                    << QT_TR_NOOP("Audio files") << QT_TR_NOOP("Video files") 
                                    << QT_TR_NOOP("Javascript files") << QT_TR_NOOP("Misc files")
                                    << QT_TR_NOOP("OPF file") << QT_TR_NOOP("NCX file") 
                                    << QT_TR_NOOP("Nav file");
    
    QStringList FMarks = QStringList() << "marker.xhtml" << "marker.css" 
                                    << "marker.jpg" << "marker.otf" << "marker.mp3" 
                                    << "marker.mp4" << "marker.js" << "marker.xml" 
                                    << "content.opf" <<"toc.ncx" << "nav.xhtml"; 
    QAction * act;
    int i = 0;
    foreach(QString filetype, FTypes) {
        QString mark = FMarks.at(i++);
        if (!m_EpubVersion.startsWith("3") && ((mark == "marker.js") || (mark == "nav.xhtml"))) continue;
        act = m_filemenu->addAction(tr(filetype.toUtf8().constData()));
        act->setData(mark);
    }
}


void EmptyLayout::fileWasRenamed(const QString &apath, const QString &oldname, const QString &newname)
{
    qDebug() << "Signal a file was renamed " << apath << oldname << newname;
}


QString EmptyLayout::GetInput(const QString& title, const QString& prompt, const QString& initvalue)
{
    QString result;
    QInputDialog dinput;
    dinput.setWindowTitle(title);
    dinput.setLabelText(prompt);
    dinput.setTextValue(initvalue);
    if (dinput.exec()) {
        result = dinput.textValue();
    }
    return result;
}


void EmptyLayout::addFolder()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    if (!index.isValid()) return;
    if (m_fsmodel->isDir(index)) {
        QString newname = GetInput(tr("Add a Folder"), tr("New Folder Name?"), tr("untitled_folder"));
	if (newname.isEmpty()) return;
        m_fsmodel->mkdir(index, newname);
    }
    view->expand(index);
    updateActions();
}


void EmptyLayout::addFile(QAction * act)
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QString filedata = act->data().toString();
    if (!index.isValid()) return;
    if (m_fsmodel->isDir(index)) {
        QString fpath = m_fsmodel->filePath(index) + "/" + filedata;
	QFile afile(fpath);
	if (afile.open(QFile::WriteOnly)) afile.close(); 
        if (filedata == "content.opf") m_hasOPF=true;
        if (filedata == "toc.ncx") m_hasNCX=true;
        if (filedata == "nav.xhtml") m_hasNAV=true;
	QFileInfo file_info = m_fsmodel->fileInfo(m_fsmodel->index(fpath));
    }
    view->expand(index);
    updateActions();
}


void EmptyLayout::renameCurrent()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    if (!index.isValid()) return;
    QString dpath = m_fsmodel->filePath(index.parent());
    QString current_name = m_fsmodel->fileName(index);
    if (current_name == "EpubRoot") return;
    if (current_name.startsWith("marker.")) return;
    if (m_fsmodel->isDir(index)) {
        QString newname = GetInput(tr("Rename a Folder"), tr("New Name for Folder?"), current_name);
	if (newname.isEmpty()) return;
        if ((newname != "EpubRoot") && (newname != current_name)) {
	    QDir folder(dpath);
	    bool success = folder.rename(current_name, newname);
	    if (!success) qDebug() << "folder rename failed";
	}
        view->expand(index);
    } else {
        // renaming a file
	QFileInfo fi = m_fsmodel->fileInfo(index);
        QString newname = GetInput(tr("Rename a File"), tr("New Name for File?"), fi.baseName());
	if (newname.isEmpty()) return;
	newname = newname + "." + fi.suffix();
	if (newname != current_name) {
	    QDir folder(dpath);
	    bool success = folder.rename(current_name, newname);
	    if (!success) qDebug() << "file rename failed";
	}
        view->expand(index.parent());
    }
    updateActions();
}


void EmptyLayout::deleteCurrent()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    if (!index.isValid()) return;
    QString current_name = m_fsmodel->fileName(index);
    if (current_name == "EpubRoot") return;
    if (m_fsmodel->isDir(index)) {
       bool success = m_fsmodel->remove(index);
       if (!success) qDebug() << "folder removal failed";
       view->expand(index);
    } else {
       QModelIndex parent = index.parent();
       bool success = m_fsmodel->remove(index);
       if (success) {
	   if (current_name.endsWith(".opf")) m_hasOPF = false;
           if (current_name.endsWith(".ncx")) m_hasNCX = false;
	   if (!current_name.startsWith("marker.") && current_name.endsWith(".xhtml")) m_hasNAV = false;
       }
       if (!success) qDebug() << "file removal failed";
       view->expand(parent);
    }
    updateActions();
}


void EmptyLayout::saveData()
{
    QString fullfolderpath = m_MainFolder + "/EpubRoot";
    QString basepath = fullfolderpath;
    QStringList bookpaths = GetPathsToFilesInFolder(fullfolderpath, basepath);

    // perform simple sanity check
    int numopf = 0; int numtxt = 0;
    int numcss = 0; int numimg = 0;
    int numncx = 0; int numnav = 0;
    foreach(QString apath, bookpaths) {
        if (apath.endsWith(".opf")) numopf++;
        if (apath.endsWith("marker.xhtml")) numtxt++;
        if (apath.endsWith("marker.css")) numcss++;
        if (apath.endsWith("marker.jpg")) numimg++;
        if (apath.endsWith(".ncx")) numncx++;
        if (apath.endsWith(".xhtml") && !apath.contains("marker.xhtml")) numnav++;
    }
    QStringList Errors;
    if (numopf != 1) Errors << tr("A single OPF file is required.");
    if (numtxt < 1)  Errors << tr("At least one xhtml marker must exist.");
    if (numimg < 1)  Errors << tr("At least one image marker must exist.");
    if (numcss < 1)  Errors << tr("At least one css marker must exist.");
    if (m_EpubVersion.startsWith("2")) {
        if (numncx != 1) Errors << tr("A single NCX file is required.");
    } else {
        if (numnav != 1) Errors << tr("A single NAV file is required.");
    }
    if (!Errors.isEmpty()) {
        QString error_message = Errors.join('\n');
        QMessageBox::warning(this, tr("Errors Detected"), error_message, QMessageBox::Ok);
        return;
    }
    m_BookPaths = bookpaths;

    // allow the user to set this layout as Sigil's default empty epub layout
    bool make_default = QMessageBox::Yes == QMessageBox::warning(this, tr("Sigil"),
				   tr("Do you want to set this layout as the default empty "
				      "Epub layout for Sigil?\n\n"),
				   QMessageBox::Yes|QMessageBox::No);

    if (make_default) {
        // create a sigil_empty_epub.ini file in Sigil Preferences folder
        QString empty_epub_ini_path = Utility::DefinePrefsDir() + "/" + "sigil_empty_epub.ini";
	SettingsStore ss(empty_epub_ini_path);
	const QString SETTINGS_GROUP = "bookpaths";
	const QString KEY_BOOKPATHS = SETTINGS_GROUP + "/" + "empty_epub_bookpaths";
	while (!ss.group().isEmpty()) {
	    ss.endGroup();
	}
	ss.setValue(KEY_BOOKPATHS, bookpaths);
    }

    WriteSettings();

    // Windows has issues removing or deleting files while the file
    // watcher is running and QFileSystemModel made private disabling the watcher
    // So try manually removing the EpubRoot folder via the QFileSystemModel
    QModelIndex index = m_fsmodel->index(m_MainFolder + "/EpubRoot");
    if (index.isValid()) {
        m_fsmodel->remove(index);
    }
    QDialog::accept();
}


void EmptyLayout::reject()
{
    // Windows has issues removing or deleting files while the file
    // watcher is running and QFileSystemModel made private disabling the watcher
    // So try manually removing the EpubRoot folder via the QFileSystemModel
    QModelIndex index = m_fsmodel->index(m_MainFolder + "/EpubRoot");
    if (index.isValid()) {
        m_fsmodel->remove(index);
    }
    WriteSettings();
    m_BookPaths = QStringList();
    QDialog::reject();
}


void EmptyLayout::updateActions()
{
    bool hasSelection = !view->selectionModel()->selection().isEmpty();
    QModelIndex index = view->selectionModel()->currentIndex();
    bool hasCurrent = index.isValid();
    QString name = "";
    if (hasCurrent) {
	name = m_fsmodel->filePath(index).split("/").last();
    }
    bool isFile = name.startsWith("marker.") || name.endsWith(".opf") || name.endsWith(".ncx");
    bool isEpubRoot = name == "EpubRoot";
    bool isMarker = name.startsWith("marker.");
    bool isOPFNCXNAV = name.endsWith(".opf") || name.endsWith(".ncx") || (name.endsWith(".xhtml") && !isMarker);
    delButton->setEnabled(hasSelection && !isEpubRoot);
    addButton->setEnabled(hasSelection && !isMarker && !isOPFNCXNAV);
    renameButton->setEnabled(hasSelection && !isEpubRoot && !isMarker);
    addFileButton->setEnabled(hasSelection && !isFile);

    // finally enable and disable file marker menu items
    QList<QAction *> menuacts = m_filemenu->actions();
    foreach(QAction * act, menuacts) {
        bool enable = true;
        QString filedata = act->data().toString();
	if ((filedata == "content.opf") && m_hasOPF) enable = false;
	if ((filedata == "toc.ncx") && m_hasNCX) enable = false;
	if ((filedata == "nav.xhtml") && m_hasNAV) enable = false;
	act->setEnabled(enable);
    }
}


void EmptyLayout::ReadSettings()
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


void EmptyLayout::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}


QStringList EmptyLayout::GetPathsToFilesInFolder(const QString &fullfolderpath, const QString &basepath)
{
    QDir folder(fullfolderpath);
    QStringList paths;
    foreach(QFileInfo fi, folder.entryInfoList()) {
        if ((fi.fileName() != ".") && (fi.fileName() != "..")) {
	    if (fi.isFile()) {
	        QString filepath = fi.absoluteFilePath();
	        QString bookpath = filepath.right(filepath.length() - basepath.length() - 1);
	        paths.append(bookpath);
	    } else {
	        paths.append(GetPathsToFilesInFolder(fi.absoluteFilePath(), basepath));
	    }
        }
    }
    return paths;
}
