/************************************************************************
**
**  Copyright (C) 2016-2019  Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2009-2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include "Misc/GumboInterface.h"
#include "BookManipulation/CleanSource.h"
#include "SourceUpdates/PerformCSSUpdates.h"
#include "SourceUpdates/PerformHTMLUpdates.h"

PerformHTMLUpdates::PerformHTMLUpdates(const QString &source,
                                       const QHash<QString, QString> &html_updates,
                                       const QHash<QString, QString> &css_updates,
                                       const QString& currentpath,
                                       const QString& version)
  :
  m_HTMLUpdates(html_updates),
  m_CSSUpdates(css_updates),
  m_CurrentPath(currentpath),
  m_source(source),
  m_version(version)
{
}


QString PerformHTMLUpdates::operator()()
{
    QString newsource = CleanSource::PreprocessSpecialCases(m_source);
    { 
        GumboInterface gi = GumboInterface(newsource, m_version, m_HTMLUpdates);
        gi.parse();
        newsource = gi.perform_source_updates(m_CurrentPath);
    }
    if (!m_CSSUpdates.isEmpty()) {
        GumboInterface gi = GumboInterface(newsource, m_version, m_CSSUpdates);
        gi.parse();
        newsource = gi.perform_style_updates(m_CurrentPath);
    }
    return CleanSource::CharToEntity(newsource, m_version);
}

