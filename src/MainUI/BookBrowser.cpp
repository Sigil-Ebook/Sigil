/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QFileInfo>
#include <QSignalMapper>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTreeView>
#include <QProgressDialog>
#include <QScrollBar>
#include <QVariant>
#include <QTimer>
#include <QPaintEvent>
#include <QStylePainter>
#include <QDebug>

#include "BookManipulation/Book.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/DeleteFiles.h"
#include "Dialogs/RenameTemplate.h"
#include "Dialogs/SemanticTargetID.h"
#include "Dialogs/MDViewer.h"
#include "Dialogs/AddSemantics.h"
#include "Dialogs/SelectFolder.h"
#include "Dialogs/RERenamer.h"
#include "Dialogs/RETable.h"
#include "Importers/ImportHTML.h"
#include "MainUI/BookBrowser.h"
#include "MainUI/MainWindow.h"
#include "MainUI/OPFModel.h"
#include "Misc/FilenameDelegate.h"
#include "Misc/KeyboardShortcutManager.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/MediaTypes.h"
#include "Misc/HTMLSpellCheckML.h"
#include "Misc/OpenExternally.h"
#include "Misc/GuideItems.h"
#include "Misc/Landmarks.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NavProcessor.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

#define DBG if(0)

#include <QMetaType>
static const QVariant QVINVALID = QVariant(QMetaType(QMetaType::UnknownType));


static const QString SETTINGS_GROUP = "bookbrowser";
static const QString OPF_NCX_EDIT_WARNING_KEY = SETTINGS_GROUP + "-opfncx-warning";
static const int COLUMN_INDENTATION = 10;

// This needs to be kept in sync with FolderKeeper
const QStringList folderkeys = QStringList() << "Text" << "Styles" << "Images" << "Fonts" <<
                                                "Audio" << "Video" << "Misc"  << "na" << "na";

BookBrowser::BookBrowser(QWidget *parent)
    :
    QDockWidget(tr("Book Browser"), parent),
    m_Book(NULL),
    m_TreeView(new QTreeView(this)),
    m_OPFModel(new OPFModel(this)),
    m_ContextMenu(new QMenu(this)),
    m_FontObfuscationContextMenu(new QMenu(this)),
    m_OpenWithContextMenu(new QMenu(m_ContextMenu)),
    m_openWithMapper(new QSignalMapper(this)),
    m_LastContextMenuType(Resource::GenericResourceType),
    m_RenamedResource(NULL),
    m_MovedResource(NULL)
{
    m_FontObfuscationContextMenu->setTitle(tr("Font Obfuscation"));
    m_OpenWithContextMenu->setTitle(tr("Open With") + "...");
    setWidget(m_TreeView);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    ReadSettings();
    SetupTreeView();
    CreateContextMenuActions();
    ConnectSignalsToSlots();
    m_OpenWithContextMenu->addAction(m_OpenWithEditor0);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor1);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor2);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor3);
    m_OpenWithContextMenu->addAction(m_OpenWithEditor4);
    m_OpenWithContextMenu->addAction(m_OpenWithOtherApp);
    m_OpenWithContextMenu->addSeparator();
    m_OpenWithContextMenu->addAction(m_OpenWithClear);
    
    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle(tr("Book Browser"));
}


BookBrowser::~BookBrowser()
{
    WriteSettings();
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
    sm->removeActionsOf(this);
}

void BookBrowser::paintEvent(QPaintEvent *event)
{
    // Allow title text to be set independently of tab text
    // (when QDockWidget is tabified).
    QStylePainter painter(this);

    if (isFloating()) {
        QStyleOptionFrame options;
        options.initFrom(this);

#ifdef Q_OS_MAC
        // This is needed for Qt6 but works on Qt5 as well
        options.palette = QApplication::palette();
#endif

        painter.drawPrimitive(QStyle::PE_FrameDockWidget, options);
    }
    QStyleOptionDockWidget options;
    initStyleOption(&options);
    options.title = windowTitle();

#ifdef Q_OS_MAC
    // This is needed for Qt6 but works on Qt5 as well
    options.palette = QApplication::palette();
#endif

    painter.drawControl(QStyle::CE_DockWidgetTitle, options);
}

void BookBrowser::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    raise();
}

void BookBrowser::SetBook(QSharedPointer<Book> book)
{
    m_Book = book;
    m_OPFModel->SetBook(book);
    connect(this, SIGNAL(BookContentModified()), m_Book.data(), SLOT(SetModified()));
    ExpandTextFolder();
    RefreshCounts();

    try {
        // Here we fake that the "first" HTML file has been double clicked
        // so that we have a default first tab opened.
        // An exception is thrown if there are no HTML files in the epub.
        EmitResourceActivated(m_OPFModel->GetFirstHTMLModelIndex());
    }
    // No exception variable since we don't use it
    catch (NoHTMLFiles&) {
        // Do nothing. No HTML files, no first file opened.
    }
}

void BookBrowser::RefreshCounts()
{
    int  mainfolder_length = m_Book->GetFolderKeeper()->GetFullPathToMainFolder().length();
    Q_UNUSED(mainfolder_length);
    for (int i = 0; i < m_OPFModel->invisibleRootItem()->rowCount(); i++) {
        QStandardItem *folder = m_OPFModel->invisibleRootItem()->child(i);
        QString tooltip;
        QString key = folderkeys.at(i);
        if (key != "na") {
            tooltip = m_Book->GetFolderKeeper()->GetDefaultFolderForGroup(folderkeys.at(i));
            int count = folder->rowCount();
            tooltip = tooltip + " " + QString(tr("%n file(s)","", count));
            folder->setToolTip(tooltip);
        }
    }
}

void BookBrowser::Refresh()
{
    m_OPFModel->Refresh();
    RefreshCounts();
    emit UpdateBrowserSelection();
}


void BookBrowser::SelectAll()
{
    QList<Resource *> resources = m_OPFModel->GetResourceListInFolder(m_LastContextMenuType);
    SelectResources(resources);
}


