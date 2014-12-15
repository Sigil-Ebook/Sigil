#include <Qt>
#include <QString>
#include <QList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QApplication>

#include "MainUI/MainWindow.h"
#include "PluginWidget.h"
#include "Misc/Plugin.h"
#include "Misc/PluginDB.h"
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"

PluginWidget::PluginWidget()
    :
    m_isDirty(false),
    m_LastFolderOpen(QString())
{
    ui.setupUi(this);
    readSettings();
    connectSignalsToSlots();
}


PluginWidget::ResultAction PluginWidget::saveSettings()
{
    if (!m_isDirty) {
        return PreferencesWidget::ResultAction_None;
    }

    PluginDB *pdb = PluginDB::instance();

    pdb->set_engine_path("python2.7", ui.editPathPy2->text());
    pdb->set_engine_path("python3.4", ui.editPathPy3->text());

    m_isDirty = false;
    return PreferencesWidget::ResultAction_None;
}

void PluginWidget::setPluginTableRow(Plugin *p, int row)
{
    QString pname(p->get_name());
    ui.pluginTable->setItem(row, PluginWidget::NameField,        new QTableWidgetItem(pname));
    ui.pluginTable->item(row,0)->setToolTip(p->get_description());
    ui.pluginTable->setItem(row, PluginWidget::VersionField,     new QTableWidgetItem(p->get_version()));
    ui.pluginTable->setItem(row, PluginWidget::AuthorField,      new QTableWidgetItem(p->get_author()));
    ui.pluginTable->setItem(row, PluginWidget::TypeField,        new QTableWidgetItem(p->get_type()));
    ui.pluginTable->setItem(row, PluginWidget::EngineField,      new QTableWidgetItem(p->get_engine()));
}


void PluginWidget::readSettings()
{
    SettingsStore settings;
    // The last folder used for saving and opening files
    m_LastFolderOpen = settings.pluginLastFolder();
    
    // Load the available plugin information
    PluginDB *pdb = PluginDB::instance();
    QHash<QString, Plugin *> plugins;
    int nrows = 0;

    ui.editPathPy2->setText(pdb->get_engine_path("python2.7"));
    ui.editPathPy3->setText(pdb->get_engine_path("python3.4"));

    // clear out the table but do NOT clear out column headings
    while (ui.pluginTable->rowCount() > 0) {
        ui.pluginTable->removeRow(0);
    }

    plugins = pdb->all_plugins();
    foreach(Plugin *p, plugins) {
        ui.pluginTable->insertRow(nrows);
        setPluginTableRow(p,nrows);
        nrows++;
    }

    ui.pluginTable->resizeColumnsToContents();
    m_isDirty = false;
}

void PluginWidget::pluginSelected(int row, int col)
{
    ui.pluginTable->setCurrentCell(row, col);
}

void PluginWidget::addPlugin()
{
    QString zippath = QFileDialog::getOpenFileName(this, tr("Select Plugin Zip Archive"), m_LastFolderOpen, tr("Plugin Files (*.zip)"));
    if (zippath.isEmpty()) {
        return;
    }

    PluginDB *pdb = PluginDB::instance();

    PluginDB::AddResult ar = pdb->add_plugin(zippath);
    
    // Save the last folder used for adding plugin zips
    m_LastFolderOpen = QFileInfo(zippath).absolutePath();
    SettingsStore settings;
    settings.setPluginLastFolder(m_LastFolderOpen);

    switch (ar) {
        case PluginDB::AR_XML:
            Utility::DisplayStdWarningDialog(tr("Error: Plugin plugin.xml file can not be read."));
            return;
        case PluginDB::AR_EXISTS:
            Utility::DisplayStdWarningDialog(tr("Warning: A plugin by that name already exists"));
            return;
        case PluginDB::AR_UNZIP:
            Utility::DisplayStdWarningDialog(tr("Error: Plugin Could Not be Unzipped."));
            return;
        case PluginDB::AR_INVALID:
            Utility::DisplayStdWarningDialog(tr("Error: Plugin not a valid Sigil plugin."));
            return;
        case PluginDB::AR_SUCCESS:
            break;
    }

    QFileInfo zipinfo(zippath);
    QString pluginname = zipinfo.baseName();
    // strip off any versioning present in zip name after first "_" to get internal folder name
    int version_index = pluginname.indexOf("_");
    if (version_index > -1) {
        pluginname.truncate(version_index);
    }

    Plugin *p = pdb->get_plugin(pluginname);

    if (p == NULL) {
        return;
    }

    int rows = ui.pluginTable->rowCount();
    ui.pluginTable->insertRow(rows);
    setPluginTableRow(p,rows);
    ui.pluginTable->resizeColumnsToContents();
}



