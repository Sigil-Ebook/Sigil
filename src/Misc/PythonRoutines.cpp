/************************************************************************
**
**  Copyright (C) 2016-2020 Kevin B. Hendricks, Stratford Ontario Canada
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


QString PythonRoutines::GenerateNcxInPython(const QString &navdata, const QString &navbkpath, 
                                            const QString &ncxdir, const QString &doctitle, 
					    const QString &mainid)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(navdata));
    args.append(QVariant(navbkpath));
    args.append(QVariant(ncxdir));
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


QString PythonRoutines::PerformRepoCommitInPython(const QString &localRepo, 
						  const QString &bookid, 
						  const QStringList &bookinfo,
						  const QString &bookroot, 
						  const QStringList &bookfiles) 
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(localRepo));
    args.append(QVariant(bookid));
    args.append(QVariant(bookinfo));
    args.append(QVariant(bookroot));
    args.append(QVariant(bookfiles));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("performCommit"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}


bool PythonRoutines::PerformRepoEraseInPython(const QString& localRepo, const QString& bookid)
{
    bool results = false;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(localRepo));
    args.append(QVariant(bookid));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("eraseRepo"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = (res.toInt() > 0);
    }
    return results;
}

QStringList PythonRoutines::GetRepoTagsInPython(const QString& localRepo, const QString& bookid)
{
    QStringList results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(localRepo));
    args.append(QVariant(bookid));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("get_tag_list"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toStringList();
    }
    return results;
}


QString PythonRoutines::GenerateEpubFromTagInPython(const QString& localRepo,
				                    const QString& bookid,
				                    const QString& tagname,
				                    const QString& filename,
				                    const QString& destpath)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(localRepo));
    args.append(QVariant(bookid));
    args.append(QVariant(tagname));
    args.append(QVariant(filename));
    args.append(QVariant(destpath));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("generate_epub_from_tag"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}


QString PythonRoutines::GenerateDiffFromCheckPoints(const QString& localRepo,
                                    const QString& bookid,
                                    const QString& leftchkpoint,
                                    const QString& rightchkpoint)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(localRepo));
    args.append(QVariant(bookid));
    args.append(QVariant(leftchkpoint));
    args.append(QVariant(rightchkpoint));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("generate_diff_from_checkpoints"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}

QString PythonRoutines::GenerateRepoLogSummaryInPython(const QString& localRepo,
						       const QString& bookid)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(localRepo));
    args.append(QVariant(bookid));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("generate_log_summary"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}


QList<DiffRecord::DiffRec> PythonRoutines::GenerateParsedNDiffInPython(const QString& path1,
						                  const QString& path2)
{
    QList<DiffRecord::DiffRec> results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(path1));
    args.append(QVariant(path2));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("generate_parsed_ndiff"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
	QVariantList vlist = res.toList();
	foreach(QVariant qv, vlist) {
	    QStringList fields = qv.toStringList();
	    DiffRecord::DiffRec dr;
	    dr.code = fields.at(0);
	    dr.line = fields.at(1);
	    dr.newline = fields.at(2);
	    dr.leftchanges = fields.at(3);
	    dr.rightchanges = fields.at(4);
	    results << dr;
	}
    }
    return results;
}



QString PythonRoutines::GenerateUnifiedDiffInPython(const QString& path1, const QString& path2)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(path1));
    args.append(QVariant(path2));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("generate_unified_diff"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}


// returns 3 string lists: deleted, added, and modified (in that order)
QList<QStringList> PythonRoutines::GetCurrentStatusVsDestDirInPython(const QString&bookroot,
						                     const QStringList& bookfiles,
						                     const QString& destdir)
{
    QList<QStringList> results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(bookroot));
    args.append(QVariant(bookfiles));
    args.append(QVariant(destdir));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("get_current_status_vs_destdir"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
	QVariantList vlist = res.toList();
	foreach(QVariant qv, vlist) {
	    results << qv.toStringList();
	}
    }
    return results;
}


QString PythonRoutines::CopyTagToDestDirInPython(const QString& localRepo,
				                 const QString& bookid,
				                 const QString& tagname,
				                 const QString& destdir)
{
    QString results;
    int rv = -1;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(localRepo));
    args.append(QVariant(bookid));
    args.append(QVariant(tagname));
    args.append(QVariant(destdir));

    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("repomanager"),
                                         QString("copy_tag_to_destdir"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv == 0) {
        results = res.toString();
    }
    return results;
}
