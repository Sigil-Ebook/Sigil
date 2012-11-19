/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QFileInfo>

#include "Importers/ImporterFactory.h"
#include "Importers/ImportEPUB.h"
#include "Importers/ImportHTML.h"
#include "Importers/ImportTXT.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

// Constructor
ImporterFactory::ImporterFactory()
    : m_importer_html(NULL),
      m_importer_epub(NULL),
      m_importer_txt(NULL)
{
}


// Destructor
ImporterFactory::~ImporterFactory()
{
    if (m_importer_html) {
        delete m_importer_html;
    }
    if (m_importer_epub) {
        delete m_importer_epub;
    }
    if (m_importer_txt) {
        delete m_importer_txt;
    }
}


// Returns a reference to the importer
// appropriate for the given filename
Importer *ImporterFactory::GetImporter(const QString &filename)
{
    QString extension = QFileInfo(filename).suffix().toLower();

    if ((extension == "xhtml") ||
        (extension == "html")  ||
        (extension == "htm")
       ) {
        if (!m_importer_html) {
            m_importer_html = new ImportHTML(filename);
        }
        return m_importer_html;
    }

    if ((extension == "txt")) {
        if (!m_importer_txt) {
            m_importer_txt = new ImportTXT(filename);
        }
        return m_importer_txt;
    }

    if ((extension == "epub")) {
        if (!m_importer_epub) {
            m_importer_epub = new ImportEPUB(filename);
        }
        return m_importer_epub; 
    }

    return NULL;
}



