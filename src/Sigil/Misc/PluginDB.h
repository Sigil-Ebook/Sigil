/************************************************************************
**
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

#pragma once
#ifndef PLUGINDB_H
#define PLUGINDB_H

#include <QHash>

class QString;
class Plugin;

/**
 * Singleton.
 */
class PluginDB : public QObject
{
    Q_OBJECT

public:
    static PluginDB *instance();
    ~PluginDB();

    enum AddResult {
        AR_SUCCESS = 0,
        AR_EXISTS,
        AR_UNZIP,
        AR_INVALID,
        AR_XML
    };

    void load_plugins_from_disk(bool force=false);
    PluginDB::AddResult add_plugin(const QString &path, bool force=false);
    void remove_plugin(const QString &name);
    void remove_all_plugins();

    Plugin *get_plugin(const QString &name);
    QHash<QString, Plugin *> all_plugins();

    QStringList engines();
    QString get_engine_path(const QString &engine);
    void set_engine_path(const QString &engine, const QString &path);

    static QString pluginsPath();
    static QString launcherRoot();
    QString getLastImportPath();
    void setLastImportPath(const QString &path);

signals:
    void plugins_changed();

private:
    PluginDB();

    PluginDB::AddResult add_plugin_int(const QString &path, bool force=false);
    Plugin *load_plugin(const QString &name);
    bool verify_plugin_zip(const QString &path, const QString &name);

    QHash<QString, Plugin *> m_plugins;
    QHash<QString, QString> m_engine_paths;
    QString m_lastImportPath;

    static PluginDB *m_instance;
};

#endif // PLUGINDB_H