void BookBrowser::SelectResources(QList<Resource *> resources)
{
    m_TreeView->selectionModel()->clearSelection();
    foreach(Resource *resource, resources) {
        QModelIndex index = m_OPFModel->GetModelItemIndex(resource, OPFModel::IndexChoice_Current);
        m_TreeView->selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void BookBrowser::SelectRenamedResource()
{
    if (m_RenamedResource == NULL) {
        return;
    }

    // Set the selection to the resource that was being renamed
    UpdateSelection(m_RenamedResource);
    m_RenamedResource = NULL;

    // Make sure Book Browser has focus so keyboard navigation works as expected
    // Delay so that setting focus happens last, *after* all other events
    QTimer::singleShot(100, this, SLOT(FocusOnBookBrowser()));
}

void BookBrowser::SelectMovedResource()
{
    if (m_MovedResource == NULL) {
        return;
    }

    // Set the selection to the resource that was being renamed
    UpdateSelection(m_MovedResource);
    m_MovedResource = NULL;
    // Make sure Book Browser has focus so keyboard navigation works as expected
    // Delay so that setting focus happens last, *after* all other events
    QTimer::singleShot(100, this, SLOT(FocusOnBookBrowser()));
}

void BookBrowser::FocusOnBookBrowser()
{
    qobject_cast<QWidget *>(m_TreeView)->setFocus();
}

void BookBrowser::UpdateSelection(Resource *resource)
{
    const int MIN_SPACE = 40;
    m_TreeView->selectionModel()->clearSelection();
    QModelIndex index = m_OPFModel->GetModelItemIndex(resource, OPFModel::IndexChoice_Current);
    m_TreeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    // Do we need to scroll the TreeView further to ensure the item is easily visible?
    QRect item_rect = m_TreeView->visualRect(index);
    QRect tv_rect = m_TreeView->rect();

    if ((item_rect.top() < MIN_SPACE) || ((tv_rect.bottom() - item_rect.bottom()) < MIN_SPACE)) {
        // Too close to the top or bottom
        m_TreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void BookBrowser::NextResource()
{
    Resource *resource = GetCurrentResource();

    if (resource != NULL) {
        QModelIndex index = m_OPFModel->GetModelItemIndex(resource, OPFModel::IndexChoice_Next);

        if (index.isValid()) {
            emit ResourceActivated(GetResourceByIndex(index));
        }
    }
}

void BookBrowser::PreviousResource()
{
    Resource *resource = GetCurrentResource();

    if (resource != NULL) {
        QModelIndex index = m_OPFModel->GetModelItemIndex(resource, OPFModel::IndexChoice_Previous);

        if (index.isValid()) {
            emit ResourceActivated(GetResourceByIndex(index));
        }
    }
}

void BookBrowser::SortHTML()
{
    QList <Resource *> resources = ValidSelectedResources();
    QMessageBox::StandardButton button_pressed;
    button_pressed = Utility::warning(this,
                                          tr("Sigil"),
                                          tr("Are you sure you want to sort the selected files alphanumerically?") % "\n" 
                                              % tr("This action cannot be reversed."),
                                          QMessageBox::Ok | QMessageBox::Cancel);

    if (button_pressed != QMessageBox::Ok) {
        return;
    }

    QList <QModelIndex> indexList = m_TreeView->selectionModel()->selectedRows(0);
    m_OPFModel->SortHTML(indexList);
    SelectResources(resources);
}

void BookBrowser::RenumberTOC()
{
    emit RenumberTOCContentsRequest();
}

Resource *BookBrowser::GetUrlResource(const QUrl &url)
{
    DBG qDebug() << "In BookBrowser::GetUrlResource with url: " << url;
    QString bookpath;
    if (url.scheme() == "book") {
        // handle our own internal links (strip root / from absolute path to make it relative)
        bookpath = url.path().remove(0,1);
    } else if (url.scheme() == "file") {
        QString fullfilepath = url.toLocalFile();
        QString main_folder_path = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();
        // Note main_folder_path *never* ends with a path separator - see Misc/TempFolder.cpp
        bookpath = fullfilepath.right(fullfilepath.length() - main_folder_path.length() - 1);
    } else if (url.scheme() == "sigil") {
        QUrl newurl(url);
        newurl.setScheme("file");
        DBG qDebug() << "what? " << newurl <<  newurl.path();
        QString fullfilepath = newurl.toLocalFile();
        QString main_folder_path = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();
        // Note main_folder_path *never* ends with a path separator - see Misc/TempFolder.cpp
        bookpath = fullfilepath.right(fullfilepath.length() - main_folder_path.length() - 1);
    }
    if (!bookpath.isEmpty()) {
        try {
            Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(bookpath);
            return resource;
        } catch (ResourceDoesNotExist&) {
            Utility::DisplayStdErrorDialog(tr("The file \"%1\" does not exist.").arg(bookpath));
        }
    }
    return NULL;
}

void BookBrowser::EmitResourceActivated(const QModelIndex &index)
{
    QString identifier(m_OPFModel->itemFromIndex(index)->data().toString());

    if (!identifier.isEmpty()) {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);
        emit ResourceActivated(resource);
    }
}


void BookBrowser::OpenContextMenu(const QPoint &point)
{
    if (!SuccessfullySetupContextMenu(point)) {
        return;
    }

    m_ContextMenu->exec(m_TreeView->viewport()->mapToGlobal(point)); 
    if (!m_ContextMenu.isNull()) {
        m_ContextMenu->clear();
        // Ensure any actions with keyboard shortcuts that might have temporarily been
        // disabled are enabled again to let the shortcut work outside the menu.
        m_Delete->setEnabled(true);
        m_Merge->setEnabled(true);
        m_Rename->setEnabled(true);
        m_RERename->setEnabled(true);
        m_Move->setEnabled(true);
    }
}

QList <Resource *> BookBrowser::ValidSelectedHTMLResources()
{
    return ValidSelectedResources(Resource::HTMLResourceType);
}

QList <Resource *> BookBrowser::ValidSelectedCSSResources()
{
    return ValidSelectedResources(Resource::CSSResourceType);
}

QList <Resource *> BookBrowser::ValidSelectedSVGResources()
{
    return ValidSelectedResources(Resource::SVGResourceType);
}

QList <Resource *> BookBrowser::ValidSelectedJSResources()
{
    QStringList mts = QStringList() << "application/javascript" <<
                                       "text/javascript" <<
                                       "application/x-javascript" <<
                                       "application/ecmacript";
    return ValidSelectedResourcesByMT(mts);
}

QList <Resource *> BookBrowser::ValidSelectedMiscXMLResources()
{
    QStringList mts = QStringList() << "application/ttml+xml" <<
                                       "application/smil+xml" <<
                                       "application/smil" <<
                                       "application/pls+xml" <<
                                       "application/oebps-page-map+xml" <<
                                       "application/vnd.adobe-page-map+xml" <<
                                       "application/adobe-page-template+xml" <<
                                       "application/vnd.adobe-page-template+xml" <<
                                       "application/xml" <<
                                       "text/xml";
    return ValidSelectedResourcesByMT(mts);
}


QList <Resource *> BookBrowser::AllHTMLResources()
{
    return m_OPFModel->GetResourceListInFolder(Resource::HTMLResourceType);
}

QList <Resource *> BookBrowser::AllImageResources()
{
#if 1
    // should this include svg resources
    QList <Resource *> resources;
    resources =  m_OPFModel->GetResourceListInFolder(Resource::ImageResourceType);
    resources.append(m_OPFModel->GetResourceListInFolder(Resource::SVGResourceType));
    return resources;
#else
    return m_OPFModel->GetResourceListInFolder(Resource::ImageResourceType);
#endif
}

QList <Resource *> BookBrowser::AllMediaResources()
{
    QList <Resource *> resources;
    resources = m_OPFModel->GetResourceListInFolder(Resource::ImageResourceType);
#if 1
    // is this needed
    resources.append(m_OPFModel->GetResourceListInFolder(Resource::SVGResourceType));
#endif
    resources.append(m_OPFModel->GetResourceListInFolder(Resource::VideoResourceType));
    resources.append(m_OPFModel->GetResourceListInFolder(Resource::AudioResourceType));
    return resources;
}

QList <Resource *> BookBrowser::AllCSSResources()
{
    return m_OPFModel->GetResourceListInFolder(Resource::CSSResourceType);
}

QList <Resource *> BookBrowser::AllJSResources()
{
    QStringList mtypes = QStringList() << "text/javascript" << "application/javascript" << "application/ecmascript";
    return m_Book->GetFolderKeeper()->GetResourceListByMediaTypes(mtypes);
}

QList <Resource *> BookBrowser::ValidSelectedResources(Resource::ResourceType resource_type)
{
    QList <Resource *> resources = ValidSelectedResources();
    foreach(Resource *resource, resources) {
        if (resource->Type() != resource_type) {
            resources.clear();
        }
    }
    return resources;
}

QList <Resource *> BookBrowser::ValidSelectedResourcesByMT(QStringList &mts)
{
    QList <Resource *> resources = ValidSelectedResources();
    foreach(Resource *resource, resources) {

        if (!mts.contains(resource->GetMediaType())) {
            resources.removeOne(resource);
        }
    }
    return resources;
}

int BookBrowser::AllSelectedItemCount()
{
  QList <QModelIndex> list = m_TreeView->selectionModel()->selectedRows(0);
  int count = list.length();

  if (count <= 1) {
    return count;
  }

  foreach(QModelIndex index, list) {
    QStandardItem *item = m_OPFModel->itemFromIndex(index);
    const QString &identifier = item->data().toString();

    // If folder item included, multiple selection is invalid                                                                              
    if (identifier.isEmpty()) {
      return -1;
    }

    Resource *resource = m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);

    // If for some reason there is something with an identifier but no resource, fail                                                      
    if (resource == NULL) {
      return -1;
    }
  }
  return count;
}

QList <Resource *> BookBrowser::AllSelectedResources()
{
    QList <Resource *> resources;
    if (AllSelectedItemCount() < 1) {
        return resources;
    }

    QList <QModelIndex> list = m_TreeView->selectionModel()->selectedRows(0);
    foreach(QModelIndex index, list) {
        QStandardItem *item = m_OPFModel->itemFromIndex(index);

        if (item != NULL) {
            const QString &identifier = item->data().toString();
            Resource *resource = m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);

            if (resource != NULL) {
                resources.append(resource);
            }
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

    QList <QModelIndex> list = m_TreeView->selectionModel()->selectedRows(0);
    foreach(QModelIndex index, list) {
        QStandardItem *item = m_OPFModel->itemFromIndex(index);

        if (item != NULL) {
            const QString &identifier = item->data().toString();
            Resource *resource = m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);

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
    QList <Resource *> all_resources = m_OPFModel->GetResourceListInFolder(resource_type);
    foreach(Resource * all_resource, all_resources) {
        foreach(Resource * resource, resources) {
            if (all_resource->GetRelativePath() == resource->GetRelativePath()) {
                sorted_resources.append(all_resource);
                break;
            }
        }
    }
    return sorted_resources;
}

// handle the case that an svg resource is also for images
bool BookBrowser::IsCongruent(Resource::ResourceType selected_type, Resource::ResourceType candidate_type)
{
    if (selected_type == candidate_type) return true;
    if ((selected_type == Resource::SVGResourceType) && (candidate_type == Resource::ImageResourceType)) return true; 
    if ((selected_type == Resource::ImageResourceType) && (candidate_type == Resource::SVGResourceType)) return true; 
    return false;;
}

int BookBrowser::ValidSelectedItemCount()
{
    QList <QModelIndex> list = m_TreeView->selectionModel()->selectedRows(0);
    int count = list.length();

    if (count <= 1) {
        return count;
    }

    Resource::ResourceType resource_type = Resource::HTMLResourceType;
    foreach(QModelIndex index, list) {
        QStandardItem *item = m_OPFModel->itemFromIndex(index);
        const QString &identifier = item->data().toString();

        // If folder item included, multiple selection is invalid
        if (identifier.isEmpty()) {
            return -1;
        }

        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);

        // If for some reason there is something with an identifier but no resource, fail
        if (resource == NULL) {
            return -1;
        }

        // Check that multiple selections only contains items of congruent types
        if (index == list[0]) {
            resource_type = resource->Type();
        } else if (!IsCongruent(resource_type, resource->Type())) {
            return -1;
        }
    }
    return count;
}

Resource* BookBrowser::AddFile(QString filepath)
{
    Resource * resource = m_Book->GetFolderKeeper()->AddContentFileToFolder(filepath);
    emit BookContentModified();
    Refresh();
    return resource;
}

void BookBrowser::AddNew()
{
    if (m_LastContextMenuType == Resource::HTMLResourceType) {
        AddNewHTML();
    } else if (m_LastContextMenuType == Resource::CSSResourceType) {
        AddNewCSS();
    } else if (m_LastContextMenuType == Resource::MiscTextResourceType ||
               m_LastContextMenuType == Resource::GenericResourceType) {
        AddNewJS();
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

    QString destfolder = Utility::startingDir(current_resource->GetRelativePath());

    HTMLResource *current_html_resource = qobject_cast<HTMLResource *>(current_resource);
    // Create an empty file
    HTMLResource *new_html_resource = m_Book->CreateEmptyHTMLFile(destfolder);
    m_Book->MoveResourceAfter(new_html_resource, current_html_resource);
    // Copy the text from the current file
    new_html_resource->SetText(current_html_resource->GetText());
    // Open the new file in a tab
    emit ResourceActivated(new_html_resource);
    emit BookContentModified();
    Refresh();
}


// Create a new HTML file and insert it after the currently selected file
void BookBrowser::AddNewHTML()
{
    Resource *current_resource = GetCurrentResource();
    HTMLResource *current_html_resource = qobject_cast<HTMLResource *>(current_resource);

    QString destfolder = "\\";
    if (current_html_resource) {
        destfolder = Utility::startingDir(current_html_resource->GetRelativePath());
    }
    HTMLResource *new_html_resource = m_Book->CreateEmptyHTMLFile(destfolder);

    if ((current_resource != NULL) && (current_html_resource != NULL)) {
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

    CSSResource *current_css_resource = qobject_cast<CSSResource *>(current_resource);
    QString destfolder = Utility::startingDir(current_resource->GetRelativePath());

    // Create an empty file
    CSSResource *new_resource = m_Book->CreateEmptyCSSFile(destfolder);
    // Copy the text from the current file
    new_resource->SetText(current_css_resource->GetText());
    // Open the new file in a tab
    emit ResourceActivated(new_resource);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::AddNewCSS()
{
    CSSResource *new_resource = m_Book->CreateEmptyCSSFile();
    // Open the new file in a tab
    emit ResourceActivated(new_resource);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::AddNewJS()
{
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    if (version.startsWith('2')) {
        Utility::warning(this, tr("Sigil"),tr("Javascript is not supported on epub2.")
                                 ,QMessageBox::Ok);
        return;
    }

    MiscTextResource *new_resource = m_Book->CreateEmptyJSFile();
    new_resource->SaveToDisk();
    // Open the new file in a tab
    emit ResourceActivated(new_resource);
    emit BookContentModified();
    Refresh();
}

void BookBrowser::AddNewSVG()
{
    SVGResource *new_resource = m_Book->CreateEmptySVGFile();
    // Open the new file in a tab
    emit ResourceActivated(new_resource);
    emit BookContentModified();
    Refresh();
}

CSSResource* BookBrowser::CreateHTMLTOCCSSFile()
{
    CSSResource *css_resource = m_Book->CreateHTMLTOCCSSFile();
    m_OPFModel->RenameResource(css_resource, SGC_TOC_CSS_FILENAME);
    emit BookContentModified();
    Refresh();
    return css_resource;
}

CSSResource* BookBrowser::CreateIndexCSSFile()
{
    CSSResource *css_resource = m_Book->CreateIndexCSSFile();
    m_OPFModel->RenameResource(css_resource, SGC_INDEX_CSS_FILENAME);
    emit BookContentModified();
    Refresh();
    return css_resource;
}

QStringList BookBrowser::AddExisting(bool only_multimedia, bool only_images)
{
    QStringList added_book_paths;

    bool replacements_made = false;

    QString filter_string = "";
    if (!QFileInfo(m_LastFolderOpen).exists()) {
        m_LastFolderOpen = "";
    }

    QFileDialog::Options options = QFileDialog::Options();
#ifdef Q_OS_MAC
    options = options | QFileDialog::DontUseNativeDialog;
#endif
    
    // filepaths are full absolute file paths to the files to be added
    QStringList filepaths = QFileDialog::getOpenFileNames(this,
                                                          tr("Add Existing Files"),
                                                          m_LastFolderOpen,
                                                          filter_string,
                                                          NULL,
                                                          options);

    if (filepaths.isEmpty()) {
        return added_book_paths;
    }

    m_LastFolderOpen = QFileInfo(filepaths.first()).absolutePath();
    QStringList invalid_filenames;
    HTMLResource *current_html_resource = qobject_cast<HTMLResource *>(GetCurrentResource());
    Resource *open_resource = NULL;
    // Display progress dialog if adding several items
    // Avoid dialog popping up over Insert File from disk for duplicate file all the time
    int progress_value = 0;
    int file_count = filepaths.count();
    // need to use the MainWindow as the parent of this QProgressDialog
    // and we can not make it modal as we may need to ask about replacing files of identical names
    QProgressDialog progress(QObject::tr("Adding Existing Files.."), 0, 0, file_count, Utility::GetMainWindow());
    if (file_count > 1) {
        progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
        progress.setValue(progress_value);
        // since not modal force it to be shown and move it to the top
        progress.show();
        progress.raise();
        progress.activateWindow();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    bool yes_to_all = false;
    bool no_to_all = false;
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
        bool CoverImageSemanticsSet = false;
        // try to see if an existing file has this filename and allow overwriting
        QString existing_book_path = m_Book->GetFolderKeeper()->GetBookPathByPathEnd(filename);

        if (!existing_book_path.isEmpty()) {
            // If this is an image prompt to replace it.
            if (IMAGE_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) ||
                SVG_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) ||
                VIDEO_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower()) ||
                AUDIO_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower())) {
                bool do_replacement = false;
                if (yes_to_all) do_replacement = true;
                if (no_to_all) do_replacement = false;
                if (!yes_to_all && !no_to_all) {
                   QMessageBox::StandardButton button_pressed;
                   button_pressed = Utility::warning(this, tr("Sigil"), 
                        tr("The multimedia file \"%1\" already exists in the book.\n\nOK to replace?").arg(filename),
                                  QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);

                   if (button_pressed == QMessageBox::YesToAll) {
                       yes_to_all = true;
                       do_replacement = true;
                   }
                   if (button_pressed == QMessageBox::NoToAll) {
                       no_to_all = true;
                       do_replacement = false;
                   }
                   if (button_pressed == QMessageBox::Yes) do_replacement = true;
                   if (button_pressed == QMessageBox::No) do_replacement = false;
                }

                if (!do_replacement) continue; 

                try {
                    Resource *old_resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(existing_book_path);
                    ImageResource* image_resource = qobject_cast<ImageResource *>(old_resource);
                    if (image_resource) {
                        CoverImageSemanticsSet = m_Book->GetOPF()->IsCoverImage(image_resource);
                    }
                    old_resource->Delete();
                    replacements_made = true;
                } catch (ResourceDoesNotExist&) {
                    Utility::DisplayStdErrorDialog(tr("Unable to delete or replace file \"%1\".").arg(filename)
                    );
                    continue;
                }
            } else {
                Utility::warning(this, tr("Sigil"), tr("Unable to load \"%1\"\n\nA file with this name already exists in the book.").arg(filename));
                continue;
            }
        }

        if (QFileInfo(filepath).fileName() == "page-map.xml") {
            Resource * res = m_Book->GetFolderKeeper()->AddContentFileToFolder(filepath, true, QString("application/oebps-page-map+xml"));
            added_book_paths << res->GetRelativePath(); 
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
            bool extract_metadata = false;
            html_import.GetBook(extract_metadata);
            QStringList importedbookpaths = html_import.GetAddedBookPaths();
            DBG qDebug() << "In BookBrowser Add Existing adding bookpaths: " << importedbookpaths;
            Resource *added_resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(importedbookpaths.at(0));
            HTMLResource *added_html_resource = qobject_cast<HTMLResource *>(added_resource);
            added_book_paths.append(importedbookpaths);
            if (current_html_resource && added_html_resource) {
                m_Book->MoveResourceAfter(added_html_resource, current_html_resource);
                current_html_resource = added_html_resource;

                // Only open HTML files as they are likely to be edited whereas other items
                // are likely to be inserted into or linked to the current file.
                // Only open the first file in any added group.
                if (!open_resource) {
                    open_resource = added_resource;
                }
            }
        } else {
            Resource *resource = m_Book->GetFolderKeeper()->AddContentFileToFolder(filepath);
            added_book_paths << resource->GetRelativePath();
            // if replacing a cover image, set the cover image semantics
            if (CoverImageSemanticsSet) {
                ImageResource* new_image_resource = qobject_cast<ImageResource *>(resource);
                if (new_image_resource) {
                    m_Book->GetOPF()->SetResourceAsCoverImage(new_image_resource);
                }
            }
            // TODO: adding a CSS file should add the referenced fonts too
            if (resource->Type() == Resource::CSSResourceType) {
                CSSResource *css_resource = qobject_cast<CSSResource *> (resource);
                css_resource->InitialLoad();
            }
        }

    }
    // turn off the QProgress Dialog by setting it as reaching its target
    progress.setValue(file_count);

    if (!invalid_filenames.isEmpty()) {
        Utility::warning(this, tr("Sigil"),
                             tr("The following file(s) were not loaded due to invalid content or not well formed XML:\n\n%1")
                             .arg(invalid_filenames.join("\n")));
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    // we still need to set modified and refresh if image file replacements have been done
    if (!added_book_paths.isEmpty() || replacements_made) {
        emit ResourcesAdded();
        if (open_resource) {
            emit ResourceActivated(open_resource);
        }
        emit BookContentModified();
        Refresh();
        emit ShowStatusMessageRequest(tr("File(s) added or replaced."));
    }
    QApplication::restoreOverrideCursor();

    return added_book_paths;
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
    QFileDialog::Options options = QFileDialog::Options();
#ifdef Q_OS_MAC
    options = options | QFileDialog::DontUseNativeDialog;
#endif
    
    QString destination = QFileDialog::getSaveFileName(this,
                          tr("Save As File"),
                          save_path,
                          filter_string,
                          &default_filter,
                          options
                                                       );

    if (destination.isEmpty()) {
        return;
    }

    // Store the folder the user saved to
    m_LastFolderSaveAs = QFileInfo(destination).absolutePath();
    m_Book->GetFolderKeeper()->SuspendWatchingResources();
    resource->SaveToDisk();
    m_Book->GetFolderKeeper()->ResumeWatchingResources();
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
    QFileDialog::Options options = QFileDialog::Options() | QFileDialog::ShowDirsOnly;
#ifdef Q_OS_MAC
    options = options | QFileDialog::DontUseNativeDialog;
#endif

    QString dirname = QFileDialog::getExistingDirectory(
                      this,
                      tr("Choose the directory to save the files to"),
                      m_LastFolderSaveAs,
                      options
                                                        );

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
        button_pressed = Utility::warning(this,
                                              tr("Sigil"), tr("One or more files already exists.  OK to overwrite?"),
                                              QMessageBox::Ok | QMessageBox::Cancel);

        if (button_pressed != QMessageBox::Ok) {
            return;
        }
    }

    m_LastFolderSaveAs = dirname;
    m_Book->GetFolderKeeper()->SuspendWatchingResources();
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
    m_Book->GetFolderKeeper()->ResumeWatchingResources();
}


void BookBrowser::OpenWithOtherApp()
{
    Resource *resource = GetCurrentResource();

    if (resource) {
        m_Book->GetFolderKeeper()->SuspendWatchingResources();
        resource->SaveToDisk();
        m_Book->GetFolderKeeper()->ResumeWatchingResources();
        const QString &editorPath = OpenExternally::selectEditorForResourceType(resource->Type(), this);

        if (!editorPath.isEmpty()) {
            if (OpenExternally::openFile(resource->GetFullPath(), editorPath)) {
                m_Book->GetFolderKeeper()->WatchResourceFile(resource);
            }
        }
    }
}


void BookBrowser::OpenWithClear()
{
    Resource *resource = GetCurrentResource();

    if (resource) {
        OpenExternally::clearEditorListForResourceType(resource->Type());
    }    
}


void BookBrowser::OpenWithEditor(int slotnum) const
{
    Resource *resource = GetCurrentResource();

    if (resource) {
        m_Book->GetFolderKeeper()->SuspendWatchingResources();
        resource->SaveToDisk();
        m_Book->GetFolderKeeper()->ResumeWatchingResources();
        QAction * oeaction = NULL;
        if (slotnum == 0) oeaction = m_OpenWithEditor0;
        if (slotnum == 1) oeaction = m_OpenWithEditor1;
        if (slotnum == 2) oeaction = m_OpenWithEditor2;
        if (slotnum == 3) oeaction = m_OpenWithEditor3;
        if (slotnum == 4) oeaction = m_OpenWithEditor4;
        if (oeaction) {
            const QVariant &editorPathData = oeaction->data();
            if (editorPathData.isValid()) {
                if (OpenExternally::openFile(resource->GetFullPath(), editorPathData.toString())) {
                    m_Book->GetFolderKeeper()->WatchResourceFile(resource);
                }
            }
        }
    }
}

// automate wholesale renaming
void BookBrowser::RenameResourceList(const QList<Resource*> &resources, const QStringList &newfilenames)
{
    m_OPFModel->RenameResourceList(resources, newfilenames);
    // Refresh();
}

// automate wholesale moves
void BookBrowser::MoveResourceList(const QList<Resource*> &resources, const QStringList & newbookpaths)
{
    m_OPFModel->MoveResourceList(resources, newbookpaths);
    // Refresh();
}


void BookBrowser::Rename()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    Resource::ResourceType resource_type = resources.first()->Type();
    Q_UNUSED(resource_type);

    if (resources.count() == 1) {
        // Save the resource so it can be re-selected
        m_RenamedResource = GetCurrentResource();
        // The actual renaming code is in OPFModel::ItemChangedHandler
        m_TreeView->edit(m_TreeView->currentIndex());
    } else {
        RenameSelected();
    }
}

void BookBrowser::REXRename()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    Resource::ResourceType resource_type = resources.first()->Type();
    Q_UNUSED(resource_type);
    
    QString retext;
    QString replacetext;
    int trycnt = 4;
    bool done = false;
    QStringList new_filenames;

    while(!done && (trycnt > 0)) {

        // Get the regular expression and its replacement from the user
        RERenamer renamer(retext, replacetext,this);
        if (renamer.exec() != QDialog::Accepted) {
            return;
        }

        retext = renamer.GetREText();
        replacetext = renamer.GetReplaceText();

        // Now make sure the user approves of the name changes
        RETable renametable(resources, retext, replacetext, this);
        if (renametable.exec() != QDialog::Accepted) {
            trycnt--;
        } else {
            new_filenames = renametable.GetNewNames();
            done = true;
        }
    }

    if (!done) return;

    // After a rename we want to keep the resources in the identical position.
    int scrollY = m_TreeView->verticalScrollBar()->value();

    // Rename the resources
    m_OPFModel->RenameResourceList(resources, new_filenames);

    SelectResources(resources);
    m_TreeView->verticalScrollBar()->setSliderPosition(scrollY);
}


void BookBrowser::Move()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    if (resources.count() == 1) {
        // Save the resource so it can be re-selected
        m_MovedResource = GetCurrentResource();
    }

    MoveSelected();
}

