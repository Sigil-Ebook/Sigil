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
#ifndef GUIDEITEMS_H
#define GUIDEITEMS_H

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QHash>

#include "Misc/DescriptiveInfo.h"


/**
 * Singleton.
 *
 * GuideItems
 */
 

class GuideItems
{
    Q_DECLARE_TR_FUNCTIONS(GuideItems)

public:
    static GuideItems *instance();

    QString GetName(QString code);
    QString GetDescriptionByCode(QString code);
    QString GetDescriptionByName(QString name);
    QString GetCode(QString name);
    QStringList GetSortedNames();
    bool isGuideItemsCode(QString code);
    bool isGuideItemsName(QString name);
    const QHash<QString, DescriptiveInfo> &GetCodeMap();

private:
    GuideItems();

    void SetGuideItemsMap();

    // code -> DescriptiveInfo -> name and description
    QHash<QString, DescriptiveInfo> m_CodeMap;
    
    // name -> code
    QHash<QString, QString> m_NameMap;

    QStringList m_sortedNames;

    static GuideItems *m_instance;
};

#endif // GUIDEITEMS_H
