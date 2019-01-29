/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2012  Daniel Pavel <daniel.pavel@gmail.com>
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

#include <QtCore/QProcess>
#if defined(Q_OS_WIN32)
#include <QProcessEnvironment>
#endif
#include <QtCore/QStandardPaths>
#include <QtWidgets/QFileDialog>

#include "Dialogs/OpenWithName.h"
#include "Misc/OpenExternally.h"
#include "Misc/SettingsStore.h"


static const QString SETTINGS_GROUP = "open_with";
static const QString EMPTY;
static const QString SEP = QString(QChar(31));

// resource types that are watched for modifications from outside Sigil
// only watch certain types of resources (auxiliaries)
static const int WATCHED_RESOURCE_TYPES = Resource::HTMLResourceType       |
        Resource::CSSResourceType        |
        Resource::AudioResourceType      |
        Resource::VideoResourceType      |
        Resource::ImageResourceType      |
        Resource::MiscTextResourceType   |
        Resource::SVGResourceType        |
        Resource::FontResourceType       |
        Resource::NCXResourceType        |
        Resource::OPFResourceType        |
        Resource::GenericResourceType;

// not very elegant, but much lighter than a std::map (and with less initialization trouble)
// the switch _must_ always be in sync with WATCHED_RESOURCE_TYPES
static inline const char *const RESOURCE_TYPE_NAME(const Resource::ResourceType type)
{
    switch (type) {
        case Resource::GenericResourceType:
            return "generic";

        case Resource::HTMLResourceType:
            return "html";

        case Resource::CSSResourceType:
            return "stylesheet";

        case Resource::ImageResourceType:
            return "image";

        case Resource::MiscTextResourceType:
            return "misctext";

        case Resource::SVGResourceType:
            return "svg";

        case Resource::FontResourceType:
            return "font";

        case Resource::NCXResourceType:
            return "ncx";

        case Resource::OPFResourceType:
            return "opf";

        default:
            return "";
    }
}


bool OpenExternally::mayOpen(const Resource::ResourceType type)
{
    return type & (WATCHED_RESOURCE_TYPES);
}

bool OpenExternally::openFile(const QString &filePath, const QString &application)
{
#if defined(Q_OS_MAC)

    if (QFile::exists(filePath) && QDir(application).exists()) {
        QStringList arguments = QStringList() << "-a" << application << filePath;
        return QProcess::startDetached("/usr/bin/open", arguments);
    }

#else

    if (QFile::exists(filePath) && QFile::exists(application)) {
        QStringList arguments = QStringList(QDir::toNativeSeparators(filePath));
        return QProcess::startDetached(QDir::toNativeSeparators(application), arguments, QFileInfo(filePath).absolutePath());
    }

#endif
    return false;
}

const QStringList OpenExternally::editorsForResourceType(const Resource::ResourceType type)
{
    QStringList editorPaths = QStringList();
    if (mayOpen(type)) {
        SettingsStore settings;
        settings.beginGroup(SETTINGS_GROUP);
        const QString editorsKey = QString("editors_") + RESOURCE_TYPE_NAME(type);
        if (settings.contains(editorsKey)) {
	    QStringList editors = settings.value(editorsKey).toStringList();
	    foreach(QString editor, editors) {
  	        const QStringList edata = editor.split(SEP);
		const QString editor_name = edata[nameField];
		const QString editor_path = edata[pathField];
	        if (QFile::exists(editor_path)) editorPaths.append(editor_path);
	    }
        }
    }
    return editorPaths;
}

const QStringList OpenExternally::editorDescriptionsForResourceType(const Resource::ResourceType type)
{
    QStringList editorDescriptions = QStringList();
    if (mayOpen(type)) {
        SettingsStore settings;
        settings.beginGroup(SETTINGS_GROUP);
        const QString editorsKey = QString("editors_") + RESOURCE_TYPE_NAME(type);
        if (settings.contains(editorsKey)) {
	    QStringList editors = settings.value(editorsKey).toStringList();
	    foreach(QString editor, editors) {
  	        const QStringList edata = editor.split(SEP);
		QString editor_name = edata[nameField];
		const QString editor_path = edata[pathField];
	        if (QFile::exists(editor_path)) {
  		    if (editor_name.isEmpty()) editor_name = prettyApplicationName(editor_path);
		    editorDescriptions.append(editor_name);
		}
	    }
        }
    }
    return editorDescriptions;
}

const QString OpenExternally::prettyApplicationName(const QString &applicationpath)
{
    return QFileInfo(applicationpath).completeBaseName();
}

const QString OpenExternally::selectEditorForResourceType(const Resource::ResourceType type)
{
    if (!mayOpen(type)) {
        return EMPTY;
    }

    const QString editorsKey = QString("editors_") + RESOURCE_TYPE_NAME(type);
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
#if defined(Q_OS_WIN32)
    // Windows barks about getenv or _wgetenv. This elicits no warnings and works with unicode paths
    static QString LAST_LOCATION = QProcessEnvironment::systemEnvironment().value("PROGRAMFILES", "").trimmed();
#else
    static QString LAST_LOCATION = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
#endif
    QStringList EmptyList = QStringList();
    QStringList editors = settings.value(editorsKey, EmptyList).toStringList();
    QString last_editor;
    QString last_name;
    if (!editors.isEmpty()) {
        QStringList edata = editors[0].split(SEP);
	last_name = edata[nameField];
	last_editor = edata[pathField];
    }
    if (last_editor.isEmpty() || !QFile::exists(last_editor)) {
        last_editor = LAST_LOCATION;

        if (!QFile::exists(last_editor)) {
            last_editor = LAST_LOCATION = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
    }

    static const QString NAME_FILTER = QObject::tr("Applications")
#if defined(Q_OS_WIN32)
                                       + " (*.exe *.com *.bat *.cmd)"
#elif defined(Q_OS_MAC)
                                       + " (*.app)"
#else
                                       + " (*)"
#endif
                                       ;
    const QString selectedFile = QFileDialog::getOpenFileName(0,
                                 QObject::tr("Open With"),
                                 last_editor,
                                 NAME_FILTER,
                                 0,
                                 QFileDialog::ReadOnly | QFileDialog::HideNameFilterDetails);

    if (!selectedFile.isEmpty()) {
        // Let the user choose a friendly menu name for the application
        QString editorDescription;
        QString prettyName = prettyApplicationName(selectedFile);
        OpenWithName name(prettyName, QApplication::activeWindow());
        name.exec();
        editorDescription = name.GetName();

        if (editorDescription.isEmpty()) {
            editorDescription = prettyName;
        }
        const QString editor_data = editorDescription + SEP + selectedFile;
	if (!editors.contains(editor_data)) { 
	    editors.prepend(editor_data);
	}
        settings.setValue(editorsKey, editors);
        LAST_LOCATION = selectedFile;
    }

    return selectedFile;
}