QString BookBrowser::GetFirstAvailableTemplateName(QString base, QString number_string)
{
    int number = number_string.toInt();

    QStringList book_filenames = m_Book->GetFolderKeeper()->GetAllFilenames();
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

    if (!template_name.isEmpty()) {
        // Verify the template name does not have any forbidden chars in the name
        const QString badchars = "<>:\"/\\|?*";
	bool has_bad_chars = false;
	foreach(QChar ch, template_name) {
	    has_bad_chars = has_bad_chars || badchars.contains(ch);
	}
        if (has_bad_chars) {
	    Utility::DisplayStdErrorDialog(tr("Filenames can not contain these characters: \"%1\".").arg(badchars));
	    return;
	}
    }

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
    QStringList book_filenames = m_Book->GetFolderKeeper()->GetAllFilenames();
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
            Utility::critical(this, tr("Sigil"), tr("Cannot rename files since this would result in duplicate filenames."));
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
    int scrollY = m_TreeView->verticalScrollBar()->value();

    // Rename the resources
    m_OPFModel->RenameResourceList(resources, new_filenames);

    SelectResources(resources);
    m_TreeView->verticalScrollBar()->setSliderPosition(scrollY);
}


void BookBrowser::MoveSelected()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    QString mediatype = resources.first()->GetMediaType();
    QString group = MediaTypes::instance()->GetGroupFromMediaType(mediatype);

    SelectFolder select_folder(group, m_Book, this);

    // Get the destination folder from the user
    if (select_folder.exec() != QDialog::Accepted) {
        return;
    }

    QString folder_path = select_folder.GetFolder();
    // Make sure folder_path is valid if abort
    bool valid_path = folder_path.indexOf("..") == -1;
    valid_path = valid_path && folder_path.indexOf("\\") == -1;
    valid_path = valid_path && !folder_path.startsWith("/");
    valid_path = valid_path && !folder_path.startsWith("./");
    valid_path = valid_path && !folder_path.startsWith(".");
    valid_path = valid_path && folder_path.indexOf("/.") == -1;
    valid_path = valid_path && !folder_path.startsWith("META-INF");
    valid_path = valid_path && folder_path.indexOf("META-INF") == -1;
    
    if (!valid_path) {
        Utility::DisplayStdErrorDialog(
            tr("Destination Folder has invalid path \"%1\"").arg(folder_path));
        return;  
    }

    // Make sure that any new folder gets created
    QString MainFolder = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();
    QDir epub_root(MainFolder);
    if (!folder_path.isEmpty()) {
        epub_root.mkpath(folder_path);
    }

    // if a new path tell FolderKeeper to add it
    QStringList group_folders = m_Book->GetFolderKeeper()->GetFoldersForGroup(group);
    if (!group_folders.contains(folder_path)) {
        group_folders << folder_path;
        m_Book->GetFolderKeeper()->SetFoldersForGroup(group, group_folders);
    }

    // Get the list of moves that will be done
    int resources_count = resources.count();
    
    QStringList new_bookpaths;
    QStringList existing_bookpaths = m_Book->GetFolderKeeper()->GetAllBookPaths();
    int num = 0;

    for (int i = 0; i < resources_count; i++) {
        QString filename = resources[i]->Filename();
        QString newbookpath = filename;
        if (!folder_path.isEmpty()) {
            newbookpath = folder_path + "/" + filename;
        }

        // Stop if the new bookpath already exists or will be used by a different entry
        if (existing_bookpaths.contains(newbookpath) || new_bookpaths.contains(newbookpath)) {
            Utility::critical(this, tr("Sigil"), tr("Cannot move files since this would result in duplicate filenames."));
            return;
        }

        new_bookpaths.append(newbookpath);

        num++;
    }


    // After a move we want to keep the resources in the identical position.
    int scrollY = m_TreeView->verticalScrollBar()->value();

    // Move the resources
    m_OPFModel->MoveResourceList(resources, new_bookpaths);

    SelectResources(resources);
    m_TreeView->verticalScrollBar()->setSliderPosition(scrollY);
    emit ResourcesMoved();
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

    QString version = m_Book->GetConstOPF()->GetEpubVersion();

    Resource *next_resource = NULL;

    Resource * nav_resource =  m_Book->GetConstOPF()->GetNavResource();
    if (nav_resource && resources.contains(nav_resource)) {
        Utility::DisplayStdErrorDialog(
            tr("The Nav document can not be removed.")
        );
        return;
    }
    // do the same for ncx under epub2
    NCXResource * ncx_resource = m_Book->GetNCX();
    if (ncx_resource && resources.contains(ncx_resource)) {
        Utility::DisplayStdErrorDialog(
            tr("The NCX can not be removed.")
        );
        return;
    }
    Resource::ResourceType resource_type = resources.first()->Type();
    if (resource_type == Resource::OPFResourceType) {
        Utility::DisplayStdErrorDialog(
            tr("The OPF is required for epub and can not be removed.")
        );
        return;
    }

    if (resource_type == Resource::HTMLResourceType &&
               resources.count() == m_Book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>().count()) {
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
        files_to_delete.append(resource->GetRelativePath());
    }
    // Confirm and select which files to delete
    // Note: DeleteFiles requires bookpaths for safety
    DeleteFiles delete_files(files_to_delete, this);
    connect(&delete_files, SIGNAL(OpenFileRequest(QString, int, int)), this, SIGNAL(OpenFileRequest(QString, int, int)));

    if (delete_files.exec() != QDialog::Accepted) {
        return;
    }

    files_to_delete = delete_files.GetFilesToDelete();

    if (files_to_delete.count() < 1) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    foreach(Resource * resource, resources) {
      if (!files_to_delete.contains(resource->GetRelativePath())) {
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
        emit ResourceActivated(next_resource);
    }


    // Delete the resources
    if (resources.size() > 50) {
        m_Book->GetFolderKeeper()->BulkRemoveResources(resources);
    } else {
        foreach(Resource * resource, resources) {
            resource->Delete();
        }
    }
    emit ResourcesDeleted();
    emit BookContentModified();
    // Avoid full refresh so selection stays for non-openable resources
    m_OPFModel->Refresh();
    RefreshCounts();

    if (keep_selection) {
        SelectResources(selected_resources);
    } else {
        if (next_resource) {
            UpdateSelection(next_resource);
        }
    }

    QApplication::restoreOverrideCursor();
}


