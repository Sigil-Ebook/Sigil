/************************************************************************
**
**  Copyright (C) 2018  Kevin Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2014  John Schember <john@nachtimwald.com>
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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QXmlStreamReader>

#include "Misc/Plugin.h"
#include "Misc/PluginDB.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"

PluginDB *PluginDB::m_instance = 0;

PluginDB *PluginDB::instance()
{
    if (m_instance == 0) {
        m_instance = new PluginDB();
    }

    return m_instance;
}

PluginDB::PluginDB()
{
    SettingsStore ss;

    m_engine_paths = ss.pluginEnginePaths();

    QDir pluginDir(pluginsPath());
    if (!pluginDir.exists()) {
        pluginDir.mkpath(pluginsPath());
    }
}

PluginDB::~PluginDB()
{
    foreach(Plugin *p, m_plugins) {
        delete p;
    }
    m_plugins.clear();

    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

QString PluginDB::pluginsPath()
{
    return Utility::DefinePrefsDir() + "/plugins";
}


QString PluginDB::buildBundledInterpPath()
{
  QString bundled_python3_path; 

#ifdef Q_OS_MAC
  // On Mac OS X QCoreApplication::applicationDirPath() points to Sigil.app/Contents/MacOS/ 
  // is located, but the Python.framework dir is in Contents/Frameworks
  QDir execdir(QCoreApplication::applicationDirPath());
  execdir.cdUp();
  bundled_python3_path = execdir.absolutePath() + PYTHON_MAIN_BIN_PATH;
#elif defined(Q_OS_WIN32)
  bundled_python3_path = QCoreApplication::applicationDirPath() + "/python3.exe";
#else
  bundled_python3_path = QCoreApplication::applicationDirPath() + "/python3/bin/python3";
#endif
  
  QFileInfo checkPython3(bundled_python3_path);
  if (checkPython3.exists() && checkPython3.isFile() && checkPython3.isReadable() && checkPython3.isExecutable() ) {
    return bundled_python3_path;
  }
  return "";
}


QString PluginDB::launcherRoot()
{
    QString     launcher_root;
    QStringList launcher_roots;
    QDir        d;

#ifdef Q_OS_MAC
    launcher_roots.append(QCoreApplication::applicationDirPath() + "/../plugin_launchers/");
#elif defined(Q_OS_WIN32)
    launcher_roots.append(QCoreApplication::applicationDirPath() + "/plugin_launchers/");
#elif !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // user supplied environment variable to 'share/sigil' directory will overrides everything
    if (!sigil_extra_root.isEmpty()) {
        launcher_root = sigil_extra_root + "/plugin_launchers/";
    } else {
        launcher_roots.append(sigil_share_root + "/plugin_launchers/");
    }
#endif

    Q_FOREACH (QString s, launcher_roots) {
        if (d.exists(s)) {
            launcher_root = s;
            break;
        }
    }

    QDir base(launcher_root);
    return base.absolutePath();
}

void PluginDB::load_plugins_from_disk(bool force)
{
    QDir        d(pluginsPath());
    QStringList dplugins;

    if (!d.exists()) {
        return;
    }

    dplugins = d.entryList(QStringList("*"), QDir::Dirs|QDir::NoDotAndDotDot);

    Q_FOREACH(QString p, dplugins) {
        add_plugin_int(p, force);
    }

    emit plugins_changed();
}

PluginDB::AddResult PluginDB::add_plugin(const QString &path, bool force)
{
    PluginDB::AddResult ret;
    QFileInfo zipinfo(path);
    QString name = zipinfo.baseName();

    // strip off any versioning present in zip name after first "_" to get internal folder name
    int version_index = name.indexOf("_");
    if (version_index > -1) {
        name.truncate(version_index);
    }

    if (!verify_plugin_zip(path, name)) {
        return PluginDB::AR_INVALID;
    }

    if (!Utility::UnZip(path, pluginsPath())) {
        return PluginDB::AR_UNZIP;
    }

    ret = add_plugin_int(name, force);
    if (ret != PluginDB::AR_SUCCESS) {
        // Couldn't load the plugin so remove it.
        Utility::removeDir(pluginsPath() + "/" + name);
    } else {
        emit plugins_changed();
    }

    return ret;
}

PluginDB::AddResult PluginDB::add_plugin_int(const QString &name, bool force)
{
    Plugin *plugin;

    if (!force && m_plugins.contains(name)) {
        return PluginDB::AR_EXISTS;
    }

    plugin = load_plugin(name);
    if (plugin == NULL) {
        return PluginDB::AR_XML;
    }

    if (force && m_plugins.contains(name)) {
        delete m_plugins.take(plugin->get_name());
    }

    m_plugins.insert(plugin->get_name(), plugin);
    return PluginDB::AR_SUCCESS;
}

bool PluginDB::verify_plugin_zip(const QString &path, const QString &name)
{
    QStringList filelist = Utility::ZipInspect(path);
    if (filelist.isEmpty()) {
        return false;
    }
    foreach (QString filepath, filelist) {
        if (name != filepath.split("/").at(0)) {
            return false;
        }
    }
    if (!filelist.contains(name + "/" + "plugin.xml")) {
        return false;
    }
    return true;
}


void PluginDB::remove_plugin(const QString &name)
{
    if (!m_plugins.contains(name)) {
        return;
    }

    Plugin *p = m_plugins.take(name);
    if (p != NULL) {
        delete p;
    }
    Utility::removeDir(pluginsPath() + "/" + name);
    emit plugins_changed();
}

void PluginDB::remove_all_plugins()
{
    Plugin *p;
    foreach (QString k, m_plugins.keys()) {
        p = m_plugins.take(k);
        delete p;
        Utility::removeDir(pluginsPath() + "/" + k);
    }
    m_plugins.clear();
    emit plugins_changed();
}

Plugin *PluginDB::get_plugin(const QString &name)
{
    return m_plugins.value(name);
}

QHash<QString, Plugin *> PluginDB::all_plugins()
{
    return m_plugins;
}

QStringList PluginDB::engines()
{
    return m_engine_paths.keys();
}

QString PluginDB::get_engine_path(const QString &engine)
{
    return m_engine_paths.value(engine);
}

void PluginDB::set_engine_path(const QString &engine, const QString &path)
{
    SettingsStore ss;

    if (path.isEmpty()) {
        m_engine_paths.remove(engine);
    } else {
        m_engine_paths.insert(engine, path);
    }

    ss.setPluginEnginePaths(m_engine_paths);
}

Plugin *PluginDB::load_plugin(const QString &name)
{
    QString xmlpath = pluginsPath() + "/" + name + "/plugin.xml";
    QFile file(xmlpath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return NULL;
    }

    Plugin           *plugin = new Plugin();
    QXmlStreamReader  reader(&file);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            if (reader.name() == "name") {
                plugin->set_name(reader.readElementText());
            } else if (reader.name() == "author") {
                plugin->set_author(reader.readElementText());
            } else if (reader.name() == "description") {
                plugin->set_description(reader.readElementText());
            } else if (reader.name() == "type") {
                plugin->set_type(reader.readElementText());
            } else if (reader.name() == "engine") {
                plugin->set_engine(reader.readElementText());
            } else if (reader.name() == "version") {
                plugin->set_version(reader.readElementText());
            } else if (reader.name() == "oslist") {
                plugin->set_oslist(reader.readElementText());
            } else if (reader.name() == "autostart") {
                plugin->set_autostart(reader.readElementText());
            } else if (reader.name() == "autoclose") {
                plugin->set_autoclose(reader.readElementText());
            }
        }
    }

    // First look for a persistent custom user-specified
    // icon in the plugin's preference folder.
    QString iconpath = pluginsPath() + "/../plugins_prefs/" + name + "/plugin.png";
    QFileInfo fileinfo(iconpath);
    if (fileinfo.exists() && fileinfo.isFile() && fileinfo.isReadable()) {
        plugin->set_iconpath(iconpath);
    } else {
        // If no custom user-supplied icon, look in the
        // plugin folder for a plugin dev-supplied icon.
        iconpath = pluginsPath() + "/" + name + "/plugin.png";
        QFileInfo fileinfo(iconpath);
        if (fileinfo.exists() && fileinfo.isFile() && fileinfo.isReadable()) {
            plugin->set_iconpath(iconpath);
        }
    }

    if (!plugin->isvalid()) {
        delete plugin;
        plugin = NULL;
    }

    return plugin;
}