void PluginWidget::removePlugin()
{
    // limited to work with one selection at a time to prevent row mixup upon removal
    QList<QTableWidgetItem *> itemlist = ui.pluginTable->selectedItems();
    if (itemlist.isEmpty()) {
        Utility::DisplayStdWarningDialog(tr("Nothing is Selected."));
        return;
    }

    PluginDB *pdb = PluginDB::instance();
    int row = ui.pluginTable->row(itemlist.at(0));
    QString pluginname = ui.pluginTable->item(row, PluginWidget::NameField)->text();
    ui.pluginTable->removeRow(row);
    pdb->remove_plugin(pluginname);
    ui.pluginTable->resizeColumnsToContents();
}

void PluginWidget::removeAllPlugins()
{
    PluginDB *pdb = PluginDB::instance();
    QMessageBox msgBox;

    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    msgBox.setWindowTitle(tr("Remove All Plugins"));
    msgBox.setText(tr("Are you sure sure you want to remove all of your plugins?"));
    QPushButton *yesButton = msgBox.addButton(QMessageBox::Yes);
    QPushButton *noButton  = msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(noButton);
    msgBox.exec();
    if (msgBox.clickedButton() == yesButton) {
        while (ui.pluginTable->rowCount() > 0) {
            ui.pluginTable->removeRow(0);
        }
    }

    pdb->remove_all_plugins();
    ui.pluginTable->resizeColumnsToContents();
}

void PluginWidget::AutoFindPy2()
{
    QString p2path = QStandardPaths::findExecutable("python2");
    if (p2path.isEmpty()) {
        p2path = QStandardPaths::findExecutable("python");
    }
    ui.editPathPy2->setText(p2path);
    m_isDirty = true;
}

void PluginWidget::AutoFindPy3()
{
    QString p3path = QStandardPaths::findExecutable("python3");
    if (p3path.isEmpty()) {
        p3path = QStandardPaths::findExecutable("python");
    }
    ui.editPathPy3->setText(p3path);
    m_isDirty = true;
}

void PluginWidget::SetPy2()
{
    QString name = QFileDialog::getOpenFileName(this, tr("Select Interpreter"));
    if (name.isEmpty()) {
        return;
    }
    ui.editPathPy2->setText(name);
    m_isDirty = true;
}

void PluginWidget::SetPy3()
{
    QString name = QFileDialog::getOpenFileName(this, tr("Select Interpreter"));
    if (name.isEmpty()) {
        return;
    }
    ui.editPathPy3->setText(name);
    m_isDirty = true;
}

void PluginWidget::enginePy2PathChanged()
{
    // make sure typed in path actually exists
    QString enginepath = ui.editPathPy2->text();
    if (!enginepath.isEmpty()) {
        QFileInfo enginfo(enginepath);
        if (!enginfo.exists() || !enginfo.isFile() || !enginfo.isReadable() || !enginfo.isExecutable() ) {
            disconnect(ui.editPathPy2, SIGNAL(editingFinished()), this, SLOT(enginePy2PathChanged()));
            Utility::DisplayStdWarningDialog(tr("Incorrect Interpreter Path selected"));
            ui.editPathPy2->setText("");
            connect(ui.editPathPy2, SIGNAL(editingFinished()), this, SLOT(enginePy2PathChanged()));
        }
    }
    m_isDirty = true;
}

void PluginWidget::enginePy3PathChanged()
{
    // make sure typed in path actually exists
    QString enginepath = ui.editPathPy3->text();
    if (!enginepath.isEmpty()) {
        QFileInfo enginfo(enginepath);
        if (!enginfo.exists() || !enginfo.isFile() || !enginfo.isReadable() || !enginfo.isExecutable() ) {
            disconnect(ui.editPathPy3, SIGNAL(editingFinished()), this, SLOT(enginePy3PathChanged()));
            Utility::DisplayStdWarningDialog(tr("Incorrect Interpreter Path selected"));
            ui.editPathPy3->setText("");
            connect(ui.editPathPy3, SIGNAL(editingFinished()), this, SLOT(enginePy3PathChanged()));
        }
    }
    m_isDirty = true;
}

void PluginWidget::connectSignalsToSlots()
{
    connect(ui.Py2Auto, SIGNAL(clicked()), this, SLOT(AutoFindPy2()));
    connect(ui.Py3Auto, SIGNAL(clicked()), this, SLOT(AutoFindPy3()));
    connect(ui.Py2Set, SIGNAL(clicked()), this, SLOT(SetPy2()));
    connect(ui.Py3Set, SIGNAL(clicked()), this, SLOT(SetPy3()));
    connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addPlugin()));
    connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removePlugin()));
    connect(ui.removeAllButton, SIGNAL(clicked()), this, SLOT(removeAllPlugins()));
    connect(ui.pluginTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(pluginSelected(int,int)));
    connect(ui.editPathPy2, SIGNAL(editingFinished()), this, SLOT(enginePy2PathChanged()));
    connect(ui.editPathPy3, SIGNAL(editingFinished()), this, SLOT(enginePy3PathChanged()));
}
