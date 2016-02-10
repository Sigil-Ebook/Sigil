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

QString PythonRoutines::GenerateNavInPython(const QString &opfdata, const QString &ncxdata, const QString &navtitle)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(opfdata));
    args.append(QVariant(ncxdata));
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



MetadataPieces PythonRoutines::GetMetadataInPython(const QString& opfdata, const QString& version) 
{
    int rv = 0;
    QString traceback;
    MetadataPieces mdp;

    QString module = "metaproc2";
    if (version.startsWith('3')) module = "metaproc3";
    QList<QVariant> args;
    args.append(QVariant(opfdata));
    EmbeddedPython* epp = EmbeddedPython::instance();
    QVariant res = epp->runInPython(module, QString("process_metadata"), args, &rv, traceback, true);
    if (rv) {
        fprintf(stderr, "process_meta error %d traceback %s\n",rv, traceback.toStdString().c_str());
    }
    PyObjectPtr mpo = PyObjectPtr(res);
    args.clear();
    res = epp->callPyObjMethod(mpo, QString("get_recognized_metadata"), args, &rv, traceback);
    if (rv) {
        fprintf(stderr, "get_recognized_metadata error %d traceback %s\n",rv, traceback.toStdString().c_str());
    }
    mdp.data = res.toString();
    args.clear();
    res = epp->callPyObjMethod(mpo, QString("get_other_meta_xml"), args, &rv, traceback);
    if (rv) {
        fprintf(stderr, "get_other_meta_xml error %d traceback %s\n",rv, traceback.toStdString().c_str());
    }
    mdp.otherxml = res.toString();
    args.clear();
    res = epp->callPyObjMethod(mpo, QString("get_id_list"), args, &rv, traceback);
    if (rv) {
        fprintf(stderr, "get_id_list error %d traceback %s\n",rv, traceback.toStdString().c_str());
    }
    mdp.idlist = res.toStringList();
    args.clear();
    res = epp->callPyObjMethod(mpo, QString("get_metadata_tag"), args, &rv, traceback);
    if (rv) {
        fprintf(stderr, "get_metadata_tag error %d traceback %s\n",rv, traceback.toStdString().c_str());
    }
    mdp.metatag = res.toString();
    return mdp;
}


QString PythonRoutines::SetNewMetadataInPython(const MetadataPieces& mdp, const QString& opfdata, const QString& version) 
{
    int rv = 0;
    QString traceback;
    QString newopfdata= opfdata;
    QString module = "metaproc2";
    if (version.startsWith('3')) module = "metaproc3";
    QList<QVariant> args;
    args.append(QVariant(mdp.data));
    args.append(QVariant(mdp.otherxml));
    args.append(QVariant(mdp.idlist));
    args.append(QVariant(mdp.metatag));
    args.append(QVariant(opfdata));
    EmbeddedPython* epp = EmbeddedPython::instance();
    QVariant res = epp->runInPython(module, QString("set_new_metadata"), args, &rv, traceback, true);
    if (rv) {
        fprintf(stderr, "set_new_metadata error %d traceback %s\n",rv, traceback.toStdString().c_str());
        return newopfdata;
    }
    newopfdata = res.toString();
    return newopfdata;
}

