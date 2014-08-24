#include <Qt>
#include <QString>
#include <QList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QApplication>

#include "Dialogs/PluginRunner.h"
#include "MainUI/MainWindow.h"
#include "PluginWidget.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

#include "Dialogs/PluginRunner.h"


PluginWidget::PluginWidget() 
    :
  m_isDirty(false)
{
    ui.setupUi(this);
    readSettings();
    connectSignalsToSlots();
    m_PluginsPath = PluginRunner::pluginsPath();
    QDir pluginDir(m_PluginsPath);
    if (!pluginDir.exists() ) {
      pluginDir.mkpath(m_PluginsPath);
    }
}


PluginWidget::ResultAction PluginWidget::saveSettings()
{
    if (!m_isDirty) {
      return PreferencesWidget::ResultAction_None;
    }
    // Save plugin information
    SettingsStore settings;
    QHash < QString, QStringList > plugininfo;
    QHash < QString, QString> enginepath;
    int nrows = ui.pluginTable->rowCount();
    for (int i = 0; i < nrows; i++) {
        QString name        = ui.pluginTable->item(i, PluginRunner::NameField       )->text();
        QString author      = ui.pluginTable->item(i, PluginRunner::AuthorField     )->text();
        QString description = ui.pluginTable->item(i, PluginRunner::DescriptionField)->text();
        QString plugintype  = ui.pluginTable->item(i, PluginRunner::TypeField       )->text();
        QString engine      = ui.pluginTable->item(i, PluginRunner::EngineField     )->text();
        QStringList pdata;
        pdata << name << author << description << plugintype << engine;
        plugininfo[name] = pdata;
    }
    // add support for additional interpreter as above
    enginepath["python2.7"] = ui.editPathPy27->text();
    enginepath["lua5.2"] = ui.editPathLua52->text();
    settings.setPluginInfo(plugininfo);
    settings.setPluginEnginePaths(enginepath);

    // we need to alert all of the MainWindows out there as to the change in Plugins
    // So that they can update their Plugin menu appropriately
    // This should work for all platforms even ones with only one main window
    // Argh!  This causes crashes on Mac OS X!
    foreach (QWidget *mw, QApplication::topLevelWidgets()) {
       MainWindow * mwptr = qobject_cast<MainWindow *>(mw);
       if (mwptr && !mwptr->isHidden()) {
           mwptr->loadPluginsMenu();
       }
    }
    m_isDirty = false;
    return PreferencesWidget::ResultAction_None;
}


void PluginWidget::readSettings()
{
    // Load the available plugin information
    SettingsStore settings;
    QHash < QString, QString > enginepath = settings.pluginEnginePaths();
    QHash < QString, QStringList > plugininfo = settings.pluginInfo();
    QStringList names = plugininfo.keys();
    int nrows = 0;
    // add support for additional interpreters below
    ui.editPathPy27->setText(enginepath.value("python2.7",""));
    ui.editPathLua52->setText(enginepath.value("lua5.2",""));
    // clear out the table but do NOT clear out column headings
    while (ui.pluginTable->rowCount() > 0) {
        ui.pluginTable->removeRow(0);
    }
    foreach (QString name, names) {
        QStringList fields = plugininfo[name];
        if (fields.size() == 5) {
            ui.pluginTable->insertRow(nrows);

            ui.pluginTable->setItem(nrows, PluginRunner::NameField, 
                                    new QTableWidgetItem(fields.at(PluginRunner::NameField)));

            ui.pluginTable->setItem(nrows, PluginRunner::AuthorField, 
                                    new QTableWidgetItem(fields.at(PluginRunner::AuthorField)));

            ui.pluginTable->setItem(nrows, PluginRunner::DescriptionField, 
                                    new QTableWidgetItem(fields.at(PluginRunner::DescriptionField)));

            ui.pluginTable->setItem(nrows, PluginRunner::TypeField, 
                                    new QTableWidgetItem(fields.at(PluginRunner::TypeField)));

            ui.pluginTable->setItem(nrows, PluginRunner::EngineField, 
                                    new QTableWidgetItem(fields.at(PluginRunner::EngineField)));
            nrows++;
        }
    }
    m_isDirty = false;
}



