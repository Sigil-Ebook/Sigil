/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include "Importers/Importer.h"

static const QString SEP = QString(QChar(31));

Importer::Importer(const QString &fullfilepath)
    :
    m_FullFilePath(fullfilepath),
    m_Book(new Book()),
    m_LoadWarnings(QStringList()),
    m_MediaTypeWarnings(QStringList())
{
}

XhtmlDoc::WellFormedError Importer::CheckValidToLoad()
{
    // Default behaviour is to assume resource is valid.
    XhtmlDoc::WellFormedError error;
    error.line = -1;
    return error;
}

QStringList Importer::GetLoadWarnings()
{
    if (m_MediaTypeWarnings.size() > 0) {
        QString warning = QObject::tr("The OPF contains missing or unrecognized media types.  Temporary media types have been generated. You should edit your OPF to fix these.");
        warning = warning + SEP + m_MediaTypeWarnings.join("\n");
        AddLoadWarning(warning);
        m_MediaTypeWarnings.clear();
    }
    return m_LoadWarnings;
}

void Importer::AddLoadWarning(const QString &warning)
{
    m_LoadWarnings.append(warning % "\n");
}

void Importer::AddMediaTypeWarning(const QString &warning)
{
    m_MediaTypeWarnings.append(warning % "\n");
}
