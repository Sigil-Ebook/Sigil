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
    m_LastFolderOpen(QString()),
    m_useBundledInterp(false)
{
    ui.setupUi(this);

    // initialize ui combobox pointer list
    m_qlcbxs.append(ui.comboBox);
    m_qlcbxs.append(ui.comboBox_2);
    m_qlcbxs.append(ui.comboBox_3);
    m_qlcbxs.append(ui.comboBox_4);
    m_qlcbxs.append(ui.comboBox_5);
    m_qlcbxs.append(ui.comboBox_6);
    m_qlcbxs.append(ui.comboBox_7);
    m_qlcbxs.append(ui.comboBox_8);
    m_qlcbxs.append(ui.comboBox_9);
    m_qlcbxs.append(ui.comboBox_10);

    readSettings();
    connectSignalsToSlots();
}


PluginWidget::ResultAction PluginWidget::saveSettings()
{
    if (!m_isDirty) {
        return PreferencesWidget::ResultAction_None;
    }

    SettingsStore settings;

    PluginDB *pdb = PluginDB::instance();

    pdb->set_engine_path("python3.4", ui.editPathPy3->text());


    // handle the 10 assignable plugin buttons
    QHash<QString, Plugin*> plugins = pdb->all_plugins();
    QStringList keys = plugins.keys();
    QStringList pluginmap;
    QStringList pnames;
    foreach(QComboBox* cb, m_qlcbxs) {
      pnames.append(cb->currentText());
    }
    foreach (QString pname, pnames) {
        if (keys.contains(pname)) {
            pluginmap.append(pname);
        } else {
            pluginmap.append("");
        }
    }
    settings.setPluginMap(pluginmap);

    if (!bundledInterpReady()) {
        settings.setUseBundledInterp(false);
    } else {
        settings.setUseBundledInterp(m_useBundledInterp);
    }

    m_isDirty = false;
    return PreferencesWidget::ResultAction_None;
}

void PluginWidget::setPluginTableRow(Plugin *p, int row)
{
    QString pname(p->get_name());
    bool sortingOn = ui.pluginTable->isSortingEnabled();
    ui.pluginTable->setSortingEnabled(false);
    ui.pluginTable->setItem(row, PluginWidget::NameField,        new QTableWidgetItem(pname));
    ui.pluginTable->item(row,0)->setToolTip(p->get_description());
    ui.pluginTable->setItem(row, PluginWidget::VersionField,     new QTableWidgetItem(p->get_version()));
    ui.pluginTable->setItem(row, PluginWidget::AuthorField,      new QTableWidgetItem(p->get_author()));
    ui.pluginTable->setItem(row, PluginWidget::TypeField,        new QTableWidgetItem(p->get_type()));
    ui.pluginTable->setItem(row, PluginWidget::EngineField,      new QTableWidgetItem(p->get_engine()));
    ui.pluginTable->setSortingEnabled(sortingOn);
}


void PluginWidget::readSettings()
{
    SettingsStore settings;

    // The last folder used for saving and opening files
    m_LastFolderOpen = settings.pluginLastFolder();

    // Should the bundled Python interpreter be used?
    m_useBundledInterp = settings.useBundledInterp();

    // Load the available plugin information
    PluginDB *pdb = PluginDB::instance();
    QHash<QString, Plugin *> plugins;

    ui.editPathPy3->setText(pdb->get_engine_path("python3.4"));

    ui.pluginTable->setSortingEnabled(false);

    // clear out the table but do NOT clear out column headings
    while (ui.pluginTable->rowCount() > 0) {
        ui.pluginTable->removeRow(0);
    }

    int nrows = 0;
    plugins = pdb->all_plugins();
    foreach(Plugin *p, plugins) {
        ui.pluginTable->insertRow(nrows);
        setPluginTableRow(p,nrows);
        nrows++;
    }

    ui.pluginTable->resizeColumnsToContents();

    // handle the assignable plugin buttons
    foreach(QComboBox* cb, m_qlcbxs) {
        cb->clear();
    }

    QStringList keys = plugins.keys();
    keys.sort(Qt::CaseInsensitive);
    QStringList items = QStringList() << "";
    items.append(keys);
    foreach(QComboBox* cb, m_qlcbxs) {
        cb->addItems(items);
    }
    foreach(QComboBox* cb, m_qlcbxs) {
        cb->setCurrentIndex(0);
    }

    QStringList pluginmap = settings.pluginMap();
    // prevent segfaults from corrupted settings files
    while (pluginmap.count() < 10) pluginmap.append(QString(""));

    // initialize each combo box current index from settings
    for (int i=0; i < 10; i++) {
        int t = m_qlcbxs.at(i)->findText(pluginmap.at(i));
	m_qlcbxs.at(i)->setCurrentIndex(t);
    }

    // If the python bundled interpreter is present/ready, enable the checkbox and set it
    // based on the value of the SettingStore Value. Otherwise keep it disabled.
    if (bundledInterpReady()) {
        ui.chkUseBundled->setEnabled(true);
        if (m_useBundledInterp) {
            ui.chkUseBundled->setCheckState(Qt::Checked);
        } else {
            ui.chkUseBundled->setCheckState(Qt::Unchecked);
        }
        enable_disable_controls();
    }

    ui.pluginTable->setSortingEnabled(true);
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
            Utility::DisplayStdWarningDialog(tr("Error: Plugin plugin.xml is invalid or not supported on your operating system."));
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

    ui.pluginTable->setSortingEnabled(false);
    int rows = ui.pluginTable->rowCount();
    ui.pluginTable->insertRow(rows);
    setPluginTableRow(p,rows);
    ui.pluginTable->resizeColumnsToContents();
    foreach(QComboBox* cb, m_qlcbxs) {
        cb->addItem(pluginname);
    }
    ui.pluginTable->setSortingEnabled(true);
}