Resource *BookBrowser::ResourceToSelectAfterRemove(QList<Resource *> selected_resources)
{
    if (selected_resources.isEmpty()) {
        return NULL;
    }

    QList <Resource *> all_resources = m_OPFModel->GetResourceListInFolder(selected_resources.first());

    if (all_resources.isEmpty()) {
        return NULL;
    }

    Resource *top_resource = NULL;
    Resource *bottom_resource = NULL;
    bool in_delete = false;
    foreach(Resource * all_resource, all_resources) {
        bool found = false;
        foreach(Resource * selected_resource, selected_resources) {
            if (all_resource->GetRelativePath() == selected_resource->GetRelativePath()) {
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
    // can not have multiple cover images
    if (resources.size() > 1) return;
    int scrollY = m_TreeView->verticalScrollBar()->value();
    
    ImageResource *image_resource = qobject_cast<ImageResource *>(GetCurrentResource());
    if (image_resource == NULL) {
        emit ShowStatusMessageRequest(tr("Unable to set file as cover image."));
        return;
    }
    m_Book->GetOPF()->SetResourceAsCoverImage(image_resource);
    m_OPFModel->Refresh();
    emit BookContentModified();

    SelectResources(resources);
    m_TreeView->verticalScrollBar()->setSliderPosition(scrollY);
}

QString BookBrowser::BuildListMD(const QStringList& lst)
{
    QString res="";
    foreach(QString rec, lst) {
        if (!rec.isEmpty()) {
            res = res + "- " + rec + "\n";
        } else {
            res = res + " \n";
        }
    }
    return res;
}

void BookBrowser::GetInfo()
{
    QList <Resource *> resources = ValidSelectedResources();
    int scrollY = m_TreeView->verticalScrollBar()->value();

    if (resources.count()!= 1) {
        return;
    }

    Resource * resource = resources.first();

    QString bookpath = resource->GetRelativePath();
    QString folder_path = resource->GetFolder();
    if (folder_path == ".") folder_path = tr("(root folder)");
    
    QString fullfilepath = resource->GetFullPath();
    QString version = resource->GetEpubVersion();
    double ffsize = QFile(fullfilepath).size() / 1024.0;
    QString fsize = QLocale().toString(ffsize, 'f', 2);
    
    QStringList mdlst;
    mdlst << "# "+ bookpath + "\n";

    mdlst << tr("FileName");
    mdlst << "- " +  resource->Filename() + "\n";

    mdlst <<  tr("Folder");
    mdlst << "- " + folder_path + "\n";

    mdlst << tr("Media Type");
    mdlst << "- " + resource->GetMediaType() + "\n";

    mdlst << tr("File Size(kb)");
    mdlst << "- " + fsize + "\n";

    mdlst << tr("Epub Version");
    mdlst << "- " + version + " \n";

    if (resource->Type() == Resource::HTMLResourceType) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (html_resource) {

            QString source = html_resource->GetText();

            QString primary_lang = html_resource->GetLanguageAttribute();
            // fallback to the primary language specified in the OPF metadata
            if (primary_lang.isEmpty()) {
                primary_lang = m_Book->GetOPF()->GetPrimaryBookLanguage();
            }

            int word_count = HTMLSpellCheckML::GetAllWords(source, primary_lang).size();

            mdlst << tr("Word Count");
            mdlst << "- " + QString::number(word_count) + "\n";
    
            mdlst << tr("Primary Language");
            if (!primary_lang.isEmpty()) {
                mdlst << "- " + primary_lang +  "\n";
            } else {
                mdlst << QString(" \n");
            }

            mdlst << tr("WellFormed");
            if (html_resource->FileIsWellFormed()) {
                mdlst << "- " + tr("Yes") + "\n";
            } else {
                mdlst << "- " + tr("No") + "\n";
            }

            mdlst << tr("Linked Stylesheets");
            mdlst << BuildListMD(XhtmlDoc::GetLinkedStylesheets(source)) + "\n";

            mdlst << tr("Linked Javascripts");
            mdlst << BuildListMD(XhtmlDoc::GetLinkedJavascripts(source)) + "\n";

            mdlst << tr("Linked Images");
            mdlst << BuildListMD(XhtmlDoc::GetAllMediaPathsFromMediaChildren(source, GIMAGE_TAGS)) + "\n";

            mdlst << tr("Linked Audio");
            mdlst << BuildListMD(XhtmlDoc::GetAllMediaPathsFromMediaChildren(source, GAUDIO_TAGS)) + "\n";

            mdlst << tr("Linked Video");
            mdlst << BuildListMD( XhtmlDoc::GetAllMediaPathsFromMediaChildren(source, GVIDEO_TAGS)) + "\n";

            // semantics
            HTMLResource * nav_resource = nullptr;
            if (version.startsWith('3')) {
                nav_resource = m_Book->GetConstOPF()->GetNavResource();
            }
            QStringList full_semantic_info;
            QStringList semantics;
            if (version.startsWith("3")) {
                NavProcessor navproc(nav_resource);
                full_semantic_info = navproc.GetAllLandmarkInfoByBookPath();
            } else {
                full_semantic_info = m_Book->GetOPF()->GetAllGuideInfoByBookPath();
            }
            foreach(QString rec, full_semantic_info) {
                QStringList parts = rec.split(QChar(30), Qt::KeepEmptyParts);
                qDebug() << parts.at(0) << parts.at(1) << parts.at(2) << parts.at(3);
                if (parts.at(0) == bookpath) {
                    if (parts.at(1).isEmpty()) {
                        semantics << parts.at(2) + ": " + parts.at(3);
                    } else {
                        semantics << parts.at(2) + ": " + parts.at(3) + " id=\"" + parts.at(1) + "\"";
                    }
                }
            }
            mdlst <<  tr("Semantics OPF Guide or Nav Landmarks");
            mdlst << BuildListMD(semantics) + "\n";

            // manifest properties
            QStringList properties;
            if (version.startsWith('3')) {
                properties = html_resource->GetManifestProperties();
                if (html_resource == nav_resource) properties << "nav";
            }
            mdlst <<  tr("Manifest Properties");
            mdlst << BuildListMD(properties) + "\n";

            mdlst << tr("Defined Ids");
            mdlst << BuildListMD(XhtmlDoc::GetAllDescendantIDs(source)) + "\n";
        }
    }
    if (resource->Type() == Resource::FontResourceType) {
        FontResource *font_resource = qobject_cast<FontResource *>(resource);
        if (font_resource) {
            mdlst << tr("Description");
            mdlst << "- " + font_resource->GetDescription() + "\n";

            mdlst << tr("Obfuscation Algorithm");
            QString obfuscate = font_resource->GetObfuscationAlgorithm();
            if (obfuscate.isEmpty()) obfuscate = tr("None");
            mdlst << "- " + obfuscate + "\n";
        }
    }
    if (resource->Type() == Resource::ImageResourceType) {
        ImageResource *image_resource = qobject_cast<ImageResource *>(resource);
        if (image_resource) {
            mdlst << tr("Description");
            mdlst << "- " + image_resource->GetDescription() + "\n";
        }
    }
        

    QString mdsrc = mdlst.join("\n");

    MDViewer mdv(mdsrc);
    mdv.exec();

    SelectResources(resources);
    m_TreeView->verticalScrollBar()->setSliderPosition(scrollY);
}
            
void BookBrowser::AddSemanticCode()
{
    QList <Resource *> resources = ValidSelectedResources();
    int scrollY = m_TreeView->verticalScrollBar()->value();

    if (resources.count()!= 1) {
        return;
    }

    Resource * resource = resources.first();
    if (resource->Type() != Resource::HTMLResourceType) {
        return;
    }
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);

    QString tgt_id = "";
    QString xhtmlsrc = html_resource->GetText();
    QStringList idlst = XhtmlDoc::GetAllDescendantIDs(xhtmlsrc);
    // run the dialog only if there are  ids defined in the xhtml source
    if (!idlst.isEmpty()) {
        SemanticTargetID getTarget(html_resource, idlst, this);
        if (getTarget.exec() == QDialog::Accepted) {
            tgt_id = getTarget.GetID();
        }
    }
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    HTMLResource * nav_resource = NULL;
    if (version.startsWith('3')) {
        nav_resource = m_Book->GetConstOPF()->GetNavResource();
    }

    QStringList codes;
    QString current_code;

    if (version.startsWith('3')) {
        NavProcessor navproc(nav_resource);
        current_code = navproc.GetLandmarkCodeForResource(resource, tgt_id);
    } else { 
        current_code = m_Book->GetOPF()->GetGuideSemanticCodeForResource(resource, tgt_id);
    }

    if (version.startsWith('3')) {
        AddSemantics addmeaning(Landmarks::instance()->GetCodeMap(), current_code, this);
        if (addmeaning.exec() == QDialog::Accepted) {
            codes = addmeaning.GetSelectedEntries();
            if (!codes.isEmpty()) {
                QString new_code = codes.at(0);
                // do allow a user to change only the toc semantics on the nav resource
                if ((html_resource != nav_resource) || (new_code == "toc")) {
                    NavProcessor navproc(nav_resource);
                    navproc.AddLandmarkCode(html_resource, new_code, true, tgt_id);
                    m_OPFModel->Refresh();
                    emit BookContentModified();
                } 
            }
        }
    } else {
        AddSemantics addmeaning(GuideItems::instance()->GetCodeMap(), current_code, this);
        if (addmeaning.exec() == QDialog::Accepted) {
            codes = addmeaning.GetSelectedEntries();
            if (!codes.isEmpty()) {
                m_Book->GetOPF()->AddGuideSemanticCode(html_resource, codes.at(0), true, tgt_id);
                m_OPFModel->Refresh();
                emit BookContentModified();
            }
        }
    }
    SelectResources(resources);
    m_TreeView->verticalScrollBar()->setSliderPosition(scrollY);
}


void BookBrowser::Merge()
{
    int scrollY = m_TreeView->verticalScrollBar()->value();
    emit MergeResourcesRequest(ValidSelectedResources(Resource::HTMLResourceType));
    m_TreeView->verticalScrollBar()->setSliderPosition(scrollY);
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

void BookBrowser::LinkJavascripts()
{
    QList <Resource *> resources = ValidSelectedResources();

    if (resources.isEmpty()) {
        return;
    }

    Resource::ResourceType resource_type = resources.first()->Type();

    if (resource_type != Resource::HTMLResourceType) {
        return;
    }

    emit LinkJavascriptsToResourcesRequest(ValidSelectedResources(Resource::HTMLResourceType));
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
    m_TreeView->expand(m_OPFModel->GetTextFolderModelIndex());
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
    m_TreeView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    m_TreeView->setSortingEnabled(false);
    m_TreeView->sortByColumn(-1, Qt::AscendingOrder);
    m_TreeView->setUniformRowHeights(true);
    m_TreeView->setDragEnabled(true);
    m_TreeView->setAcceptDrops(false);
    m_TreeView->setDropIndicatorShown(true);
    m_TreeView->setDragDropMode(QAbstractItemView::InternalMove);
    m_TreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_TreeView->setItemDelegate(new FilenameDelegate);
    m_TreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_TreeView->setModel(m_OPFModel);

    for (int i = 1; i < m_OPFModel->columnCount(); ++i) {
        m_TreeView->hideColumn(i);
    }

    m_TreeView->setIndentation(COLUMN_INDENTATION);
    m_TreeView->setHeaderHidden(true);
    m_TreeView->setTextElideMode(Qt::ElideMiddle);
}


void BookBrowser::CreateContextMenuActions()
{
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
    m_SelectAll               = new QAction(tr("Select All"),               this);
    m_AddNewHTML              = new QAction(tr("Add Blank HTML File"),      this);
    m_AddNewCSS               = new QAction(tr("Add Blank Stylesheet"),     this);
    m_AddNewJS                = new QAction(tr("Add Blank Javascript"),     this);
    m_AddNewSVG               = new QAction(tr("Add Blank SVG Image"),      this);
    m_AddExisting             = new QAction(tr("Add Existing Files..."),    this);
    m_CopyHTML                = new QAction(tr("Add Copy"),                 this);
    m_CopyCSS                 = new QAction(tr("Add Copy"),                 this);
    m_Rename                  = new QAction(tr("Rename") + "...",           this);
    m_RERename                = new QAction(tr("RegEx Rename") + "...",     this);
    m_Move                    = new QAction(tr("Move") + "...",             this);
    m_Delete                  = new QAction(tr("Delete") + "...",           this);
    m_CoverImage              = new QAction(tr("Cover Image"),              this);
    m_Merge                   = new QAction(tr("Merge"),                    this);
    m_NoObfuscationMethod     = new QAction(tr("None"),                     this);
    m_AdobesObfuscationMethod = new QAction(tr("Use Adobe's Method"),       this);
    m_IdpfsObfuscationMethod  = new QAction(tr("Use IDPF's Method"),        this);
    m_SortHTML                = new QAction(tr("Sort") + "...",             this);
    m_RenumberTOC             = new QAction(tr("Renumber TOC Entries"),     this);
    m_LinkStylesheets         = new QAction(tr("Link Stylesheets..."),      this);
    m_LinkJavascripts         = new QAction(tr("Link Javascripts..."),      this);
    m_AddSemantics            = new QAction(tr("Add Semantics..."),         this);
    m_GetInfo                 = new QAction(tr("Get Info..."),              this);
    m_ValidateWithW3C         = new QAction(tr("Validate with W3C"),        this);
    m_SaveAs                  = new QAction(tr("Save As") + "...",          this);
    m_OpenWithEditor0         = new QAction("",                             this);
    m_OpenWithEditor1         = new QAction("",                             this);
    m_OpenWithEditor2         = new QAction("",                             this);
    m_OpenWithEditor3         = new QAction("",                             this);
    m_OpenWithEditor4         = new QAction("",                             this);
    m_OpenWithOtherApp        = new QAction(tr("Other Application")+"...",  this);
    m_OpenWithClear           = new QAction(tr("Clear Editor List"),        this);
    m_CoverImage             ->setCheckable(true);
    m_NoObfuscationMethod    ->setCheckable(true);
    m_AdobesObfuscationMethod->setCheckable(true);
    m_IdpfsObfuscationMethod ->setCheckable(true);
    m_CopyHTML->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Y));
    sm->registerAction(this, m_CopyHTML, "MainWindow.BookBrowser.CopyHTML");
    m_Delete->setShortcut(QKeySequence::Delete);
    m_Merge->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    m_Merge->setToolTip(tr("Merge with previous file, or merge multiple files into one."));
    sm->registerAction(this, m_Merge, "MainWindow.BookBrowser.Merge");
    m_Rename->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R));
    m_Rename->setToolTip(tr("Rename selected file(s)"));
    sm->registerAction(this, m_Rename, "MainWindow.BookBrowser.Rename");
    m_RERename->setToolTip(tr("Use Regular Expressions to Rename selected file(s)"));
    sm->registerAction(this, m_RERename, "MainWindow.BookBrowser.RERename");
    // m_Move->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R));
    m_Move->setToolTip(tr("Move selected file(s) to a new folder"));
    sm->registerAction(this, m_Move, "MainWindow.BookBrowser.Move");
    m_LinkStylesheets->setToolTip(tr("Link Stylesheets to selected file(s)."));
    sm->registerAction(this, m_LinkStylesheets, "MainWindow.BookBrowser.LinkStylesheets");
    m_LinkJavascripts->setToolTip(tr("Link Javascripts to selected file(s)."));
    sm->registerAction(this, m_LinkJavascripts, "MainWindow.BookBrowser.LinkJavascripts");
    m_AddSemantics->setToolTip(tr("Add Semantics to selected file(s)."));
    sm->registerAction(this, m_AddSemantics, "MainWindow.BookBrowser.AddSemantics");
    m_GetInfo->setToolTip(tr("Show Information about selected file."));
    sm->registerAction(this, m_GetInfo, "MainWindow.BookBrowser.GetInfo");
    // Has to be added to the book browser itself as well
    // for the keyboard shortcut to work.
    addAction(m_CopyHTML);
    addAction(m_Delete);
    addAction(m_Merge);
    addAction(m_Rename);
    addAction(m_RERename);
    addAction(m_Move);
    addAction(m_LinkStylesheets);
    addAction(m_LinkJavascripts);
    addAction(m_AddSemantics);
    addAction(m_GetInfo);
}


