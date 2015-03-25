/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QFileInfo>
#include <QtCore/QSignalMapper>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QScrollBar>

#include "BookManipulation/Book.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/Metadata.h"
#include "Dialogs/DeleteFiles.h"
#include "Dialogs/RenameTemplate.h"
#include "Importers/ImportHTML.h"
#include "MainUI/BookBrowser.h"
#include "MainUI/MainWindow.h"
#include "MainUI/OPFModel.h"
#include "Misc/FilenameDelegate.h"
#include "Misc/KeyboardShortcutManager.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/OpenExternally.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

static const QString SETTINGS_GROUP = "bookbrowser";
static const QString OPF_NCX_EDIT_WARNING_KEY = SETTINGS_GROUP + "-opfncx-warning";
static const int COLUMN_INDENTATION = 10;

BookBrowser::BookBrowser(QWidget *parent)
    :
    QDockWidget(tr("Book Browser"), parent),
    m_Book(NULL),
    m_TreeView(*new QTreeView(this)),
    m_OPFModel(*new OPFModel(this)),
    m_ContextMenu(*new QMenu(this)),
    m_SemanticsContextMenu(*new QMenu(this)),
    m_FontObfuscationContextMenu(*new QMenu(this)),
    m_OpenWithContextMenu(*new QMenu(this)),
    m_GuideSemanticMapper(*new QSignalMapper(this)),
    m_LastContextMenuType(Resource::GenericResourceType),
    m_RenamedResource(NULL)
{
    m_SemanticsContextMenu      .setTitle(tr("Add Semantics"));
    m_FontObfuscationContextMenu.setTitle(tr("Font Obfuscation"));
    m_OpenWithContextMenu       .setTitle(tr("Open With"));
    setWidget(&m_TreeView);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    ReadSettings();
    SetupTreeView();
    CreateContextMenuActions();
    ConnectSignalsToSlots();
    m_OpenWithContextMenu.addAction(m_OpenWithEditor);
    m_OpenWithContextMenu.addAction(m_OpenWith);
}


BookBrowser::~BookBrowser()
{
    WriteSettings();
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
    sm->removeActionsOf(this);
}

void BookBrowser::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    raise();
}

void BookBrowser::SetBook(QSharedPointer<Book> book)
{
    m_Book = book;
    m_OPFModel.SetBook(book);
    connect(this, SIGNAL(BookContentModified()), m_Book.data(), SLOT(SetModified()));
    ExpandTextFolder();
    RefreshCounts();

    try {
        // Here we fake that the "first" HTML file has been double clicked
        // so that we have a default first tab opened.
        // An exception is thrown if there are no HTML files in the epub.
        EmitResourceActivated(m_OPFModel.GetFirstHTMLModelIndex());
    }
    // No exception variable since we don't use it
    catch (NoHTMLFiles) {
        // Do nothing. No HTML files, no first file opened.
    }
}

void BookBrowser::RefreshCounts()
{
    for (int i = 0; i < m_OPFModel.invisibleRootItem()->rowCount(); i++) {
        QStandardItem *folder = m_OPFModel.invisibleRootItem()->child(i);
        int count = folder->rowCount();
        QString tooltip = QString("%1 ").arg(count);

        if (count == 1) {
            tooltip.append(tr("file"));
        } else {
            tooltip.append(tr("files"));
        }

        folder->setToolTip(tooltip);
    }
}

void BookBrowser::Refresh()
{
    m_OPFModel.Refresh();
    RefreshCounts();
    emit UpdateBrowserSelection();
}


void BookBrowser::SelectAll()
{
    QList<Resource *> resources = m_OPFModel.GetResourceListInFolder(m_LastContextMenuType);
    SelectResources(resources);
}