void PluginWidget::pluginSelected(int row, int col)
{
    ui.pluginTable->setCurrentCell(row, col);
}


void PluginWidget::addPlugin()
{
    QString zippath = QFileDialog::getOpenFileName(this, "Select Plugin Zip Archive", "./", "Plugin Files (*.zip)");
    if (zippath.isEmpty()) {
        return;
    }
    QFileInfo zipinfo(zippath);
    QStringList names;
    QString pluginname = zipinfo.baseName();
    int nrows = ui.pluginTable->rowCount();
    for (int i = 0; i < nrows; i++) {
        names.append(ui.pluginTable->item(i, PluginRunner::NameField)->text());
    }
    if (names.contains(pluginname, Qt::CaseInsensitive)) {
        Utility::DisplayStdWarningDialog("Warning: A plugin by that name already exists");
        return;
    }

    if (!Utility::UnZip(zippath,m_PluginsPath)) {
        Utility::DisplayStdWarningDialog("Error: Plugin Could Not be Unzipped.");
        return;
    }
    QString xmlpath = m_PluginsPath + "/" + pluginname + "/plugin.xml";
    QFile file(xmlpath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         Utility::DisplayStdWarningDialog("Error: Plugin plugin.xml file can not be read.");
         return;
    }
    QXmlStreamReader reader(&file);
    QString name;
    QString author;
    QString description;
    QString plugintype;
    QString engine;
    while (! reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            if (reader.name() == "name") {
                    name = reader.readElementText();
            } else if (reader.name() == "author") {
                    author = reader.readElementText();
            } else if (reader.name() == "description") {
                    description = reader.readElementText();
            } else if (reader.name() == "type") {
                    plugintype = reader.readElementText();
            } else if (reader.name() == "engine") {
                    engine = reader.readElementText();
            }
        }
    }
    int rows = ui.pluginTable->rowCount();
    ui.pluginTable->insertRow(rows);
    ui.pluginTable->setItem(rows, PluginRunner::NameField,        new QTableWidgetItem(name));
    ui.pluginTable->setItem(rows, PluginRunner::AuthorField,      new QTableWidgetItem(author));
    ui.pluginTable->setItem(rows, PluginRunner::DescriptionField, new QTableWidgetItem(description));
    ui.pluginTable->setItem(rows, PluginRunner::TypeField,        new QTableWidgetItem(plugintype));
    ui.pluginTable->setItem(rows, PluginRunner::EngineField,      new QTableWidgetItem(engine));
    m_isDirty = true;
}

void PluginWidget::removePlugin()
{
  // limited to work with one selection at a time to prevent row mixup upon removal
    QList<QTableWidgetItem*> itemlist = ui.pluginTable->selectedItems();
    if (itemlist.isEmpty()) {
         Utility::DisplayStdWarningDialog("Nothing is Selected.");
         return;
    }
    int row = ui.pluginTable->row(itemlist.at(0));
    QString pluginname = ui.pluginTable->item(row, PluginRunner::NameField)->text();
    QString plugin = m_PluginsPath + "/" + pluginname;
    if (QDir(plugin).exists()) {
        Utility::removeDir(plugin);
    } 
    ui.pluginTable->removeRow(row);
    m_isDirty = true;
}


