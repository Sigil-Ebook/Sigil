/************************************************************************
**
**  Copyright (C) 2016-2021 Kevin B. Hendricks, Stratford Ontario Canada
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

#pragma once
#ifndef PYTHONROUTINES_H
#define PYTHONROUTINES_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMetaType>

#include "EmbedPython/DiffRec.h"

struct MetadataPieces {
    QString data;
    QString otherxml;
    QStringList idlist;
    QString metatag;
};

class PythonRoutines
{

public:

    PythonRoutines() {};

    QString GenerateNcxInPython(const QString &navdata, const QString &navbkpath,
                                const QString &ncx_dir, const QString &doctitle, const QString &mainid);

    MetadataPieces GetMetadataInPython(const QString& opfdata, const QString& version);

    QString SetNewMetadataInPython(const MetadataPieces& mdp, const QString& opfdata, const QString& version);

    QString PerformRepoCommitInPython(  const QString&     localRepo,
                                        const QString&     bookid,
                                        const QStringList& bookinfo,
                                        const QString&     bookroot,
                                        const QStringList& bookfiles );

    bool PerformRepoEraseInPython(      const QString& localRepo, 
                                        const QString& bookid ); 

    QStringList GetRepoTagsInPython(    const QString& localRepo, 
                                        const QString& bookid );

    QString GenerateEpubFromTagInPython(const QString& localRepo, 
                                        const QString& bookid,
                                        const QString& tagname,
                                        const QString& filename, 
                                        const QString& destpath );

    QString GenerateDiffFromCheckPoints(const QString& localRepo,
                        const QString& bookid,
                        const QString& leftchkpoint,
                        const QString& rightchkpoint);

    QString GenerateRepoLogSummaryInPython(const QString& localRepo,
                                           const QString& bookid);

    QList<DiffRecord::DiffRec> GenerateParsedNDiffInPython(const QString& path1, const QString& path2);

    QString GenerateUnifiedDiffInPython(const QString& path1, const QString& path2);

    QString CopyTagToDestDirInPython(const QString& localRepo,
                                     const QString& bookid,
                                     const QString& tagname,
                                     const QString& destdir);

    // returns 3 stringlists in the following order: deleted, added, modified
    QList<QStringList> GetCurrentStatusVsDestDirInPython(const QString& bookroot,
                                                         const QStringList& bookfiles,
                                                         const QString& destdir);
    

private:

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////
    
};

Q_DECLARE_METATYPE(QList<int>);
#endif // PYTHONROUTINES_H