void BookBrowser::SelectResources(QList<Resource *> resources)
{
    m_TreeView.selectionModel()->clearSelection();
    foreach(Resource * resource, resources) {
        QModelIndex index = m_OPFModel.GetModelItemIndex(*resource, OPFModel::IndexChoice_Current);
        m_TreeView.selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void BookBrowser::SelectRenamedResource()
{
    if (m_RenamedResource == NULL) {
        return;
    }

    // Set the selection to the resource that was being renamed
    UpdateSelection(*m_RenamedResource);
    // Make sure Book Browser has focus so keyboard navigation works as expected
    qobject_cast<QWidget *>(&m_TreeView)->setFocus();
    m_RenamedResource = NULL;
}

void BookBrowser::UpdateSelection(Resource &resource)
{
    const int MIN_SPACE = 40;
    m_TreeView.selectionModel()->clearSelection();
    QModelIndex index = m_OPFModel.GetModelItemIndex(resource, OPFModel::IndexChoice_Current);
    m_TreeView.selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    // Do we need to scroll the TreeView further to ensure the item is easily visible?
    QRect item_rect = m_TreeView.visualRect(index);
    QRect tv_rect = m_TreeView.rect();

    if ((item_rect.top() < MIN_SPACE) || ((tv_rect.bottom() - item_rect.bottom()) < MIN_SPACE)) {
        // Too close to the top or bottom
        m_TreeView.scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void BookBrowser::NextResource()
{
    Resource *resource = GetCurrentResource();

    if (resource != NULL) {
        QModelIndex index = m_OPFModel.GetModelItemIndex(*resource, OPFModel::IndexChoice_Next);

        if (index.isValid()) {
            emit ResourceActivated(*GetResourceByIndex(index));
        }
    }
}

void BookBrowser::PreviousResource()
{
    Resource *resource = GetCurrentResource();

    if (resource != NULL) {
        QModelIndex index = m_OPFModel.GetModelItemIndex(*resource, OPFModel::IndexChoice_Previous);

        if (index.isValid()) {
            emit ResourceActivated(*GetResourceByIndex(index));
        }
    }
}

void BookBrowser::SortHTML()
{
    QList <Resource *> resources = ValidSelectedResources();
    QMessageBox::StandardButton button_pressed;
    button_pressed = QMessageBox::warning(this,
                                          tr("Sigil"), tr("Are you sure you want to sort the selected files alphanumerically?") % "\n" % tr("This action cannot be reversed."),
                                          QMessageBox::Ok | QMessageBox::Cancel
                                         );

    if (button_pressed != QMessageBox::Ok) {
        return;
    }

    QList <QModelIndex> indexList = m_TreeView.selectionModel()->selectedRows(0);
    m_OPFModel.SortHTML(indexList);
    SelectResources(resources);
}

void BookBrowser::RenumberTOC()
{
    emit RenumberTOCContentsRequest();
}

Resource *BookBrowser::GetUrlResource(const QUrl &url)
{
    const QString &filename = QFileInfo(url.path()).fileName();

    try {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename(filename);
        return &resource;
    } catch (ResourceDoesNotExist) {
        Utility::DisplayStdErrorDialog(
            tr("The file \"%1\" does not exist.")
            .arg(filename)
        );
    }

    return NULL;
}


void BookBrowser::EmitResourceActivated(const QModelIndex &index)
{
    QString identifier(m_OPFModel.itemFromIndex(index)->data().toString());

    if (!identifier.isEmpty()) {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByIdentifier(identifier);
        emit ResourceActivated(resource);
    }
}


void BookBrowser::OpenContextMenu(const QPoint &point)
{
    if (!SuccessfullySetupContextMenu(point)) {
        return;
    }

    m_ContextMenu.exec(m_TreeView.viewport()->mapToGlobal(point));
    m_ContextMenu.clear();
    m_SemanticsContextMenu.clear();
    // Ensure any actions with keyboard shortcuts that might have temporarily been
    // disabled are enabled again to let the shortcut work outside the menu.
    m_Delete->setEnabled(true);
    m_Merge->setEnabled(true);
    m_Rename->setEnabled(true);
}

QList <Resource *> BookBrowser::ValidSelectedHTMLResources()
{
    return ValidSelectedResources(Resource::HTMLResourceType);
}

QList <Resource *> BookBrowser::AllHTMLResources()
{
    return m_OPFModel.GetResourceListInFolder(Resource::HTMLResourceType);
}

QList <Resource *> BookBrowser::AllImageResources()
{
    return m_OPFModel.GetResourceListInFolder(Resource::ImageResourceType);
}

QList <Resource *> BookBrowser::AllMediaResources()
{
    QList <Resource *> resources;
    resources = m_OPFModel.GetResourceListInFolder(Resource::ImageResourceType);
    resources.append(m_OPFModel.GetResourceListInFolder(Resource::VideoResourceType));
    resources.append(m_OPFModel.GetResourceListInFolder(Resource::AudioResourceType));
    return resources;
}

QList <Resource *> BookBrowser::AllCSSResources()
{
    return m_OPFModel.GetResourceListInFolder(Resource::CSSResourceType);
}

QList <Resource *> BookBrowser::ValidSelectedResources(Resource::ResourceType resource_type)
{
    QList <Resource *> resources = ValidSelectedResources();
    foreach(Resource * resource, resources) {
        if (resource->Type() != resource_type) {
            resources.clear();
        }
    }
    return resources;
}

QList <Resource *> BookBrowser::ValidSelectedResources()
{
    QList <Resource *> resources;
    Resource::ResourceType resource_type = Resource::HTMLResourceType;

    if (ValidSelectedItemCount() < 1) {
        return resources;
    }

    QList <QModelIndex> list = m_TreeView.selectionModel()->selectedRows(0);
    foreach(QModelIndex index, list) {
        QStandardItem *item = m_OPFModel.itemFromIndex(index);

        if (item != NULL) {
            const QString &identifier = item->data().toString();
            Resource *resource = &m_Book->GetFolderKeeper().GetResourceByIdentifier(identifier);

            if (resource != NULL) {
                resources.append(resource);
                resource_type = resource->Type();
            }
        }
    }

    // TOC and contents are not in a folder and are always just one file
    if (resource_type == Resource::OPFResourceType || resource_type == Resource::NCXResourceType) {
        return resources;
    }

    // Sort according to treeview order
    QList <Resource *> sorted_resources;
    QList <Resource *> all_resources = m_OPFModel.GetResourceListInFolder(resource_type);
    foreach(Resource * all_resource, all_resources) {
        foreach(Resource * resource, resources) {
            if (all_resource->Filename() == resource->Filename()) {
                sorted_resources.append(all_resource);
                break;
            }
        }
    }
    return sorted_resources;
}


int BookBrowser::ValidSelectedItemCount()
{
    QList <QModelIndex> list = m_TreeView.selectionModel()->selectedRows(0);
    int count = list.length();

    if (count <= 1) {
        return count;
    }

    Resource::ResourceType resource_type = Resource::HTMLResourceType;
    foreach(QModelIndex index, list) {
        QStandardItem *item = m_OPFModel.itemFromIndex(index);
        const QString &identifier = item->data().toString();

        // If folder item included, multiple selection is invalid
        if (identifier == NULL) {
            return -1;
        }

        Resource *resource = &m_Book->GetFolderKeeper().GetResourceByIdentifier(identifier);

        // If for some reason there is something with an identifier but no resource, fail
        if (resource == NULL) {
            return -1;
        }

        // Check that multiple selection only contains items of the same type
        if (index == list[0]) {
            resource_type = resource->Type();
        } else if (resource_type != resource->Type()) {
            return -1;
        }
    }
    return count;
}

void BookBrowser::AddFile(QString filepath)
{
    m_Book->GetFolderKeeper().AddContentFileToFolder(filepath);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::AddNew()
{
    if (m_LastContextMenuType == Resource::HTMLResourceType) {
        AddNewHTML();
    } else if (m_LastContextMenuType == Resource::CSSResourceType) {
        AddNewCSS();
    } else if (m_LastContextMenuType == Resource::ImageResourceType) {
        AddNewSVG();
    }
}


// Create a new HTML file and copy the text from the current file into it
void BookBrowser::CopyHTML()
{
    Resource *current_resource = GetCurrentResource();

    if (!current_resource) {
        return;
    }

    HTMLResource &current_html_resource = *qobject_cast<HTMLResource *>(current_resource);
    // Create an empty file
    HTMLResource &new_html_resource = m_Book->CreateEmptyHTMLFile(current_html_resource);
    m_Book->MoveResourceAfter(new_html_resource, current_html_resource);
    // Copy the text from the current file
    new_html_resource.SetText(current_html_resource.GetText());
    // Open the new file in a tab
    emit ResourceActivated(new_html_resource);
    emit BookContentModified();
    Refresh();
}


// Create a new HTML file and insert it after the currently selected file
void BookBrowser::AddNewHTML()
{
    Resource *current_resource = GetCurrentResource();
    HTMLResource &current_html_resource = *qobject_cast<HTMLResource *>(current_resource);
    HTMLResource &new_html_resource = m_Book->CreateEmptyHTMLFile(current_html_resource);

    if (current_resource != NULL) {
        m_Book->MoveResourceAfter(new_html_resource, current_html_resource);
    }

    // Open the new file in a tab
    emit ResourceActivated(new_html_resource);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::CopyCSS()
{
    Resource *current_resource = GetCurrentResource();

    if (!current_resource) {
        return;
    }

    CSSResource &current_css_resource = *qobject_cast<CSSResource *>(current_resource);
    // Create an empty file
    CSSResource &new_resource = m_Book->CreateEmptyCSSFile();
    // Copy the text from the current file
    new_resource.SetText(current_css_resource.GetText());
    // Open the new file in a tab
    emit ResourceActivated(new_resource);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::AddNewCSS()
{
    CSSResource &new_resource = m_Book->CreateEmptyCSSFile();
    // Open the new file in a tab
    emit ResourceActivated(new_resource);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::AddNewSVG()
{
    SVGResource &new_resource = m_Book->CreateEmptySVGFile();
    // Open the new file in a tab
    emit ResourceActivated(new_resource);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::CreateHTMLTOCCSSFile()
{
    CSSResource &css_resource = m_Book->CreateHTMLTOCCSSFile();
    m_OPFModel.RenameResource(css_resource, SGC_TOC_CSS_FILENAME);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::CreateIndexCSSFile()
{
    CSSResource &css_resource = m_Book->CreateIndexCSSFile();
    m_OPFModel.RenameResource(css_resource, SGC_INDEX_CSS_FILENAME);
    emit BookContentModified();
    Refresh();
}

QStringList BookBrowser::AddExisting(bool only_multimedia, bool only_images)
{
    QStringList added_files;

    QString filter_string = "";
    if (!QFileInfo(m_LastFolderOpen).exists()) {
        m_LastFolderOpen = "";
    }

    QStringList filepaths = QFileDialog::getOpenFileNames(this,
                            tr("Add Existing Files"),
                            m_LastFolderOpen,
                            filter_string);

    if (filepaths.isEmpty()) {
        return added_files;
    }

    m_LastFolderOpen = QFileInfo(filepaths.first()).absolutePath();
    // We need to store the current metadata since the
    // GetBook call will clear it.
    QList<Metadata::MetaElement> old_metadata = m_Book->GetMetadata();
    QStringList current_filenames = m_Book->GetFolderKeeper().GetAllFilenames();
    QStringList invalid_filenames;
    HTMLResource *current_html_resource = qobject_cast<HTMLResource *>(GetCurrentResource());
    Resource *open_resource = NULL;
    // Display progress dialog if adding several items
    // Avoid dialog popping up over Insert File from disk for duplicate file all the time
    int progress_value = 0;
    int file_count = filepaths.count();
    QProgressDialog progress(QObject::tr("Adding Existing Files.."), 0, 0, file_count, this);
    if (file_count > 1) {
        progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
        progress.setValue(progress_value);
    }
    foreach(QString filepath, filepaths) {
        if (file_count > 1) {
            // Set progress value and ensure dialog has time to display when doing extensive updates
            // Set ahead of actual add since it can abort in several places
            progress.setValue(progress_value++);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        // Check if the file matches the type requested for adding
        // Only used for inserting images from disk
        if (only_images) {
            if (!IMAGE_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower())) {
                Utility::DisplayStdErrorDialog(
                    tr("File is not an image and cannot be used:\n\n\"%1\".").arg(filepath));
                continue;
            }
        } else if (only_multimedia) {
            if (!IMAGE_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) &&
                !SVG_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) &&
                !VIDEO_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) &&
                !AUDIO_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower())) {
                Utility::DisplayStdErrorDialog(
                    tr("File is not multimedia (image, video, audio) and cannot be inserted:\n\n\"%1\".") .arg(filepath));
                continue;
            }
        }

        QString filename = QFileInfo(filepath).fileName();

        if (current_filenames.contains(filename)) {
            // If this is an image prompt to replace it.
            if (IMAGE_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) ||
                SVG_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) ||
                VIDEO_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) ||
                AUDIO_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower())) {
                QMessageBox::StandardButton button_pressed;
                button_pressed = QMessageBox::warning(this,
                                                      tr("Sigil"), tr("The multimedia file \"%1\" already exists in the book.\n\nOK to replace?").arg(filename),
                                                      QMessageBox::Ok | QMessageBox::Cancel);

                if (button_pressed != QMessageBox::Ok) {
                    continue;
                }

                try {
                    Resource &old_resource = m_Book->GetFolderKeeper().GetResourceByFilename(filename);
                    old_resource.Delete();
                } catch (ResourceDoesNotExist) {
                    Utility::DisplayStdErrorDialog(
                        tr("Unable to delete or replace file \"%1\".")
                        .arg(filename)
                    );
                    continue;
                }
            } else {
                QMessageBox::warning(this,
                                     tr("Sigil"),
                                     tr("Unable to load \"%1\"\n\nA file with this name already exists in the book.")
                                     .arg(filename)
                                    );
                continue;
            }
        }

        if (QFileInfo(filepath).fileName() == "page-map.xml") {
            m_Book->GetFolderKeeper().AddContentFileToFolder(filepath);
        } else if (TEXT_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower())) {
            ImportHTML html_import(filepath);
            XhtmlDoc::WellFormedError error = html_import.CheckValidToLoad();

            if (error.line != -1) {
                invalid_filenames << QString("%1 (line %2: %3)").arg(QDir::toNativeSeparators(filepath)).arg(error.line).arg(error.message);
                continue;
            }

            html_import.SetBook(m_Book, true);
            // Since we set the Book manually,
            // this call merely mutates our Book.
            html_import.GetBook();
            Resource &added_resource = m_Book->GetFolderKeeper().GetResourceByFilename(filename);
            HTMLResource *added_html_resource = qobject_cast<HTMLResource *>(&added_resource);

            if (current_html_resource && added_html_resource) {
                m_Book->MoveResourceAfter(*added_html_resource, *current_html_resource);
                current_html_resource = added_html_resource;

                // Only open HTML files as they are likely to be edited whereas other items
                // are likely to be inserted into or linked to the current file.
                // Only open the first file in any added group.
                if (!open_resource) {
                    open_resource = &added_resource;
                }
            }
        } else {
            // TODO: adding a CSS file should add the referenced fonts too
            Resource & resource = m_Book->GetFolderKeeper().AddContentFileToFolder(filepath);
            if (resource.Type() == Resource::CSSResourceType) {
                CSSResource *css_resource = qobject_cast<CSSResource *> (&resource);
                css_resource->InitialLoad();
            }
        }

        added_files.append(filepath);
    }

    if (!invalid_filenames.isEmpty()) {
        progress.cancel();
        QMessageBox::warning(this, tr("Sigil"),
                             tr("The following file(s) were not loaded due to invalid content or not well formed XML:\n\n%1")
                             .arg(invalid_filenames.join("\n")));
    }

    if (!added_files.isEmpty()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        emit ResourcesAdded();

        if (open_resource) {
            emit ResourceActivated(*open_resource);
        }

        m_Book->SetMetadata(old_metadata);
        emit BookContentModified();
        Refresh();
        emit ShowStatusMessageRequest(tr("File(s) added."));
        QApplication::restoreOverrideCursor();
    }

    return added_files;
}

void BookBrowser::SaveAsUrl(const QUrl &url)
{
    SaveAsFile(GetUrlResource(url));
}

void BookBrowser::SaveAs()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.count() == 1) {
        Resource *resource = resources.first();
        SaveAsFile(resource);
    } else if (resources.count() > 1) {
        SaveAsFiles();
    }
}

