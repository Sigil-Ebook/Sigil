/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Dave Heiland
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

#include "Dialogs/PreferenceWidgets/GeneralSettingsWidget.h"

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

#include "Misc/Utility.h"
#include "sigil_constants.h"

GeneralSettingsWidget::GeneralSettingsWidget()
    :
    m_refreshClipboardHistoryLimit(false)
{
    ui.setupUi(this);
    readSettings();
    ExtendUI();
    connectSignalsToSlots();
}

PreferencesWidget::ResultAction GeneralSettingsWidget::saveSettings()
{

    int new_clean_on_level = 0;
    QString new_epub_version = "2.0";

    if (ui.EpubVersion3->isChecked()) {
        new_epub_version = "3.0";
    }
    if (ui.MendOnOpen->isChecked()) {
        new_clean_on_level |= CLEANON_OPEN;
    }
    if (ui.MendOnSave->isChecked()) {
        new_clean_on_level |= CLEANON_SAVE;
    }

    QString css_epub2_spec = "css21";

    if (ui.Epub2css20->isChecked()) {
        css_epub2_spec = "css2";
    } else if (ui.Epub2css30->isChecked()) {
        css_epub2_spec = "css3";
    }

    QString css_epub3_spec = "css3";

    if (ui.Epub3css20->isChecked()) {
        css_epub3_spec = "css2";
    } else if (ui.Epub3css21->isChecked()) {
        css_epub3_spec = "css21";
    }

    int new_remote_on_level = 0;

    if (ui.AllowRemote->isChecked()) {
        new_remote_on_level = 1;
    }

    int new_javascript_on_level = 0;

    if (ui.AllowJavascript->isChecked()) {
        new_javascript_on_level = 1;
    }

    QString new_temp_folder_home = "<SIGIL_DEFAULT_TEMP_HOME>";
    if (!ui.lineEdit->text().isEmpty()) {
         new_temp_folder_home = ui.lineEdit->text();
    }

    QString new_xeditor_path = "";
    if (!ui.lineEdit7->text().isEmpty()) {
         QString xeditorpath = ui.lineEdit7->text();
	 if (QFileInfo(xeditorpath).exists()) {
	     new_xeditor_path = xeditorpath;
	 }
    }

    SettingsStore settings;
    settings.setCleanOn(new_clean_on_level);
    settings.setDefaultVersion(new_epub_version);
    settings.setCssEpub2ValidationSpec(css_epub2_spec);
    settings.setCssEpub3ValidationSpec(css_epub3_spec);
    settings.setRemoteOn(new_remote_on_level);
    settings.setJavascriptOn(new_javascript_on_level);
    settings.setClipboardHistoryLimit(int(ui.clipLimitSpin->value()));
    settings.setTempFolderHome(new_temp_folder_home);
    settings.setExternalXEditorPath(new_xeditor_path);

    if (!m_refreshClipboardHistoryLimit) {
        return PreferencesWidget::ResultAction_None;
    }
    return PreferencesWidget::ResultAction_RefreshClipHistoryLimit;
}

void GeneralSettingsWidget::readSettings()
{
    SettingsStore settings;
    QString version = settings.defaultVersion();
    ui.EpubVersion2->setChecked(version == "2.0");
    ui.EpubVersion3->setChecked(version == "3.0");
    QString css_epub2_spec = settings.cssEpub2ValidationSpec();
    ui.Epub2css20->setChecked(css_epub2_spec == "css2");
    ui.Epub2css21->setChecked(css_epub2_spec == "css21");
    ui.Epub2css30->setChecked(css_epub2_spec == "css3");
    QString css_epub3_spec = settings.cssEpub3ValidationSpec();
    ui.Epub3css20->setChecked(css_epub3_spec == "css2");
    ui.Epub3css21->setChecked(css_epub3_spec == "css21");
    ui.Epub3css30->setChecked(css_epub3_spec == "css3");
    int cleanOn = settings.cleanOn();
    ui.MendOnOpen->setChecked(cleanOn & CLEANON_OPEN);
    ui.MendOnSave->setChecked(cleanOn & CLEANON_SAVE);
    int remoteOn = settings.remoteOn();
    ui.AllowRemote->setChecked(remoteOn);
    int javascriptOn = settings.javascriptOn();
    ui.AllowJavascript->setChecked(javascriptOn);
    ui.clipLimitSpin->setValue(int(settings.clipboardHistoryLimit()));
    QString temp_folder_home = settings.tempFolderHome();
    ui.lineEdit->setText(temp_folder_home);
    QString xeditor_path = settings.externalXEditorPath();
    ui.lineEdit7->setText(xeditor_path);
}

void GeneralSettingsWidget::clearXEditorPath()
{
    ui.lineEdit7->setText("");
}