void PluginWidget::removePlugin()
{
    // limited to work with one selection at a time to prevent row mixup upon removal
    QList<QTableWidgetItem *> itemlist = ui.pluginTable->selectedItems();
    if (itemlist.isEmpty()) {
        Utility::DisplayStdWarningDialog(tr("Nothing is Selected."));
        return;
    }

    ui.pluginTable->setSortingEnabled(false);

    PluginDB *pdb = PluginDB::instance();
    int row = ui.pluginTable->row(itemlist.at(0));
    QString pluginname = ui.pluginTable->item(row, PluginWidget::NameField)->text();
    ui.pluginTable->removeRow(row);
    pdb->remove_plugin(pluginname);
    ui.pluginTable->resizeColumnsToContents();


    // now update the toolbar plugin assignments
    // get their current settings
    QStringList vals;
    foreach(QComboBox* cb, m_qlcbxs) {
        vals.append(cb->currentText());
    }

    // all 10 have the identical lists
    // so remove the same item from each list
    int item_to_remove = ui.comboBox->findText(pluginname);
    if (item_to_remove > -1) {
        foreach(QComboBox* cb, m_qlcbxs) {
	    cb->removeItem(item_to_remove);
        }
    }
    // now put back their current assigned plugins
    int i = 0;
    foreach(QComboBox* cb, m_qlcbxs) {
        int target = cb->findText(vals.at(i));
        cb->setCurrentIndex(target);
        i++;
    }

    ui.pluginTable->setSortingEnabled(true);
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
        ui.pluginTable->setSortingEnabled(false);
        while (ui.pluginTable->rowCount() > 0) {
            ui.pluginTable->removeRow(0);
        }
        pdb->remove_all_plugins();
        ui.pluginTable->resizeColumnsToContents();
        ui.pluginTable->setSortingEnabled(true);
    }
    foreach(QComboBox* cb, m_qlcbxs) {
        cb->clear();
	cb->setCurrentIndex(-1);
    }
}


void PluginWidget::pluginMapChanged(int i) {
    m_isDirty = true;
}


bool PluginWidget::bundledInterpReady()
{
    QString bpath;
    bpath = PluginDB::buildBundledInterpPath();
    if (bpath != "") {
        QFileInfo checkPython3(bpath);
        if (checkPython3.exists() && checkPython3.isFile() && checkPython3.isReadable() && checkPython3.isExecutable() ) {
            return true;
        }
    }
    return false;
}

void PluginWidget::AutoFindPy3()
{
    // Search for a system python 3
    QString p3path = QStandardPaths::findExecutable("python3");
    if (p3path.isEmpty()) {
        p3path = QStandardPaths::findExecutable("python");
    }
    ui.editPathPy3->setText(p3path);
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

void PluginWidget::enable_disable_controls()
{
    // Disable/enable the Python3 interpreter path widgets based
    // on the value of the useBundledInterpreter checkbox.
    ui.editPathPy3->setEnabled(!m_useBundledInterp);
    ui.Py3Auto->setEnabled(!m_useBundledInterp);
    ui.Py3Set->setEnabled(!m_useBundledInterp);
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

void PluginWidget::useBundledPy3Changed(int)
{
    if (bundledInterpReady()) {
        m_useBundledInterp = ui.chkUseBundled->isChecked();
    }
    enable_disable_controls();
    m_isDirty = true;
}

void PluginWidget::connectSignalsToSlots()
{
    connect(ui.Py3Auto, SIGNAL(clicked()), this, SLOT(AutoFindPy3()));
    connect(ui.Py3Set, SIGNAL(clicked()), this, SLOT(SetPy3()));
    connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addPlugin()));
    connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removePlugin()));
    connect(ui.removeAllButton, SIGNAL(clicked()), this, SLOT(removeAllPlugins()));
    connect(ui.pluginTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(pluginSelected(int,int)));
    connect(ui.editPathPy3, SIGNAL(editingFinished()), this, SLOT(enginePy3PathChanged()));
    connect(ui.chkUseBundled, SIGNAL(stateChanged(int)), this, SLOT(useBundledPy3Changed(int)));
    foreach(QComboBox* cb, m_qlcbxs) {
        connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(pluginMapChanged(int)));
    }
}