void PluginWidget::removeAllPlugins()
{

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    msgBox.setWindowTitle("Remove All Plugins");
    msgBox.setText("Are you sure sure you want to remove all of your plugins?");
    QPushButton * yesButton = msgBox.addButton(QMessageBox::Yes);
    QPushButton * noButton =  msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(noButton);
    msgBox.exec();
    if (msgBox.clickedButton() == yesButton) {
        while (ui.pluginTable->rowCount() > 0) {
            int row = 0;
            QString pluginname = ui.pluginTable->item(row, PluginRunner::NameField)->text();
            QString plugin = m_PluginsPath + "/" + pluginname;
            if (QDir(plugin).exists()) {
                Utility::removeDir(plugin);
            } 
            ui.pluginTable->removeRow(0);
        }
    }

}


void PluginWidget::AutoFindPy27()
{
    ui.editPathPy27->setText(QStandardPaths::findExecutable("python"));
    m_isDirty = true;
}

void PluginWidget::AutoFindLua52()
{
    ui.editPathLua52->setText(QStandardPaths::findExecutable("lua"));
    m_isDirty = true;
}

void PluginWidget::SetPy27()
{
    QString name = QFileDialog::getOpenFileName(this, "Select Interpreter");
    if (name.isEmpty()) {
        return;
    }
    ui.editPathPy27->setText(name);
    m_isDirty = true;
}

void PluginWidget::SetLua52()
{
    QString name = QFileDialog::getOpenFileName(this, "Select Interpreter");
    if (name.isEmpty()) {
        return;
    }
    ui.editPathLua52->setText(name);
    m_isDirty = true;
}

void PluginWidget::enginePy27PathChanged()
{
    // add additional interpreter support by adding their own find buttons and line edit paths
    // make sure typed in path actually exists
    QString enginepath = ui.editPathPy27->text();
    if (!enginepath.isEmpty()) {
        QFileInfo enginfo(enginepath);
        if (!enginfo.exists() || !enginfo.isFile() || !enginfo.isReadable() || !enginfo.isExecutable() ){
            disconnect(ui.editPathPy27, SIGNAL(editingFinished()), this, SLOT(enginePy27PathChanged()));
            Utility::DisplayStdWarningDialog("Incorrect Interpreter Path selected");
            ui.editPathPy27->setText("");
            connect(ui.editPathPy27, SIGNAL(editingFinished()), this, SLOT(enginePy27PathChanged()));
        }
    }
    m_isDirty = true;
}

void PluginWidget::engineLua52PathChanged()
{
    // add additional interpreter support by adding their own find buttons and line edit paths
    // make sure typed in path actually exists
    QString enginepath = ui.editPathLua52->text();
    if (!enginepath.isEmpty()) {
        QFileInfo enginfo(enginepath);
        if (!enginfo.exists() || !enginfo.isFile() || !enginfo.isReadable() || !enginfo.isExecutable() ){
            disconnect(ui.editPathLua52, SIGNAL(editingFinished()), this, SLOT(engineLua52PathChanged()));
            Utility::DisplayStdWarningDialog("Incorrect Interpreter Path selected");
            ui.editPathLua52->setText("");
            connect(ui.editPathLua52, SIGNAL(editingFinished()), this, SLOT(engineLua52PathChanged()));
        }
    }
    m_isDirty = true;
}


void PluginWidget::connectSignalsToSlots()
{
    connect(ui.Py27Auto, SIGNAL(clicked()), this, SLOT(AutoFindPy27()));
    connect(ui.Lua52Auto, SIGNAL(clicked()), this, SLOT(AutoFindLua52()));
    connect(ui.Py27Set, SIGNAL(clicked()), this, SLOT(SetPy27()));
    connect(ui.Lua52Set, SIGNAL(clicked()), this, SLOT(SetLua52()));
    connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addPlugin()));
    connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removePlugin()));
    connect(ui.removeAllButton, SIGNAL(clicked()), this, SLOT(removeAllPlugins()));
    connect(ui.pluginTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(pluginSelected(int,int)));
    connect(ui.editPathPy27, SIGNAL(editingFinished()), this, SLOT(enginePy27PathChanged()));
    connect(ui.editPathLua52, SIGNAL(editingFinished()), this, SLOT(engineLua52PathChanged()));
}