bool BookBrowser::SuccessfullySetupContextMenu(const QPoint &point)
{
    QModelIndex index = m_TreeView->indexAt(point);

    if (!index.isValid()) {
        return false;
    }

    int item_count = ValidSelectedItemCount();

    if (item_count < 1) {
        return false;
    }

    QStandardItem *item = m_OPFModel->itemFromIndex(index);
    Q_ASSERT(item);
    m_LastContextMenuType = m_OPFModel->GetResourceType(item);
    QList<Resource *> resources = m_OPFModel->GetResourceListInFolder(m_LastContextMenuType);
    Resource *resource = GetCurrentResource();

    if (resource) {
        m_ContextMenu->addSeparator();

        // Normal case handle Delete, Rename, and Move
        if (m_LastContextMenuType != Resource::OPFResourceType &&
            m_LastContextMenuType != Resource::NCXResourceType) {
            m_ContextMenu->addAction(m_Delete);
            m_Delete->setEnabled(m_LastContextMenuType != Resource::HTMLResourceType ||
                                 (AllHTMLResources().count() > 1 && resources.count() != item_count));
            m_ContextMenu->addAction(m_Rename);
            m_ContextMenu->addAction(m_RERename);
            m_ContextMenu->addAction(m_Move);
        }
        if (resource->Type() == Resource::HTMLResourceType) {
            m_ContextMenu->addAction(m_Merge);
            m_Merge->setEnabled(item_count > 1 ||
                                (AllHTMLResources().count() > 1 &&
                                 AllHTMLResources().at(0) != ValidSelectedResources().at(0)));
            m_ContextMenu->addAction(m_SortHTML);
            m_SortHTML->setEnabled(item_count > 1);
            m_ContextMenu->addAction(m_LinkStylesheets);
            m_LinkStylesheets->setEnabled(AllCSSResources().count() > 0);
            m_ContextMenu->addAction(m_LinkJavascripts);
            m_LinkJavascripts->setEnabled(AllJSResources().count() > 0);
            m_ContextMenu->addAction(m_AddSemantics);
            m_ContextMenu->addAction(m_GetInfo);
        }

        if (resource->Type() == Resource::FontResourceType) {
            SetupFontObfuscationMenu();
            m_ContextMenu->addAction(m_GetInfo);
        }

        if (resource->Type() == Resource::OPFResourceType) {
            m_ContextMenu->addAction(m_Rename);
            m_ContextMenu->addAction(m_Move);
            m_ContextMenu->addAction(m_GetInfo);
        }

        if (resource->Type() == Resource::NCXResourceType) {
            m_ContextMenu->addAction(m_RenumberTOC);
            m_ContextMenu->addAction(m_Rename);
            m_ContextMenu->addAction(m_Move);
            m_ContextMenu->addAction(m_GetInfo);
        }

        if (resource->Type() == Resource::CSSResourceType) {
            m_ContextMenu->addAction(m_ValidateWithW3C);
            m_ContextMenu->addAction(m_GetInfo);
        }

        if (resource->Type() == Resource::ImageResourceType) {
            SetupImageSemanticContextMenu(resource);
            m_ContextMenu->addAction(m_GetInfo);
        }

        m_ContextMenu->addSeparator();

        // Open With
        if (OpenExternally::mayOpen(resource->Type())) {
            const QStringList editor_paths = OpenExternally::editorsForResourceType(resource->Type());
            const QStringList editor_names = OpenExternally::editorDescriptionsForResourceType(resource->Type());
            if (editor_paths.isEmpty()) {
                m_OpenWithEditor0->setData(QVINVALID);
                m_OpenWithEditor1->setData(QVINVALID);
                m_OpenWithEditor2->setData(QVINVALID);
                m_OpenWithEditor3->setData(QVINVALID);
                m_OpenWithEditor4->setData(QVINVALID);
                m_OpenWithClear->setEnabled(false);
                m_OpenWithClear->setVisible(false);
                m_OpenWithOtherApp->setVisible(item_count == 1);
                m_OpenWithOtherApp->setEnabled(item_count == 1);
                m_OpenWithOtherApp->setText(tr("Open With") + "...");
                m_ContextMenu->addAction(m_OpenWithOtherApp);
                                
            } else {
                // clear previous open with action info
                for (int k = 0; k < 5; k++) {
                    QAction * oeaction = NULL;
                    if (k==0) oeaction = m_OpenWithEditor0;
                    if (k==1) oeaction = m_OpenWithEditor1;
                    if (k==2) oeaction = m_OpenWithEditor2;
                    if (k==3) oeaction = m_OpenWithEditor3;
                    if (k==4) oeaction = m_OpenWithEditor4;
                    if (oeaction) {
                        oeaction->setData(QVINVALID);
                        oeaction->setText("");
                        oeaction->setEnabled(false);
                        oeaction->setVisible(false);
                    }
                }
                int i = 0;
                foreach(QString apath, editor_paths) {
                    QString aprettyname = editor_names[i];
                    QAction * oeaction = NULL;
                    if (i == 0) oeaction = m_OpenWithEditor0;
                    if (i == 1) oeaction = m_OpenWithEditor1;
                    if (i == 2) oeaction = m_OpenWithEditor2;
                    if (i == 3) oeaction = m_OpenWithEditor3;
                    if (i == 4) oeaction = m_OpenWithEditor4;
                    if (oeaction) {
                        oeaction->setText(aprettyname);
                        oeaction->setData(apath);
                        oeaction->setEnabled(item_count == 1);
                        oeaction->setVisible(true);
                    }
                    i = i + 1;
                }
                m_OpenWithClear->setEnabled(item_count == 1);
                m_OpenWithClear->setVisible(item_count == 1);
                m_OpenWithOtherApp->setEnabled(item_count == 1);
                m_OpenWithOtherApp->setVisible(item_count == 1);
                m_OpenWithOtherApp->setText(tr("Other Application") + "...");
                m_OpenWithContextMenu->setEnabled(item_count == 1);
                m_ContextMenu->addMenu(m_OpenWithContextMenu);
                
            }
        }

        // Save As
        m_ContextMenu->addAction(m_SaveAs);
    }

    // Applies to Menus and Resources
    // Add Existing and Add specific file types
    m_ContextMenu->addSeparator();

    if (m_LastContextMenuType == Resource::HTMLResourceType) {
        m_ContextMenu->addAction(m_AddNewHTML);
        m_ContextMenu->addAction(m_CopyHTML);
        m_CopyHTML->setEnabled(item_count == 1);
    } else if (m_LastContextMenuType == Resource::CSSResourceType) {
        m_ContextMenu->addAction(m_AddNewCSS);
        m_ContextMenu->addAction(m_CopyCSS);
        m_CopyCSS->setEnabled(item_count == 1);
    } else if (m_LastContextMenuType == Resource::MiscTextResourceType ||
               m_LastContextMenuType == Resource::GenericResourceType) {
        QString version = m_Book->GetConstOPF()->GetEpubVersion();
        if (version.startsWith('3')) {
            m_ContextMenu->addAction(m_AddNewJS);
        }
    } else if (m_LastContextMenuType == Resource::ImageResourceType || m_LastContextMenuType == Resource::SVGResourceType) {
        m_ContextMenu->addAction(m_AddNewSVG);
    }

    m_ContextMenu->addAction(m_AddExisting);

    // Select All
    if (m_LastContextMenuType != Resource::OPFResourceType &&
        m_LastContextMenuType != Resource::NCXResourceType) {
        m_ContextMenu->addSeparator();
        m_ContextMenu->addAction(m_SelectAll);
        m_SelectAll->setEnabled(item_count > 0 || (!resource && resources.count() > 0));
    }

    return true;
}