void BookBrowser::SaveAsFile(Resource *resource)
{
    if (!resource) {
        return;
    }

    const QString &filename = resource->Filename();
    QString save_path = m_LastFolderSaveAs + "/" + filename;
    QString filter_string = "";
    QString default_filter = "";
    QString destination = QFileDialog::getSaveFileName(this,
                          tr("Save As File"),
                          save_path,
                          filter_string,
                          &default_filter
                                                      );

    if (destination.isEmpty()) {
        return;
    }

    // Store the folder the user saved to
    m_LastFolderSaveAs = QFileInfo(destination).absolutePath();
    m_Book->GetFolderKeeper().SuspendWatchingResources();
    resource->SaveToDisk();
    m_Book->GetFolderKeeper().ResumeWatchingResources();
    QString source = resource->GetFullPath();

    if (QFileInfo(destination).exists()) {
        QFile::remove(destination);
    }

    if (!QFile::copy(source, destination)) {
        Utility::DisplayStdErrorDialog(tr("Unable to save the file."));
    }
}

void BookBrowser::SaveAsFiles()
{
    QList <Resource *> resources = ValidSelectedResources();
    QString dirname = QFileDialog::getExistingDirectory(this,
                      tr("Choose the directory to save the files to"),
                      m_LastFolderSaveAs);

    if (dirname.isEmpty()) {
        return;
    }

    bool files_exist = false;
    foreach(Resource * resource, resources) {
        QString fullfilepath = dirname + "/" + resource->Filename();

        if (QFileInfo(fullfilepath).exists()) {
            files_exist = true;
            break;
        }
    }
    QMessageBox::StandardButton button_pressed;

    if (files_exist) {
        button_pressed = QMessageBox::warning(this,
                                              tr("Sigil"), tr("One or more files already exists.  OK to overwrite?"),
                                              QMessageBox::Ok | QMessageBox::Cancel);

        if (button_pressed != QMessageBox::Ok) {
            return;
        }
    }

    m_LastFolderSaveAs = dirname;
    m_Book->GetFolderKeeper().SuspendWatchingResources();
    foreach(Resource * resource, resources) {
        resource->SaveToDisk();
        QString source = resource->GetFullPath();
        QString destination = dirname + "/" + resource->Filename();

        if (QFileInfo(destination).exists()) {
            if (!QFileInfo(destination).isFile()) {
                Utility::DisplayStdErrorDialog(tr("Unable to save files.  Destination may be a directory."));
                break;
            }

            QFile::remove(destination);
        }

        if (!QFile::copy(source, destination)) {
            Utility::DisplayStdErrorDialog(tr("Unable to save files."));
            break;
        }
    }
    m_Book->GetFolderKeeper().ResumeWatchingResources();
}

