/****************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, ON Canada
**  Based on the MetaEditor, AddMetadata Dialogs design
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
#include <QChar>
#include <QString>
#include <QFileInfo>
#include <QTextStream>
#include <QDate>
#include <QFile>
#include <QShortcut>
#include <QInputDialog>
#include <QDebug>

#include "Dialogs/TreeModel.h"
#include "Dialogs/AddAutomateTool.h"
#include "Dialogs/AddAutomatePlugin.h"
#include "MainUI/MainWindow.h"
#include "Misc/SettingsStore.h"
#include "Misc/Plugin.h"
#include "Misc/PluginDB.h"
#include "Dialogs/AutomateEditor.h"


static const QString SETTINGS_GROUP = "automate_editor";
static const QString _IN = "  ";
static const QString _GS = QString(QChar(29)); // Ascii Group Separator
static const QString _RS = QString(QChar(30)); // Ascii Record Separator
static const QString _US = QString(QChar(31)); // Ascii Unit Separator

AutomateEditor::AutomateEditor(const QString& automate_path, QWidget *parent)
  : QDialog(parent),
    m_mainWindow(qobject_cast<MainWindow *>(parent)),
    m_RemoveRow(new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Delete),this, 0, 0, Qt::WidgetWithChildrenShortcut)),
    m_automate_path(automate_path)
{
    setupUi(this);
    view->setTextElideMode(Qt::ElideNone);
    view->setUniformRowHeights(false);
    view->setWordWrap(true);
    loadToolElements();
    loadPluginElements();
    
    QStringList headers;
    headers << tr("Command") << tr("Parameter");

    // Read in any existing automate path file
    QString data = GetAutomateList();
    qDebug() << "using data: " << data;
    TreeModel *model = new TreeModel(headers, data);
    view->setModel(model);
    for (int column = 0; column < model->columnCount(); ++column) {
        if (column != 1) {
            view->resizeColumnToContents(column);
        } else {
            view->setColumnWidth(column,300);
        }
    }
    
    if (!isVisible()) {
        ReadSettings();
    }

    connect(view->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &,
                                    const QItemSelection &)),
            this, SLOT(updateActions()));

    connect(delButton, SIGNAL(clicked()), this, SLOT(removeRow()));
    connect(tbMoveUp, SIGNAL(clicked()), this, SLOT(moveRowUp()));
    connect(tbMoveDown, SIGNAL(clicked()), this, SLOT(moveRowDown()));
    connect(m_RemoveRow, SIGNAL(activated()), this, SLOT(removeRow()));

    connect(addToolButton,   SIGNAL(clicked()), this, SLOT(selectTool()));
    connect(addPluginButton, SIGNAL(clicked()), this, SLOT(selectPlugin()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveData()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    updateActions();
}


AutomateEditor::~AutomateEditor()
{
    m_RemoveRow->deleteLater();
}


QString AutomateEditor::GetAutomateList()
{
    QStringList autodata;
    if (QFile::exists(m_automate_path)) {
        QString data = Utility::ReadUnicodeTextFile(m_automate_path);
        QStringList datalines = data.split('\n');
        QStringList commands;
        foreach(QString aline, datalines) {
            QString cmd = aline.trimmed();
            QString value = "";
            if (cmd.startsWith("RunSavedSearchReplaceAll")) {
                value = cmd.mid(25,-1).trimmed();
                cmd = "RunSavedSearchReplaceAll";
            } else if (cmd.startsWith("SetPluginParameter")) {
                value = cmd.mid(19,-1).trimmed();
                cmd = "SetPluginParameter";
            }
            autodata << cmd + _GS + "" + _US + value + _GS + "" + _RS;
        }
    }
    return autodata.join("");
}


QString AutomateEditor::SetNewAutomateList(QString& data) 
{
    QString newdata = "";
    QStringList dlist = data.split(_RS);
    QStringList nlist;
    foreach(QString rc, dlist) {
            // treat as element with content
            QStringList parts = rc.split(_US);
            QString value = parts.at(1);
            QString aline = parts.at(0);
            if (!value.isEmpty()) {
                aline = aline + " " + value;
            }
            nlist << aline;
    }
    newdata = nlist.join('\n');
    if (!newdata.endsWith('\n')) newdata = newdata + "\n";
    Utility::WriteUnicodeTextFile(newdata, m_automate_path);
    return newdata;
}


void AutomateEditor::selectTool()
{
    QStringList codes;
    {
         AddAutomateTool addelement(m_ToolInfo, this);
         if (addelement.exec() == QDialog::Accepted) {
            codes = addelement.GetSelectedEntries();
         }
    }
    foreach(QString code, codes) {
        if (code == "RunSavedSearchReplaceAll") {
            QString content = tr("[SavedSearch full name here]");
            insertRow(code, code, content, "");
        } else if (code == "SetPluginParameter") {
            QString content = tr("[String parameter for next Plugin run here]");
            insertRow(code, code, content, "");
        } else {
            insertRow(code, code, "", "");
        }
    }
}

void AutomateEditor::selectPlugin()
{
    QStringList codes;
    {
         AddAutomatePlugin addelement(m_PluginInfo, this);
         if (addelement.exec() == QDialog::Accepted) {
            codes = addelement.GetSelectedEntries();
         }
    }
    foreach(QString code, codes) {
        insertRow(code, code, "", "");
    }
}


void AutomateEditor::saveData()
{
    WriteSettings();

    TreeModel *model = qobject_cast<TreeModel *>(view->model());
    QString data = model->getAllModelData();
    qDebug() << "received from model: " << data;
    QString newdata = SetNewAutomateList(data);
    qDebug() << "wrote out: " << newdata;
    QDialog::accept();
}

void AutomateEditor::reject()
{
    WriteSettings();
    QDialog::reject();
}


// used to insert a new automate element into the tree
void AutomateEditor::insertRow(const QString& code, const QString& tip, const QString& contents, const QString& vtip)
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    // force all row insertions to be children of the root item
    while(index.parent() != QModelIndex()) {
        index = index.parent();
    }

    if (!model->insertRow(index.row()+1, index.parent()))
        return;


    updateActions();

    QModelIndex child = model->index(index.row()+1, 0, index.parent());
    model->setData(child, QVariant(code), Qt::EditRole);
    model->setData(child, QVariant(tip), Qt::ToolTipRole);
    for (int column = 1; column < model->columnCount(index.parent()); ++column) {
        QModelIndex nchild = model->index(index.row()+1, column, index.parent());
        if (!contents.isEmpty()) {
            model->setData(nchild, QVariant(contents), Qt::EditRole);
            model->setData(nchild, QVariant(vtip), Qt::ToolTipRole);
        } else {
            model->setData(nchild, QVariant(""), Qt::EditRole);
            model->setData(nchild, QVariant(""), Qt::ToolTipRole);
        }
    }

    // force newly inserted row to be the currently selected item so that any
    // follow-on insertChild calls use this as their parent.
    view->selectionModel()->setCurrentIndex(child, QItemSelectionModel::ClearAndSelect);
    updateActions();
}


void AutomateEditor::removeRow()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
}


void AutomateEditor::moveRowUp()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    TreeModel *model = qobject_cast<TreeModel *>(view->model());
    if (model->moveRowUp(index.row(), index.parent()))
        updateActions();
}


void AutomateEditor::moveRowDown()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    TreeModel *model = qobject_cast<TreeModel *>(view->model());
    if (model->moveRowDown(index.row(), index.parent()))
        updateActions();
}


void AutomateEditor::updateActions()
{
    bool hasSelection = !view->selectionModel()->selection().isEmpty();
    delButton->setEnabled(hasSelection);

    bool hasCurrent = view->selectionModel()->currentIndex().isValid();

    if (hasCurrent) {
        view->closePersistentEditor(view->selectionModel()->currentIndex());
    }
}

void AutomateEditor::loadToolElements()
{

    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_ToolInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         "AddCover" << "AddCover" << tr("Add Cover to epub.") <<
         "CreateHTMLTOC" << "CreateHTMLTOC" << tr("Create HTML Table Of Contents.") <<
         "DeleteUnusedMedia" << "DeleteUnusedMedia" << tr("Delete Unused Media Resource.") <<
         "DeleteUnusedStyles" << "DeleteUnusedStyles" << tr("Delete Unused CSS Selectors.") <<
         "GenerateNCXGuideFromNav" << "GenerateNCXGuideFromNav" << tr("Generate NCX and OPF Guide from Epub3 Nav.") <<
         "GenerateTOC" << "GenerateTOC" << tr("Generate TOC from Heading Tags.") <<
         "MendPrettifyHTML" << "MendPrettifyHTML" << tr("Mend and Prettify all XHtml files.") <<
         "MendHTML" << "MendHTML"  << tr("Mend All XHtml files.") <<
         "ReformatCSSMultipleLines" << "ReformatCSSMultipleLines" << tr("Reformat All CSS to Multiple Lines format.") <<
         "ReformatCSSSingleLines" << "ReformatCSSSingleLines" << tr("Reformat All CSS to Single Lines format.") <<
         "RemoveNCXGuideFromEpub3" << "RemoveNCXGuideFromEpub3" << tr("Remove NCX and OPF Guide from Epub3.") <<
         "RepoCommit" << "RepoCommit" << tr("Save a Checkpoint of the current epub.") << 
         "RunSavedSearchReplaceAll" << "RunSavedSearchReplaceAll" << tr("Run the named Saved Search with Replace All.") <<
         "Save" << "Save" << tr("Save the current epub.") <<
         "SetBookBrowserToAllCSS" << "SetBookBrowserToAllCSS" << tr("Select all CSS Files in BookBrowser") << 
         "SetBookBrowserToAllHTML" << "SetBookBrowserToAllHTML" << tr("Select all HTML Files in BookBrowser") << 
         "SetBookBrowserToAllImages" << "SetBookBrowserToAllImages" << tr("Select all Image Files in BookBrowser") << 
         "SetBookBrowserToInitialSelection" << "SetBookBrowserToInitialSelection" << tr("Reset BookBrowser to its initial selection") <<
         "SetPluginParameter" << "SetPluginParameter" << tr("set a string parameter to be passed to the next plugin.") <<

         "SplitOnSGFSectionMarkers" << "SplitOnSGFSectionMarkers" << tr("Split XHtml files on Sigil Section Markers") <<
         "StandardizeEpub" << "StandardizeEpub" << tr("Convert Epub layout to Sigil's historic Standard form.") <<
         "UpdateManifestProperties" << "UpdateManifestProperties" << tr("Update Epub3 OPF Manifest properties.") <<
         "ValidateStylesheetsWithW3C" << "ValidateStylesheetsWithW3C" << tr("Validate All Stylesheets with W3C in external browser.") <<
         "WellFormedCheckEpub" << "WellFormedCheckEpub" << tr("Perform a basic Well-Formed Check on Epub XHtml files.");
    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_ToolInfo.insert(code, minfo);
        m_ToolCode.insert(name, code);
    }
}


// Loads the basic metadata types, names, and descriptions
void AutomateEditor::loadPluginElements()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_PluginInfo.isEmpty()) {
        return;
    }

    QStringList data;
    PluginDB *pdb = PluginDB::instance();
    QHash<QString, Plugin *> plugins = pdb->all_plugins();
    QStringList plugin_names = plugins.keys();
    foreach(QString name, plugin_names) {
        QString desc = plugins[name]->get_description();
        data << name << name << desc;
    }
    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_PluginInfo.insert(code, minfo);
        m_PluginCode.insert(name, code);
    }
}

void AutomateEditor::ReadSettings()
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


void AutomateEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}