void BookBrowser::SetupImageSemanticContextMenu(Resource *resource)
{
    m_ContextMenu->addAction(m_CoverImage);
    ImageResource *image_resource = qobject_cast<ImageResource *>(GetCurrentResource());
    Q_ASSERT(image_resource);
    m_CoverImage->setChecked(false);

    if (m_Book->GetOPF()->IsCoverImage(image_resource)) {
        m_CoverImage->setChecked(true);
    }
}

void BookBrowser::SetupFontObfuscationMenu()
{
    m_FontObfuscationContextMenu->addAction(m_NoObfuscationMethod);
    m_FontObfuscationContextMenu->addAction(m_AdobesObfuscationMethod);
    m_FontObfuscationContextMenu->addAction(m_IdpfsObfuscationMethod);
    SetFontObfuscationActionCheckState();
    m_ContextMenu->addMenu(m_FontObfuscationContextMenu);
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
    connect(m_TreeView, SIGNAL(activated(const QModelIndex &)),
            this,         SLOT(EmitResourceActivated(const QModelIndex &)));
    connect(m_TreeView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this,        SLOT(OpenContextMenu(const QPoint &)));
    connect(m_OPFModel, SIGNAL(ResourceRenamed()),
            this,        SLOT(SelectRenamedResource()));
    connect(m_OPFModel, SIGNAL(ResourceMoved()),
            this,        SLOT(SelectMovedResource()));
    connect(m_SelectAll,               SIGNAL(triggered()), this, SLOT(SelectAll()));
    connect(m_CopyHTML,                SIGNAL(triggered()), this, SLOT(CopyHTML()));
    connect(m_CopyCSS,                 SIGNAL(triggered()), this, SLOT(CopyCSS()));
    connect(m_AddNewHTML,              SIGNAL(triggered()), this, SLOT(AddNewHTML()));
    connect(m_RenumberTOC,             SIGNAL(triggered()), this, SLOT(RenumberTOC()));
    connect(m_SortHTML,                SIGNAL(triggered()), this, SLOT(SortHTML()));
    connect(m_AddNewCSS,               SIGNAL(triggered()), this, SLOT(AddNewCSS()));
    connect(m_AddNewJS,                SIGNAL(triggered()), this, SLOT(AddNewJS()));
    connect(m_AddNewSVG,               SIGNAL(triggered()), this, SLOT(AddNewSVG()));
    connect(m_AddExisting,             SIGNAL(triggered()), this, SLOT(AddExisting()));
    connect(m_Rename,                  SIGNAL(triggered()), this, SLOT(Rename()));
    connect(m_RERename,                SIGNAL(triggered()), this, SLOT(REXRename()));
    connect(m_Move,                    SIGNAL(triggered()), this, SLOT(Move()));
    connect(m_Delete,                  SIGNAL(triggered()), this, SLOT(Delete()));
    connect(m_CoverImage,              SIGNAL(triggered()), this, SLOT(SetCoverImage()));
    connect(m_Merge,                   SIGNAL(triggered()), this, SLOT(Merge()));
    connect(m_LinkStylesheets,         SIGNAL(triggered()), this, SLOT(LinkStylesheets()));
    connect(m_LinkJavascripts,         SIGNAL(triggered()), this, SLOT(LinkJavascripts()));
    connect(m_AddSemantics,            SIGNAL(triggered()), this, SLOT(AddSemanticCode()));
    connect(m_GetInfo,                 SIGNAL(triggered()), this, SLOT(GetInfo()));
    connect(m_SaveAs,                  SIGNAL(triggered()), this, SLOT(SaveAs()));
    connect(m_ValidateWithW3C,         SIGNAL(triggered()), this, SLOT(ValidateStylesheetWithW3C()));
    connect(m_OpenWithEditor0, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor0, 0);
    connect(m_OpenWithEditor1, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor1, 1);
    connect(m_OpenWithEditor2, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor2, 2);
    connect(m_OpenWithEditor3, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor3, 3);
    connect(m_OpenWithEditor4, SIGNAL(triggered()),  m_openWithMapper, SLOT(map()));
    m_openWithMapper->setMapping(m_OpenWithEditor4, 4);
    connect(m_openWithMapper, SIGNAL(mappedInt(int)), this, SLOT(OpenWithEditor(int)));
    connect(m_OpenWithClear, SIGNAL(triggered()), this, SLOT(OpenWithClear()));
    connect(m_OpenWithOtherApp, SIGNAL(triggered()), this, SLOT(OpenWithOtherApp()));
    connect(m_AdobesObfuscationMethod, SIGNAL(triggered()), this, SLOT(AdobesObfuscationMethod()));
    connect(m_IdpfsObfuscationMethod,  SIGNAL(triggered()), this, SLOT(IdpfsObfuscationMethod()));
    connect(m_NoObfuscationMethod,     SIGNAL(triggered()), this, SLOT(NoObfuscationMethod()));

}


Resource *BookBrowser::GetCurrentResource() const
{
    return GetResourceByIndex(m_TreeView->currentIndex());
}


Resource *BookBrowser::GetResourceByIndex(QModelIndex index) const
{
    if (!index.isValid()) {
        return NULL;
    }

    QStandardItem *item = m_OPFModel->itemFromIndex(index);
    Q_ASSERT(item);
    const QString &identifier = item->data().toString();
    return m_Book->GetFolderKeeper()->GetResourceByIdentifier(identifier);
}