void BookBrowser::OpenWith() const
{
    Resource *resource = GetCurrentResource();

    if (resource) {
        m_Book->GetFolderKeeper().SuspendWatchingResources();
        resource->SaveToDisk();
        m_Book->GetFolderKeeper().ResumeWatchingResources();
        const QString &editorPath = OpenExternally::selectEditorForResourceType(resource->Type());

        if (!editorPath.isEmpty()) {
            if (OpenExternally::openFile(resource->GetFullPath(), editorPath)) {
                m_Book->GetFolderKeeper().WatchResourceFile(*resource);
            }
        }
    }
}

void BookBrowser::OpenWithEditor() const
{
    Resource *resource = GetCurrentResource();

    if (resource) {
        m_Book->GetFolderKeeper().SuspendWatchingResources();
        resource->SaveToDisk();
        m_Book->GetFolderKeeper().ResumeWatchingResources();
        const QVariant &editorPathData = m_OpenWithEditor->data();

        if (editorPathData.isValid()) {
            if (OpenExternally::openFile(resource->GetFullPath(), editorPathData.toString())) {
                m_Book->GetFolderKeeper().WatchResourceFile(*resource);
            }
        }
    }
}

void BookBrowser::Rename()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    Resource::ResourceType resource_type = resources.first()->Type();

    if (resource_type == Resource::OPFResourceType || resource_type == Resource::NCXResourceType) {
        return;
    }

    if (resources.count() == 1) {
        // Save the resource so it can be re-selected
        m_RenamedResource = GetCurrentResource();
        // The actual renaming code is in OPFModel::ItemChangedHandler
        m_TreeView.edit(m_TreeView.currentIndex());
    } else {
        RenameSelected();
    }
}

QString BookBrowser::GetFirstAvailableTemplateName(QString base, QString number_string)
{
    int number = number_string.toInt();

    QStringList book_filenames = m_Book->GetFolderKeeper().GetAllFilenames();
    QStringList short_filenames;
    foreach(QString filename, book_filenames) {
        short_filenames.append(filename.left(filename.lastIndexOf(".")));
    }

    QString template_name;
    while (true) {
        template_name = QString("%1%2").arg(base).arg(number, number_string.length(), 10, QChar('0'));

        if (!short_filenames.contains(template_name)) {
            break;
        }

        number++;
    }

    return template_name;
}


void BookBrowser::RenameSelected()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    // Load initial value from stored preferences
    SettingsStore settings;
    QString template_name = settings.renameTemplate();

    // If no template set it to the first available default name
    if (template_name.isEmpty()) {
        template_name = GetFirstAvailableTemplateName("Section", "0001");
    }

    RenameTemplate rename_template(template_name, this);

    // Get the template from the user
    if (rename_template.exec() != QDialog::Accepted) {
        return;
    }

    template_name = rename_template.GetTemplateName();

    // Save the template for later - save now in case of abort before final save
    settings.setRenameTemplate(template_name);

    // Get the name without extension, and get the extension if any
    QString new_extension;
    if (template_name.contains(".")) {
        new_extension = template_name.right(template_name.length() - template_name.lastIndexOf("."));
        template_name = template_name.left(template_name.lastIndexOf("."));
    }

    // Get the base text and starting number if the template is not just an extension
    QString template_base;
    QString template_number_string;

    if (!template_name.isEmpty()) {
        int pos = template_name.length() - 1;
        while (pos >= 0 && template_name[pos].isDigit()) {
            template_number_string.prepend(QString("%1").arg(template_name[pos]));
            pos--;
        }

        template_base = template_name.left(pos + 1);

        if (template_number_string == "") {
            template_number_string = "1";
        }
    }

    // Get the list of new names that will be created
    int resources_count = resources.count();
    int template_number_start = template_number_string.toInt();

    QStringList new_filenames;
    QStringList book_filenames = m_Book->GetFolderKeeper().GetAllFilenames();
    int template_number = template_number_start;

    for (int i = 0; i < resources_count; i++) {
        QString file_extension = new_extension;
        QString old_filename = resources[i]->Filename();

        // If the user gave a new file extension, use it, else use the old name's extension
        if (file_extension.isEmpty() && old_filename.contains('.')) {
            file_extension = old_filename.right(old_filename.length() - old_filename.lastIndexOf('.'));
        }

        // Save the name
        QString name;
        if (template_number_string.isEmpty()) {
            // If no name or number given for template, then use old name but with new extension
            QString old_filename_no_extension = old_filename.left(old_filename.lastIndexOf('.'));
            name = QString("%1").arg(old_filename_no_extension).append(file_extension);
        } else {
            name = QString("%1%2").arg(template_base).arg(template_number, template_number_string.length(), 10, QChar('0')).append(file_extension);
        }

        // Stop if the new name is already used or will be used by a different entry
        if ((book_filenames.contains(name) && resources[i]->Filename() != name) || new_filenames.contains(name)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot rename files since this would result in duplicate filenames."));
            return;
        }

        new_filenames.append(name);

        template_number++;
    }

    foreach (QString s, new_filenames) {
        if (!Utility::use_filename_warning(s)) {
            return;
        }
    }

    // Save the next name in the sequence for later
    QString next_name_template;

    if (!template_number_string.isEmpty()) {
        QString next_template_number_string = QString("%1").arg(template_number, template_number_string.length(), 10, QChar('0'));
        next_name_template = GetFirstAvailableTemplateName(template_base, next_template_number_string);
    }

    if (!new_extension.isEmpty()) {
        next_name_template.append(new_extension);
    }

    settings.setRenameTemplate(next_name_template);

    // After a rename we want to keep the resources in the identical position.
    int scrollY = m_TreeView.verticalScrollBar()->value();

    // Rename the resources
    m_OPFModel.RenameResourceList(resources, new_filenames);

    SelectResources(resources);
    m_TreeView.verticalScrollBar()->setSliderPosition(scrollY);
}


