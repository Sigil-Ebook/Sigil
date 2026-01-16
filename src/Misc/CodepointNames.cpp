/************************************************************************
**
**  Copyright (C) 2026 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QHash>

#include "EmbedPython/PythonRoutines.h"
#include "Misc/CodepointNames.h"

static const QString XHTML_CHARS =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "1234567890!@#$%^&*()_-+={}[]:;\"'<>,.?/|\\…„”“’»«"
    "ąćęłńóśżźĄĆĘŁŃÓŚŻŹáàâäãåÁÀÂÄÃÅéèêëÉÈÊËíìîïÍÌÎÏ"
    "òôöõøÓÒÔÖÕØúùûüÚÙÛÜýÿÝŸçÇñÑšžŠŽđĐœæŒÆß";

CodepointNames *CodepointNames::m_instance = 0;

CodepointNames *CodepointNames::instance()
{
    if (m_instance == 0) {
        m_instance = new CodepointNames();
    }

    return m_instance;
}

CodepointNames::CodepointNames()
{
    SetNameCache();
}


QString CodepointNames::GetName(int cp)
{
    if (cp < 0) return QString("");
    QString name = m_NameCache.value(cp, QString());
    if (name.isEmpty()) {
        PythonRoutines pr;
	name = pr.GetNameOfCurrentCodepointInPython(cp);
        m_NameCache.insert(cp, name);
    }
    return name;
}


void CodepointNames::SetNameCache()
{
    if (!m_NameCache.isEmpty()) {
        return;
    }

    PythonRoutines pr;
    for (int i=0; i < XHTML_CHARS.length(); i++) {
        int cp = (int) XHTML_CHARS.at(i).unicode();
        m_NameCache.insert(cp, pr.GetNameOfCurrentCodepointInPython(cp));
    }
}
