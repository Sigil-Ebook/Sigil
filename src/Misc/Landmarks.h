/************************************************************************
**
**  Copyright (C) 2016  Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef LANDMARKS_H
#define LANDMARKS_H

#include <QCoreApplication>
#include <QHash>
class QString;
class QStringList;

#include "Misc/DescriptiveInfo.h"


/**
 * Singleton.
 *
 * Landmarks
 */
 

class Landmarks
{
    Q_DECLARE_TR_FUNCTIONS(Landmarks)

public:

    static Landmarks *instance();

    QString GetName(QString code);
    QString GetDescriptionByCode(QString code);
    QString GetDescriptionByName(QString name);
    QString GetCode(QString name);
    QStringList GetSortedNames();
    bool isLandmarksCode(QString code);
    bool isLandmarksName(QString name);
    const QHash<QString, DescriptiveInfo> &GetCodeMap();
    QString GuideLandMapping(QString code);

private:

    Landmarks();

    void SetLandmarksMap();

    void SetGuideLandMap();

    // code -> DescriptiveInfo -> name and description
    QHash<QString, DescriptiveInfo> m_CodeMap;
    
    // name -> code
    QHash<QString, QString> m_NameMap;

    QStringList m_sortedNames;

    QHash<QString, QString> m_GuideLandMap;
    
    static Landmarks *m_instance;
};

#endif // LANDMARKS_H