void BookBrowser::Delete()
{
    emit RemoveResourcesRequest();
}


void BookBrowser::RemoveSelection(QList<Resource *> tab_resources)
{
    QList <Resource *> resources = ValidSelectedResources();
    RemoveResources(tab_resources, resources);
}

void BookBrowser::RemoveResources(QList<Resource *> tab_resources, QList<Resource *> resources)
{
    if (resources.isEmpty()) {
        return;
    }

    Resource *next_resource = NULL;
    Resource::ResourceType resource_type = resources.first()->Type();

    if (resource_type == Resource::OPFResourceType ||
        resource_type == Resource::NCXResourceType) {
        Utility::DisplayStdErrorDialog(
            tr("Neither the NCX nor the OPF can be removed.")
        );
        return;
    } else if (resource_type == Resource::HTMLResourceType &&
               resources.count() == m_Book->GetFolderKeeper().GetResourceTypeList<HTMLResource>().count()) {
        {
            Utility::DisplayStdErrorDialog(
                tr("You cannot remove all html files.\n"
                   "There always has to be at least one.")
            );
            return;
        }
    }

    QStringList files_to_delete;
    foreach(Resource * resource, resources) {
        files_to_delete.append(resource->Filename());
    }
    // Confirm and select which files to delete
    DeleteFiles delete_files(files_to_delete, this);
    connect(&delete_files, SIGNAL(OpenFileRequest(QString, int)), this, SIGNAL(OpenFileRequest(QString, int)));

    if (delete_files.exec() != QDialog::Accepted) {
        return;
    }

    files_to_delete = delete_files.GetFilesToDelete();

    if (files_to_delete.count() < 1) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    foreach(Resource * resource, resources) {
        if (!files_to_delete.contains(resource->Filename())) {
            resources.removeOne(resource);
        }
    }

    // Find next resource to select
    QList<Resource *> selected_resources = ValidSelectedResources();
    bool keep_selection = true;
    foreach (Resource *resource, selected_resources) {
        if (resources.contains(resource)) {
            keep_selection = false;
            break;
        }
    }
    if (keep_selection) {
        next_resource = selected_resources.first();
    } else {
        next_resource = ResourceToSelectAfterRemove(resources);
    }

    // Check if any tabs will remain after deleting resources
    bool tab_remaining = false;
    foreach(Resource * tab_resource, tab_resources) {
        if (!resources.contains(tab_resource)) {
            tab_remaining = true;
        }
    }

    // If no tabs will be left, make sure at least one tab is opened.
    // next_resource will always be an openable type resource if no
    // tabs remain because at least one tab was open before deletion.
    if (!tab_remaining && next_resource) {
        emit ResourceActivated(*next_resource);
    }

    // Delete the resources
    foreach(Resource * resource, resources) {
        resource->Delete();
    }
    emit ResourcesDeleted();
    emit BookContentModified();
    // Avoid full refresh so selection stays for non-openable resources
    m_OPFModel.Refresh();
    RefreshCounts();

    if (keep_selection) {
        SelectResources(selected_resources);
    } else {
        if (next_resource) {
            UpdateSelection(*next_resource);
        }
    }

    QApplication::restoreOverrideCursor();
}


Resource *BookBrowser::ResourceToSelectAfterRemove(QList<Resource *> selected_resources)
{
    if (selected_resources.isEmpty()) {
        return NULL;
    }

    QList <Resource *> all_resources = m_OPFModel.GetResourceListInFolder(selected_resources.first());

    if (all_resources.isEmpty()) {
        return NULL;
    }

    Resource *top_resource = NULL;
    Resource *bottom_resource = NULL;
    bool in_delete = false;
    foreach(Resource * all_resource, all_resources) {
        bool found = false;
        foreach(Resource * selected_resource, selected_resources) {
            if (all_resource->Filename() == selected_resource->Filename()) {
                in_delete = true;
                found = true;
                break;
            }
        }

        if (!in_delete && !found) {
            top_resource = all_resource;
        }

        if (in_delete && !found && !bottom_resource) {
            bottom_resource = all_resource;
        }
    }

    if (bottom_resource) {
        top_resource = bottom_resource;
    } else if (!top_resource) {
        all_resources = AllHTMLResources();

        if (!all_resources.isEmpty()) {
            top_resource = all_resources.first();
        }
    }

    return top_resource;
}


void BookBrowser::SetCoverImage()
{
    QList <Resource *> resources = ValidSelectedResources();
    int scrollY = m_TreeView.verticalScrollBar()->value();

    ImageResource *image_resource = qobject_cast<ImageResource *>(GetCurrentResource());
    if (image_resource == NULL) {
        emit ShowStatusMessageRequest(tr("Unable to set file as cover image."));
        return;
    }
    m_Book->GetOPF().SetResourceAsCoverImage(*image_resource);
    m_OPFModel.Refresh();
    emit BookContentModified();

    SelectResources(resources);
    m_TreeView.verticalScrollBar()->setSliderPosition(scrollY);
}


void BookBrowser::AddGuideSemanticType(int type)
{
    QList <Resource *> resources = ValidSelectedResources();
    int scrollY = m_TreeView.verticalScrollBar()->value();

    if (resources.isEmpty()) {
        return;
    }

    foreach(Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        m_Book->GetOPF().AddGuideSemanticType(*html_resource, (GuideSemantics::GuideSemanticType) type);

    }
    m_OPFModel.Refresh();
    emit BookContentModified();

    SelectResources(resources);
    m_TreeView.verticalScrollBar()->setSliderPosition(scrollY);
}

