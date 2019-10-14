/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include "Misc/Utility.h"
#include "SourceUpdates/PerformXMLUpdates.h"
#include "sigil_constants.h"


PerformXMLUpdates::PerformXMLUpdates(const QString &source,
				     const QString &newbookpath,
                                     const QHash<QString, QString> &xml_updates,
                                     const QString &currentpath,
                                     const QString &mtype)
    :
    m_Source(source),
    m_XMLUpdates(xml_updates),
    m_CurrentPath(currentpath),
    m_MediaType(mtype),
    m_newbookpath(newbookpath)
{
}


QString PerformXMLUpdates::operator()()
{
    QString newsource = m_Source;

    // serialize the hash for passing to python
    QStringList dictkeys = m_XMLUpdates.keys();
    QStringList dictvals;
    foreach(QString key, dictkeys) {
      dictvals.append(m_XMLUpdates.value(key));
    }

    int rv = 0;
    QString error_traceback;

    QList<QVariant> args;
    args.append(QVariant(newsource));
    args.append(QVariant(m_newbookpath));
    args.append(QVariant(m_CurrentPath));
    args.append(QVariant(dictkeys));
    args.append(QVariant(dictvals));

    QString routine;

    // MISC_XML_MIMETYPES is defined in BookManipulation/FolderKeeper.cpp and sigil_constants.h
    if (MISC_XML_MIMETYPES.contains(m_MediaType)) {
        if (m_MediaType == "application/smil+xml") {
            routine = "performSMILUpdates";
        } else if (m_MediaType == "application/oebps-page-map+xml")  {
            routine = "performPageMapUpdates";
        } else {
            // We allow editing, but currently have no python parsing/repair/link-updating routines. 
	    // Make no changes.
            // application/adobe-page-template+xml, application/vnd.adobe-page-template+xml, "application/pls+xml"
            return newsource;
        }
    // Utterly unsupported XML mimetypes
    } else {
        Utility::DisplayStdWarningDialog(QString("Unsupported XML media-type: ") + m_MediaType); 
        // make no changes
        return newsource;
    }

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("xmlprocessor"),
                                         routine,
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in xmlprocessor performXMLUpdates: ") + QString::number(rv), 
                                     error_traceback);
        // an error happened - make no changes
        return newsource;
    }

    return res.toString();
}


