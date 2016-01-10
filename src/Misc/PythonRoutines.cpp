/************************************************************************
**
**  Copyright (C) 2016, Kevin B. Hendricks, Stratford, Ontario
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
#include <QString>
#include <QList>
#include <QVariant>

#include "Misc/PythonRoutines.h"

QString PythonRoutines::GenerateNavInPython(const QString &bookroot, const QString &navtitle)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(bookroot));
    args.append(QVariant(navtitle));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("navgenerator"),
                                         QString("generateNav"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}


QString PythonRoutines::GenerateNcxInPython(const QString &navdata, const QString &navname, 
                                            const QString &doctitle, const QString & mainid)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(navdata));
    args.append(QVariant(navname));
    args.append(QVariant(doctitle));
    args.append(QVariant(mainid));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("ncxgenerator"),
                                         QString("generateNCX"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}


QList<QStringList> PythonRoutines::UpdateGuideFromNavInPython(const QString &navdata, const QString &navname)
{
    QList<QStringList> results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(navdata));
    args.append(QVariant(navname));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("ncxgenerator"),
                                         QString("generateGuideEntries"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        QList<QVariant> lst = res.toList();
        foreach(QVariant qv, lst) {
            results.append(qv.toStringList());
        }
        return results;
    }

    // The return value is the following sequence of value stored in a list 
    // (guide_type, href (unquoted), title)  
    QStringList gentry = QStringList() << "" << "" << "";
    results.append(gentry);
    return results;
}