void BookBrowser::Merge()
{
    int scrollY = m_TreeView.verticalScrollBar()->value();
    emit MergeResourcesRequest(ValidSelectedResources(Resource::HTMLResourceType));
    m_TreeView.verticalScrollBar()->setSliderPosition(scrollY);
}


void BookBrowser::LinkStylesheets()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    Resource::ResourceType resource_type = resources.first()->Type();

    if (resource_type != Resource::HTMLResourceType) {
        return;
    }

    emit LinkStylesheetsToResourcesRequest(ValidSelectedResources(Resource::HTMLResourceType));
}


void BookBrowser::NoObfuscationMethod()
{
    foreach(Resource * resource, ValidSelectedResources()) {
        FontResource *font_resource = qobject_cast<FontResource *>(resource);
        Q_ASSERT(font_resource);
        font_resource->SetObfuscationAlgorithm("");
        emit BookContentModified();
    }
}


void BookBrowser::AdobesObfuscationMethod()
{
    foreach(Resource * resource, ValidSelectedResources()) {
        FontResource *font_resource = qobject_cast<FontResource *>(resource);
        Q_ASSERT(font_resource);
        font_resource->SetObfuscationAlgorithm(ADOBE_FONT_ALGO_ID);
        emit BookContentModified();
    }
}


void BookBrowser::IdpfsObfuscationMethod()
{
    foreach(Resource * resource, ValidSelectedResources()) {
        FontResource *font_resource = qobject_cast<FontResource *>(resource);
        Q_ASSERT(font_resource);
        font_resource->SetObfuscationAlgorithm(IDPF_FONT_ALGO_ID);
        emit BookContentModified();
    }
}


void BookBrowser::ValidateStylesheetWithW3C()
{
    QList <Resource *> resources = ValidSelectedResources(Resource::CSSResourceType);
    foreach(Resource * resource, resources) {
        CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
        Q_ASSERT(css_resource);
        css_resource->ValidateStylesheetWithW3C();
    }
}


void BookBrowser::ExpandTextFolder()
{
    m_TreeView.expand(m_OPFModel.GetTextFolderModelIndex());
}


void BookBrowser::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    m_LastFolderOpen = settings.value("lastfolderopen").toString();
    m_LastFolderSaveAs = settings.value("lastfoldersaveas").toString();
    settings.endGroup();
}


void BookBrowser::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("lastfolderopen", m_LastFolderOpen);
    settings.setValue("lastfoldersaveas", m_LastFolderSaveAs);
    settings.endGroup();
}


void BookBrowser::SetupTreeView()
{
    m_TreeView.setEditTriggers(QAbstractItemView::EditKeyPressed);
    m_TreeView.setSortingEnabled(false);
    m_TreeView.sortByColumn(-1);
    m_TreeView.setUniformRowHeights(true);
    m_TreeView.setDragEnabled(true);
    m_TreeView.setAcceptDrops(false);
    m_TreeView.setDropIndicatorShown(true);
    m_TreeView.setDragDropMode(QAbstractItemView::InternalMove);
    m_TreeView.setContextMenuPolicy(Qt::CustomContextMenu);
    m_TreeView.setItemDelegate(new FilenameDelegate);
    m_TreeView.setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_TreeView.setModel(&m_OPFModel);

    for (int i = 1; i < m_OPFModel.columnCount(); ++i) {
        m_TreeView.hideColumn(i);
    }

    m_TreeView.setIndentation(COLUMN_INDENTATION);
    m_TreeView.setHeaderHidden(true);
}


void BookBrowser::CreateContextMenuActions()
{
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
    m_SelectAll               = new QAction(tr("Select All"),            this);
    m_AddNewHTML              = new QAction(tr("Add Blank HTML File"),   this);
    m_AddNewCSS               = new QAction(tr("Add Blank Stylesheet"),  this);
    m_AddNewSVG               = new QAction(tr("Add Blank SVG Image"),   this);
    m_AddExisting             = new QAction(tr("Add Existing Files..."), this);
    m_CopyHTML                = new QAction(tr("Add Copy"),              this);
    m_CopyCSS                 = new QAction(tr("Add Copy"),              this);
    m_Rename                  = new QAction(tr("Rename") + "...",        this);
    m_Delete                  = new QAction(tr("Delete") + "...",        this);
    m_CoverImage              = new QAction(tr("Cover Image"),           this);
    m_Merge                   = new QAction(tr("Merge"),                 this);
    m_NoObfuscationMethod     = new QAction(tr("None"),                  this);
    m_AdobesObfuscationMethod = new QAction(tr("Use Adobe's Method"),    this);
    m_IdpfsObfuscationMethod  = new QAction(tr("Use IDPF's Method"),     this);
    m_SortHTML                = new QAction(tr("Sort") + "...",          this);
    m_RenumberTOC             = new QAction(tr("Renumber TOC Entries"),  this);
    m_LinkStylesheets         = new QAction(tr("Link Stylesheets..."),   this);
    m_ValidateWithW3C         = new QAction(tr("Validate with W3C"),     this);
    m_OpenWith                = new QAction(tr("Open With") + "...",     this);
    m_SaveAs                  = new QAction(tr("Save As") + "...",       this);
    m_OpenWithEditor          = new QAction("",                            this);
    m_CoverImage             ->setCheckable(true);
    m_NoObfuscationMethod    ->setCheckable(true);
    m_AdobesObfuscationMethod->setCheckable(true);
    m_IdpfsObfuscationMethod ->setCheckable(true);
    m_CopyHTML->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Y));
    sm->registerAction(this, m_CopyHTML, "MainWindow.BookBrowser.CopyHTML");
    m_Delete->setShortcut(QKeySequence::Delete);
    m_Merge->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    m_Merge->setToolTip("Merge with previous file, or merge multiple files into one.");
    sm->registerAction(this, m_Merge, "MainWindow.BookBrowser.Merge");
    m_Rename->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_R));
    m_Rename->setToolTip("Rename selected file(s)");
    sm->registerAction(this, m_Rename, "MainWindow.BookBrowser.Rename");
    m_LinkStylesheets->setToolTip("Link Stylesheets to selected file(s).");
    sm->registerAction(this, m_LinkStylesheets, "MainWindow.BookBrowser.LinkStylesheets");
    // Has to be added to the book browser itself as well
    // for the keyboard shortcut to work.
    addAction(m_CopyHTML);
    addAction(m_Delete);
    addAction(m_Merge);
    addAction(m_Rename);
    addAction(m_LinkStylesheets);
    CreateGuideSemanticActions();
}


