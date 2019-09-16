/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, Ontario Canada
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

#include "Misc/EmbeddedPython.h"
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include "Misc/Utility.h"
#include "SourceUpdates/PerformOPFUpdates.h"


PerformOPFUpdates::PerformOPFUpdates(const QString &source,
                                     const QHash<QString, QString> &xml_updates,
                                     const QString& currentpath)
  :
  m_XMLUpdates(xml_updates),
  m_CurrentPath(currentpath),
  m_source(source)
{
}


QString PerformOPFUpdates::operator()()
{
  QString newsource = m_source;

  // serialize the hash for passing to python
  QStringList dictkeys = m_XMLUpdates.keys();
  QStringList dictvals;
  foreach(QString key, dictkeys) {
    dictvals.append(m_XMLUpdates.value(key));
  }

  int rv = 0;
  QString error_traceback;

  QList<QVariant> args;
  args.append(QVariant(m_source));
  args.append(QVariant(m_CurrentPath));
  args.append(QVariant(dictkeys));
  args.append(QVariant(dictvals));

  EmbeddedPython * epython  = EmbeddedPython::instance();

  QVariant res = epython->runInPython( QString("xmlprocessor"),
                                       QString("performOPFSourceUpdates"),
                                       args,
                                       &rv,
                                       error_traceback);    
  if (rv != 0) {
    Utility::DisplayStdWarningDialog(QString("error in xmlprocessor performOPFSourceUpdates: ") + QString::number(rv), 
                                     error_traceback);
    // an error happened - make no changes
    return newsource;
  }
  return res.toString();
}