void GeneralSettingsWidget::setXEditorPath()
{
#if defined(Q_OS_WIN32)
    static QString LAST_LOCATION = Utility::GetEnvironmentVar("PROGRAMFILES");
#else
    static QString LAST_LOCATION = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
#endif

    static const QString NAME_FILTER = QObject::tr("Applications")
#if defined(Q_OS_WIN32)
                                       + " (*.exe *.com *.bat *.cmd)"
#elif defined(Q_OS_MAC)
                                       + " (*.app)"
#else
                                       + " (*)"
#endif
      ;
    QFileDialog::Options options = QFileDialog::Options() | QFileDialog::ReadOnly | QFileDialog::HideNameFilterDetails;
#ifdef Q_OS_MAC
    options = options | QFileDialog::DontUseNativeDialog;
#endif

    QString xeditorPath = QFileDialog::getOpenFileName(0,
						       QObject::tr("Select External Xhtml Editor"),
						       LAST_LOCATION,
						       NAME_FILTER,
						       0,
                                                       options);
    ui.lineEdit7->setText(xeditorPath);
}

void GeneralSettingsWidget::XEditorPathChanged()
{
    // make sure typed in path actually exists
    QString xeditorpath = ui.lineEdit7->text();
    if (!xeditorpath.isEmpty()) {
        QFileInfo xeditinfo(xeditorpath);
        if (!xeditinfo.exists() || !xeditinfo.isReadable()) {
            disconnect(ui.lineEdit7, SIGNAL(editingFinished()), this, SLOT(XEditorPathChanged()));
            Utility::DisplayStdWarningDialog(tr("Incorrect Path for External Xhtml Editor selected"));
            ui.lineEdit7->setText("");
            connect(ui.lineEdit7, SIGNAL(editingFinished()), this, SLOT(XEditorPathChanged()));
        }
    }
}

void GeneralSettingsWidget::autoTempFolder()
{
    ui.lineEdit->setText("<SIGIL_DEFAULT_TEMP_HOME>");
}

void GeneralSettingsWidget::setTempFolder()
{
    QFileDialog::Options options = QFileDialog::Options() | QFileDialog::ShowDirsOnly;
#ifdef Q_OS_MAC
    options = options | QFileDialog::DontUseNativeDialog;
#endif

    QString name = QFileDialog::getExistingDirectory(this, 
						     tr("Select Folder for Temporary Files"),
						     QString(),
						     options);
    if (name.isEmpty()) {
        return;
    }
    ui.lineEdit->setText(name);
}

void GeneralSettingsWidget::tempFolderPathChanged()
{
    // make sure typed in path actually exists and is writeable
    QString tempfolderpath = ui.lineEdit->text();
    if (!tempfolderpath.isEmpty() && (tempfolderpath != "<SIGIL_DEFAULT_TEMP_HOME>")) {
        QFileInfo tempinfo(tempfolderpath);
        if (!tempinfo.exists() || !tempinfo.isDir() || !tempinfo.isReadable() || !tempinfo.isWritable() ) {
            disconnect(ui.lineEdit, SIGNAL(editingFinished()), this, SLOT(tempFolderPathChanged()));
            Utility::DisplayStdWarningDialog(tr("Incorrect Folder for Temporary Files selected"));
            ui.lineEdit->setText("<SIGIL_DEFAULT_TEMP_HOME>");
            connect(ui.lineEdit, SIGNAL(editingFinished()), this, SLOT(tempFolderPathChanged()));
        }
    }
}

void GeneralSettingsWidget::clipLimitValueChanged() {
    m_refreshClipboardHistoryLimit = true;
}

void GeneralSettingsWidget::ExtendUI() {
    // Make sure no one can enter anything other than 0 - CLIPBOARD_HISTORY_MAX
    ui.clipLimitSpin->setMinimum(0);
    ui.clipLimitSpin->setMaximum(CLIPBOARD_HISTORY_MAX);
}

void GeneralSettingsWidget::connectSignalsToSlots()
{
    connect(ui.autoButton, SIGNAL(clicked()), this, SLOT(autoTempFolder()));
    connect(ui.browseButton, SIGNAL(clicked()), this, SLOT(setTempFolder()));
    connect(ui.lineEdit, SIGNAL(editingFinished()), this, SLOT(tempFolderPathChanged()));
    connect(ui.clipLimitSpin, SIGNAL(valueChanged(int)), this, SLOT(clipLimitValueChanged()));
    connect(ui.clearButton7, SIGNAL(clicked()), this, SLOT(clearXEditorPath()));
    connect(ui.browseButton7, SIGNAL(clicked()), this, SLOT(setXEditorPath()));
    connect(ui.lineEdit7, SIGNAL(editingFinished()), this, SLOT(XEditorPathChanged()));
}