void BookBrowser::CreateGuideSemanticActions()
{
    QAction *action = NULL;
    action = new QAction(tr("Cover"), this);
    action->setData(GuideSemantics::Cover);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Cover);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Title Page"), this);
    action->setData(GuideSemantics::TitlePage);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::TitlePage);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Table Of Contents"), this);
    action->setData(GuideSemantics::TableOfContents);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::TableOfContents);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Index"), this);
    action->setData(GuideSemantics::Index);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Index);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Glossary"), this);
    action->setData(GuideSemantics::Glossary);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Glossary);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Acknowledgements"), this);
    action->setData(GuideSemantics::Acknowledgements);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Acknowledgements);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Bibliography"), this);
    action->setData(GuideSemantics::Bibliography);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Bibliography);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Colophon"), this);
    action->setData(GuideSemantics::Colophon);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Colophon);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Copyright Page"), this);
    action->setData(GuideSemantics::CopyrightPage);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::CopyrightPage);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Dedication"), this);
    action->setData(GuideSemantics::Dedication);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Dedication);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Epigraph"), this);
    action->setData(GuideSemantics::Epigraph);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Epigraph);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Foreword"), this);
    action->setData(GuideSemantics::Foreword);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Foreword);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("List Of Illustrations"), this);
    action->setData(GuideSemantics::ListOfIllustrations);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::ListOfIllustrations);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("List Of Tables"), this);
    action->setData(GuideSemantics::ListOfTables);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::ListOfTables);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Notes"), this);
    action->setData(GuideSemantics::Notes);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Notes);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Preface"), this);
    action->setData(GuideSemantics::Preface);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Preface);
    m_GuideSemanticActions.append(action);
    action = new QAction(tr("Text"), this);
    action->setData(GuideSemantics::Text);
    m_GuideSemanticMapper.setMapping(action, GuideSemantics::Text);
    m_GuideSemanticActions.append(action);
    foreach(QAction * action, m_GuideSemanticActions) {
        action->setCheckable(true);
    }
}


bool BookBrowser::SuccessfullySetupContextMenu(const QPoint &point)
{
    QModelIndex index = m_TreeView.indexAt(point);

    if (!index.isValid()) {
        return false;
    }

    int item_count = ValidSelectedItemCount();

    if (item_count < 1) {
        return false;
    }

    QStandardItem *item = m_OPFModel.itemFromIndex(index);
    Q_ASSERT(item);
    m_LastContextMenuType = m_OPFModel.GetResourceType(item);
    QList<Resource *> resources = m_OPFModel.GetResourceListInFolder(m_LastContextMenuType);
    Resource *resource = GetCurrentResource();

    if (resource) {
        m_ContextMenu.addSeparator();

        // Delete and Rename
        if (m_LastContextMenuType != Resource::OPFResourceType &&
            m_LastContextMenuType != Resource::NCXResourceType) {
            m_ContextMenu.addAction(m_Delete);
            m_Delete->setEnabled(m_LastContextMenuType != Resource::HTMLResourceType ||
                                 (AllHTMLResources().count() > 1 && resources.count() != item_count));
            m_ContextMenu.addAction(m_Rename);
        }

        if (resource->Type() == Resource::HTMLResourceType) {
            m_ContextMenu.addAction(m_Merge);
            m_Merge->setEnabled(item_count > 1 ||
                                (AllHTMLResources().count() > 1 &&
                                 AllHTMLResources().at(0) != ValidSelectedResources().at(0)));
            m_ContextMenu.addAction(m_SortHTML);
            m_SortHTML->setEnabled(item_count > 1);
            m_ContextMenu.addAction(m_LinkStylesheets);
            m_LinkStylesheets->setEnabled(AllCSSResources().count() > 0);
        }

        if (resource->Type() == Resource::FontResourceType) {
            SetupFontObfuscationMenu();
        }

        if (resource->Type() == Resource::NCXResourceType) {
            m_ContextMenu.addAction(m_RenumberTOC);
        }

        if (resource->Type() == Resource::CSSResourceType) {
            m_ContextMenu.addAction(m_ValidateWithW3C);
        }

        // Semantic Menu
        SetupSemanticContextmenu(resource);
        m_ContextMenu.addSeparator();

        // Open With
        if (OpenExternally::mayOpen(resource->Type())) {
            const QString &editorPath = OpenExternally::editorForResourceType(resource->Type());
            const QString &editorDescription = OpenExternally::editorDescriptionForResourceType(resource->Type());

            if (editorPath.isEmpty()) {
                m_OpenWithEditor->setData(QVariant::Invalid);
                m_OpenWith->setText(tr("Open With") + "...");
                m_ContextMenu.addAction(m_OpenWith);
            } else {
                m_OpenWithEditor->setText(editorDescription);
                m_OpenWithEditor->setData(editorPath);
                m_OpenWithEditor->setEnabled(item_count == 1);
                m_OpenWith->setText(tr("Other Application") + "...");
                m_ContextMenu.addMenu(&m_OpenWithContextMenu);
            }

            m_OpenWith->setEnabled(item_count == 1);
            m_OpenWithContextMenu.setEnabled(item_count == 1);
        }

        // Save As
        m_ContextMenu.addAction(m_SaveAs);
    }

    // Applies to Menus and Resources
    // Add Existing and Add specific file types
    m_ContextMenu.addSeparator();

    if (m_LastContextMenuType == Resource::HTMLResourceType) {
        m_ContextMenu.addAction(m_AddNewHTML);
        m_ContextMenu.addAction(m_CopyHTML);
        m_CopyHTML->setEnabled(item_count == 1);
    } else if (m_LastContextMenuType == Resource::CSSResourceType) {
        m_ContextMenu.addAction(m_AddNewCSS);
        m_ContextMenu.addAction(m_CopyCSS);
        m_CopyCSS->setEnabled(item_count == 1);
    } else if (m_LastContextMenuType == Resource::ImageResourceType || m_LastContextMenuType == Resource::SVGResourceType) {
        m_ContextMenu.addAction(m_AddNewSVG);
    }

    m_ContextMenu.addAction(m_AddExisting);

    // Select All
    if (m_LastContextMenuType != Resource::OPFResourceType &&
        m_LastContextMenuType != Resource::NCXResourceType) {
        m_ContextMenu.addSeparator();
        m_ContextMenu.addAction(m_SelectAll);
        m_SelectAll->setEnabled(item_count > 0 || (!resource && resources.count() > 0));
    }

    return true;
}


void BookBrowser::SetupSemanticContextmenu(Resource *resource)
{
    if (resource->Type() == Resource::HTMLResourceType) {
        SetupHTMLSemanticContextMenu(resource);
        m_ContextMenu.addMenu(&m_SemanticsContextMenu);
    } else if (resource->Type() == Resource::ImageResourceType) {
        SetupImageSemanticContextMenu(resource);
        m_ContextMenu.addMenu(&m_SemanticsContextMenu);
    }

    m_SemanticsContextMenu.setEnabled(ValidSelectedItemCount() == 1);
}


void BookBrowser::SetupHTMLSemanticContextMenu(Resource *resource)
{
    foreach(QAction * action, m_GuideSemanticActions) {
        m_SemanticsContextMenu.addAction(action);
    }
    SetHTMLSemanticActionCheckState(resource);
}


