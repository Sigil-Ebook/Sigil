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

#include <QString>
#include <QStringList>

#include "Misc/Plugin.h"
#include "Dialogs/PluginRunner.h"

#if defined(__APPLE__)
static const QString POS = "osx";
#elif defined(_WIN32)
static const QString POS = "win";
#else
static const QString POS = "unx";
#endif

Plugin::Plugin()
{
}

Plugin::Plugin(const QHash<QString, QString> &info)
{
    if (info.contains("name")) {
        set_name(info.value("name"));
    }
    if (info.contains("author")) {
        set_author(info.value("author"));
    }
    if (info.contains("description")) {
        set_description(info.value("description"));
    }
    if (info.contains("type")) {
        set_type(info.value("type"));
    }
    if (info.contains("version")) {
        set_version(info.value("version"));
    }
    if (info.contains("engine")) {
        set_engine(info.value("engine"));
    }
    if (info.contains("oslist")) {
        set_engine(info.value("oslist"));
    }
}

Plugin::~Plugin()
{
}

QHash<QString, QString> Plugin::serialize()
{
    QHash <QString, QString> info;

    info.insert("name", get_name());
    info.insert("author", get_author());
    info.insert("description", get_description());
    info.insert("type", get_type());
    info.insert("version", get_version());
    info.insert("engine", get_engine());
    info.insert("oslist", get_oslist());

    return info;
}

bool Plugin::isvalid()
{
    return (!m_name.isEmpty()   &&
            !m_type.isEmpty()   &&
            (!m_engine.isEmpty() && PluginRunner::SupportedEngines().contains(m_engine)) &&
            (m_oslist.isEmpty() || m_oslist.split(',', QString::SkipEmptyParts).contains(POS)));
}

QString Plugin::get_name()
{
    return m_name;
}

QString Plugin::get_author()
{
    return m_author;
}

QString Plugin::get_description()
{
    return m_description;
}

QString Plugin::get_type()
{
    return m_type;
}

QString Plugin::get_version()
{
    return m_version;
}

QString Plugin::get_engine()
{
    return m_engine;
}

QString Plugin::get_oslist()
{
    return m_oslist;
}

void Plugin::set_name(const QString &val)
{
    m_name = val;
}

void Plugin::set_author(const QString &val)
{
    m_author = val;
}

void Plugin::set_description(const QString &val)
{
    m_description = val;
}

void Plugin::set_type(const QString &val)
{
    m_type = val;
}

void Plugin::set_version(const QString &val)
{
    m_version = val;
}

// multiple engines are possible
void Plugin::set_engine(const QString &val)
{
    if (!m_engine.isEmpty()) {
        m_engine = m_engine + "," + val;
    } else {
        m_engine = val;
    }
}

void Plugin::set_oslist(const QString &val)
{
    m_oslist = val;
}