void BookBrowser::SetupImageSemanticContextMenu(Resource *resource)
{
    m_SemanticsContextMenu.addAction(m_CoverImage);
    ImageResource *image_resource = qobject_cast<ImageResource *>(GetCurrentResource());
    Q_ASSERT(image_resource);
    m_CoverImage->setChecked(false);

    if (m_Book->GetOPF().IsCoverImage(*image_resource)) {
        m_CoverImage->setChecked(true);
    }
}


void BookBrowser::SetHTMLSemanticActionCheckState(Resource *resource)
{
    if (resource->Type() != Resource::HTMLResourceType) {
        return;
    }

    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
    Q_ASSERT(html_resource);
    foreach(QAction * action, m_GuideSemanticActions) {
        action->setChecked(false);
    }
    GuideSemantics::GuideSemanticType semantic_type =
        m_Book->GetOPF().GetGuideSemanticTypeForResource(*html_resource);

    if (semantic_type == GuideSemantics::NoType) {
        return;
    }

    foreach(Resource * valid_resource, ValidSelectedResources()) {
        HTMLResource *valid_html_resource = qobject_cast<HTMLResource *>(valid_resource);
        Q_ASSERT(html_resource);
        GuideSemantics::GuideSemanticType valid_semantic_type =
            m_Book->GetOPF().GetGuideSemanticTypeForResource(*valid_html_resource);

        if (valid_semantic_type != semantic_type) {
            return;
        }
    }
    foreach(QAction * action, m_GuideSemanticActions) {
        if (action->data().toInt() == semantic_type) {
            action->setChecked(true);
            break;
        }
    }
}


void BookBrowser::SetupFontObfuscationMenu()
{
    m_FontObfuscationContextMenu.addAction(m_NoObfuscationMethod);
    m_FontObfuscationContextMenu.addAction(m_AdobesObfuscationMethod);
    m_FontObfuscationContextMenu.addAction(m_IdpfsObfuscationMethod);
    SetFontObfuscationActionCheckState();
    m_ContextMenu.addMenu(&m_FontObfuscationContextMenu);
}


void BookBrowser::SetFontObfuscationActionCheckState()
{
    Resource *resource = GetCurrentResource();

    if (resource->Type() != Resource::FontResourceType) {
        return;
    }

    // Get the algorithm used by the first font
    FontResource *font_resource = qobject_cast<FontResource *>(resource);
    Q_ASSERT(font_resource);
    QString algorithm = font_resource->GetObfuscationAlgorithm();
    // Now compare against all the other fonts and if the same set a checkmark
    bool same_algorithm = true;
    foreach(Resource * resource, ValidSelectedResources()) {
        FontResource *font_resource2 = qobject_cast<FontResource *>(resource);
        Q_ASSERT(font_resource);
        QString algorithm2 = font_resource2->GetObfuscationAlgorithm();

        if (algorithm2 != algorithm) {
            same_algorithm = false;
            break;
        }
    }
    m_NoObfuscationMethod->setChecked(same_algorithm && (algorithm.isEmpty() || algorithm == ""));
    m_AdobesObfuscationMethod->setChecked(same_algorithm && algorithm == ADOBE_FONT_ALGO_ID);
    m_IdpfsObfuscationMethod ->setChecked(same_algorithm && algorithm == IDPF_FONT_ALGO_ID);
}


void BookBrowser::ConnectSignalsToSlots()
{
    connect(&m_TreeView, SIGNAL(activated(const QModelIndex &)),
            this,         SLOT(EmitResourceActivated(const QModelIndex &)));
    connect(&m_TreeView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this,        SLOT(OpenContextMenu(const QPoint &)));
    connect(&m_OPFModel, SIGNAL(ResourceRenamed()),
            this,        SLOT(SelectRenamedResource()));
    connect(m_SelectAll,               SIGNAL(triggered()), this, SLOT(SelectAll()));
    connect(m_CopyHTML,                SIGNAL(triggered()), this, SLOT(CopyHTML()));
    connect(m_CopyCSS,                 SIGNAL(triggered()), this, SLOT(CopyCSS()));
    connect(m_AddNewHTML,              SIGNAL(triggered()), this, SLOT(AddNewHTML()));
    connect(m_RenumberTOC,             SIGNAL(triggered()), this, SLOT(RenumberTOC()));
    connect(m_SortHTML,                SIGNAL(triggered()), this, SLOT(SortHTML()));
    connect(m_AddNewCSS,               SIGNAL(triggered()), this, SLOT(AddNewCSS()));
    connect(m_AddNewSVG,               SIGNAL(triggered()), this, SLOT(AddNewSVG()));
    connect(m_AddExisting,             SIGNAL(triggered()), this, SLOT(AddExisting()));
    connect(m_Rename,                  SIGNAL(triggered()), this, SLOT(Rename()));
    connect(m_Delete,                  SIGNAL(triggered()), this, SLOT(Delete()));
    connect(m_CoverImage,              SIGNAL(triggered()), this, SLOT(SetCoverImage()));
    connect(m_Merge,                   SIGNAL(triggered()), this, SLOT(Merge()));
    connect(m_LinkStylesheets,         SIGNAL(triggered()), this, SLOT(LinkStylesheets()));
    connect(m_SaveAs,                  SIGNAL(triggered()), this, SLOT(SaveAs()));
    connect(m_ValidateWithW3C,         SIGNAL(triggered()), this, SLOT(ValidateStylesheetWithW3C()));
    connect(m_OpenWith,                SIGNAL(triggered()), this, SLOT(OpenWith()));
    connect(m_OpenWithEditor,          SIGNAL(triggered()), this, SLOT(OpenWithEditor()));
    connect(m_AdobesObfuscationMethod, SIGNAL(triggered()), this, SLOT(AdobesObfuscationMethod()));
    connect(m_IdpfsObfuscationMethod,  SIGNAL(triggered()), this, SLOT(IdpfsObfuscationMethod()));
    connect(m_NoObfuscationMethod,     SIGNAL(triggered()), this, SLOT(NoObfuscationMethod()));
    foreach(QAction * action, m_GuideSemanticActions) {
        connect(action, SIGNAL(triggered()), &m_GuideSemanticMapper, SLOT(map()));
    }
    connect(&m_GuideSemanticMapper, SIGNAL(mapped(int)), this, SLOT(AddGuideSemanticType(int)));
}


Resource *BookBrowser::GetCurrentResource() const
{
    return GetResourceByIndex(m_TreeView.currentIndex());
}


Resource *BookBrowser::GetResourceByIndex(QModelIndex index) const
{
    if (!index.isValid()) {
        return NULL;
    }

    QStandardItem *item = m_OPFModel.itemFromIndex(index);
    Q_ASSERT(item);
    const QString &identifier = item->data().toString();
    return &m_Book->GetFolderKeeper().GetResourceByIdentifier(identifier);
}
